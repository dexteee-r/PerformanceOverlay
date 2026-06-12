import QtQuick
import QtQuick.Layouts
import PerformanceOverlay

// Barre de capacité LED segmentée (maquette : stockage). Segments allumés selon
// le remplissage ; couleur accent < 75 %, ambre ≥ 75 %, rouge ≥ 90 %. Le segment
// de tête « brille ».
Item {
    id: bar

    property real value: 0.0          // 0..1
    property int segments: 24
    readonly property color onColor: value >= 0.9 ? Theme.crit
                                    : value >= 0.75 ? Theme.warn : Theme.accent

    property real shown: value
    Behavior on shown { NumberAnimation { duration: 550; easing.type: Easing.OutCubic } }

    implicitHeight: 14

    RowLayout {
        anchors.fill: parent
        spacing: 3

        Repeater {
            model: bar.segments
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                readonly property real frac: (index + 1) / bar.segments
                readonly property bool isOn: frac <= bar.shown + 0.0001
                readonly property bool isLead: isOn && (frac + 1 / bar.segments) > bar.shown + 0.0001
                color: isOn ? bar.onColor : Qt.rgba(1, 1, 1, 0.05)
                opacity: isOn ? (isLead ? 1.0 : 0.85) : 1.0

                // Lueur du segment de tête
                Rectangle {
                    anchors.fill: parent
                    visible: parent.isLead
                    color: "transparent"
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.5)
                }
            }
        }
    }
}
