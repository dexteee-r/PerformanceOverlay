#include "claude_usage_provider.h"

#include <QDir>
#include <QFile>
#include <QSet>
#include <QByteArray>
#include <QDateTime>

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

namespace {

constexpr int kBaselineDays = 28;          // fenêtre de moyenne glissante
constexpr qint64 kRescanMs = 60000;        // re-scan au plus 1×/min
constexpr qint64 kMaxFileBytes = 128LL * 1024 * 1024;

// Jours sériels depuis une époque fixe (algo Hinnant) → comparaison de dates.
long daysFromCivil(int y, int m, int d)
{
    y -= (m <= 2);
    const long era = (y >= 0 ? y : y - 399) / 400;
    const long yoe = y - era * 400;
    const long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

quint64 fnvHash(const char *s, size_t len)
{
    quint64 h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= static_cast<unsigned char>(s[i]);
        h *= 1099511628211ULL;
    }
    return h;
}

// Valeur entière d'une clé JSON ("clé":NOMBRE). key inclut les guillemets.
long long extractLong(const char *line, const char *key)
{
    const char *p = std::strstr(line, key);
    if (!p)
        return 0;
    const char *colon = std::strchr(p + std::strlen(key), ':');
    if (!colon)
        return 0;
    return std::strtoll(colon + 1, nullptr, 10);   // strtoll saute les espaces
}

// Valeur chaîne ("clé":"valeur"). keyColonQuote = début littéral, ex "\"id\":\"".
bool extractStr(const char *line, const char *keyColonQuote, char *out, size_t outSize)
{
    const char *p = std::strstr(line, keyColonQuote);
    if (!p)
        return false;
    p += std::strlen(keyColonQuote);
    const char *end = std::strchr(p, '"');
    if (!end)
        return false;
    size_t len = static_cast<size_t>(end - p);
    if (len >= outSize)
        len = outSize - 1;
    std::memcpy(out, p, len);
    out[len] = '\0';
    return true;
}

// Traite une ligne JSONL (message assistant avec usage) → agrège dans buckets.
void processLine(char *line, long long *buckets, long todayIndex, QSet<quint64> &seen)
{
    if (!std::strstr(line, "\"type\":\"assistant\""))
        return;
    if (!std::strstr(line, "\"input_tokens\""))
        return;

    char ts[32];
    if (!extractStr(line, "\"timestamp\":\"", ts, sizeof(ts)))
        return;
    int y = 0, mo = 0, d = 0;
    if (std::sscanf(ts, "%d-%d-%d", &y, &mo, &d) != 3)
        return;

    const long age = todayIndex - daysFromCivil(y, mo, d);
    if (age < 0 || age >= kBaselineDays)
        return;

    // Dédup : requestId + "|msg_" + suffixe message.id.
    char reqId[64] = "";
    char msgId[64] = "";
    extractStr(line, "\"requestId\":\"", reqId, sizeof(reqId));
    extractStr(line, "\"id\":\"msg_", msgId, sizeof(msgId));
    char keyBuf[160];
    const int kl = std::snprintf(keyBuf, sizeof(keyBuf), "%s|msg_%s", reqId, msgId);
    if (kl > 0) {
        const quint64 key = fnvHash(keyBuf, static_cast<size_t>(kl));
        if (seen.contains(key))
            return;
        seen.insert(key);
    }

    long long tok = 0;
    tok += extractLong(line, "\"input_tokens\"");
    tok += extractLong(line, "\"output_tokens\"");
    tok += extractLong(line, "\"cache_creation_input_tokens\"");
    tok += extractLong(line, "\"cache_read_input_tokens\"");

    buckets[age] += tok;
}

void processFile(const QString &path, long long *buckets, long todayIndex, QSet<quint64> &seen)
{
    QFile f(path);
    if (f.size() <= 0 || f.size() > kMaxFileBytes)
        return;
    if (!f.open(QIODevice::ReadOnly))
        return;

    QByteArray data = f.readAll();
    f.close();

    char *buf = data.data();              // mutable : on coupe sur place
    const int n = data.size();
    char *lineStart = buf;
    for (int i = 0; i < n; ++i) {
        if (buf[i] == '\n') {
            buf[i] = '\0';
            processLine(lineStart, buckets, todayIndex, seen);
            lineStart = buf + i + 1;
        }
    }
    if (lineStart < buf + n)
        processLine(lineStart, buckets, todayIndex, seen);
}

} // namespace

void ClaudeUsageProvider::poll()
{
    if (m_firstScan || !m_since.isValid() || m_since.elapsed() >= kRescanMs) {
        scan();
        m_since.restart();
        m_firstScan = false;
    }
}

void ClaudeUsageProvider::scan()
{
    const QString root = QDir::homePath() + QStringLiteral("/.claude/projects");
    QDir projects(root);

    const bool prevHasData = m_hasData;
    const double prevDay = m_dayPercent;
    const double prevWeek = m_weekPercent;

    auto publish = [&]() {
        if (m_hasData != prevHasData || m_dayPercent != prevDay
            || m_weekPercent != prevWeek)
            emit changed();
    };

    if (!projects.exists()) {
        m_hasData = false;
        publish();
        return;
    }

    // Index du jour courant en UTC (timestamps "...Z").
    const QDate today = QDateTime::currentDateTimeUtc().date();
    const long todayIndex = daysFromCivil(today.year(), today.month(), today.day());

    long long buckets[kBaselineDays] = {0};
    QSet<quint64> seen;

    const QStringList sessions =
        projects.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &session : sessions) {
        QDir sub(projects.filePath(session));
        const QStringList files =
            sub.entryList({QStringLiteral("*.jsonl")}, QDir::Files);
        for (const QString &file : files)
            processFile(sub.filePath(file), buckets, todayIndex, seen);
    }

    long long tokToday = buckets[0];
    long long tok7d = 0;
    long long tok28d = 0;
    for (int i = 0; i < kBaselineDays; ++i) {
        tok28d += buckets[i];
        if (i < 7)
            tok7d += buckets[i];
    }

    if (tok28d <= 0) {
        m_hasData = false;
        publish();
        return;
    }

    const double avgDay = static_cast<double>(tok28d) / kBaselineDays;
    const double avgWeek = static_cast<double>(tok28d) / (kBaselineDays / 7.0);

    m_dayPercent = avgDay > 0 ? static_cast<double>(tokToday) / avgDay * 100.0 : 0.0;
    m_weekPercent = avgWeek > 0 ? static_cast<double>(tok7d) / avgWeek * 100.0 : 0.0;
    m_hasData = true;
    publish();
}
