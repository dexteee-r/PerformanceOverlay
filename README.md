# Performance Overlay

Overlay de performances système pour Windows 11 — un **cockpit** temps réel
(CPU, GPU, RAM, disques, réseau, volume, processus, prière, conso Claude Code),
plus un mode **compact**, un **gestionnaire de tâches** et des **réglages**.

> Réécriture complète en **Qt 6 / QML + C++**. L'ancienne version (C + Win32/GDI)
> reste consultable dans l'historique git (commits ≤ `14f8bbd`). Une référence
> visuelle de l'overlay compact d'origine est conservée dans
> `notes_dev/legacy-compact-reference/` (hors dépôt).

## Stack
- **UI** : Qt 6 Quick / QML (scene graph, `QtQuick.Shapes`, `QtQuick.Effects`)
- **Backend** : C++ (providers de métriques `QObject` + `Q_PROPERTY`/`NOTIFY`)
- **Build** : CMake + Ninja

## Architecture
```
src/
├── app/                     point d'entrée (QApplication + moteur QML)
├── core/
│   ├── metrics/             11 providers (cpu, ram, gpu, disk, …) + MetricsService
│   ├── config/              ConfigManager (config.ini via QSettings)
│   ├── services/            tray, démarrage auto, task killer
│   └── platform/windows/    click-through Win32 (OverlayController)
└── ui/qml/
    ├── Main.qml AppShell.qml (routeur d'écrans)
    ├── screens/             Cockpit · Compact · Tasks · Settings
    ├── components/          Panel, CircularGauge, StatBox, SegmentBar, Sparkline, CenterVisual…
    └── theme/               Theme.qml (design tokens)
```

## Construire (MinGW, Qt 6.11)
```powershell
$cmake = "C:\Qt\Tools\CMake_64\bin\cmake.exe"
& $cmake --preset mingw -S .
& $cmake --build build/mingw
```
Lancer (DLL Qt + runtime MinGW sur le PATH) :
```powershell
$env:PATH = "C:\Qt\6.11.1\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;$env:PATH"
& build\mingw\appPerformanceOverlay.exe
```

## Raccourcis
- **F1 / F2 / F4 / F10** : Cockpit · Compact · Tâches · Réglages
- **Ctrl+Alt+O** (global) : bascule interactif ↔ passif (click-through)
- **Échap** : quitter

## Configuration
`config.ini` (à côté de l'exécutable, généré au 1er lancement) : intervalle de
mesure, position de la fenêtre, paramètres de prière (ville / pays / méthode).
