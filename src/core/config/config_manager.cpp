#include "config_manager.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{
    m_path = QCoreApplication::applicationDirPath() + QStringLiteral("/config.ini");
    load();
}

void ConfigManager::reload()
{
    load();
    emit loaded();
}

void ConfigManager::load()
{
    if (!QFile::exists(m_path))
        writeDefault();

    QSettings s(m_path, QSettings::IniFormat);

    m_refreshIntervalMs = s.value(QStringLiteral("Performance/refresh_interval_ms"), 2000).toInt();

    m_hasWindowPos = s.contains(QStringLiteral("Window/x")) && s.contains(QStringLiteral("Window/y"));
    m_windowX = s.value(QStringLiteral("Window/x"), 0).toInt();
    m_windowY = s.value(QStringLiteral("Window/y"), 0).toInt();

    m_prayerEnabled  = s.value(QStringLiteral("Prayer/prayer_enabled"), true).toBool();
    m_prayerUseApi   = s.value(QStringLiteral("Prayer/prayer_use_api"), true).toBool();
    m_prayerCity     = s.value(QStringLiteral("Prayer/prayer_city"), QStringLiteral("Brussels")).toString();
    m_prayerCountry  = s.value(QStringLiteral("Prayer/prayer_country"), QStringLiteral("Belgium")).toString();
    m_prayerMethod   = s.value(QStringLiteral("Prayer/prayer_method"), 2).toInt();
}

void ConfigManager::writeDefault()
{
    QFile f(m_path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&f);
    out << "; Performance Overlay — configuration\n"
        << "; Edite ce fichier puis recharge depuis le tray (ou relance).\n\n"
        << "[Window]\n"
        << "; Position de l'overlay (laisser commente = centre par defaut)\n"
        << "; x = 40\n"
        << "; y = 40\n\n"
        << "[Performance]\n"
        << "refresh_interval_ms = 2000\n\n"
        << "[Prayer]\n"
        << "prayer_enabled = true\n"
        << "prayer_use_api = true\n"
        << "prayer_city = Brussels\n"
        << "prayer_country = Belgium\n"
        << "; Methode de calcul : 2=ISNA, 3=MWL, 5=Egypt, ...\n"
        << "prayer_method = 2\n";
    f.close();
}
