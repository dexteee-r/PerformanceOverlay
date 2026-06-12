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
    QString configPath() const { return m_path; }

    Q_INVOKABLE void reload();

signals:
    void loaded();

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
};
