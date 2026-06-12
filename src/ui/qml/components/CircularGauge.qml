import QtQuick
import QtQuick.Shapes
import QtQuick.Effects
import PerformanceOverlay

// Jauge radiale : piste complète discrète + arc de progression coloré.
// Maquette : start 135°, sweep 270° (ouverture en bas). La valeur affichée glisse
// (Behavior) au lieu de sauter. Contenu central via le slot `center`.
// `glow: true` ajoute un halo coloré (copie floutée de l'arc, ne se redessine
// qu'au changement de valeur).
Item {
    id: g

    property real value: 0.0                 // cible 0..1
    property color arcColor: Theme.accent
    property real lineWidth: 10
    property real startAngle: 135
    property real sweepRange: 270
    property bool glow: false

    default property alias center: centerSlot.data

    property real shown: value
    Behavior on shown { NumberAnimation { duration: 550; easing.type: Easing.OutCubic } }

    readonly property real _cx: width / 2
    readonly property real _cy: height / 2
    readonly property real _r: Math.min(width, height) / 2 - lineWidth / 2
    readonly property real _sx: _cx + _r * Math.cos(startAngle * Math.PI / 180)
    readonly property real _sy: _cy + _r * Math.sin(startAngle * Math.PI / 180)
    readonly property real _clamped: Math.max(0, Math.min(1, shown))

    // ---- Source du halo (copie de l'arc), floutée derrière ----
    Shape {
        id: glowSrc
        anchors.fill: parent
        visible: false
        layer.enabled: g.glow
        preferredRendererType: Shape.CurveRenderer
        ShapePath {
            strokeColor: g.arcColor; strokeWidth: g.lineWidth; fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: g._sx; startY: g._sy
            PathAngleArc {
                centerX: g._cx; centerY: g._cy; radiusX: g._r; radiusY: g._r
                startAngle: g.startAngle; sweepAngle: g.sweepRange * g._clamped
            }
        }
    }
    MultiEffect {
        source: glowSrc
        anchors.fill: glowSrc
        visible: g.glow
        z: -1
        blurEnabled: true
        blur: 1.0
        blurMax: 28
        opacity: 0.8
        autoPaddingEnabled: true
    }

    // ---- Piste + arc net ----
    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: Qt.rgba(1, 1, 1, 0.07)
            strokeWidth: g.lineWidth
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: g._sx; startY: g._sy
            PathAngleArc {
                centerX: g._cx; centerY: g._cy; radiusX: g._r; radiusY: g._r
                startAngle: g.startAngle; sweepAngle: g.sweepRange
            }
        }
        ShapePath {
            strokeColor: g.arcColor
            strokeWidth: g.lineWidth
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            startX: g._sx; startY: g._sy
            PathAngleArc {
                centerX: g._cx; centerY: g._cy; radiusX: g._r; radiusY: g._r
                startAngle: g.startAngle; sweepAngle: g.sweepRange * g._clamped
            }
        }
    }

    Item {
        id: centerSlot
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
    }
}
