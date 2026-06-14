#include "prayer_provider.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrl>
#include <QDate>
#include <QTime>

PrayerProvider::PrayerProvider(QObject *parent)
    : MetricProvider(parent), m_nam(new QNetworkAccessManager(this))
{
    // Pas de fetch ici : configure() (appelé par Main.qml au démarrage) lance le 1er
    // fetch avec le bon mode (mawaqit si un slug est configuré, sinon Aladhan). Fetcher
    // ici déclencherait un Aladhan parasite qui bloquerait le fetch mawaqit (course).
    computeNext();
}

void PrayerProvider::configure(const QString &city, const QString &country,
                               int method, bool enabled, bool useApi,
                               const QString &mosqueId)
{
    m_city = city;
    m_country = country;
    m_method = method;
    m_enabled = enabled;
    m_usingApi = useApi;
    m_mosqueId = mosqueId;
    m_apiFailed = false;
    m_lastFetchDay = -1;   // force un nouveau fetch

    emit enabledChanged();
    emit usingApiChanged();

    if (m_enabled && m_usingApi)   // pas de garde m_fetching : le gen supersède l'en-vol
        fetchFromApi();
    computeNext();
}

void PrayerProvider::setMosque(const QString &slug)
{
    if (slug == m_mosqueId)
        return;
    m_mosqueId = slug;
    m_apiFailed = false;
    m_lastFetchDay = -1;
    if (slug.isEmpty() && !m_mosqueName.isEmpty()) {   // retour Aladhan → oublie le nom
        m_mosqueName.clear();
        emit mosqueNameChanged();
    }
    if (m_enabled && m_usingApi)   // pas de garde m_fetching : le gen supersède l'en-vol
        fetchFromApi();
    computeNext();
}

void PrayerProvider::poll()
{
    if (!m_enabled)
        return;

    if (m_usingApi && !m_fetching && QDate::currentDate().day() != m_lastFetchDay)
        fetchFromApi();

    computeNext();
}

void PrayerProvider::fetchFromApi()
{
    if (!m_mosqueId.isEmpty())
        fetchFromMawaqit();
    else
        fetchFromAladhan();
}

void PrayerProvider::fetchFromAladhan()
{
    const int gen = ++m_fetchGen;
    m_fetching = true;

    const QUrl url(QStringLiteral("https://api.aladhan.com/v1/timingsByCity"
                                  "?city=%1&country=%2&method=%3")
                       .arg(m_city, m_country)
                       .arg(m_method));

    QNetworkReply *reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, gen]() {
        reply->deleteLater();
        if (gen != m_fetchGen) return;   // requête supersédée par une config plus récente
        m_fetching = false;

        const bool wasApi = usingApi();

        if (reply->error() != QNetworkReply::NoError) {
            m_apiFailed = true;
            if (wasApi != usingApi()) emit usingApiChanged();
            return;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject timings =
            doc.object().value(QStringLiteral("data")).toObject()
               .value(QStringLiteral("timings")).toObject();
        if (timings.isEmpty()) {
            m_apiFailed = true;
            if (wasApi != usingApi()) emit usingApiChanged();
            return;
        }

        for (Prayer &p : m_prayers) {
            // Aladhan renvoie "05:23" ou "05:23 (CET)" → on garde "HH:MM".
            const QString raw = timings.value(p.name).toString().section(' ', 0, 0);
            const QTime t = QTime::fromString(raw, QStringLiteral("HH:mm"));
            if (t.isValid()) {
                p.hour = t.hour();
                p.minute = t.minute();
            }
        }

        m_apiFailed = false;
        m_lastFetchDay = QDate::currentDate().day();
        if (!m_mosqueName.isEmpty()) {   // on est en Aladhan → pas de nom de mosquée
            m_mosqueName.clear();
            emit mosqueNameChanged();
        }
        if (wasApi != usingApi()) emit usingApiChanged();
        computeNext();
    });
}

void PrayerProvider::fetchFromMawaqit()
{
    const int gen = ++m_fetchGen;
    m_fetching = true;

    // mawaqit.net rend la page de la mosquée avec un objet JS `var confData = {...};`
    // contenant les horaires du jour (times[0..4] = fajr,dohr,asr,maghreb,icha) et le
    // nom. On extrait ce JSON (même approche que l'API mawaqit, sans dépendance).
    QNetworkRequest req(QUrl(QStringLiteral("https://mawaqit.net/fr/%1").arg(m_mosqueId)));
    req.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("Mozilla/5.0"));
    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, gen]() {
        reply->deleteLater();
        if (gen != m_fetchGen) return;   // requête supersédée
        m_fetching = false;

        const bool wasApi = usingApi();
        if (reply->error() != QNetworkReply::NoError) {
            m_apiFailed = true;
            if (wasApi != usingApi()) emit usingApiChanged();
            return;
        }

        const QString html = QString::fromUtf8(reply->readAll());
        static const QRegularExpression re(
            QStringLiteral("confData\\s*=\\s*(\\{.*?\\});"),
            QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch mm = re.match(html);
        if (!mm.hasMatch()) {
            m_apiFailed = true;
            if (wasApi != usingApi()) emit usingApiChanged();
            return;
        }

        const QJsonObject conf = QJsonDocument::fromJson(mm.captured(1).toUtf8()).object();
        const QJsonArray times = conf.value(QStringLiteral("times")).toArray();
        if (times.size() < 5) {
            m_apiFailed = true;
            if (wasApi != usingApi()) emit usingApiChanged();
            return;
        }

        for (int i = 0; i < 5; ++i) {
            const QTime t = QTime::fromString(times.at(i).toString(), QStringLiteral("HH:mm"));
            if (t.isValid()) {
                m_prayers[i].hour = t.hour();
                m_prayers[i].minute = t.minute();
            }
        }

        const QString nm = conf.value(QStringLiteral("name")).toString();
        if (nm != m_mosqueName) { m_mosqueName = nm; emit mosqueNameChanged(); }

        m_apiFailed = false;
        m_lastFetchDay = QDate::currentDate().day();
        if (wasApi != usingApi()) emit usingApiChanged();
        computeNext();
    });
}

void PrayerProvider::computeNext()
{
    const QTime now = QTime::currentTime();
    const int cur = now.hour() * 60 + now.minute();

    // Minutes du jour « déroulées » : si une prière tombe plus tôt que la précédente,
    // c'est qu'elle passe minuit (ex. Isha à 00:03 en été à nos latitudes) → +24h, pour
    // garder l'ordre chronologique et la détecter comme prochaine en soirée.
    int eff[5];
    int prev = -1;
    for (int i = 0; i < 5; ++i) {
        int pm = m_prayers[i].hour * 60 + m_prayers[i].minute;
        if (pm < prev) pm += 24 * 60;
        eff[i] = pm;
        prev = pm;
    }

    int idx = 0;
    int remaining = 0;
    bool found = false;
    for (int i = 0; i < 5; ++i) {
        if (eff[i] > cur) {
            idx = i;
            remaining = eff[i] - cur;
            found = true;
            break;
        }
    }
    if (!found) {
        // Toutes passées → Fajr du lendemain.
        idx = 0;
        remaining = (24 * 60 - cur) + eff[0];
    }

    const QString name = m_prayers[idx].name;
    const int h = m_prayers[idx].hour;
    const int m = m_prayers[idx].minute;

    if (name != m_nextName || h != m_nextHour || m != m_nextMinute
        || remaining != m_remaining) {
        m_nextName = name;
        m_nextHour = h;
        m_nextMinute = m;
        m_remaining = remaining;
        emit nextChanged();
    }
}
