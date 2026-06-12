/*
 * plugin_claude_usage.c
 * Plugin pour afficher l'utilisation de tokens Claude Code (locale).
 *
 * Lit les logs locaux de Claude Code (~/.claude/projects/<session>/ *.jsonl),
 * agrege les tokens consommes par jour sur une fenetre glissante, et affiche
 * un pourcentage journalier et hebdomadaire RELATIF a la moyenne de l'usager.
 *
 *   % jour    = tokens_aujourdhui / (somme_28j / 28) * 100
 *   % semaine = tokens_7j_glissants / (somme_28j / 4) * 100
 *
 * 100% = une journee/semaine "normale" pour toi. >100% = au-dessus de ta moyenne.
 *
 * Note: reflete la conso Claude Code sur CETTE machine, pas le quota officiel
 * Anthropic (qui n'a pas d'API publique).
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

// ===== Etat / cache (re-scan throttle) =====
static float     g_pctDay     = 0.0f;
static float     g_pctWeek    = 0.0f;
static BOOL      g_hasData     = FALSE;
static ULONGLONG g_lastScanTick = 0;
static BOOL      g_firstScan   = TRUE;

// ===== Table de hachage pour deduplication (requestId + message.id) =====
// Les logs repetent parfois le meme message; on deduplique par cle unique.
#define DEDUP_SLOTS (1u << 18)   // 262144 slots (~2 Mo en .bss)
static uint64_t g_seen[DEDUP_SLOTS];

/*
 * FNV-1a 64 bits sur une chaine.
 */
static uint64_t FnvHash(const char* s, size_t len) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < len; i++) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h;
}

/*
 * DedupReset : vide la table avant un scan complet.
 */
static void DedupReset(void) {
    memset(g_seen, 0, sizeof(g_seen));
}

/*
 * DedupCheckAndAdd : retourne TRUE si la cle etait deja vue (=> doublon a ignorer).
 * Sinon l'ajoute et retourne FALSE. Open addressing (sondage lineaire).
 */
static BOOL DedupCheckAndAdd(uint64_t key) {
    if (key == 0) key = 1;  // 0 = slot vide, on evite cette valeur
    uint32_t idx = (uint32_t)(key & (DEDUP_SLOTS - 1));
    for (uint32_t probe = 0; probe < DEDUP_SLOTS; probe++) {
        uint32_t slot = (idx + probe) & (DEDUP_SLOTS - 1);
        if (g_seen[slot] == 0) {
            g_seen[slot] = key;
            return FALSE;  // nouveau
        }
        if (g_seen[slot] == key) {
            return TRUE;   // doublon
        }
    }
    return FALSE;  // table pleine (improbable) : on compte
}

/*
 * DaysFromCivil : nombre de jours serie depuis une epoque fixe (algo Hinnant).
 * Permet de comparer des dates et calculer un "age en jours".
 */
static long DaysFromCivil(int y, int m, int d) {
    y -= (m <= 2);
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = (long)(y - era * 400);                          // [0, 399]
    long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; // [0, 365]
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;          // [0, 146096]
    return era * 146097 + doe - 719468;
}

/*
 * ExtractLong : lit la valeur entiere associee a une cle JSON ("cle":NOMBRE).
 * searchKey doit inclure les guillemets, ex: "\"input_tokens\"".
 * Le guillemet fermant dans la cle evite les faux positifs sur le contenu echappe.
 */
static long long ExtractLong(const char* line, const char* searchKey) {
    const char* p = strstr(line, searchKey);
    if (!p) return 0;
    const char* colon = strchr(p + strlen(searchKey), ':');
    if (!colon) return 0;
    colon++;
    while (*colon == ' ' || *colon == '\t') colon++;
    long long v = 0;
    if (sscanf(colon, "%lld", &v) != 1) return 0;
    return v;
}

/*
 * ExtractStr : copie la valeur chaine associee a une cle ("cle":"valeur").
 * keyWithColonQuote = debut litteral, ex: "\"requestId\":\"".
 */
static BOOL ExtractStr(const char* line, const char* keyWithColonQuote,
                       char* out, size_t outSize) {
    const char* p = strstr(line, keyWithColonQuote);
    if (!p) return FALSE;
    p += strlen(keyWithColonQuote);
    const char* end = strchr(p, '"');
    if (!end) return FALSE;
    size_t len = (size_t)(end - p);
    if (len >= outSize) len = outSize - 1;
    memcpy(out, p, len);
    out[len] = '\0';
    return TRUE;
}

/*
 * ProcessLine : traite une ligne JSONL si c'est un message assistant avec usage.
 * Agrege les tokens dans le bon bucket journalier (apres deduplication).
 */
static void ProcessLine(char* line, long long* buckets, long todayIndex) {
    // Filtre : seulement les lignes assistant avec un bloc usage reel.
    if (!strstr(line, "\"type\":\"assistant\"")) return;
    if (!strstr(line, "\"input_tokens\"")) return;

    // Date (timestamp UTC "YYYY-MM-DDTHH:..."), on garde YYYY-MM-DD.
    char ts[32];
    if (!ExtractStr(line, "\"timestamp\":\"", ts, sizeof(ts))) return;
    int y = 0, mo = 0, d = 0;
    if (sscanf(ts, "%d-%d-%d", &y, &mo, &d) != 3) return;

    long dayIdx = DaysFromCivil(y, mo, d);
    long age = todayIndex - dayIdx;
    if (age < 0 || age >= CLAUDE_BASELINE_DAYS) return;  // hors fenetre

    // Deduplication : cle = requestId + "|" + message.id
    char reqId[64] = "";
    char msgId[64] = "";
    ExtractStr(line, "\"requestId\":\"", reqId, sizeof(reqId));
    ExtractStr(line, "\"id\":\"msg_", msgId, sizeof(msgId));  // capte le suffixe
    char keyBuf[160];
    int kl = snprintf(keyBuf, sizeof(keyBuf), "%s|msg_%s", reqId, msgId);
    if (kl > 0) {
        uint64_t key = FnvHash(keyBuf, (size_t)kl);
        if (DedupCheckAndAdd(key)) return;  // doublon
    }

    // Somme des 4 compteurs de tokens.
    long long tok = 0;
    tok += ExtractLong(line, "\"input_tokens\"");
    tok += ExtractLong(line, "\"output_tokens\"");
    tok += ExtractLong(line, "\"cache_creation_input_tokens\"");
    tok += ExtractLong(line, "\"cache_read_input_tokens\"");

    buckets[age] += tok;
}

/*
 * ProcessFile : charge un .jsonl en memoire et traite chaque ligne.
 */
static void ProcessFile(const char* path, long long* buckets, long todayIndex) {
    FILE* f = fopen(path, "rb");
    if (!f) return;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0 || size > 128 * 1024 * 1024) {  // garde-fou 128 Mo
        fclose(f);
        return;
    }

    char* buf = (char*)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return;
    }
    size_t read = fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[read] = '\0';

    // Decoupe par lignes (on remplace '\n' par '\0' sur place).
    char* lineStart = buf;
    for (size_t i = 0; i < read; i++) {
        if (buf[i] == '\n') {
            buf[i] = '\0';
            ProcessLine(lineStart, buckets, todayIndex);
            lineStart = buf + i + 1;
        }
    }
    if (lineStart < buf + read) {
        ProcessLine(lineStart, buckets, todayIndex);  // derniere ligne sans \n
    }

    free(buf);
}

/*
 * ScanAll : parcourt ~/.claude/projects/<session>/ *.jsonl et calcule les %.
 */
static void ScanAll(void) {
    char projects[MAX_PATH];
    DWORD n = GetEnvironmentVariableA("USERPROFILE", projects, sizeof(projects));
    if (n == 0 || n >= sizeof(projects)) {
        g_hasData = FALSE;
        return;
    }
    strncat(projects, "\\.claude\\projects", sizeof(projects) - strlen(projects) - 1);

    // Index du jour courant (UTC, pour coller aux timestamps "Z").
    SYSTEMTIME now;
    GetSystemTime(&now);
    long todayIndex = DaysFromCivil(now.wYear, now.wMonth, now.wDay);

    long long buckets[CLAUDE_BASELINE_DAYS];
    memset(buckets, 0, sizeof(buckets));
    DedupReset();

    // Enumerer les sous-dossiers de "projects"
    char dirPattern[MAX_PATH];
    snprintf(dirPattern, sizeof(dirPattern), "%s\\*", projects);

    WIN32_FIND_DATAA fd;
    HANDLE hDir = FindFirstFileA(dirPattern, &fd);
    if (hDir == INVALID_HANDLE_VALUE) {
        g_hasData = FALSE;
        return;
    }

    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;

        // Enumerer les *.jsonl du sous-dossier
        char filePattern[1024];
        snprintf(filePattern, sizeof(filePattern), "%s\\%s\\*.jsonl",
                 projects, fd.cFileName);

        WIN32_FIND_DATAA ff;
        HANDLE hFile = FindFirstFileA(filePattern, &ff);
        if (hFile == INVALID_HANDLE_VALUE) continue;

        do {
            if (ff.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
            char fullPath[1024];
            snprintf(fullPath, sizeof(fullPath), "%s\\%s\\%s",
                     projects, fd.cFileName, ff.cFileName);
            ProcessFile(fullPath, buckets, todayIndex);
        } while (FindNextFileA(hFile, &ff));
        FindClose(hFile);

    } while (FindNextFileA(hDir, &fd));
    FindClose(hDir);

    // Calcul des agregats.
    long long tokToday = buckets[0];
    long long tok7d = 0;
    long long tok28d = 0;
    for (int i = 0; i < CLAUDE_BASELINE_DAYS; i++) {
        tok28d += buckets[i];
        if (i < 7) tok7d += buckets[i];
    }

    if (tok28d <= 0) {
        g_hasData = FALSE;
        return;
    }

    double avgDay  = (double)tok28d / CLAUDE_BASELINE_DAYS;
    double avgWeek = (double)tok28d / (CLAUDE_BASELINE_DAYS / 7.0);  // 28/7 = 4 semaines

    g_pctDay  = avgDay  > 0 ? (float)((double)tokToday / avgDay * 100.0)  : 0.0f;
    g_pctWeek = avgWeek > 0 ? (float)((double)tok7d   / avgWeek * 100.0) : 0.0f;
    g_hasData = TRUE;
}

// ===== Interface plugin =====

static void claude_init(void) {
    g_firstScan = TRUE;
    g_lastScanTick = 0;
}

static void claude_update(MetricData* data) {
    data->line_count = 1;

    // Throttle : ne re-scanner qu'une fois par minute.
    ULONGLONG nowTick = GetTickCount64();
    if (g_firstScan || (nowTick - g_lastScanTick) >= CLAUDE_RESCAN_MS) {
        ScanAll();
        g_lastScanTick = nowTick;
        g_firstScan = FALSE;
    }

    if (!g_hasData) {
        snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
                 "Claude  -- (pas de logs)");
        data->value = 0;
        data->color = COLOR_TEXT_MUTED;
        return;
    }

    // Clamp d'affichage pour rester lisible.
    int pd = (int)(g_pctDay  + 0.5f);
    int pw = (int)(g_pctWeek + 0.5f);
    if (pd > 999) pd = 999;
    if (pw > 999) pw = 999;

    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "Claude  J%3d%%  S%3d%%", pd, pw);

    // Barre = % semaine (borne 0-100).
    float v = g_pctWeek;
    if (v > 100.0f) v = 100.0f;
    data->value = v;

    // Couleur selon l'intensite hebdo.
    if (g_pctWeek >= 150.0f) {
        data->color = RGB(255, 100, 100);   // rouge : bien au-dessus de la moyenne
    } else if (g_pctWeek >= 120.0f) {
        data->color = RGB(255, 200, 100);   // orange : au-dessus
    } else {
        data->color = RGB(150, 255, 200);   // vert : dans la norme
    }
}

static void claude_cleanup(void) {
    // Rien a nettoyer (table statique).
}

static BOOL claude_is_available(void) {
    return TRUE;
}

// Declaration du plugin
MetricPlugin ClaudeUsagePlugin = {
    .plugin_name = "ClaudeUsage",
    .description = "Utilisation tokens Claude Code (% vs moyenne)",
    .init = claude_init,
    .update = claude_update,
    .cleanup = claude_cleanup,
    .is_available = claude_is_available,
    .next = NULL
};
