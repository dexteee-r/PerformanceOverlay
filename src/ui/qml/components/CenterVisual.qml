import QtQuick
import PerformanceOverlay

// Noyau « FLUX DE CHARGE » : routeur entre le rendu 3D (nuage de points, fidèle à
// la maquette) et le repli 2D léger, selon Config.effect3dEnabled. La lecture
// centrale (charge %) est posée par-dessus dans les deux cas.
//
// Réactivités de la sphère dérivées des métriques (cf. spéc maquette) :
//   bass = GPU%, mid = CPU%, high = temp GPU (42→87°C) *0.7 + débit ↓ (→5 Mo/s) *0.3.
Item {
    id: cv

    property real load: 0.0
    property color coreColor: Theme.statusColor(load)

    property real shown: load
    Behavior on shown { NumberAnimation { duration: 700; easing.type: Easing.OutCubic } }

    readonly property real rBass: Metrics.gpu.available ? Metrics.gpu.usagePercent / 100 : 0
    readonly property real rMid: Metrics.cpu.usagePercent / 100
    readonly property real rHigh: Math.min(1, Math.max(0, (Metrics.gpu.temperatureC - 42) / 45)) * 0.7
                                  + Math.min(1, Metrics.network.downBytesPerSec / 5.0e6) * 0.3

    // ---- Rendu 3D (chargé seulement si l'effet est activé) ----
    Loader {
        anchors.fill: parent
        active: Config.effect3dEnabled
        sourceComponent: sphere3dC
    }
    Component {
        id: sphere3dC
        Sphere3D { bass: cv.rBass; mid: cv.rMid; high: cv.rHigh }
    }

    // ---- Repli 2D (chargé seulement si l'effet est désactivé) ----
    Loader {
        anchors.fill: parent
        active: !Config.effect3dEnabled
        sourceComponent: fallback2dC
    }
    Component {
        id: fallback2dC
        CenterVisual2D { load: cv.load }
    }

    // ---- Lecture centrale (toujours au-dessus) ----
    Column {
        anchors.centerIn: parent
        spacing: 4
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2
            Text {
                id: bign
                text: Math.round(cv.shown * 100)
                color: "#FFFFFF"
                font.family: Theme.fontMono; font.pixelSize: 58; font.weight: Font.Black
                font.features: ({ "tnum": 1 })
            }
            Text {
                text: "%"; color: Theme.muted; font.family: Theme.fontMono; font.pixelSize: 22
                anchors.bottom: bign.bottom; anchors.bottomMargin: 9
            }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "CHARGE SYSTÈME"; color: Theme.muted
            font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 4
        }
    }
}
