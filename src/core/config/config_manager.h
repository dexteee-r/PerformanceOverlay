#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Lecture de config.ini (format INI, via QSettings). Singleton QML `Config`.
// Le fichier est cherché à côté de l'exécutable ; un défaut commenté est écrit
// s'il est absent. Le QML applique ces valeurs (cf. Main.qml applyConfig()).
class ConfigManager : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Config)
    QML_SINGLETON

    Q_PROPERTY(int refreshIntervalMs READ refreshIntervalMs NOTIFY loaded)
    Q_PROPERTY(bool hasWindowPos READ hasWindowPos NOTIFY loaded)
    Q_PROPERTY(int windowX READ windowX NOTIFY loaded)
    Q_PROPERTY(int windowY READ windowY NOTIFY loaded)
    Q_PROPERTY(bool prayerEnabled READ prayerEnabled NOTIFY loaded)
    Q_PROPERTY(bool prayerUseApi READ prayerUseApi NOTIFY loaded)
    Q_PROPERTY(QString prayerCity READ prayerCity NOTIFY loaded)
    Q_PROPERTY(QString prayerCountry READ prayerCountry NOTIFY loaded)
    Q_PROPERTY(int prayerMethod READ prayerMethod NOTIFY loaded)
    Q_PROPERTY(bool effect3dEnabled READ effect3dEnabled WRITE setEffect3dEnabled NOTIFY effect3dEnabledChanged)
    Q_PROPERTY(int sphereDensity READ sphereDensity WRITE setSphereDensity NOTIFY sphereDensityChanged)
    Q_PROPERTY(bool animateInBackground READ animateInBackground WRITE setAnimateInBackground NOTIFY animateInBackgroundChanged)
    Q_PROPERTY(double micSensitivity READ micSensitivity WRITE setMicSensitivity NOTIFY micSensitivityChanged)
    Q_PROPERTY(QString micDeviceId READ micDeviceId WRITE setMicDeviceId NOTIFY micDeviceIdChanged)
    Q_PROPERTY(QString themePreset READ themePreset WRITE setThemePreset NOTIFY themePresetChanged)
    Q_PROPERTY(QString prayerMosqueId READ prayerMosqueId WRITE setPrayerMosqueId NOTIFY prayerMosqueIdChanged)
    Q_PROPERTY(QString configPath READ configPath CONSTANT)

public:
    explicit ConfigManager(QObject *parent = nullptr);

    int refreshIntervalMs() const { return m_refreshIntervalMs; }
    bool hasWindowPos() const { return m_hasWindowPos; }
    int windowX() const { return m_windowX; }
    int windowY() const { return m_windowY; }
    bool prayerEnabled() const { return m_prayerEnabled; }
    bool prayerUseApi() const { return m_prayerUseApi; }
    QString prayerCity() const { return m_prayerCity; }
    QString prayerCountry() const { return m_prayerCountry; }
    int prayerMethod() const { return m_prayerMethod; }
    bool effect3dEnabled() const { return m_effect3dEnabled; }
    void setEffect3dEnabled(bool on);
    int sphereDensity() const { return m_sphereDensity; }
    void setSphereDensity(int n);
    bool animateInBackground() const { return m_animateInBackground; }
    void setAnimateInBackground(bool on);
    double micSensitivity() const { return m_micSensitivity; }
    void setMicSensitivity(double v);
    QString micDeviceId() const { return m_micDeviceId; }
    void setMicDeviceId(const QString &id);
    QString themePreset() const { return m_themePreset; }
    void setThemePreset(const QString &name);
    QString prayerMosqueId() const { return m_prayerMosqueId; }
    void setPrayerMosqueId(const QString &id);
    QString configPath() const { return m_path; }

    Q_INVOKABLE void reload();

signals:
    void loaded();
    void effect3dEnabledChanged();
    void sphereDensityChanged();
    void animateInBackgroundChanged();
    void micSensitivityChanged();
    void micDeviceIdChanged();
    void themePresetChanged();
    void prayerMosqueIdChanged();

private:
    void load();
    void writeDefault();

    QString m_path;
    int m_refreshIntervalMs = 2000;
    bool m_hasWindowPos = false;
    int m_windowX = 0;
    int m_windowY = 0;
    bool m_prayerEnabled = true;
    bool m_prayerUseApi = true;
    QString m_prayerCity = QStringLiteral("Brussels");
    QString m_prayerCountry = QStringLiteral("Belgium");
    int m_prayerMethod = 2;
    bool m_effect3dEnabled = true;
    int m_sphereDensity = 4200;
    bool m_animateInBackground = false;   // défaut : pause hors-focus (économe GPU)
    double m_micSensitivity = 6.0;        // gain micro → amplitude des vagues (× courbe √)
    QString m_micDeviceId;                // micro choisi (vide = défaut Windows)
    QString m_themePreset = QStringLiteral("cyan");
    QString m_prayerMosqueId;             // slug mawaqit.net (vide = API Aladhan par ville)
};
