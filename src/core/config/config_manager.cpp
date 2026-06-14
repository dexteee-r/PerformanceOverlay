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

    m_effect3dEnabled = s.value(QStringLiteral("Display/effect_3d"), true).toBool();
    m_sphereDensity   = s.value(QStringLiteral("Display/sphere_density"), 4200).toInt();
    m_animateInBackground = s.value(QStringLiteral("Display/animate_in_background"), false).toBool();
    m_micSensitivity  = s.value(QStringLiteral("Audio/mic_sensitivity"), 6.0).toDouble();
    m_micDeviceId     = s.value(QStringLiteral("Audio/mic_device_id"), QString()).toString();
    m_themePreset     = s.value(QStringLiteral("Theme/preset"), QStringLiteral("cyan")).toString();
}

void ConfigManager::setEffect3dEnabled(bool on)
{
    if (on == m_effect3dEnabled)
        return;
    m_effect3dEnabled = on;
    QSettings s(m_path, QSettings::IniFormat);
    s.setValue(QStringLiteral("Display/effect_3d"), on);
    s.sync();
    emit effect3dEnabledChanged();
}

void ConfigManager::setSphereDensity(int n)
{
    if (n < 500)        n = 500;
    else if (n > 16000) n = 16000;   // borné comme SpherePointGeometry
    if (n == m_sphereDensity)
        return;
    m_sphereDensity = n;
    QSettings s(m_path, QSettings::IniFormat);
    s.setValue(QStringLiteral("Display/sphere_density"), n);
    s.sync();
    emit sphereDensityChanged();
}

void ConfigManager::setAnimateInBackground(bool on)
{
    if (on == m_animateInBackground)
        return;
    m_animateInBackground = on;
    QSettings s(m_path, QSettings::IniFormat);
    s.setValue(QStringLiteral("Display/animate_in_background"), on);
    s.sync();
    emit animateInBackgroundChanged();
}

void ConfigManager::setMicSensitivity(double v)
{
    if (qFuzzyCompare(v, m_micSensitivity))
        return;
    m_micSensitivity = v;
    QSettings s(m_path, QSettings::IniFormat);
    s.setValue(QStringLiteral("Audio/mic_sensitivity"), v);
    s.sync();
    emit micSensitivityChanged();
}

void ConfigManager::setMicDeviceId(const QString &id)
{
    if (id == m_micDeviceId)
        return;
    m_micDeviceId = id;
    QSettings s(m_path, QSettings::IniFormat);
    s.setValue(QStringLiteral("Audio/mic_device_id"), id);
    s.sync();
    emit micDeviceIdChanged();
}

void ConfigManager::setThemePreset(const QString &name)
{
    if (name == m_themePreset)
        return;
    m_themePreset = name;
    QSettings s(m_path, QSettings::IniFormat);
    s.setValue(QStringLiteral("Theme/preset"), name);
    s.sync();
    emit themePresetChanged();
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
        << "[Theme]\n"
        << "; Palette d'accents : cyan | lime | amber | violet | ice\n"
        << "preset = cyan\n\n"
        << "[Display]\n"
        << "; Noyau « Flux de charge » en nuage de points 3D (false = repli 2D leger)\n"
        << "effect_3d = true\n"
        << "; Densite du nuage de points (500..16000)\n"
        << "sphere_density = 4200\n"
        << "; Continuer d'animer la sphere quand l'overlay n'a PAS le focus\n"
        << "; (false = pause auto en arriere-plan = econome GPU ; true = anime en continu)\n"
        << "animate_in_background = false\n\n"
        << "[Audio]\n"
        << "; Sensibilite du micro pour animer la sphere (gain x courbe racine ; ~4 faible .. 9 fort)\n"
        << "; Plus bas = plus d'ecart entre parler et crier ; plus haut = sature vite.\n"
        << "mic_sensitivity = 6.0\n"
        << "; Micro qui anime la sphere (vide = peripherique d'enregistrement par defaut)\n"
        << "; Choisis-le dans Reglages plutot que d'editer cet ID a la main.\n"
        << "mic_device_id =\n\n"
        << "[Prayer]\n"
        << "prayer_enabled = true\n"
        << "prayer_use_api = true\n"
        << "prayer_city = Brussels\n"
        << "prayer_country = Belgium\n"
        << "; Methode de calcul : 2=ISNA, 3=MWL, 5=Egypt, ...\n"
        << "prayer_method = 2\n";
    f.close();
}
