#pragma once

#include <QObject>
#include <QWindow>
#include <QAbstractNativeEventFilter>
#include <QtQml/qqmlregistration.h>

// Service plateforme (Windows) pilotant le comportement « overlay » de la fenêtre :
//  - click-through : la souris traverse l'overlay vers le bureau (WS_EX_TRANSPARENT) ;
//  - raccourci GLOBAL Ctrl+Alt+O pour basculer interactif ↔ passif, même quand
//    l'overlay ne capte pas le focus (capté via QAbstractNativeEventFilter / WM_HOTKEY).
// Singleton QML accessible sous le nom `Overlay`.
class OverlayController : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Overlay)
    QML_SINGLETON

    Q_PROPERTY(bool clickThrough READ clickThrough WRITE setClickThrough NOTIFY clickThroughChanged)

public:
    explicit OverlayController(QObject *parent = nullptr);
    ~OverlayController() override;

    bool clickThrough() const { return m_clickThrough; }
    void setClickThrough(bool on);

    // Appelé depuis QML (Main.qml) pour fournir la fenêtre overlay.
    Q_INVOKABLE void attach(QWindow *window);
    Q_INVOKABLE void toggleClickThrough() { setClickThrough(!m_clickThrough); }

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    void clickThroughChanged();

private:
    void applyExStyle();

    void *m_hwnd = nullptr;          // HWND (opaque pour garder windows.h hors du header)
    bool m_clickThrough = false;
    bool m_hotkeyRegistered = false;
};
