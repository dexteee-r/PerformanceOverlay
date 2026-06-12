import QtQuick
import PerformanceOverlay

// Barre supérieure : point de statut + titre à gauche, horloge à droite.
// L'horloge utilise des chiffres tabulaires (tnum) pour ne pas "sauter".
Item {
    id: bar

    Row {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        Rectangle {
            width: 8; height: 8; radius: 4
            color: Theme.ok
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: "SYSTEM MONITOR"
            color: Theme.textHi
            font.family: Theme.fontUi
            font.pixelSize: 13
            font.letterSpacing: 3
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Text {
        id: clock
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: Theme.textMid
        font.family: Theme.fontMono
        font.pixelSize: 14
        font.features: ({ "tnum": 1 })   // chiffres à largeur constante
        text: Qt.formatDateTime(new Date(), "hh:mm:ss")

        Timer {
            interval: 1000; running: true; repeat: true
            onTriggered: clock.text = Qt.formatDateTime(new Date(), "hh:mm:ss")
        }
    }
}
