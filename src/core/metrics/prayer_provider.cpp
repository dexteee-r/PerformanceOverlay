#include "prayer_provider.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDate>
#include <QTime>

PrayerProvider::PrayerProvider(QObject *parent)
    : MetricProvider(parent), m_nam(new QNetworkAccessManager(this))
{
    if (m_usingApi)
        fetchFromApi();
    computeNext();
}

void PrayerProvider::configure(const QString &city, const QString &country,
                               int method, bool enabled, bool useApi)
{
    m_city = city;
    m_country = country;
    m_method = method;
    m_enabled = enabled;
    m_usingApi = useApi;
    m_apiFailed = false;
    m_lastFetchDay = -1;   // force un nouveau fetch

    emit enabledChanged();
    emit usingApiChanged();

    if (m_enabled && m_usingApi && !m_fetching)
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
    m_fetching = true;

    const QUrl url(QStringLiteral("https://api.aladhan.com/v1/timingsByCity"
                                  "?city=%1&country=%2&method=%3")
                       .arg(m_city, m_country)
                       .arg(m_method));

    QNetworkReply *reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
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
        if (wasApi != usingApi()) emit usingApiChanged();
        computeNext();
    });
}

void PrayerProvider::computeNext()
{
    const QTime now = QTime::currentTime();
    const int cur = now.hour() * 60 + now.minute();

    int idx = 0;
    int remaining = 0;
    bool found = false;
    for (int i = 0; i < static_cast<int>(m_prayers.size()); ++i) {
        const int pm = m_prayers[i].hour * 60 + m_prayers[i].minute;
        if (pm > cur) {
            idx = i;
            remaining = pm - cur;
            found = true;
            break;
        }
    }
    if (!found) {
        // Toutes passées → Fajr du lendemain.
        idx = 0;
        const int fajr = m_prayers[0].hour * 60 + m_prayers[0].minute;
        remaining = (24 * 60 - cur) + fajr;
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
