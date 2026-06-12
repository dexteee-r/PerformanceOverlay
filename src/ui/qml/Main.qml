import QtQuick
import PerformanceOverlay

// Fenêtre overlay : sans cadre, transparente, toujours au-dessus, tool window.
// Déplaçable à la souris (mode interactif) ; Échap quitte pendant le dev.
// Le click-through (WS_EX_TRANSPARENT) viendra via un helper Win32 plus tard.
Window {
    id: win
    width: Nav.view === "compact" ? 360 : 1280
    height: Nav.view === "compact" ? 560 : 720
    visible: true
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    title: "Performance Overlay"

    // Fournit la fenêtre au service plateforme + applique la config au démarrage.
    Component.onCompleted: { Overlay.attach(win); win.applyConfig() }

    // Applique config.ini : intervalle de refresh, config prière, position fenêtre.
    function applyConfig() {
        Metrics.intervalMs = Config.refreshIntervalMs
        Metrics.prayer.configure(Config.prayerCity, Config.prayerCountry,
                                 Config.prayerMethod, Config.prayerEnabled, Config.prayerUseApi)
        if (Config.hasWindowPos) { win.x = Config.windowX; win.y = Config.windowY }
    }

    // Rechargement à chaud (tray → Recharger config).
    Connections {
        target: Config
        function onLoaded() { win.applyConfig() }
    }

    // Fond de glissement (déclaré en premier = couche basse). Les futurs
    // contrôles d'AppShell seront au-dessus et captureront leurs propres clics.
    MouseArea {
        id: dragArea
        anchors.fill: parent
        property point clickPos
        onPressed: (m) => clickPos = Qt.point(m.x, m.y)
        onPositionChanged: (m) => {
            win.x += m.x - clickPos.x
            win.y += m.y - clickPos.y
        }
    }

    AppShell {
        anchors.fill: parent
    }

    Shortcut { sequence: "Escape"; onActivated: Qt.quit() }
    Shortcut { sequence: "F1";  onActivated: Nav.view = "cockpit" }
    Shortcut { sequence: "F2";  onActivated: Nav.view = "compact" }
    Shortcut { sequence: "F4";  onActivated: Nav.view = "tasks" }
    Shortcut { sequence: "F10"; onActivated: Nav.view = "settings" }
}
