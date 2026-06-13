import QtQuick
import PerformanceOverlay

// Barre de navigation entre les vues (lit/écrit le singleton Nav).
Row {
    id: nav
    spacing: 6

    readonly property var items: [
        ["cockpit", "COCKPIT"],
        ["fullpage", "PLEIN"],
        ["compact", "COMPACT"],
        ["tasks", "TÂCHES"],
        ["settings", "RÉGLAGES"]
    ]

    Repeater {
        model: nav.items
        Rectangle {
            id: btn
            required property var modelData
            readonly property bool on: Nav.view === modelData[0]
            width: lbl.implicitWidth + 24
            height: 28
            color: on ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : "transparent"
            border.width: 1
            border.color: on ? Theme.accent : Theme.border

            Text {
                id: lbl
                anchors.centerIn: parent
                text: btn.modelData[1]
                color: btn.on ? Theme.accent : Theme.muted
                font.family: Theme.fontUi
                font.pixelSize: 11
                font.letterSpacing: 1.5
                font.weight: btn.on ? Font.DemiBold : Font.Normal
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: Nav.view = btn.modelData[0]
            }
        }
    }
}
