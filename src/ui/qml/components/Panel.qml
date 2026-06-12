import QtQuick
import PerformanceOverlay

// Panneau « cockpit » : fond dégradé + filet 1px + barre d'accent en haut-gauche
// + en-tête (point de statut, titre, tag). Le contenu déclaré dans Panel { ... }
// atterrit dans la zone body sous l'en-tête.
Item {
    id: root

    property string title: ""
    property string tag: ""
    property color accent: Theme.accent
    property color statusColor: Theme.ok
    property bool alert: false               // pulse de bordure sur état critique

    default property alias content: body.data

    // Fond + bordure
    Rectangle {
        id: bg
        anchors.fill: parent
        radius: Theme.radius
        border.width: 1
        border.color: root.alert ? Theme.crit : Theme.border
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.panelTop }
            GradientStop { position: 1.0; color: Theme.panelBot }
        }

        // Pulse d'alerte (bordure + halo) quand alert == true
        SequentialAnimation on border.color {
            running: root.alert
            loops: Animation.Infinite
            ColorAnimation { to: Theme.crit; duration: 700 }
            ColorAnimation { to: Qt.rgba(Theme.crit.r, Theme.crit.g, Theme.crit.b, 0.35); duration: 700 }
        }
    }

    // Barre d'accent en haut-gauche
    Rectangle {
        x: 0; y: 0
        width: 34; height: 2
        color: root.accent
    }

    // En-tête
    Item {
        id: header
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: Theme.pad }
        height: 18

        Rectangle {
            id: dot
            width: 8; height: 8; radius: 4
            color: root.statusColor
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            anchors.left: dot.right; anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: Theme.text
            font.family: Theme.fontUi
            font.pixelSize: 13
            font.letterSpacing: 2.4
            font.weight: Font.DemiBold
        }
        Text {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: root.tag
            visible: root.tag !== ""
            color: Theme.muted
            font.family: Theme.fontMono
            font.pixelSize: 11
            font.letterSpacing: 1.4
        }
    }

    // Zone de contenu
    Item {
        id: body
        anchors {
            left: parent.left; right: parent.right
            top: header.bottom; bottom: parent.bottom
            leftMargin: Theme.pad; rightMargin: Theme.pad
            topMargin: 10; bottomMargin: Theme.pad
        }
    }
}
