/*
 * constants.h
 * Constantes de configuration centralisées
 * Toutes les valeurs configurables de l'application sont ici
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <windows.h>

// ===== DIMENSIONS FENÊTRE =====
// (dimensionnées pour des polices +2px, cf. section Polices)
#define WINDOW_WIDTH 372
#define WINDOW_WIDTH_COMPACT 232   // Largeur réduite en mode compact
#define WINDOW_HEIGHT_FULL 338     // Augmenté (polices +2px + ligne ClaudeUsage)
#define WINDOW_HEIGHT_MIN 242      // Augmenté pour les plugins essentiels
#define WINDOW_HEIGHT_COMPACT 102  // Mode compact (CPU + RAM + DateTime + Prière + Claude)
#define WINDOW_HEIGHT_TASKS 320    // Hauteur pour la page Task Killer
#define WINDOW_HEIGHT_SETTINGS 350 // Hauteur pour la page Settings

// ===== PAGES / ONGLETS =====
#define PAGE_PERF 0
#define PAGE_TASKS 1
#define PAGE_SETTINGS 2
#define PAGE_COUNT 3

// Dimensions onglets
#define TAB_HEIGHT 20
#define TAB_WIDTH 88
#define TAB_MARGIN 4

// ===== TIMERS ET PERFORMANCE =====
#define TIMER_ID 1
#define TIMER_INTERVAL 2000  // Millisecondes (2 secondes)

// ===== PALETTE COULEURS NEON =====
// Fond et panneaux
#define COLOR_BG RGB(8, 10, 18)
#define COLOR_PANEL RGB(16, 20, 30)
#define COLOR_BORDER RGB(40, 45, 60)

// Couleurs d'accent
#define COLOR_CYAN RGB(0, 230, 255)
#define COLOR_MAGENTA RGB(255, 64, 180)

// Texte
#define COLOR_TEXT_PRIMARY RGB(200, 210, 230)
#define COLOR_TEXT_MUTED RGB(120, 130, 150)

// ===== TRANSPARENCE =====
#define WINDOW_OPACITY 230  // 0-255 (230 = ~90% opaque)

// ===== LIMITES =====
#define MAX_DISKS 8  // Nombre maximum de disques à surveiller

// ===== INTERFACE =====
#define WINDOW_CLASS_NAME "PerformanceOverlayClass"

// Polices (taille +2px par rapport à l'origine 16/12/11)
#define FONT_TITLE_NAME "Consolas"
#define FONT_TITLE_SIZE 18
#define FONT_NORMAL_NAME "Consolas"
#define FONT_NORMAL_SIZE 14
#define FONT_SMALL_NAME "Consolas"
#define FONT_SMALL_SIZE 13

// Largeurs des barres de progression
#define PROGRESS_BAR_WIDTH 22

// ===== FICHIERS =====
#define CONFIG_FILE "config.txt"
#define CONFIG_FILE_INI "config.ini"

// ===== CLAUDE USAGE =====
#define CLAUDE_BASELINE_DAYS 28   // Fenêtre de moyenne glissante (jours)
#define CLAUDE_RESCAN_MS 60000    // Intervalle de re-scan des logs (1 min)

// ===== TASK KILLER =====
#define MAX_PROCESSES 50      // Nombre max de processus affichés
#define PROCESS_LINE_HEIGHT 20

// Couleurs spécifiques Task Killer
#define COLOR_PROCESS_ACTIVE RGB(0, 255, 128)   // Vert pour processus actif
#define COLOR_PROCESS_INACTIVE RGB(100, 100, 100)  // Gris pour inactif
#define COLOR_KILL_BUTTON RGB(255, 80, 80)      // Rouge pour bouton Kill

#endif // CONSTANTS_H
