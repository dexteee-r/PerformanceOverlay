import QtQuick
import QtQuick.Layouts
import PerformanceOverlay

// Vue compacte : petit widget vertical (l'essentiel). Fond fourni par AppShell.
Item {
    id: compact

    component CRow: ColumnLayout {
        property string label: ""
        property string value: ""
        property real frac: 0
        property color clr: Theme.accent
        Layout.fillWidth: true
        spacing: 5
        RowLayout {
            Layout.fillWidth: true
            Text { text: label; color: Theme.muted; font.family: Theme.fontUi
                   font.pixelSize: 12; font.letterSpacing: 2 }
            Item { Layout.fillWidth: true }
            Text { text: value; color: Theme.textHi; font.family: Theme.fontMono
                   font.pixelSize: 20; font.weight: Font.Bold; font.features: ({ "tnum": 1 }) }
        }
        Rectangle {
            Layout.fillWidth: true; height: 6; color: Qt.rgba(1, 1, 1, 0.06)
            Rectangle {
                height: parent.height
                width: parent.width * Math.max(0, Math.min(1, frac))
                color: clr
                Behavior on width { NumberAnimation { duration: 450; easing.type: Easing.OutCubic } }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            Rectangle { width: 8; height: 8; radius: 4; color: Theme.ok }
            Text { text: "OVERLAY"; color: Theme.textHi; font.family: Theme.fontUi
                   font.pixelSize: 12; font.letterSpacing: 3 }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: 32; height: 26; color: "transparent"; border.width: 1; border.color: Theme.border
                Text { anchors.centerIn: parent; text: "⤢"; color: Theme.muted; font.pixelSize: 14 }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                            onClicked: Nav.view = "cockpit" }
            }
        }

        Text { Layout.alignment: Qt.AlignHCenter
               text: Metrics.dateTime.now.toLocaleTimeString(Qt.locale("fr_FR"), "HH:mm:ss")
               color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 42
               font.weight: Font.Bold; font.features: ({ "tnum": 1 }) }
        Text { Layout.alignment: Qt.AlignHCenter
               text: Metrics.dateTime.now.toLocaleDateString(Qt.locale("fr_FR"), "ddd dd MMMM").toUpperCase()
               color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 2 }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        CRow { label: "CPU"; value: Metrics.cpu.usagePercent.toFixed(0) + " %"
               frac: Metrics.cpu.usagePercent / 100; clr: Theme.statusColor(Metrics.cpu.usagePercent / 100) }
        CRow { label: "RAM"; value: Metrics.ram.usagePercent.toFixed(0) + " %"
               frac: Metrics.ram.usagePercent / 100; clr: Theme.statusColor(Metrics.ram.usagePercent / 100) }
        CRow { label: "GPU"; value: Metrics.gpu.available ? Metrics.gpu.usagePercent.toFixed(0) + " %" : "n/a"
               frac: Metrics.gpu.usagePercent / 100
               clr: Metrics.gpu.temperatureC >= 85 ? Theme.crit
                    : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.accent }

        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }

        RowLayout {
            Layout.fillWidth: true
            Text { text: "PRIÈRE"; color: Theme.muted; font.family: Theme.fontUi
                   font.pixelSize: 11; font.letterSpacing: 2 }
            Item { Layout.fillWidth: true }
            Text { text: Metrics.prayer.nextName + "  " + Fmt.countdown(Metrics.prayer.remainingMinutes)
                   color: Theme.accent2; font.family: Theme.fontMono; font.pixelSize: 14; font.weight: Font.DemiBold }
        }
        RowLayout {
            Layout.fillWidth: true
            Text { text: "CLAUDE"; color: Theme.muted; font.family: Theme.fontUi
                   font.pixelSize: 11; font.letterSpacing: 2 }
            Item { Layout.fillWidth: true }
            Text { text: Metrics.claudeUsage.hasData
                         ? "J " + Metrics.claudeUsage.dayPercent.toFixed(0) + " %  ·  S " + Metrics.claudeUsage.weekPercent.toFixed(0) + " %"
                         : "—"
                   color: Theme.text; font.family: Theme.fontMono; font.pixelSize: 13 }
        }

        Item { Layout.fillHeight: true }
    }
}
