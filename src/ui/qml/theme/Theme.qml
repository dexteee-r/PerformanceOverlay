pragma Singleton
import QtQuick

// Source unique de vérité du design system (palette « cockpit » de la maquette
// FEAT - inspi full page). AUCUNE couleur / espacement / rayon codé en dur dans
// les composants : tout passe par Theme.
QtObject {
    // --- Surfaces : presque noir, panneaux à dégradé translucide ---
    readonly property color bgBase:   "#080A12"               // couche la plus profonde
    readonly property color bgBase2:  "#0B0E18"
    readonly property color bgPanel:  Qt.rgba(1, 1, 1, 0.04)  // (compat anciens composants)
    readonly property color bgRaised: Qt.rgba(1, 1, 1, 0.06)
    readonly property color panelTop: Qt.rgba(20 / 255, 25 / 255, 38 / 255, 0.82) // haut panneau
    readonly property color panelBot: Qt.rgba(11 / 255, 14 / 255, 24 / 255, 0.90) // bas panneau
    readonly property color border:   "#282D3C"               // filet 1px
    readonly property color border2:  "#1B2030"               // filet interne discret
    readonly property color stroke:     "#282D3C"             // alias (compat)
    readonly property color strokeSoft: Qt.rgba(1, 1, 1, 0.06)

    // --- Texte : hiérarchie claire ---
    readonly property color textHi:  "#E6ECF7"               // valeurs
    readonly property color text:    "#C8D2E6"               // texte courant
    readonly property color textMid: "#788296"               // labels (muted)
    readonly property color muted:   "#788296"
    readonly property color textLow: "#4A5266"               // unités/légendes (compat)
    readonly property color faint:   "#4A5266"

    // --- Accents (identité projet) + états sémantiques ---
    readonly property color accent:  "#00E6FF"               // cyan primaire
    readonly property color accent2: "#FF40B4"               // magenta secondaire
    readonly property color ok:      "#3DDC84"
    readonly property color warn:    "#FFA62E"
    readonly property color crit:    "#FF3B3B"

    // --- Géométrie : cockpit = coins quasi nets ---
    readonly property real radius:   3
    readonly property real radiusSm: 2
    readonly property real gap:      16   // unité de base ; multiplier, ne pas inventer
    readonly property real pad:      16

    // --- Typographie (mono pour la « logique data ») ---
    readonly property string fontUi:   "JetBrains Mono"
    readonly property string fontMono: "JetBrains Mono"

    // Mappe une charge 0..1 vers une couleur sémantique.
    function statusColor(t) {
        return t > 0.9 ? crit : t > 0.72 ? warn : ok
    }
}
