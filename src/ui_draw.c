/*
 * ui_draw.c
 * Implementation des fonctions de dessin de l'interface
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../include/ui_draw.h"
#include "../include/theme.h"
#include "../include/constants.h"
#include "../include/taskkiller.h"
#include "../include/metric_plugin.h"

// Variables d'animation (définies dans main.c)
extern int g_hoverTabIndex;
extern int g_hoverThemeIndex;
extern BOOL g_alertPulse;

/*
 * DrawTabs
 * Dessine les onglets de navigation en haut du widget
 */
void DrawTabs(HDC hdc, int width, HFONT hFontSmall) {
    (void)width;
    int tabX = 16;
    int tabY = 12;

    SelectObject(hdc, hFontSmall);

    for (int i = 0; i < PAGE_COUNT; i++) {
        RECT tabRect = {tabX, tabY, tabX + TAB_WIDTH, tabY + TAB_HEIGHT};

        // Fond de l'onglet avec effet hover
        HBRUSH tabBrush;
        if (i == g_currentPage) {
            tabBrush = CreateSolidBrush(g_colorAccent2);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else if (i == g_hoverTabIndex) {
            // Effet hover: couleur intermédiaire
            tabBrush = CreateSolidBrush(g_colorAccent);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else {
            tabBrush = CreateSolidBrush(g_colorBorder);
            SetTextColor(hdc, g_colorTextMuted);
        }
        FillRect(hdc, &tabRect, tabBrush);
        DeleteObject(tabBrush);

        // Texte de l'onglet
        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, g_pageNames[i], -1, &tabRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        tabX += TAB_WIDTH + TAB_MARGIN;
    }
}

/*
 * DrawSettingsPage
 * Dessine la page des parametres/skins
 */
void DrawSettingsPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall) {
    int y = 50;  // Apres les onglets

    SelectObject(hdc, hFontNormal);
    SetTextColor(hdc, g_colorAccent);
    TextOut(hdc, 16, y, ":: THEMES / SKINS ::", 20);
    y += 30;

    SelectObject(hdc, hFontSmall);

    // Afficher les themes disponibles
    for (int i = 0; i < g_themeCount; i++) {
        RECT themeRect = {16, y, width - 16, y + 28};

        // Fond du bouton theme avec effet hover
        HBRUSH themeBrush;
        BOOL isHovered = (i == g_hoverThemeIndex);
        if (i == g_selectedTheme) {
            themeBrush = CreateSolidBrush(g_themes[i].accent);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else if (isHovered) {
            // Hover: mélange panel + accent
            themeBrush = CreateSolidBrush(g_themes[i].border);
            SetTextColor(hdc, g_themes[i].text);
        } else {
            themeBrush = CreateSolidBrush(g_themes[i].panel);
            SetTextColor(hdc, g_themes[i].text);
        }
        FillRect(hdc, &themeRect, themeBrush);
        DeleteObject(themeBrush);

        // Bordure (plus épaisse si hover)
        HPEN borderPen = CreatePen(PS_SOLID, isHovered ? 2 : 1, g_themes[i].accent);
        SelectObject(hdc, borderPen);
        MoveToEx(hdc, themeRect.left, themeRect.top, NULL);
        LineTo(hdc, themeRect.right, themeRect.top);
        LineTo(hdc, themeRect.right, themeRect.bottom);
        LineTo(hdc, themeRect.left, themeRect.bottom);
        LineTo(hdc, themeRect.left, themeRect.top);
        DeleteObject(borderPen);

        // Apercu couleur
        RECT previewRect = {20, y + 4, 40, y + 24};
        HBRUSH previewBrush = CreateSolidBrush(g_themes[i].bg);
        FillRect(hdc, &previewRect, previewBrush);
        DeleteObject(previewBrush);

        // Nom du theme
        SetBkMode(hdc, TRANSPARENT);
        RECT textRect = {50, y, width - 20, y + 28};
        DrawText(hdc, g_themes[i].name, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Indicateur selectionne
        if (i == g_selectedTheme) {
            SetTextColor(hdc, RGB(0, 0, 0));
            TextOut(hdc, width - 40, y + 6, "[OK]", 4);
        }

        y += 32;
    }

    // Instructions
    y += 10;
    SetTextColor(hdc, g_colorTextMuted);
    TextOut(hdc, 16, y, "Cliquez sur un theme pour l'appliquer", 37);
}

/*
 * DrawTaskKillerPage
 * Dessine le contenu de la page Task Killer
 */
void DrawTaskKillerPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall) {
    TaskKillerData* data = GetTaskKillerData();
    int y = 50;  // Apres les onglets

    SelectObject(hdc, hFontSmall);

    // ===== TOGGLE ALL/PORTS =====
    const char* modeAll = "All";
    const char* modePorts = "Ports";
    int toggleX = 16;

    // Bouton "All"
    RECT allRect = {toggleX, y, toggleX + 40, y + 18};
    HBRUSH allBrush = CreateSolidBrush(data->view_mode == VIEW_MODE_ALL ? COLOR_MAGENTA : COLOR_BORDER);
    FillRect(hdc, &allRect, allBrush);
    DeleteObject(allBrush);
    SetTextColor(hdc, data->view_mode == VIEW_MODE_ALL ? RGB(0, 0, 0) : COLOR_TEXT_MUTED);
    DrawText(hdc, modeAll, -1, &allRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Bouton "Ports"
    RECT portsRect = {toggleX + 42, y, toggleX + 92, y + 18};
    HBRUSH portsBrush = CreateSolidBrush(data->view_mode == VIEW_MODE_PORTS ? COLOR_MAGENTA : COLOR_BORDER);
    FillRect(hdc, &portsRect, portsBrush);
    DeleteObject(portsBrush);
    SetTextColor(hdc, data->view_mode == VIEW_MODE_PORTS ? RGB(0, 0, 0) : COLOR_TEXT_MUTED);
    DrawText(hdc, modePorts, -1, &portsRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    y += 22;

    // ===== CHAMP DE RECHERCHE =====
    RECT searchRect = {16, y, width - 16, y + 20};
    HBRUSH searchBg = CreateSolidBrush(data->filter_active ? RGB(40, 40, 50) : RGB(25, 25, 35));
    FillRect(hdc, &searchRect, searchBg);
    DeleteObject(searchBg);

    // Bordure du champ
    HPEN searchPen = CreatePen(PS_SOLID, 1, data->filter_active ? COLOR_CYAN : COLOR_BORDER);
    SelectObject(hdc, searchPen);
    MoveToEx(hdc, searchRect.left, searchRect.top, NULL);
    LineTo(hdc, searchRect.right, searchRect.top);
    LineTo(hdc, searchRect.right, searchRect.bottom);
    LineTo(hdc, searchRect.left, searchRect.bottom);
    LineTo(hdc, searchRect.left, searchRect.top);
    DeleteObject(searchPen);

    // Icone loupe
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    TextOut(hdc, 20, y + 3, ">", 1);

    // Texte du filtre ou placeholder
    if (data->filter_text[0] != '\0') {
        SetTextColor(hdc, COLOR_TEXT_PRIMARY);
        TextOut(hdc, 32, y + 3, data->filter_text, (int)strlen(data->filter_text));
        if (data->filter_active) {
            int cursorX = 32 + (int)strlen(data->filter_text) * 7;
            SetTextColor(hdc, COLOR_CYAN);
            TextOut(hdc, cursorX, y + 3, "_", 1);
        }
    } else {
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        TextOut(hdc, 32, y + 3, "Filter...", 9);
    }

    // Bouton clear si filtre actif
    if (data->filter_text[0] != '\0') {
        SetTextColor(hdc, COLOR_KILL_BUTTON);
        TextOut(hdc, width - 30, y + 3, "X", 1);
    }

    y += 24;

    // Separation
    HPEN sepPen = CreatePen(PS_SOLID, 1, COLOR_MAGENTA);
    SelectObject(hdc, sepPen);
    MoveToEx(hdc, 14, y, NULL);
    LineTo(hdc, width - 14, y);
    DeleteObject(sepPen);
    y += 6;

    // Positions des colonnes selon le mode
    const int COL_INDICATOR = 16;
    int COL_MEM_OR_PORT, COL_PROCESS, COL_PID;

    if (data->view_mode == VIEW_MODE_ALL) {
        COL_MEM_OR_PORT = 28;
        COL_PROCESS = 80;
        COL_PID = 190;
    } else {
        COL_MEM_OR_PORT = 28;
        COL_PROCESS = 90;
        COL_PID = 200;
    }

    // Constantes scrollbar
    const int SCROLLBAR_WIDTH = 8;
    const int SCROLLBAR_X = width - 12;
    const int LIST_START_Y = y + 16;
    const int maxVisible = 6;
    const int LIST_HEIGHT = maxVisible * PROCESS_LINE_HEIGHT;

    // Nombre de processus filtres
    int filteredCount = GetFilteredProcessCount(data);

    // En-tete
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    if (data->view_mode == VIEW_MODE_ALL) {
        TextOut(hdc, COL_MEM_OR_PORT, y, "RAM", 3);
    } else {
        TextOut(hdc, COL_MEM_OR_PORT, y, "PORT", 4);
    }
    TextOut(hdc, COL_PROCESS, y, "PROCESS", 7);
    TextOut(hdc, COL_PID, y, "PID", 3);
    y += 18;

    // Liste des processus (filtres)
    SelectObject(hdc, hFontNormal);
    int startIdx = data->scroll_offset;
    int endIdx = startIdx + maxVisible;
    if (endIdx > filteredCount) endIdx = filteredCount;

    for (int visibleIdx = startIdx; visibleIdx < endIdx; visibleIdx++) {
        ProcessInfo* proc = GetFilteredProcessByVisibleIndex(data, visibleIdx);
        if (proc == NULL) continue;

        // Indicateur actif/critique
        if (proc->is_critical) {
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            TextOut(hdc, COL_INDICATOR, y, "!", 1);
        } else {
            SetTextColor(hdc, proc->is_active ? COLOR_PROCESS_ACTIVE : COLOR_PROCESS_INACTIVE);
            TextOut(hdc, COL_INDICATOR, y, "*", 1);
        }

        // RAM ou Port selon le mode
        char line[64];
        if (data->view_mode == VIEW_MODE_ALL) {
            snprintf(line, sizeof(line), "%luM", (unsigned long)proc->memory_mb);
        } else {
            snprintf(line, sizeof(line), ":%d", proc->port);
        }
        SetTextColor(hdc, COLOR_CYAN);
        TextOut(hdc, COL_MEM_OR_PORT, y, line, (int)strlen(line));

        // Nom du processus (tronque si trop long)
        char procName[20];
        int maxLen = (data->view_mode == VIEW_MODE_ALL) ? 14 : 12;
        strncpy(procName, proc->name, maxLen);
        procName[maxLen] = '\0';
        if ((int)strlen(proc->name) > maxLen) strcat(procName, "..");
        SetTextColor(hdc, proc->is_critical ? COLOR_TEXT_MUTED : COLOR_TEXT_PRIMARY);
        TextOut(hdc, COL_PROCESS, y, procName, (int)strlen(procName));

        // PID
        snprintf(line, sizeof(line), "%lu", proc->pid);
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        TextOut(hdc, COL_PID, y, line, (int)strlen(line));

        // Bouton Kill
        RECT killRect = {width - 70, y - 2, width - 26, y + 16};
        HBRUSH killBrush = CreateSolidBrush(proc->is_critical ? COLOR_BORDER : COLOR_KILL_BUTTON);
        FillRect(hdc, &killRect, killBrush);
        DeleteObject(killBrush);
        SetTextColor(hdc, proc->is_critical ? COLOR_TEXT_MUTED : RGB(255, 255, 255));
        SelectObject(hdc, hFontSmall);
        DrawText(hdc, "Kill", -1, &killRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hFontNormal);

        y += PROCESS_LINE_HEIGHT;
    }

    // ===== SCROLLBAR LATERALE =====
    if (filteredCount > maxVisible) {
        RECT trackRect = {SCROLLBAR_X - SCROLLBAR_WIDTH/2, LIST_START_Y,
                          SCROLLBAR_X + SCROLLBAR_WIDTH/2, LIST_START_Y + LIST_HEIGHT};
        HBRUSH trackBrush = CreateSolidBrush(COLOR_BORDER);
        FillRect(hdc, &trackRect, trackBrush);
        DeleteObject(trackBrush);

        int thumbHeight = (LIST_HEIGHT * maxVisible) / filteredCount;
        if (thumbHeight < 20) thumbHeight = 20;

        int maxScroll = filteredCount - maxVisible;
        int thumbY = LIST_START_Y;
        if (maxScroll > 0) {
            thumbY = LIST_START_Y + (data->scroll_offset * (LIST_HEIGHT - thumbHeight)) / maxScroll;
        }

        RECT thumbRect = {SCROLLBAR_X - SCROLLBAR_WIDTH/2, thumbY,
                          SCROLLBAR_X + SCROLLBAR_WIDTH/2, thumbY + thumbHeight};
        HBRUSH thumbBrush = CreateSolidBrush(COLOR_MAGENTA);
        FillRect(hdc, &thumbRect, thumbBrush);
        DeleteObject(thumbBrush);
    }

    // Message si aucun processus
    if (filteredCount == 0) {
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        if (data->filter_text[0] != '\0') {
            TextOut(hdc, 16, y, "No matching processes", 21);
        } else {
            TextOut(hdc, 16, y, "No processes with open ports", 28);
        }
        y += 20;
    }

    // Boutons en bas
    y = height - 40;

    // Bouton Refresh
    RECT refreshRect = {16, y, 100, y + 24};
    HBRUSH refreshBrush = CreateSolidBrush(COLOR_CYAN);
    FillRect(hdc, &refreshRect, refreshBrush);
    DeleteObject(refreshBrush);
    SetTextColor(hdc, RGB(0, 0, 0));
    SelectObject(hdc, hFontSmall);
    DrawText(hdc, "Refresh", -1, &refreshRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Info count
    char countStr[64];
    if (data->filter_text[0] != '\0') {
        snprintf(countStr, sizeof(countStr), "%d/%d filtered", filteredCount, data->count);
    } else if (filteredCount > maxVisible) {
        snprintf(countStr, sizeof(countStr), "%d processes (%d-%d)",
                 filteredCount, data->scroll_offset + 1,
                 (data->scroll_offset + maxVisible > filteredCount) ? filteredCount : data->scroll_offset + maxVisible);
    } else {
        snprintf(countStr, sizeof(countStr), "%d processes", data->count);
    }
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    TextOut(hdc, 120, y + 5, countStr, (int)strlen(countStr));
}

/*
 * DrawCompactMode
 * Dessine l'interface en mode ultra-compact (CPU + RAM sur une ligne)
 */
void DrawCompactMode(HDC hdc, int width, int height, HFONT hFont) {
    (void)height;  // Positions hardcodées sur WINDOW_HEIGHT_COMPACT
    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);

    MetricData* cpu      = GetMetricByName("CPU");
    MetricData* ram      = GetMetricByName("RAM");
    MetricData* datetime = GetMetricByName("DateTime");
    MetricData* prayer   = GetMetricByName("Prayer");
    MetricData* claude   = GetMetricByName("ClaudeUsage");

    // === Ligne 1 : CPU | RAM (y=8) ===
    int x  = 12;
    int y1 = 8;

    if (cpu && cpu->enabled && cpu->line_count > 0) {
        float cpuPct = 0;
        sscanf(cpu->display_lines[0], "CPU %f", &cpuPct);

        SetTextColor(hdc, g_colorAccent);
        TextOut(hdc, x, y1, "CPU", 3);
        x += 28;

        char val[16];
        snprintf(val, sizeof(val), "%.0f%%", cpuPct);
        SetTextColor(hdc, g_colorText);
        TextOut(hdc, x, y1, val, (int)strlen(val));
        x += 35;
    }

    SetTextColor(hdc, g_colorBorder);
    TextOut(hdc, x, y1, "|", 1);
    x += 14;

    if (ram && ram->enabled && ram->line_count > 0) {
        float used = 0, total = 0;
        sscanf(ram->display_lines[0], "RAM %f/%f", &used, &total);

        SetTextColor(hdc, g_colorAccent2);
        TextOut(hdc, x, y1, "RAM", 3);
        x += 30;

        char val[24];
        snprintf(val, sizeof(val), "%.1f/%.0fGB", used, total);
        SetTextColor(hdc, g_colorText);
        TextOut(hdc, x, y1, val, (int)strlen(val));
    }

    // === Séparateur horizontal (y=31) ===
    HPEN sepPen = CreatePen(PS_SOLID, 1, g_colorBorder);
    SelectObject(hdc, sepPen);
    MoveToEx(hdc, 8, 31, NULL);
    LineTo(hdc, width - 8, 31);
    DeleteObject(sepPen);

    // === Ligne 2 : Date + Heure (y=38) ===
    // Format source: "Time  Lun 20 Jan  14:35:22"  → offset 6 = "Time  "
    if (datetime && datetime->enabled && datetime->line_count > 0) {
        char dtDay[4] = {0}, dtMonth[4] = {0};
        int dtDayNum = 0, dtH = 0, dtM = 0, dtS = 0;
        sscanf(datetime->display_lines[0] + 6, "%3s %d %3s %d:%d:%d",
               dtDay, &dtDayNum, dtMonth, &dtH, &dtM, &dtS);

        char dtLine[32];
        snprintf(dtLine, sizeof(dtLine), "%s %02d %s  %02d:%02d",
                 dtDay, dtDayNum, dtMonth, dtH, dtM);

        SetTextColor(hdc, datetime->color);
        TextOut(hdc, 12, 38, dtLine, (int)strlen(dtLine));
    }

    // === Ligne 3 : Prière (y=59) ===
    // Format source: "Priere? %-8s %02d:%02d  (%dh%02d)"  (? = ' ' ou '*')
    // Offset 8 = longueur de "Priere? " → on tombe directement sur le nom
    if (prayer && prayer->enabled && prayer->line_count > 0) {
        char prayerName[16] = {0};
        char prayerTime[8]  = {0};
        char remaining[16]  = {0};
        sscanf(prayer->display_lines[0] + 8, "%15s %7s %15s",
               prayerName, prayerTime, remaining);

        x = 12;

        SetTextColor(hdc, prayer->color);
        TextOut(hdc, x, 59, prayerName, (int)strlen(prayerName));
        x += 58;

        SetTextColor(hdc, g_colorTextMuted);
        TextOut(hdc, x, 59, prayerTime, (int)strlen(prayerTime));
        x += 44;

        SetTextColor(hdc, prayer->color);
        TextOut(hdc, x, 59, remaining, (int)strlen(remaining));
    }

    // === Ligne 4 : Claude usage (y=80) ===
    // Format source: "Claude  J%3d%%  S%3d%%" ou "Claude  -- (pas de logs)"
    if (claude && claude->enabled && claude->line_count > 0) {
        SetTextColor(hdc, claude->color);
        TextOut(hdc, 12, 80, claude->display_lines[0],
                (int)strlen(claude->display_lines[0]));
    }
}
