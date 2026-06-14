import QtQuick
import PerformanceOverlay

// Noyau « FLUX DE CHARGE » : routeur entre le rendu 3D (nuage de points, fidèle à
// la maquette) et le repli 2D léger, selon Config.effect3dEnabled. La lecture
// centrale (charge %) est posée par-dessus dans les deux cas.
//
// Réactivité de la sphère pilotée par le MICRO (ta voix) :
//   bass + high = niveau micro (micLevel) → amplitude des vagues + pics.
//   mid = CPU% → vitesse de rotation/bruit de fond (la sphère vit même en silence).
Item {
    id: cv

    property real load: 0.0
    property color coreColor: Theme.statusColor(load)

    property real shown: load
    Behavior on shown { NumberAnimation { duration: 700; easing.type: Easing.OutCubic } }

    // Micro : noise gate (ignore le souffle) + courbe perceptuelle √ (booste les
    // sons faibles → réagit à voix normale sans coller le micro) × gain réglable.
    // Gate bas (0.006) : capte la voix à distance normale ; remonter si ça « jitter »
    // au repos (souffle de la pièce).
    readonly property real micRaw: Math.max(0, Metrics.volume.micLevel - 0.006)
    // Plafond > 1 (1.5) = MARGE pour parler fort / crier : la parole normale tourne
    // vers 0.5–0.8, crier pousse jusqu'à 1.5 → vagues nettement plus longues (le
    // shader sphere.vert amplifie au-delà de 1). Baisser le gain (Config.micSensitivity)
    // élargit l'écart parole↔cri ; le monter sature plus vite.
    readonly property real audioLevel: Math.min(1.5, Math.sqrt(micRaw) * Config.micSensitivity)
    readonly property real rBass: audioLevel
    readonly property real rMid: Metrics.cpu.usagePercent / 100
    readonly property real rHigh: audioLevel

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
