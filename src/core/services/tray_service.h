#pragma once

#include <QObject>

class QSystemTrayIcon;
class QMenu;
class QWindow;
class OverlayController;
class ConfigManager;

// Icône de zone de notification (tray) : afficher/masquer l'overlay, basculer le
// mode passif (click-through), recharger la config, quitter.
class TrayService : public QObject
{
    Q_OBJECT
public:
    TrayService(QWindow *window, OverlayController *overlay,
                ConfigManager *config, QObject *parent = nullptr);

private:
    void toggleVisible();

    QSystemTrayIcon *m_tray = nullptr;
    QMenu *m_menu = nullptr;
    QWindow *m_window = nullptr;
    OverlayController *m_overlay = nullptr;
    ConfigManager *m_config = nullptr;
};
