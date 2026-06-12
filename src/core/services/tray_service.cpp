#include "tray_service.h"
#include "overlay_controller.h"
#include "config_manager.h"

#include <QSystemTrayIcon>
#include <QMenu>
#include <QWindow>
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QAction>

namespace {
// Petite icône générée (anneau cyan + point magenta) — pas de fichier requis.
QIcon makeTrayIcon()
{
    QPixmap pm(32, 32);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(QStringLiteral("#00E6FF")), 4));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(5, 5, 22, 22);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(QStringLiteral("#FF40B4")));
    p.drawEllipse(12, 12, 8, 8);
    p.end();
    return QIcon(pm);
}
}

TrayService::TrayService(QWindow *window, OverlayController *overlay,
                         ConfigManager *config, QObject *parent)
    : QObject(parent), m_window(window), m_overlay(overlay), m_config(config)
{
    m_tray = new QSystemTrayIcon(makeTrayIcon(), this);
    m_tray->setToolTip(QStringLiteral("Performance Overlay"));

    m_menu = new QMenu();

    QAction *showAct = m_menu->addAction(QStringLiteral("Afficher / Masquer"));
    connect(showAct, &QAction::triggered, this, &TrayService::toggleVisible);

    if (m_overlay) {
        QAction *ctAct = m_menu->addAction(QStringLiteral("Mode passif (click-through)"));
        ctAct->setCheckable(true);
        ctAct->setChecked(m_overlay->clickThrough());
        connect(ctAct, &QAction::toggled, m_overlay, &OverlayController::setClickThrough);
        connect(m_overlay, &OverlayController::clickThroughChanged, ctAct,
                [this, ctAct] { ctAct->setChecked(m_overlay->clickThrough()); });
    }

    if (m_config) {
        QAction *reloadAct = m_menu->addAction(QStringLiteral("Recharger la config"));
        connect(reloadAct, &QAction::triggered, m_config, &ConfigManager::reload);
    }

    m_menu->addSeparator();
    QAction *quitAct = m_menu->addAction(QStringLiteral("Quitter"));
    connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);

    m_tray->setContextMenu(m_menu);
    connect(m_tray, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger
                    || reason == QSystemTrayIcon::DoubleClick)
                    toggleVisible();
            });
    m_tray->show();
}

void TrayService::toggleVisible()
{
    if (!m_window)
        return;
    m_window->setVisible(!m_window->isVisible());
    if (m_window->isVisible()) {
        m_window->raise();
        m_window->requestActivate();
    }
}
