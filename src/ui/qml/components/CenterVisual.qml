import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import PerformanceOverlay

// Noyau central « Flux de charge » : arc de charge système lissé + halo coloré +
// anneau de graduations en rotation lente. Réagit à la charge (couleur + halo).
// Équivalent QML léger de la sphère de particules de la maquette (sans Three.js).
Item {
    id: cv

    property real load: 0.0                      // 0..1
    property color coreColor: Theme.statusColor(load)

    property real shown: load
    Behavior on shown { NumberAnimation { duration: 700; easing.type: Easing.OutCubic } }

    readonly property real _d: Math.min(width, height)
    readonly property real _cx: width / 2
    readonly property real _cy: height / 2

    // ---- Coeur (piste + arc de charge) : flatté en texture pour le halo ----
    Item {
        id: coreLayer
        anchors.centerIn: parent
        width: cv._d; height: cv._d
        visible: false
        layer.enabled: true

        property real r: width / 2 - 16
        property real sx: width / 2 + r * Math.cos(135 * Math.PI / 180)
        property real sy: height / 2 + r * Math.sin(135 * Math.PI / 180)

        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            // piste
            ShapePath {
                strokeColor: Qt.rgba(1, 1, 1, 0.06); strokeWidth: 6; fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                startX: coreLayer.sx; startY: coreLayer.sy
                PathAngleArc { centerX: coreLayer.width / 2; centerY: coreLayer.height / 2
                    radiusX: coreLayer.r; radiusY: coreLayer.r; startAngle: 135; sweepAngle: 270 }
            }
            // arc de charge
            ShapePath {
                strokeColor: cv.coreColor; strokeWidth: 6; fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                startX: coreLayer.sx; startY: coreLayer.sy
                PathAngleArc { centerX: coreLayer.width / 2; centerY: coreLayer.height / 2
                    radiusX: coreLayer.r; radiusY: coreLayer.r; startAngle: 135
                    sweepAngle: 270 * Math.max(0, Math.min(1, cv.shown)) }
            }
        }
    }

    // ---- Halo coloré (glow) : ne se redessine qu'au changement de charge ----
    MultiEffect {
        source: coreLayer
        anchors.fill: coreLayer
        shadowEnabled: true
        shadowColor: cv.coreColor
        shadowBlur: 1.0
        shadowScale: 1.0
        autoPaddingEnabled: true
        opacity: 0.4 + 0.5 * cv.shown
    }

    // ---- Onde d'énergie qui se propage vers l'extérieur (réactive à la charge) ----
    Rectangle {
        anchors.centerIn: parent
        width: cv._d * 0.46; height: width; radius: width / 2
        color: "transparent"
        border.width: 2
        border.color: cv.coreColor
        SequentialAnimation on scale {
            loops: Animation.Infinite; running: true
            NumberAnimation { from: 0.7; to: 1.55; duration: 2600; easing.type: Easing.OutQuad }
            PauseAnimation { duration: 200 }
        }
        SequentialAnimation on opacity {
            loops: Animation.Infinite; running: true
            NumberAnimation { from: 0.0; to: 0.55 * (0.4 + cv.shown); duration: 600 }
            NumberAnimation { to: 0.0; duration: 2200 }
        }
    }

    // ---- Anneau de graduations en rotation (décor, net, pas de blur) ----
    Item {
        id: ticks
        anchors.centerIn: parent
        width: cv._d * 0.74; height: cv._d * 0.74
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: Qt.rgba(cv.coreColor.r, cv.coreColor.g, cv.coreColor.b, 0.45)
                strokeWidth: 2; fillColor: "transparent"
                strokeStyle: ShapePath.DashLine
                dashPattern: [0.5, 3.5]
                PathAngleArc { centerX: ticks.width / 2; centerY: ticks.height / 2
                    radiusX: ticks.width / 2; radiusY: ticks.height / 2; startAngle: 0; sweepAngle: 360 }
            }
        }
        RotationAnimator on rotation {
            from: 0; to: 360; duration: 44000; loops: Animation.Infinite; running: true
        }
    }

    // ---- Anneau interne, rotation inverse plus rapide ----
    Item {
        id: ticks2
        anchors.centerIn: parent
        width: cv._d * 0.5; height: cv._d * 0.5
        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer
            ShapePath {
                strokeColor: Qt.rgba(1, 1, 1, 0.10); strokeWidth: 1.5; fillColor: "transparent"
                strokeStyle: ShapePath.DashLine
                dashPattern: [1, 6]
                PathAngleArc { centerX: ticks2.width / 2; centerY: ticks2.height / 2
                    radiusX: ticks2.width / 2; radiusY: ticks2.height / 2; startAngle: 0; sweepAngle: 360 }
            }
        }
        RotationAnimator on rotation {
            from: 360; to: 0; duration: 26000; loops: Animation.Infinite; running: true
        }
    }

    // ---- Lecture centrale ----
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
