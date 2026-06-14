import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import PerformanceOverlay

// Réglages : démarrage auto, intervalle de refresh, click-through, thème, prière
// (lecture config.ini), rechargement. Page scrollable (le contenu peut dépasser la
// hauteur visible). Header fourni par AppShell.
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

    ScrollView {
        id: sv
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        GridLayout {
        width: sv.availableWidth
        columns: 2
        columnSpacing: Theme.gap
        rowSpacing: Theme.gap

        // ---- DÉMARRAGE ----
        Panel {
            Layout.fillWidth: true; Layout.preferredHeight: 140
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
            Layout.fillWidth: true; Layout.preferredHeight: 140
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
            Layout.fillWidth: true; Layout.preferredHeight: 360
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

                RowLayout {
                    Layout.fillWidth: true; Layout.topMargin: 2
                    opacity: Config.effect3dEnabled ? 1 : 0.4
                    Text { text: "Densité du nuage"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Choice { text: "FAIBLE"; on: Config.sphereDensity === 2500; onClicked: Config.sphereDensity = 2500 }
                    Choice { text: "MOYEN";  on: Config.sphereDensity === 4200; onClicked: Config.sphereDensity = 4200 }
                    Choice { text: "ÉLEVÉ";  on: Config.sphereDensity === 8000; onClicked: Config.sphereDensity = 8000 }
                }
                RowLayout {
                    Layout.fillWidth: true
                    opacity: Config.effect3dEnabled ? 1 : 0.4
                    Text { text: "Animer en arrière-plan"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Toggle { on: Config.animateInBackground
                             onClicked: Config.animateInBackground = !Config.animateInBackground }
                }
                Text { text: "Garde la sphère animée hors-focus (sinon pause auto = économe GPU)"
                       color: Theme.muted; font.family: Theme.fontMono; font.pixelSize: 11 }

                RowLayout {
                    Layout.fillWidth: true; Layout.topMargin: 2
                    opacity: Config.effect3dEnabled ? 1 : 0.4
                    Text { text: "Sensibilité micro (sphère)"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Choice { text: "FAIBLE"; on: Config.micSensitivity < 5.0; onClicked: Config.micSensitivity = 4.0 }
                    Choice { text: "MOYEN";  on: Config.micSensitivity >= 5.0 && Config.micSensitivity < 7.5; onClicked: Config.micSensitivity = 6.0 }
                    Choice { text: "ÉLEVÉ";  on: Config.micSensitivity >= 7.5; onClicked: Config.micSensitivity = 9.0 }
                }
                Text { text: "FAIBLE = plus d'écart parler↔crier (parole subtile) · ÉLEVÉ = réagit fort dès la parole."
                       color: Theme.muted; font.family: Theme.fontMono; font.pixelSize: 11 }
                Item { Layout.fillHeight: true }
            }
        }

        // ---- PRIÈRE ----
        Panel {
            Layout.fillWidth: true; Layout.preferredHeight: 180
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

        // ---- MICRO (anime la sphère) ----
        Panel {
            id: micPanel
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 132
            title: "MICRO (SPHÈRE)"
            property var micList: [{ id: "", name: "Défaut Windows" }].concat(Metrics.volume.inputDevices())
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Micro qui anime la sphère — parle pour la faire onduler"; color: Theme.text
                           font.family: Theme.fontUi; font.pixelSize: 14; Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: rl.implicitWidth + 22; Layout.preferredHeight: 28
                        color: "transparent"; border.width: 1; border.color: Theme.border
                        Text { id: rl; anchors.centerIn: parent; text: "ACTUALISER"; color: Theme.muted
                               font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 1 }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: micPanel.micList = [{ id: "", name: "Défaut Windows" }].concat(Metrics.volume.inputDevices()) }
                    }
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 8
                    Repeater {
                        model: micPanel.micList
                        Rectangle {
                            required property var modelData
                            readonly property bool on: Config.micDeviceId === modelData.id
                            width: mlbl.implicitWidth + 24; height: 30
                            color: on ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : "transparent"
                            border.width: 1; border.color: on ? Theme.accent : Theme.border
                            Text { id: mlbl; anchors.centerIn: parent; text: modelData.name
                                   color: on ? Theme.accent : Theme.muted; font.family: Theme.fontUi
                                   font.pixelSize: 11; font.letterSpacing: 0.5 }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                        onClicked: { Config.micDeviceId = modelData.id
                                                     Metrics.volume.useMicDevice(modelData.id) } }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
        }

        // ---- THÈME (pleine largeur) ----
        Panel {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 110
            title: "THÈME"
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                Text { text: "Palette d'accents (retinte jauges, sphère, courbes)"; color: Theme.text
                       font.family: Theme.fontUi; font.pixelSize: 14 }
                Flow {
                    Layout.fillWidth: true
                    spacing: 10
                    Repeater {
                        model: Theme.presetKeys
                        Rectangle {
                            required property string modelData
                            readonly property var def: Theme.presets[modelData]
                            readonly property bool on: Config.themePreset === modelData
                            width: sw.implicitWidth + 44; height: 36
                            color: on ? Qt.rgba(def.accent.r, def.accent.g, def.accent.b, 0.14) : "transparent"
                            border.width: 1; border.color: on ? def.accent : Theme.border
                            Row {
                                id: sw
                                anchors.centerIn: parent
                                spacing: 8
                                Row {
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 3
                                    Rectangle { width: 12; height: 12; radius: 6; color: def.accent }
                                    Rectangle { width: 12; height: 12; radius: 6; color: def.accent2 }
                                }
                                Text { anchors.verticalCenter: parent.verticalCenter; text: def.label
                                       color: on ? Theme.textHi : Theme.muted; font.family: Theme.fontUi
                                       font.pixelSize: 11; font.letterSpacing: 1 }
                            }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                        onClicked: Config.themePreset = parent.modelData }
                        }
                    }
                }
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
}
