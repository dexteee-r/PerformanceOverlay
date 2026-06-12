#include "overlay_controller.h"

#include <QWindow>
#include <QGuiApplication>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

namespace {
constexpr int kHotkeyId = 0xA001;   // Ctrl+Alt+O
}

OverlayController::OverlayController(QObject *parent) : QObject(parent)
{
    qApp->installNativeEventFilter(this);
    // Raccourci global (hwnd nul → posté dans la file de messages du thread,
    // capté par nativeEventFilter). MOD_NOREPEAT : un seul WM_HOTKEY par appui.
    m_hotkeyRegistered =
        RegisterHotKey(nullptr, kHotkeyId, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'O');
}

OverlayController::~OverlayController()
{
    if (m_hotkeyRegistered)
        UnregisterHotKey(nullptr, kHotkeyId);
    if (qApp)
        qApp->removeNativeEventFilter(this);
}

void OverlayController::attach(QWindow *window)
{
    if (!window)
        return;
    m_hwnd = reinterpret_cast<void *>(window->winId());
    applyExStyle();
}

void OverlayController::setClickThrough(bool on)
{
    if (m_clickThrough == on)
        return;
    m_clickThrough = on;
    applyExStyle();
    emit clickThroughChanged();
}

void OverlayController::applyExStyle()
{
    if (!m_hwnd)
        return;
    HWND hwnd = reinterpret_cast<HWND>(m_hwnd);
    LONG_PTR ex = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (m_clickThrough) {
        // WS_EX_LAYERED est déjà posé par la fenêtre transparente Qt ; on ajoute
        // WS_EX_TRANSPARENT pour que le hit-test laisse passer la souris.
        ex |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
    } else {
        ex &= ~static_cast<LONG_PTR>(WS_EX_TRANSPARENT);   // garder LAYERED
    }
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex);
}

bool OverlayController::nativeEventFilter(const QByteArray &, void *message, qintptr *)
{
    const MSG *msg = static_cast<const MSG *>(message);
    if (msg->message == WM_HOTKEY && static_cast<int>(msg->wParam) == kHotkeyId) {
        toggleClickThrough();
        return true;
    }
    return false;
}
