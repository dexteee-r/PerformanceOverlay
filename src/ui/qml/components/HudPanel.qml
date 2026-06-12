import QtQuick
import QtQuick.Effects
import PerformanceOverlay

// Surface "verre" sur laquelle tous les widgets reposent.
// Remplissage translucide + filet 1px + ombre douce GPU (MultiEffect).
// `glow` (0..1) monte sur les états d'alerte pour intensifier le halo.
Item {
    id: root
    default property alias content: body.data
    property real glow: 0.0

    // Surface dessinée UNIQUEMENT via le MultiEffect (visible:false + layer)
    // → un seul rendu : la surface nette + son ombre/halo, sans double-dessin.
    Rectangle {
        id: surface
        anchors.fill: parent
        visible: false
        layer.enabled: true
        radius: Theme.radius
        color: Theme.bgPanel
        border.width: 1
        border.color: Theme.stroke
        // dégradé sommet éclairé → vend la lecture "verre surélevé"
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.05) }
            GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.00) }
        }
    }

    MultiEffect {
        source: surface
        anchors.fill: surface
        shadowEnabled: true
        shadowColor: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b,
                             0.22 + 0.45 * root.glow)
        shadowBlur: 0.55 + 0.4 * root.glow
        shadowVerticalOffset: 4
    }

    // Zone de contenu (les enfants déclarés dans HudPanel { ... } atterrissent ici)
    Item {
        id: body
        anchors.fill: parent
        anchors.margins: Theme.pad
    }
}
