import QtQuick
import PerformanceOverlay

// Voile d'alerte plein écran : vignette rouge en bordure + badge, posé au-dessus de
// toutes les vues. Ne capte PAS la souris (aucun MouseArea) → n'interfère ni avec le
// mode interactif ni avec le click-through. N'anime QUE si `active` (statique au repos).
Item {
    id: veil

    property bool active: false
    property string label: ""

    readonly property color c: Theme.crit
    readonly property int band: 120

    visible: opacity > 0.01
    // Respire entre 0.55 et 1.0 tant que l'alerte est active ; sinon disparaît en fondu.
    property real pulse: 1.0
    opacity: active ? pulse : 0.0
    Behavior on opacity { enabled: !veil.active; NumberAnimation { duration: 320 } }
    SequentialAnimation on pulse {
        running: veil.active
        loops: Animation.Infinite
        NumberAnimation { from: 0.55; to: 1.0; duration: 720; easing.type: Easing.InOutSine }
        NumberAnimation { from: 1.0; to: 0.55; duration: 720; easing.type: Easing.InOutSine }
    }

    // ---- Vignette : 4 bandes dégradées rouge → transparent vers le centre ----
    Rectangle {
        width: parent.width; height: veil.band; anchors.top: parent.top
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(veil.c.r, veil.c.g, veil.c.b, 0.5) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
    Rectangle {
        width: parent.width; height: veil.band; anchors.bottom: parent.bottom
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: Qt.rgba(veil.c.r, veil.c.g, veil.c.b, 0.5) }
        }
    }
    Rectangle {
        height: parent.height; width: veil.band; anchors.left: parent.left
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Qt.rgba(veil.c.r, veil.c.g, veil.c.b, 0.5) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
    Rectangle {
        height: parent.height; width: veil.band; anchors.right: parent.right
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: Qt.rgba(veil.c.r, veil.c.g, veil.c.b, 0.5) }
        }
    }

    // Liseré net pour cadrer la zone d'alerte.
    Rectangle {
        anchors.fill: parent; color: "transparent"
        border.width: 2; border.color: Qt.rgba(veil.c.r, veil.c.g, veil.c.b, 0.7)
    }

    // ---- Badge d'alerte (haut, centré) ----
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top; anchors.topMargin: 16
        width: badge.implicitWidth + 28; height: 32; radius: 4
        color: Qt.rgba(veil.c.r, veil.c.g, veil.c.b, 0.18)
        border.width: 1; border.color: veil.c
        Row {
            id: badge
            anchors.centerIn: parent
            spacing: 9
            Rectangle { width: 9; height: 9; radius: 4.5; color: veil.c; anchors.verticalCenter: parent.verticalCenter }
            Text { anchors.verticalCenter: parent.verticalCenter; text: veil.label
                   color: Theme.textHi; font.family: Theme.fontUi; font.pixelSize: 13
                   font.weight: Font.DemiBold; font.letterSpacing: 2 }
        }
    }
}
