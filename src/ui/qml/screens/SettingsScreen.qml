import QtQuick
import QtQuick.Layouts
import PerformanceOverlay

// Réglages : démarrage auto, intervalle de refresh, click-through, prière (lecture
// config.ini), rechargement de la config. Header fourni par AppShell.
Item {
    id: settings

    // Interrupteur on/off.
    component Toggle: Rectangle {
        property bool on: false
        signal clicked()
        width: 48; height: 26; radius: 13
        color: on ? Qt.rgba(Theme.ok.r, Theme.ok.g, Theme.ok.b, 0.25) : Qt.rgba(1, 1, 1, 0.06)
        border.width: 1; border.color: on ? Theme.ok : Theme.border
        Rectangle {
            width: 18; height: 18; radius: 9; y: 3
            x: on ? parent.width - 21 : 3
            color: on ? Theme.ok : Theme.muted
            Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }
        }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked() }
    }
    // Petit bouton (valeur sélectionnable).
    component Choice: Rectangle {
        property string text: ""
        property bool on: false
        signal clicked()
        width: ct.implicitWidth + 22; height: 28
        color: on ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : "transparent"
        border.width: 1; border.color: on ? Theme.accent : Theme.border
        Text { id: ct; anchors.centerIn: parent; text: parent.text
               color: on ? Theme.accent : Theme.muted; font.family: Theme.fontUi
               font.pixelSize: 12; font.letterSpacing: 1 }
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: parent.clicked() }
    }

    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: Theme.gap
        rowSpacing: Theme.gap

        // ---- DÉMARRAGE ----
        Panel {
            Layout.fillWidth: true; Layout.fillHeight: true
            title: "DÉMARRAGE"
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Lancer au démarrage de Windows"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Toggle { on: Startup.enabled; onClicked: Startup.enabled = !Startup.enabled }
                }
                Text { text: "Écrit dans HKCU\\…\\Run"; color: Theme.muted
                       font.family: Theme.fontMono; font.pixelSize: 11 }
                Item { Layout.fillHeight: true }
            }
        }

        // ---- PERFORMANCE ----
        Panel {
            Layout.fillWidth: true; Layout.fillHeight: true
            title: "RAFRAÎCHISSEMENT"
            ColumnLayout {
                anchors.fill: parent
                spacing: 12
                Text { text: "Intervalle de mesure"; color: Theme.text
                       font.family: Theme.fontUi; font.pixelSize: 14 }
                RowLayout {
                    spacing: 8
                    Choice { text: "1 s"; on: Metrics.intervalMs === 1000; onClicked: Metrics.intervalMs = 1000 }
                    Choice { text: "2 s"; on: Metrics.intervalMs === 2000; onClicked: Metrics.intervalMs = 2000 }
                    Choice { text: "4 s"; on: Metrics.intervalMs === 4000; onClicked: Metrics.intervalMs = 4000 }
                }
                Text { text: "Persistant via refresh_interval_ms dans config.ini"; color: Theme.muted
                       font.family: Theme.fontMono; font.pixelSize: 11 }
                Item { Layout.fillHeight: true }
            }
        }

        // ---- AFFICHAGE ----
        Panel {
            Layout.fillWidth: true; Layout.fillHeight: true
            title: "AFFICHAGE"
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Mode passif (click-through)"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Toggle { on: Overlay.clickThrough; onClicked: Overlay.clickThrough = !Overlay.clickThrough }
                }
                Text { text: "Raccourci global : Ctrl+Alt+O"; color: Theme.muted
                       font.family: Theme.fontMono; font.pixelSize: 11 }

                Rectangle { Layout.fillWidth: true; Layout.topMargin: 4; implicitHeight: 1; color: Theme.border }

                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Effet 3D (noyau Flux de charge)"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Toggle { on: Config.effect3dEnabled; onClicked: Config.effect3dEnabled = !Config.effect3dEnabled }
                }
                Text { text: "Sphère de points GPU · pause auto en jeu / mode passif"; color: Theme.muted
                       font.family: Theme.fontMono; font.pixelSize: 11 }
                Item { Layout.fillHeight: true }
            }
        }

        // ---- PRIÈRE ----
        Panel {
            Layout.fillWidth: true; Layout.fillHeight: true
            title: "PRIÈRE"
            accent: Theme.accent2
            statusColor: Theme.accent2
            ColumnLayout {
                anchors.fill: parent
                spacing: 8
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Ville"; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 13; Layout.fillWidth: true }
                    Text { text: Config.prayerCity; color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 13 }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Pays"; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 13; Layout.fillWidth: true }
                    Text { text: Config.prayerCountry; color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 13 }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Méthode"; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 13; Layout.fillWidth: true }
                    Text { text: "" + Config.prayerMethod; color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 13 }
                }
                Text { text: "Modifier dans config.ini puis recharger"; color: Theme.muted
                       font.family: Theme.fontMono; font.pixelSize: 11 }
                Item { Layout.fillHeight: true }
            }
        }

        // ---- CONFIG (pleine largeur) ----
        Panel {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 92
            title: "CONFIGURATION"
            RowLayout {
                anchors.fill: parent
                spacing: 14
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Text { text: "Fichier"; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 12 }
                    Text { text: Config.configPath; color: Theme.text; font.family: Theme.fontMono
                           font.pixelSize: 12; elide: Text.ElideMiddle; Layout.fillWidth: true }
                }
                Rectangle {
                    Layout.preferredWidth: rcl.implicitWidth + 26; Layout.preferredHeight: 32
                    color: "transparent"; border.width: 1; border.color: Theme.accent
                    Text { id: rcl; anchors.centerIn: parent; text: "RECHARGER"; color: Theme.accent
                           font.family: Theme.fontUi; font.pixelSize: 12; font.letterSpacing: 1 }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: Config.reload() }
                }
            }
        }
    }
}
