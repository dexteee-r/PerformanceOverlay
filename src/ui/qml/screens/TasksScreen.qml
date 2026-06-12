import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import PerformanceOverlay

// Gestionnaire de tâches : liste des processus (modèle C++ TaskKiller) triés par
// mémoire, avec kill (processus critiques protégés). Header fourni par AppShell.
Item {
    id: tasks

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 14
            Text { text: "GESTIONNAIRE DE TÂCHES"; color: Theme.textHi; font.family: Theme.fontUi
                   font.pixelSize: 16; font.letterSpacing: 2; font.weight: Font.DemiBold }
            Text { text: TaskKiller.count + " processus"; color: Theme.muted
                   font.family: Theme.fontMono; font.pixelSize: 13 }
            Item { Layout.fillWidth: true }
            Text { text: TaskKiller.lastMessage; color: Theme.warn
                   font.family: Theme.fontMono; font.pixelSize: 12 }
            Rectangle {
                width: rl.implicitWidth + 26; height: 30; color: "transparent"
                border.width: 1; border.color: Theme.accent
                Text { id: rl; anchors.centerIn: parent; text: "RAFRAÎCHIR"; color: Theme.accent
                       font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 1 }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: TaskKiller.refresh() }
            }
        }

        Panel {
            Layout.fillWidth: true; Layout.fillHeight: true
            title: "PROCESSUS"
            tag: "tri mémoire ↓"

            ListView {
                anchors.fill: parent
                clip: true
                model: TaskKiller
                spacing: 1
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                delegate: Rectangle {
                    required property int index
                    required property string name
                    required property int pid
                    required property double memoryMb
                    required property bool critical
                    width: ListView.view.width
                    height: 34
                    color: index % 2 ? Qt.rgba(1, 1, 1, 0.02) : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8; anchors.rightMargin: 8
                        spacing: 10
                        Text { text: name; color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 13
                               Layout.preferredWidth: 280; elide: Text.ElideRight }
                        Text { text: "pid " + pid; color: Theme.muted; font.family: Theme.fontMono
                               font.pixelSize: 12; Layout.preferredWidth: 90 }
                        Item { Layout.fillWidth: true }
                        Text { text: Fmt.mem(memoryMb); color: Theme.text; font.family: Theme.fontMono
                               font.pixelSize: 13; horizontalAlignment: Text.AlignRight
                               Layout.preferredWidth: 100; font.features: ({ "tnum": 1 }) }
                        Rectangle {
                            Layout.preferredWidth: 66; Layout.preferredHeight: 24
                            visible: !critical
                            color: "transparent"; border.width: 1; border.color: Theme.crit
                            Text { anchors.centerIn: parent; text: "KILL"; color: Theme.crit
                                   font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 1 }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                        onClicked: TaskKiller.kill(pid) }
                        }
                        Text { visible: critical; text: "PROTÉGÉ"; color: Theme.muted
                               font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 1
                               Layout.preferredWidth: 66; horizontalAlignment: Text.AlignHCenter }
                    }
                }
            }
        }
    }
}
