#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <array>
#include "metric_provider.h"

class QNetworkAccessManager;

// Provider Prayer : prochaine prière et temps restant.
// Horaires récupérés via l'API Aladhan (async, QNetworkAccessManager) une fois
// par jour ; repli sur des horaires par défaut si l'API échoue.
// La config (ville/pays/méthode) sera branchée sur config.ini plus tard ;
// défauts actuels = Brussels / Belgium / méthode 2 (ISNA).
class PrayerProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
    Q_PROPERTY(bool usingApi READ usingApi NOTIFY usingApiChanged)
    Q_PROPERTY(QString mosqueName READ mosqueName NOTIFY mosqueNameChanged)   // vide si Aladhan
    Q_PROPERTY(QString nextName READ nextName NOTIFY nextChanged)
    Q_PROPERTY(int nextHour READ nextHour NOTIFY nextChanged)
    Q_PROPERTY(int nextMinute READ nextMinute NOTIFY nextChanged)
    Q_PROPERTY(int remainingMinutes READ remainingMinutes NOTIFY nextChanged)

public:
    explicit PrayerProvider(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    bool usingApi() const { return m_usingApi && !m_apiFailed; }
    QString mosqueName() const { return m_mosqueName; }
    QString nextName() const { return m_nextName; }
    int nextHour() const { return m_nextHour; }
    int nextMinute() const { return m_nextMinute; }
    int remainingMinutes() const { return m_remaining; }

    void poll() override;

    // Applique la config (depuis config.ini) : ville/pays/méthode + activation.
    // mosqueId = slug mawaqit.net (prioritaire si non vide ; sinon Aladhan par ville).
    Q_INVOKABLE void configure(const QString &city, const QString &country,
                               int method, bool enabled, bool useApi,
                               const QString &mosqueId);

    // Change la mosquée mawaqit à chaud (depuis Réglages) et relance un fetch.
    Q_INVOKABLE void setMosque(const QString &slug);

signals:
    void enabledChanged();
    void usingApiChanged();
    void mosqueNameChanged();
    void nextChanged();

private:
    void fetchFromApi();         // route : mawaqit si slug, sinon Aladhan
    void fetchFromAladhan();
    void fetchFromMawaqit();
    void computeNext();

    struct Prayer { QString name; int hour; int minute; };
    std::array<Prayer, 5> m_prayers{ {
        {QStringLiteral("Fajr"), 6, 0},
        {QStringLiteral("Dhuhr"), 13, 0},
        {QStringLiteral("Asr"), 16, 0},
        {QStringLiteral("Maghrib"), 19, 0},
        {QStringLiteral("Isha"), 21, 0},
    } };

    // Config (défauts ; config.ini à brancher plus tard).
    QString m_city = QStringLiteral("Brussels");
    QString m_country = QStringLiteral("Belgium");
    int m_method = 2;
    bool m_enabled = true;
    bool m_usingApi = true;
    QString m_mosqueId;        // slug mawaqit.net (vide = Aladhan par ville)
    QString m_mosqueName;      // nom résolu depuis mawaqit (vide si Aladhan)

    QNetworkAccessManager *m_nam = nullptr;
    int m_lastFetchDay = -1;
    bool m_fetching = false;
    bool m_apiFailed = false;
    int m_fetchGen = 0;        // jeton : une réponse dont le gen ≠ courant est ignorée

    QString m_nextName;
    int m_nextHour = 0;
    int m_nextMinute = 0;
    int m_remaining = 0;
};
