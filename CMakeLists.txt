cmake_minimum_required(VERSION 3.21)

project(PerformanceOverlay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 6.8+ requis (machine : 6.11.1 MinGW). Modules de la couche UI.
find_package(Qt6 6.8 REQUIRED COMPONENTS Quick QuickControls2 Gui Network Widgets Quick3D)

# Active AUTOMOC/AUTORCC + helpers QML modernes.
qt_standard_project_setup(REQUIRES 6.8)

# --- Exécutable (app GUI Windows : pas de console — cf. WIN32_EXECUTABLE plus bas) ---
qt_add_executable(appPerformanceOverlay
    src/app/main.cpp
    # core/metrics — providers de métriques (backend C++)
    src/core/metrics/history.h
    src/core/metrics/metric_provider.h
    src/core/metrics/cpu_provider.h
    src/core/metrics/cpu_provider.cpp
    src/core/metrics/ram_provider.h
    src/core/metrics/ram_provider.cpp
    src/core/metrics/gpu_provider.h
    src/core/metrics/gpu_provider.cpp
    src/core/metrics/disk_provider.h
    src/core/metrics/disk_provider.cpp
    src/core/metrics/uptime_provider.h
    src/core/metrics/uptime_provider.cpp
    src/core/metrics/datetime_provider.h
    src/core/metrics/datetime_provider.cpp
    src/core/metrics/process_provider.h
    src/core/metrics/process_provider.cpp
    src/core/metrics/network_provider.h
    src/core/metrics/network_provider.cpp
    src/core/metrics/volume_provider.h
    src/core/metrics/volume_provider.cpp
    src/core/metrics/prayer_provider.h
    src/core/metrics/prayer_provider.cpp
    src/core/metrics/claude_usage_provider.h
    src/core/metrics/claude_usage_provider.cpp
    src/core/metrics/metrics_service.h
    src/core/metrics/metrics_service.cpp
    # core/platform/windows — services plateforme (overlay/click-through)
    src/core/platform/windows/overlay_controller.h
    src/core/platform/windows/overlay_controller.cpp
    # core/config — lecture config.ini
    src/core/config/config_manager.h
    src/core/config/config_manager.cpp
    # core/services — tray, démarrage auto, task killer
    src/core/services/tray_service.h
    src/core/services/tray_service.cpp
    src/core/services/startup_service.h
    src/core/services/startup_service.cpp
    src/core/services/task_killer_model.h
    src/core/services/task_killer_model.cpp
    # presentation/visual3d — géométrie du nuage de points (noyau Flux de charge)
    src/presentation/visual3d/sphere_point_geometry.h
    src/presentation/visual3d/sphere_point_geometry.cpp
)

# App GUI Windows : aucune fenêtre console ne s'ouvre au lancement.
set_target_properties(appPerformanceOverlay PROPERTIES WIN32_EXECUTABLE TRUE)

# Include paths : `src` pour les chemins longs, et les dossiers contenant des
# types QML_ELEMENT pour que le code généré par qmltyperegistrar (qui inclut les
# en-têtes par nom de base, ex <cpu_provider.h>) les trouve.
target_include_directories(appPerformanceOverlay PRIVATE
    src
    src/core/metrics
    src/core/platform/windows
    src/core/config
    src/core/services
    src/presentation/visual3d
)

# Theme.qml est un singleton : doit être marqué AVANT qt_add_qml_module.
set_source_files_properties(src/ui/qml/theme/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(
    src/ui/qml/Nav.qml
    src/ui/qml/Fmt.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# --- Module QML unique : URI "PerformanceOverlay" ---
# Tous les .qml deviennent des types du même module, accessibles par leur
# nom de base via `import PerformanceOverlay`.
qt_add_qml_module(appPerformanceOverlay
    URI PerformanceOverlay
    VERSION 1.0
    QML_FILES
        src/ui/qml/Main.qml
        src/ui/qml/AppShell.qml
        src/ui/qml/Nav.qml
        src/ui/qml/Fmt.qml
        src/ui/qml/theme/Theme.qml
        src/ui/qml/components/TopBar.qml
        src/ui/qml/components/HudPanel.qml
        src/ui/qml/components/Panel.qml
        src/ui/qml/components/CircularGauge.qml
        src/ui/qml/components/StatBox.qml
        src/ui/qml/components/SegmentBar.qml
        src/ui/qml/components/CenterVisual.qml
        src/ui/qml/components/CenterVisual2D.qml
        src/ui/qml/components/Sphere3D.qml
        src/ui/qml/components/Sparkline.qml
        src/ui/qml/components/AlertVeil.qml
        src/ui/qml/components/NavBar.qml
        src/ui/qml/screens/CockpitScreen.qml
        src/ui/qml/screens/CompactScreen.qml
        src/ui/qml/screens/TasksScreen.qml
        src/ui/qml/screens/SettingsScreen.qml
)

# Police JetBrains Mono (OFL) embarquée — chargée dans main.cpp via QFontDatabase.
qt_add_resources(appPerformanceOverlay "appfonts"
    PREFIX "/fonts"
    BASE "assets/fonts"
    FILES
        assets/fonts/JetBrainsMono-Regular.ttf
        assets/fonts/JetBrainsMono-Medium.ttf
        assets/fonts/JetBrainsMono-Bold.ttf
        assets/fonts/JetBrainsMono-ExtraBold.ttf
        assets/fonts/Satoshi-Regular.ttf
        assets/fonts/Satoshi-Medium.ttf
        assets/fonts/Satoshi-Bold.ttf
        assets/fonts/Satoshi-Black.ttf
)

# Shaders du nuage de points (snippets CustomMaterial Qt Quick 3D), accessibles
# via qrc:/shaders/... depuis Sphere3D.qml.
qt_add_resources(appPerformanceOverlay "sphereshaders"
    PREFIX "/shaders"
    BASE "src/ui/qml/shaders"
    FILES
        src/ui/qml/shaders/sphere.vert
        src/ui/qml/shaders/sphere.frag
)

target_link_libraries(appPerformanceOverlay PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Gui
    Qt6::Quick3D          # noyau Flux de charge : nuage de points 3D
    Qt6::Network          # IP locale + API horaires de prière
    Qt6::Widgets          # QSystemTrayIcon (zone de notification)
)

# Libs Win32 : advapi32 (registre), ole32 (COM volume), psapi (process),
# iphlpapi (débit réseau GetIfTable2).
if(WIN32)
    target_link_libraries(appPerformanceOverlay PRIVATE advapi32 ole32 psapi iphlpapi)
endif()
