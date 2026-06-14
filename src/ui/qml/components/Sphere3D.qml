import QtQuick
import QtQuick.Window
import QtQuick3D
import PerformanceOverlay

// Noyau « FLUX DE CHARGE » en nuage de points 3D (fidèle à la maquette WebGL).
// Une sphère de points déformée par du bruit simplex, réactive à la charge :
//   bass (GPU) → amplitude + taille des points + scale global
//   mid  (CPU) → vitesse du bruit + vitesse de rotation Y
//   high (temp+réseau) → pics + blanchiment
//
// ⚠️ GARDE-FOUS PERF : un UNIQUE Timer 30 Hz pilote TOUTE l'animation (temps,
// rotation, lissage des réactions). Il ne tourne QUE si l'effet est activé, la
// fenêtre a le focus, le mode n'est pas passif (click-through) et l'item est
// visible → cap ~30 fps + pause auto en jeu / overlay passif (le GPU est le même
// même sur un 2ᵉ écran). Quand il s'arrête, la scène ne se redessine plus.
Item {
    id: root

    // Entrées de réaction (0..1), lissées en interne.
    property real bass: 0.0
    property real mid: 0.0
    property real high: 0.0

    // Réglages : densité pilotée par la config (persistante, réglable en Réglages).
    // La taille des points suit ~1.3/sqrt(count) → la sphère reste aussi « pleine »
    // quelle que soit la densité (plus de points = points plus fins).
    // Plein écran : la sphère est étalée sur beaucoup de pixels → on densifie (×3,
    // plafonné à 32000) pour garder une vraie résolution. En cockpit (petit) on garde
    // la densité configurée (léger). La géométrie se reconstruit au changement de vue.
    property int pointCount: Nav.view === "fullpage"
                             ? Math.min(32000, Config.sphereDensity * 3)
                             : Config.sphereDensity
    property real pointSize: 1.3 / Math.sqrt(Math.max(1, pointCount))

    // Pause auto quand l'overlay n'a pas le focus → 0 coût GPU au repos. L'option
    // « Animer en arrière-plan » (Config) relâche UNIQUEMENT cette exigence de focus
    // (le mode passif click-through continue, lui, de mettre en pause).
    readonly property bool active3d: Config.effect3dEnabled
                                     && root.visible
                                     && (Window.active || Config.animateInBackground)
                                     && !Overlay.clickThrough

    // Valeurs lissées + animées (mises à jour à 30 Hz par le ticker).
    property real fB: 0.0
    property real fM: 0.0
    property real fH: 0.0
    property real rotX: -18.0
    property real rotY: 0.0
    property real cloudScale: 1.0

    View3D {
        anchors.fill: parent
        camera: cam
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Transparent
            // En plein écran la sphère est étalée sur beaucoup de pixels → l'aliasing
            // des points (quads à masque rond en alpha) devient visible (« effet 480p »).
            // On supersample (SSAA) UNIQUEMENT en plein écran ; en cockpit (petit) le
            // NoAA suffit et évite de payer le GPU pour rien.
            antialiasingMode: Nav.view === "fullpage" ? SceneEnvironment.SSAA
                                                       : SceneEnvironment.NoAA
            antialiasingQuality: SceneEnvironment.VeryHigh   // SSAA 2× quand actif
        }

        PerspectiveCamera {
            id: cam
            position: Qt.vector3d(0, 0, 3.9)
            fieldOfView: 45
            clipNear: 0.1   // défaut Qt = 10 → clipperait la sphère (à ~4 unités) !
            clipFar: 100
        }

        Model {
            id: cloud
            geometry: SpherePointGeometry { count: root.pointCount }
            scale: Qt.vector3d(root.cloudScale, root.cloudScale, root.cloudScale)
            eulerRotation.x: root.rotX
            eulerRotation.y: root.rotY
            materials: [
                CustomMaterial {
                    id: sphereMat
                    shadingMode: CustomMaterial.Unshaded
                    cullMode: Material.NoCulling
                    depthDrawMode: Material.NeverDepthDraw
                    // Blend additif → nuage lumineux qui « glow ».
                    sourceBlend: CustomMaterial.SrcAlpha
                    destinationBlend: CustomMaterial.One
                    vertexShader: "qrc:/shaders/sphere.vert"
                    fragmentShader: "qrc:/shaders/sphere.frag"

                    property real uTime: 0.0
                    property real uBass: 0.0
                    property real uMid: 0.0
                    property real uHigh: 0.0
                    property real uPointSize: root.pointSize
                    property color uColA: Theme.accent    // cyan
                    property color uColB: Theme.accent2   // magenta
                }
            ]
        }
    }

    // ---- Ticker unique 30 Hz : temps + rotation + lissage des réactions ----
    Timer {
        interval: 33
        repeat: true
        running: root.active3d
        onTriggered: {
            sphereMat.uTime += 0.033

            // Lissage (lerp) comme dans la maquette.
            root.fB += (root.bass - root.fB) * 0.18
            root.fM += (root.mid  - root.fM) * 0.10
            root.fH += (root.high - root.fH) * 0.25
            sphereMat.uBass = root.fB
            sphereMat.uMid  = root.fM
            sphereMat.uHigh = root.fH

            root.cloudScale = 1.0 + root.fB * 0.12

            // 114.59 = (60fps/30fps) * 180/π : conserve la vitesse angulaire de la
            // maquette (incréments par frame@60) sur notre tick@30, en degrés.
            root.rotY = (root.rotY + (0.0016 + root.fM * 0.006) * 114.59) % 360
            root.rotX = (root.rotX + 0.0004 * 114.59) % 360
        }
    }
}
