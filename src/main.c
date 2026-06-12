/*
 * main.c
 * Programme principal - Overlay de performance systeme
 * Version 3.1 - Code refactorise
 */

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../include/performance.h"
#include "../include/config_parser.h"
#include "../include/startup.h"
#include "../include/constants.h"
#include "../include/metric_plugin.h"
#include "../include/all_plugins.h"
#include "../include/taskkiller.h"
#include "../include/theme.h"
#include "../include/tray.h"
#include "../include/ui_draw.h"

// Variables globales
HWND g_hwnd = NULL;
ConfigINI g_config;
BOOL g_isDragging = FALSE;
POINT g_dragOffset;
char g_bannerTop[64] = "==[ SYSTEM OVERLAY v3 ]==";
char g_bannerBottom[64] = ":: PLUGIN SYSTEM ::";

// Systeme de pages
int g_currentPage = PAGE_PERF;
const char* g_pageNames[] = {"Perf", "Tasks", "Settings"};

// Animation fade-in
#define TIMER_ANIM 2
#define TIMER_ANIM_INTERVAL 30  // ~33 FPS
static BYTE g_currentAlpha = 0;
static BYTE g_targetAlpha = WINDOW_OPACITY;
static BOOL g_animating = FALSE;

// Animation hover et pulse
int g_hoverTabIndex = -1;      // Onglet survolé (-1 = aucun)
int g_hoverThemeIndex = -1;    // Thème survolé (-1 = aucun)
static float g_pulsePhase = 0; // Phase du pulse (0 à 2*PI)
BOOL g_alertPulse = FALSE;     // TRUE si CPU ou RAM en alerte

// Polices GDI (créées une fois au démarrage, réutilisées à chaque WM_PAINT)
static HFONT g_hFontTitle  = NULL;
static HFONT g_hFontNormal = NULL;
static HFONT g_hFontSmall  = NULL;

static int GetWindowHeightForMode(BOOL minimal) {
    if (g_currentPage == PAGE_TASKS) {
        return WINDOW_HEIGHT_TASKS;
    }
    if (g_currentPage == PAGE_SETTINGS) {
        return WINDOW_HEIGHT_SETTINGS;
    }
    return minimal ? WINDOW_HEIGHT_COMPACT : WINDOW_HEIGHT_FULL;
}

static int GetWindowWidthForMode(BOOL minimal) {
    if (g_currentPage != PAGE_PERF) {
        return WINDOW_WIDTH;
    }
    return minimal ? WINDOW_WIDTH_COMPACT : WINDOW_WIDTH;
}

/*
 * InitBanner
 * Prepare les lignes d'entete
 */
static void InitBanner(void) {
    char user[64] = {0};
    DWORD sz = (DWORD)sizeof(user);
    if (GetUserNameA(user, &sz) && sz > 1) {
        snprintf(g_bannerTop, sizeof(g_bannerTop), "==[ %s OVERLAY ]==", "SYSTEM");
        snprintf(g_bannerBottom, sizeof(g_bannerBottom), ":: %s @ overlay ::", user);
    } else {
        snprintf(g_bannerTop, sizeof(g_bannerTop), "==[ SYSTEM OVERLAY ]==");
        snprintf(g_bannerBottom, sizeof(g_bannerBottom), ":: PERF OVERLAY v3 ::");
    }
}

/*
 * UpdateDisplay
 * Met a jour l'affichage
 */
void UpdateDisplay(void) {
    InvalidateRect(g_hwnd, NULL, TRUE);
}

/*
 * WindowProc
 * Fonction de traitement des messages de la fenetre
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            SetTimer(hwnd, TIMER_ANIM, TIMER_ANIM_INTERVAL, NULL);
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            CreateTrayIcon(hwnd, hInst);
            // Créer les polices une seule fois
            g_hFontTitle = CreateFont(FONT_TITLE_SIZE, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, FONT_TITLE_NAME);
            g_hFontNormal = CreateFont(FONT_NORMAL_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, FONT_NORMAL_NAME);
            g_hFontSmall = CreateFont(FONT_SMALL_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, FONT_SMALL_NAME);
            // Démarrer avec fade-in
            g_currentAlpha = 0;
            g_targetAlpha = WINDOW_OPACITY;
            g_animating = TRUE;
            SetLayeredWindowAttributes(hwnd, 0, g_currentAlpha, LWA_ALPHA);
            return 0;
        }

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                ShowTrayMenu(hwnd);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
                if (IsWindowVisible(hwnd)) {
                    SetForegroundWindow(hwnd);
                }
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_SHOW:
                    ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
                    if (IsWindowVisible(hwnd)) {
                        SetForegroundWindow(hwnd);
                    }
                    break;
                case ID_TRAY_SETTINGS:
                    g_currentPage = PAGE_SETTINGS;
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0,
                                GetWindowWidthForMode(g_config.minimal_mode),
                                GetWindowHeightForMode(g_config.minimal_mode),
                                SWP_NOMOVE | SWP_NOZORDER);
                    ShowWindow(hwnd, SW_SHOW);
                    SetForegroundWindow(hwnd);
                    UpdateDisplay();
                    break;
                case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;

        case WM_TIMER:
            if (wParam == TIMER_ID) {
                // Timer principal: mise à jour des métriques
                UpdateAllPlugins();
                UpdateDisplay();
            } else if (wParam == TIMER_ANIM) {
                // Timer animation: fade
                if (g_animating) {
                    if (g_currentAlpha < g_targetAlpha) {
                        g_currentAlpha += 15;
                        if (g_currentAlpha >= g_targetAlpha) {
                            g_currentAlpha = g_targetAlpha;
                            g_animating = FALSE;
                        }
                    } else if (g_currentAlpha > g_targetAlpha) {
                        if (g_currentAlpha > 15) g_currentAlpha -= 15;
                        else g_currentAlpha = 0;
                        if (g_currentAlpha <= g_targetAlpha) {
                            g_currentAlpha = g_targetAlpha;
                            g_animating = FALSE;
                        }
                    }
                    SetLayeredWindowAttributes(hwnd, 0, g_currentAlpha, LWA_ALPHA);
                }
                // Pulse animation pour alertes
                if (g_config.animations_enabled) {
                    g_pulsePhase += 0.15f;
                    if (g_pulsePhase > 6.28318f) g_pulsePhase -= 6.28318f;
                    // Vérifier si CPU/RAM en alerte
                    MetricData* cpu = GetMetricByName("CPU");
                    MetricData* ram = GetMetricByName("RAM");
                    float cpuPct = 0, ramPct = 0;
                    if (cpu && cpu->enabled) sscanf(cpu->display_lines[0], "CPU %f", &cpuPct);
                    if (ram && ram->enabled) {
                        float used = 0, total = 0;
                        sscanf(ram->display_lines[0], "RAM %f/%f", &used, &total);
                        if (total > 0) ramPct = (used / total) * 100.0f;
                    }
                    BOOL wasAlert = g_alertPulse;
                    g_alertPulse = (cpuPct >= g_config.alert_cpu_threshold) || (ramPct >= g_config.alert_ram_threshold);
                    if (g_alertPulse || wasAlert) UpdateDisplay();
                }
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right;
            int height = clientRect.bottom;

            // Fond
            HBRUSH bgBrush = CreateSolidBrush(g_colorBg);
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            // Polices (réutilisées depuis les globaux créés dans WM_CREATE)
            HFONT hFontNormal = g_hFontNormal;
            HFONT hFontSmall  = g_hFontSmall;

            SetBkMode(hdc, TRANSPARENT);

            // Mode compact: interface minimaliste
            if (g_config.minimal_mode && g_currentPage == PAGE_PERF) {
                // Bordure simple
                HPEN borderPen = CreatePen(PS_SOLID, 2, g_colorAccent);
                SelectObject(hdc, borderPen);
                MoveToEx(hdc, 0, 0, NULL);
                LineTo(hdc, width, 0);
                LineTo(hdc, width, height);
                LineTo(hdc, 0, height);
                LineTo(hdc, 0, 0);
                DeleteObject(borderPen);

                DrawCompactMode(hdc, width, height, hFontNormal);

                EndPaint(hwnd, &ps);
                return 0;
            }

            // Mode normal: panneau complet
            RECT panelRect = {6, 8, width - 6, height - 6};

            // Panneau
            HBRUSH panelBrush = CreateSolidBrush(g_colorPanel);
            FillRect(hdc, &panelRect, panelBrush);
            DeleteObject(panelBrush);

            // Bordure avec pulse si alerte
            COLORREF borderColor = g_colorBorder;
            int borderWidth = 1;
            if (g_alertPulse && g_config.animations_enabled) {
                float pulse = (float)(sin(g_pulsePhase) * 0.5 + 0.5);
                int r = (int)(255 * pulse + GetRValue(g_colorBorder) * (1 - pulse));
                int g = (int)(50 * pulse + GetGValue(g_colorBorder) * (1 - pulse));
                int b = (int)(50 * pulse + GetBValue(g_colorBorder) * (1 - pulse));
                borderColor = RGB(r, g, b);
                borderWidth = 2;
            }

            HPEN borderPen = CreatePen(PS_SOLID, borderWidth, borderColor);
            SelectObject(hdc, borderPen);
            MoveToEx(hdc, panelRect.left, panelRect.top, NULL);
            LineTo(hdc, panelRect.right, panelRect.top);
            LineTo(hdc, panelRect.right, panelRect.bottom);
            LineTo(hdc, panelRect.left, panelRect.bottom);
            LineTo(hdc, panelRect.left, panelRect.top);
            DeleteObject(borderPen);

            // Accent haut avec pulse si alerte
            COLORREF accentColor = g_colorAccent2;
            if (g_alertPulse && g_config.animations_enabled) {
                float pulse = (float)(sin(g_pulsePhase) * 0.5 + 0.5);
                int r = (int)(255 * pulse + GetRValue(g_colorAccent2) * (1 - pulse));
                int g = (int)(100 * pulse + GetGValue(g_colorAccent2) * (1 - pulse));
                int b = (int)(100 * pulse + GetBValue(g_colorAccent2) * (1 - pulse));
                accentColor = RGB(r, g, b);
            }
            HBRUSH accent = CreateSolidBrush(accentColor);
            RECT accentRect = {panelRect.left, panelRect.top, panelRect.right, panelRect.top + 4};
            FillRect(hdc, &accentRect, accent);
            DeleteObject(accent);

            // Bouton X
            RECT closeRect = {width - 28, 10, width - 6, 28};
            HBRUSH closeBrush = CreateSolidBrush(g_colorAccent2);
            FillRect(hdc, &closeRect, closeBrush);
            DeleteObject(closeBrush);

            // Onglets
            DrawTabs(hdc, width, hFontSmall);

            // Bouton X texte
            SetTextColor(hdc, RGB(0, 0, 0));
            SelectObject(hdc, hFontSmall);
            TextOut(hdc, closeRect.left + 5, closeRect.top + 2, "X", 1);

            // Contenu selon la page
            if (g_currentPage == PAGE_TASKS) {
                DrawTaskKillerPage(hdc, width, height, hFontNormal, hFontSmall);
            } else if (g_currentPage == PAGE_SETTINGS) {
                DrawSettingsPage(hdc, width, height, hFontNormal, hFontSmall);
            } else {
                // Page Performance normale
                int y = 53;

                // Uptime
                MetricData* uptime = GetMetricByName("Uptime");
                if (uptime && uptime->enabled && g_config.show_uptime) {
                    SetTextColor(hdc, uptime->color);
                    TextOut(hdc, 16, y, uptime->display_lines[0], (int)strlen(uptime->display_lines[0]));
                    y += 14;
                }

                // Prayer
                MetricData* prayer = GetMetricByName("Prayer");
                if (prayer && prayer->enabled && g_config.prayer_enabled) {
                    SetTextColor(hdc, prayer->color);
                    TextOut(hdc, 16, y, prayer->display_lines[0], (int)strlen(prayer->display_lines[0]));
                    y += 16;
                }

                // Separation
                HPEN sepPen = CreatePen(PS_SOLID, 1, g_colorAccent2);
                SelectObject(hdc, sepPen);
                MoveToEx(hdc, 14, y + 6, NULL);
                LineTo(hdc, width - 14, y + 6);
                DeleteObject(sepPen);
                y += 16;

                // Metriques
                const char* pluginOrder[] = {"CPU", "RAM", "GPU", "Disk", "Process", "Volume", "DateTime", "ClaudeUsage"};
                int pluginCount = 8;

                for (int i = 0; i < pluginCount; i++) {
                    MetricData* metric = GetMetricByName(pluginOrder[i]);
                    if (metric && metric->enabled) {
                        if (!g_config.show_processes && strcmp(pluginOrder[i], "Process") == 0) continue;

                        SetTextColor(hdc, metric->color);
                        for (int line = 0; line < metric->line_count; line++) {
                            TextOut(hdc, 16, y, metric->display_lines[line],
                                    (int)strlen(metric->display_lines[line]));
                            y += 20;
                        }
                    }
                }

                // Footer
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, g_colorTextMuted);
                TextOut(hdc, 16, height - 29, "F2 compact | F4 tasks", 21);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right;
            int height = clientRect.bottom;

            // Bouton X
            if (pt.x >= width - 28 && pt.x <= width - 6 && pt.y >= 10 && pt.y <= 28) {
                PostQuitMessage(0);
                return 0;
            }

            // Onglets
            int tabX = 16;
            int tabY = 12;
            for (int i = 0; i < PAGE_COUNT; i++) {
                if (pt.x >= tabX && pt.x <= tabX + TAB_WIDTH &&
                    pt.y >= tabY && pt.y <= tabY + TAB_HEIGHT) {
                    if (g_currentPage != i) {
                        g_currentPage = i;
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0,
                                    GetWindowWidthForMode(g_config.minimal_mode),
                                    GetWindowHeightForMode(g_config.minimal_mode),
                                    SWP_NOMOVE | SWP_NOZORDER);
                        if (g_currentPage == PAGE_TASKS) {
                            RefreshCurrentView(GetTaskKillerData());
                        }
                        UpdateDisplay();
                    }
                    return 0;
                }
                tabX += TAB_WIDTH + TAB_MARGIN;
            }

            // Page Task Killer
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                const int TOGGLE_Y = 50;
                const int TOGGLE_HEIGHT = 18;
                const int SEARCH_Y = 72;
                const int SEARCH_HEIGHT = 20;
                const int SCROLLBAR_WIDTH = 8;
                const int SCROLLBAR_X = width - 12;
                const int LIST_START_Y = 118;
                const int maxVisible = 6;
                const int LIST_HEIGHT = maxVisible * PROCESS_LINE_HEIGHT;
                int filteredCount = GetFilteredProcessCount(data);

                // Toggle All
                if (pt.x >= 16 && pt.x <= 56 && pt.y >= TOGGLE_Y && pt.y <= TOGGLE_Y + TOGGLE_HEIGHT) {
                    if (data->view_mode != VIEW_MODE_ALL) {
                        data->view_mode = VIEW_MODE_ALL;
                        data->scroll_offset = 0;
                        RefreshCurrentView(data);
                        UpdateDisplay();
                    }
                    return 0;
                }

                // Toggle Ports
                if (pt.x >= 58 && pt.x <= 108 && pt.y >= TOGGLE_Y && pt.y <= TOGGLE_Y + TOGGLE_HEIGHT) {
                    if (data->view_mode != VIEW_MODE_PORTS) {
                        data->view_mode = VIEW_MODE_PORTS;
                        data->scroll_offset = 0;
                        RefreshCurrentView(data);
                        UpdateDisplay();
                    }
                    return 0;
                }

                // Champ recherche
                if (pt.x >= 16 && pt.x <= width - 16 && pt.y >= SEARCH_Y && pt.y <= SEARCH_Y + SEARCH_HEIGHT) {
                    if (pt.x >= width - 35 && data->filter_text[0] != '\0') {
                        data->filter_text[0] = '\0';
                        data->scroll_offset = 0;
                    }
                    data->filter_active = TRUE;
                    UpdateDisplay();
                    return 0;
                } else if (data->filter_active) {
                    data->filter_active = FALSE;
                    UpdateDisplay();
                }

                // Bouton Refresh
                if (pt.x >= 16 && pt.x <= 100 && pt.y >= height - 40 && pt.y <= height - 16) {
                    RefreshCurrentView(data);
                    data->scroll_offset = 0;
                    UpdateDisplay();
                    return 0;
                }

                // Scrollbar
                if (filteredCount > maxVisible &&
                    pt.x >= SCROLLBAR_X - SCROLLBAR_WIDTH && pt.x <= SCROLLBAR_X + SCROLLBAR_WIDTH &&
                    pt.y >= LIST_START_Y && pt.y <= LIST_START_Y + LIST_HEIGHT) {
                    int clickPos = pt.y - LIST_START_Y;
                    int maxScroll = filteredCount - maxVisible;
                    int newOffset = (clickPos * maxScroll) / LIST_HEIGHT;
                    if (newOffset < 0) newOffset = 0;
                    if (newOffset > maxScroll) newOffset = maxScroll;
                    data->scroll_offset = newOffset;
                    UpdateDisplay();
                    return 0;
                }

                // Boutons Kill
                int y = LIST_START_Y;
                int startIdx = data->scroll_offset;
                int endIdx = startIdx + maxVisible;
                if (endIdx > filteredCount) endIdx = filteredCount;

                for (int visibleIdx = startIdx; visibleIdx < endIdx; visibleIdx++) {
                    if (pt.x >= width - 70 && pt.x <= width - 26 && pt.y >= y - 2 && pt.y <= y + 16) {
                        int realIdx = GetRealIndexFromVisibleIndex(data, visibleIdx);
                        if (realIdx >= 0 && KillProcessByIndex(data, realIdx)) {
                            UpdateDisplay();
                        }
                        return 0;
                    }
                    y += PROCESS_LINE_HEIGHT;
                }
            }

            // Page Settings
            if (g_currentPage == PAGE_SETTINGS) {
                int y = 80;
                for (int i = 0; i < g_themeCount; i++) {
                    if (pt.x >= 16 && pt.x <= width - 16 && pt.y >= y && pt.y <= y + 28) {
                        if (i == 5) {
                            // Custom theme: ouvrir config.ini pour editer les couleurs
                            char configPath[MAX_PATH];
                            GetModuleFileNameA(NULL, configPath, MAX_PATH);
                            char* lastSlash = strrchr(configPath, '\\');
                            if (lastSlash) {
                                strcpy(lastSlash + 1, CONFIG_FILE_INI);
                            }
                            ShellExecuteA(NULL, "open", configPath, NULL, NULL, SW_SHOWNORMAL);
                        }
                        ApplyTheme(i);
                        g_config.theme_index = i;
                        SaveConfigINI(&g_config, CONFIG_FILE_INI);
                        UpdateDisplay();
                        return 0;
                    }
                    y += 32;
                }
            }

            // Drag fenetre
            g_isDragging = TRUE;
            g_dragOffset.x = pt.x;
            g_dragOffset.y = pt.y;
            SetCapture(hwnd);
            return 0;
        }

        case WM_LBUTTONUP:
            if (g_isDragging) {
                g_isDragging = FALSE;
                ReleaseCapture();
                RECT rect;
                GetWindowRect(hwnd, &rect);
                g_config.x = rect.left;
                g_config.y = rect.top;
                SaveConfigINI(&g_config, CONFIG_FILE_INI);
            }
            return 0;

        case WM_MOUSEMOVE: {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            if (g_isDragging) {
                GetCursorPos(&pt);
                SetWindowPos(hwnd, HWND_TOPMOST, pt.x - g_dragOffset.x, pt.y - g_dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            } else if (g_config.animations_enabled) {
                // Hover tracking pour les onglets
                int oldHoverTab = g_hoverTabIndex;
                g_hoverTabIndex = -1;
                int tabX = 16;
                for (int i = 0; i < PAGE_COUNT; i++) {
                    if (pt.x >= tabX && pt.x <= tabX + TAB_WIDTH &&
                        pt.y >= 12 && pt.y <= 12 + TAB_HEIGHT) {
                        g_hoverTabIndex = i;
                        break;
                    }
                    tabX += TAB_WIDTH + TAB_MARGIN;
                }
                // Hover tracking pour les thèmes (page Settings)
                int oldHoverTheme = g_hoverThemeIndex;
                g_hoverThemeIndex = -1;
                if (g_currentPage == PAGE_SETTINGS) {
                    int y = 80;
                    for (int i = 0; i < g_themeCount; i++) {
                        if (pt.x >= 16 && pt.x <= WINDOW_WIDTH - 16 &&
                            pt.y >= y && pt.y <= y + 28) {
                            g_hoverThemeIndex = i;
                            break;
                        }
                        y += 32;
                    }
                }
                // Redraw si hover a changé
                if (oldHoverTab != g_hoverTabIndex || oldHoverTheme != g_hoverThemeIndex) {
                    UpdateDisplay();
                }
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                const int maxVisible = 6;
                int filteredCount = GetFilteredProcessCount(data);

                if (filteredCount > maxVisible) {
                    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                    data->scroll_offset -= delta / WHEEL_DELTA;
                    int maxScroll = filteredCount - maxVisible;
                    if (data->scroll_offset < 0) data->scroll_offset = 0;
                    if (data->scroll_offset > maxScroll) data->scroll_offset = maxScroll;
                    UpdateDisplay();
                }
            }
            return 0;
        }

        case WM_CHAR: {
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                if (data->filter_active) {
                    char c = (char)wParam;
                    int len = (int)strlen(data->filter_text);

                    if (c == '\b' && len > 0) {
                        data->filter_text[len - 1] = '\0';
                        data->scroll_offset = 0;
                        UpdateDisplay();
                    } else if (c == 27) {
                        data->filter_active = FALSE;
                        UpdateDisplay();
                    } else if (c >= 32 && c < 127 && len < 30) {
                        data->filter_text[len] = c;
                        data->filter_text[len + 1] = '\0';
                        data->scroll_offset = 0;
                        UpdateDisplay();
                    }
                    return 0;
                }
            }
            break;
        }

        case WM_KEYDOWN:
            if (wParam == g_config.toggle_minimal_key && g_currentPage == PAGE_PERF) {
                g_config.minimal_mode = !g_config.minimal_mode;
                SaveConfigINI(&g_config, CONFIG_FILE_INI);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0,
                            GetWindowWidthForMode(g_config.minimal_mode),
                            GetWindowHeightForMode(g_config.minimal_mode), SWP_NOMOVE | SWP_NOZORDER);
                UpdateDisplay();
            } else if (wParam == VK_F4) {
                g_currentPage = (g_currentPage == PAGE_PERF) ? PAGE_TASKS : PAGE_PERF;
                if (g_currentPage == PAGE_TASKS) {
                    RefreshProcessList(GetTaskKillerData());
                }
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0,
                            GetWindowWidthForMode(g_config.minimal_mode),
                            GetWindowHeightForMode(g_config.minimal_mode), SWP_NOMOVE | SWP_NOZORDER);
                UpdateDisplay();
            } else if (wParam == g_config.reload_config_key) {
                LoadConfigINI(&g_config, CONFIG_FILE_INI);
                SetCustomTheme(g_config.custom_bg, g_config.custom_panel, g_config.custom_border,
                               g_config.custom_accent, g_config.custom_accent2,
                               g_config.custom_text, g_config.custom_text_muted);
                ApplyTheme(g_config.theme_index);
                UpdateDisplay();
            }
            return 0;

        case WM_DESTROY:
            RemoveTrayIcon();
            KillTimer(hwnd, TIMER_ID);
            KillTimer(hwnd, TIMER_ANIM);
            CleanupTaskKiller();
            CleanupPluginSystem();
            CleanupPerformanceMonitoring();
            if (g_hFontTitle)  { DeleteObject(g_hFontTitle);  g_hFontTitle  = NULL; }
            if (g_hFontNormal) { DeleteObject(g_hFontNormal); g_hFontNormal = NULL; }
            if (g_hFontSmall)  { DeleteObject(g_hFontSmall);  g_hFontSmall  = NULL; }
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
 * CreateOverlayWindow
 */
HWND CreateOverlayWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = g_config.x;
    int posY = g_config.y;

    if (posX < 0 || posX > screenWidth - 100 || posY < 0 || posY > screenHeight - 100) {
        posX = (screenWidth - WINDOW_WIDTH) / 2;
        posY = (screenHeight - GetWindowHeightForMode(g_config.minimal_mode)) / 2;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        WINDOW_CLASS_NAME,
        "Performance Overlay",
        WS_POPUP,
        posX, posY,
        WINDOW_WIDTH,
        GetWindowHeightForMode(g_config.minimal_mode),
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, WINDOW_OPACITY, LWA_ALPHA);
    return hwnd;
}

/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    InitPerformanceMonitoring();
    LoadConfigINI(&g_config, CONFIG_FILE_INI);

    // Charger le theme custom depuis la config
    SetCustomTheme(g_config.custom_bg, g_config.custom_panel, g_config.custom_border,
                   g_config.custom_accent, g_config.custom_accent2,
                   g_config.custom_text, g_config.custom_text_muted);
    ApplyTheme(g_config.theme_index);

    // Plugins
    InitPluginSystem();
    RegisterMetricPlugin(&CPUPlugin);
    RegisterMetricPlugin(&RAMPlugin);
    RegisterMetricPlugin(&DiskPlugin);
    RegisterMetricPlugin(&UptimePlugin);
    RegisterMetricPlugin(&ProcessPlugin);
    RegisterMetricPlugin(&NetworkPlugin);
    RegisterMetricPlugin(&DateTimePlugin);
    RegisterMetricPlugin(&VolumePlugin);
    RegisterMetricPlugin(&GPUPlugin);
    RegisterMetricPlugin(&PrayerPlugin);
    RegisterMetricPlugin(&ClaudeUsagePlugin);

    SetPrayerConfig(g_config.prayer_city, g_config.prayer_country,
                    g_config.prayer_method, g_config.prayer_use_api,
                    g_config.prayer_enabled,
                    g_config.prayer_fajr, g_config.prayer_dhuhr,
                    g_config.prayer_asr, g_config.prayer_maghrib,
                    g_config.prayer_isha);

    EnablePlugin("CPU", g_config.cpu_enabled);
    EnablePlugin("RAM", g_config.ram_enabled);
    EnablePlugin("Disk", g_config.disk_enabled);
    EnablePlugin("Uptime", g_config.uptime_enabled);
    EnablePlugin("Process", g_config.process_enabled);
    EnablePlugin("Prayer", g_config.prayer_enabled);
    EnablePlugin("Network", TRUE);
    EnablePlugin("DateTime", TRUE);
    EnablePlugin("Volume", TRUE);
    EnablePlugin("GPU", TRUE);
    EnablePlugin("ClaudeUsage", TRUE);

    InitTaskKiller();
    InitBanner();
    AddToStartup();
    UpdateAllPlugins();

    g_hwnd = CreateOverlayWindow(hInstance);
    if (g_hwnd == NULL) {
        MessageBox(NULL, "Erreur de creation de la fenetre!", "Erreur", MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
