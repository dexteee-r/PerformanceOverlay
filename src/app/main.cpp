#include <QApplication>
#include <QQmlApplicationEngine>
#include <QFontDatabase>
#include <QFont>
#include <QStringList>
#include <QQuickWindow>

#include "overlay_controller.h"
#include "config_manager.h"
#include "tray_service.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

// Point d'entrée. QApplication (QtWidgets) car le tray (QSystemTrayIcon) en a
// besoin ; l'UI reste 100 % Qt Quick. Logique applicative via services/modèles
// C++ exposés au QML (Metrics, Overlay, Config) — ici on démarre et on câble.
int main(int argc, char *argv[])
{
    // --- Single-instance : mutex nommé. Si déjà présent → on ne relance pas
    //     (sinon les instances s'empilent et verrouillent l'exe). ---
    HANDLE instanceMutex = CreateMutexW(nullptr, FALSE, L"PerformanceOverlay_SingleInstance");
    if (instanceMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(instanceMutex);
        return 0;
    }

    QApplication app(argc, argv);
    app.setApplicationName("Performance Overlay");
    app.setOrganizationName("Mazani");
    app.setQuitOnLastWindowClosed(false);   // l'overlay survit masqué dans le tray

    // Polices embarquées (cf. CMake qt_add_resources /fonts) : Satoshi (UI) +
    // JetBrains Mono (chiffres live, alignement tabulaire).
    const QStringList jbWeights{"Regular", "Medium", "Bold", "ExtraBold"};
    for (const QString &w : jbWeights)
        QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/JetBrainsMono-%1.ttf").arg(w));
    const QStringList satoshiWeights{"Regular", "Medium", "Bold", "Black"};
    for (const QString &w : satoshiWeights)
        QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Satoshi-%1.ttf").arg(w));

    // Chiffres tabulaires partout (feature OpenType tnum) → les nombres qui changent
    // (horloge, métriques) gardent une largeur fixe = aucun sautillement, même avec
    // une police proportionnelle comme Satoshi. Hérité par les Text qui ne fixent que
    // family/pixelSize.
    QFont baseFont = app.font();
    baseFont.setFeature(QFont::Tag("tnum"), 1);
    app.setFont(baseFont);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("PerformanceOverlay", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    // --- Tray : relie la fenêtre overlay, le click-through et la config. ---
    auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
    auto *overlay = engine.singletonInstance<OverlayController *>("PerformanceOverlay", "Overlay");
    auto *config = engine.singletonInstance<ConfigManager *>("PerformanceOverlay", "Config");
    TrayService tray(window, overlay, config);

    const int rc = app.exec();
    if (instanceMutex) {
        ReleaseMutex(instanceMutex);
        CloseHandle(instanceMutex);
    }
    return rc;
}
