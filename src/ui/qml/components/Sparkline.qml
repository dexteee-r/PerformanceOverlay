import QtQuick
import QtQuick.Shapes
import PerformanceOverlay

// Mini-courbe défilante d'un historique (liste de nombres 0..maxValue).
// Ligne + aire dégradée sous la courbe. Rendu scene-graph (Shape/PathPolyline).
Item {
    id: sp

    property var values: []
    property real maxValue: 100
    property color lineColor: Theme.accent
    property real lineWidth: 1.6
    property bool area: true

    readonly property int _n: values ? values.length : 0
    function _x(i) { return _n > 1 ? i / (_n - 1) * width : 0 }
    function _y(v) { return height - Math.max(0, Math.min(1, v / maxValue)) * height }

    readonly property var _line: {
        let pts = []
        for (let i = 0; i < _n; i++) pts.push(Qt.point(_x(i), _y(values[i])))
        return pts
    }
    readonly property var _fill: {
        let pts = []
        if (_n > 1) {
            pts.push(Qt.point(0, height))
            for (let i = 0; i < _n; i++) pts.push(Qt.point(_x(i), _y(values[i])))
            pts.push(Qt.point(width, height))
        }
        return pts
    }

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            // chemin vide si moins de 2 points → rien dessiné
            strokeColor: "transparent"
            fillGradient: LinearGradient {
                x1: 0; y1: 0; x2: 0; y2: sp.height
                GradientStop { position: 0.0; color: Qt.rgba(sp.lineColor.r, sp.lineColor.g, sp.lineColor.b, 0.30) }
                GradientStop { position: 1.0; color: Qt.rgba(sp.lineColor.r, sp.lineColor.g, sp.lineColor.b, 0.0) }
            }
            PathPolyline { path: sp.area ? sp._fill : [] }
        }
        ShapePath {
            strokeColor: sp.lineColor
            strokeWidth: sp.lineWidth
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            PathPolyline { path: sp._line }
        }
    }
}
