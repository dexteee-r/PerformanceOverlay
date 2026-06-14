import QtQuick
import QtQuick.Shapes
import PerformanceOverlay

// Courbe d'historique long avec grille horizontale + labels d'axe Y. Multi-séries.
//   series = [ { values: [0..maxValue], color, label, area(bool) }, ... ]
// Rendu scene-graph (Shape / PathPolyline), comme Sparkline mais avec repères.
Item {
    id: lc

    property var series: []
    property real maxValue: 100
    property var gridLines: [25, 50, 75]   // valeurs des filets horizontaux
    property var yLabels: [0, 50, 100]     // graduations affichées à gauche
    property real lineWidth: 1.8
    property int leftPad: 30                // gouttière des labels Y

    readonly property real plotW: Math.max(0, width - leftPad)
    readonly property real plotH: height

    function _x(i, n) { return leftPad + (n > 1 ? i / (n - 1) * plotW : 0) }
    function _y(v) { return plotH - Math.max(0, Math.min(1, v / maxValue)) * plotH }

    // ---- Grille + graduations ----
    Repeater {
        model: lc.gridLines
        Rectangle {
            required property real modelData
            x: lc.leftPad; width: lc.plotW; height: 1
            y: lc._y(modelData); color: Theme.border2
        }
    }
    Repeater {
        model: lc.yLabels
        Text {
            required property real modelData
            x: 0; y: Math.min(lc.plotH - 11, Math.max(0, lc._y(modelData) - 5))
            text: modelData + ""; color: Theme.faint
            font.family: Theme.fontMono; font.pixelSize: 9
        }
    }
    Rectangle { x: lc.leftPad; y: lc.plotH - 1; width: lc.plotW; height: 1; color: Theme.border }

    // ---- Séries ----
    Repeater {
        model: lc.series
        Shape {
            required property var modelData
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer

            readonly property var vals: modelData.values || []
            readonly property int n: vals.length
            readonly property var linePts: {
                let pts = []
                for (let i = 0; i < n; i++) pts.push(Qt.point(lc._x(i, n), lc._y(vals[i])))
                return pts
            }
            readonly property var fillPts: {
                let pts = []
                if (modelData.area && n > 1) {
                    pts.push(Qt.point(lc.leftPad, lc.plotH))
                    for (let i = 0; i < n; i++) pts.push(Qt.point(lc._x(i, n), lc._y(vals[i])))
                    pts.push(Qt.point(lc.leftPad + lc.plotW, lc.plotH))
                }
                return pts
            }

            ShapePath {
                strokeColor: "transparent"
                fillGradient: LinearGradient {
                    x1: 0; y1: 0; x2: 0; y2: lc.plotH
                    GradientStop { position: 0.0; color: Qt.rgba(modelData.color.r, modelData.color.g, modelData.color.b, 0.22) }
                    GradientStop { position: 1.0; color: Qt.rgba(modelData.color.r, modelData.color.g, modelData.color.b, 0.0) }
                }
                PathPolyline { path: fillPts }
            }
            ShapePath {
                strokeColor: modelData.color
                strokeWidth: lc.lineWidth
                fillColor: "transparent"
                capStyle: ShapePath.RoundCap
                joinStyle: ShapePath.RoundJoin
                PathPolyline { path: linePts }
            }
        }
    }
}
