import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import PerformanceOverlay

// Routeur d'écrans : fond ambiant commun + header partagé (avec NavBar) + Loader
// qui charge la vue active (cockpit / tasks / settings). La vue « compact »
// occupe tout le cadre (sa propre chrome). Vue pilotée par le singleton Nav.
Item {
    id: shell

    component Chip: Column {
        property string k: ""
        property string v: ""
        spacing: 2
        Text { text: k; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 2 }
        Text { text: v; color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 16
               font.weight: Font.DemiBold; font.features: ({ "tnum": 1 }) }
    }

    // ================= FOND =================
    Rectangle { anchors.fill: parent; color: Theme.bgBase }
    Item {
        anchors.fill: parent
        Rectangle { id: orbA; width: 640; height: 640; radius: width / 2; x: -180; y: -240
                    color: Theme.accent; visible: false; layer.enabled: true }
        MultiEffect { source: orbA; anchors.fill: orbA; blurEnabled: true; blur: 1.0; blurMax: 64; opacity: 0.10 }
        Rectangle { id: orbB; width: 720; height: 720; radius: width / 2
                    x: parent.width - 500; y: parent.height - 480
                    color: Theme.accent2; visible: false; layer.enabled: true }
        MultiEffect { source: orbB; anchors.fill: orbB; blurEnabled: true; blur: 1.0; blurMax: 64; opacity: 0.09 }
    }
    Rectangle { anchors.fill: parent; color: "transparent"; border.width: 1; border.color: Theme.border }

    // ================= VUE COMPACTE (plein cadre) =================
    Loader {
        anchors.fill: parent
        active: Nav.view === "compact"
        sourceComponent: compactC
    }

    // ================= VUES STANDARD (header + écran) =================
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.gap
        spacing: Theme.gap
        visible: Nav.view !== "compact"

        // ---------- HEADER PARTAGÉ ----------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 66
            border.width: 1; border.color: Theme.border
            gradient: Gradient {
                GradientStop { position: 0.0; color: Theme.panelTop }
                GradientStop { position: 1.0; color: Theme.bgBase2 }
            }
            Rectangle { x: 0; y: 0; width: 44; height: 2; color: Theme.accent }

            Row {
                anchors.left: parent.left; anchors.leftMargin: 22
                anchors.verticalCenter: parent.verticalCenter
                spacing: 14
                Rectangle { width: 10; height: 10; radius: 5; color: Theme.ok; anchors.verticalCenter: parent.verticalCenter }
                Column {
                    spacing: 3
                    Text { text: "PERFORMANCE OVERLAY"; color: Theme.textHi; font.family: Theme.fontUi
                           font.pixelSize: 18; font.weight: Font.Bold; font.letterSpacing: 4 }
                    Text { text: Nav.view.toUpperCase() + " · QT 6 / QML"; color: Theme.muted
                           font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 5 }
                }
            }

            NavBar {
                anchors.left: parent.left; anchors.leftMargin: 360
                anchors.verticalCenter: parent.verticalCenter
            }

            Row {
                anchors.right: parent.right; anchors.rightMargin: 22
                anchors.verticalCenter: parent.verticalCenter
                spacing: 26
                Row {
                    spacing: 9
                    anchors.verticalCenter: parent.verticalCenter
                    Rectangle { width: 9; height: 9; radius: 4.5
                                color: Overlay.clickThrough ? Theme.warn : Theme.ok
                                anchors.verticalCenter: parent.verticalCenter }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Text { text: Overlay.clickThrough ? "PASSIF" : "INTERACTIF"
                               color: Theme.textHi; font.family: Theme.fontUi; font.pixelSize: 12; font.letterSpacing: 1 }
                        Text { text: "Ctrl+Alt+O"; color: Theme.muted; font.family: Theme.fontUi
                               font.pixelSize: 9; font.letterSpacing: 1 }
                    }
                }
                Rectangle { width: 1; height: 40; color: Theme.border; anchors.verticalCenter: parent.verticalCenter }
                Chip { k: "PROCESSUS"; v: "" + Metrics.process.count; anchors.verticalCenter: parent.verticalCenter }
                Chip { k: "UPTIME"; v: Fmt.uptime(Metrics.uptime.seconds); anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 1; height: 40; color: Theme.border; anchors.verticalCenter: parent.verticalCenter }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    Text { anchors.right: parent.right
                           text: Metrics.dateTime.now.toLocaleTimeString(Qt.locale("fr_FR"), "HH:mm:ss")
                           color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 30
                           font.weight: Font.Bold; font.features: ({ "tnum": 1 }) }
                    Text { anchors.right: parent.right
                           text: Metrics.dateTime.now.toLocaleDateString(Qt.locale("fr_FR"), "ddd dd MMM").toUpperCase()
                           color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 3 }
                }
            }
        }

        // ---------- ÉCRAN ACTIF ----------
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: Nav.view === "tasks" ? tasksC
                             : Nav.view === "settings" ? settingsC : cockpitC
        }
    }

    // ================= VOILE D'ALERTE (au-dessus de tout) =================
    // Seuils critiques : GPU très chaud, ou CPU/RAM quasi saturés.
    readonly property bool _gpuHot: Metrics.gpu.available && Metrics.gpu.temperatureC >= 88
    readonly property bool _cpuMax: Metrics.cpu.usagePercent >= 96
    readonly property bool _ramMax: Metrics.ram.usagePercent >= 95
    readonly property string _alertLabel:
        _gpuHot ? "ALERTE THERMIQUE GPU · " + Metrics.gpu.temperatureC.toFixed(0) + "°C"
        : _cpuMax ? "CHARGE CPU CRITIQUE · " + Metrics.cpu.usagePercent.toFixed(0) + "%"
        : _ramMax ? "MÉMOIRE SATURÉE · " + Metrics.ram.usagePercent.toFixed(0) + "%" : ""

    AlertVeil {
        anchors.fill: parent
        active: shell._gpuHot || shell._cpuMax || shell._ramMax
        label: shell._alertLabel
    }

    Component { id: cockpitC;  CockpitScreen {} }
    Component { id: tasksC;    TasksScreen {} }
    Component { id: settingsC; SettingsScreen {} }
    Component { id: compactC;  CompactScreen {} }
}
