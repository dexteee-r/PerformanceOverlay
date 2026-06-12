/*
 * plugin_gpu.c
 * Plugin pour afficher les metriques GPU.
 *
 * Strategie :
 *  - NVIDIA : charge dynamiquement nvml.dll (livre avec le driver, pas besoin
 *    du CUDA SDK) et lit utilisation %, VRAM, temperature, conso.
 *  - Repli : si NVML indisponible (pas NVIDIA, dll absente), affiche le nom
 *    du GPU depuis le registre Windows (comportement historique).
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

// ===== Declarations minimales de l'API NVML (evite de dependre du SDK) =====
typedef int nvmlReturn_t;            // NVML_SUCCESS = 0
typedef void* nvmlDevice_t;

typedef struct { unsigned int gpu; unsigned int memory; } nvmlUtilization_t;
typedef struct {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
} nvmlMemory_t;

#define NVML_SUCCESS 0
#define NVML_TEMPERATURE_GPU 0

// Pointeurs de fonctions NVML
typedef nvmlReturn_t (*PFN_nvmlInit)(void);
typedef nvmlReturn_t (*PFN_nvmlShutdown)(void);
typedef nvmlReturn_t (*PFN_nvmlDeviceGetHandleByIndex)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t (*PFN_nvmlDeviceGetUtilizationRates)(nvmlDevice_t, nvmlUtilization_t*);
typedef nvmlReturn_t (*PFN_nvmlDeviceGetMemoryInfo)(nvmlDevice_t, nvmlMemory_t*);
typedef nvmlReturn_t (*PFN_nvmlDeviceGetTemperature)(nvmlDevice_t, int, unsigned int*);
typedef nvmlReturn_t (*PFN_nvmlDeviceGetPowerUsage)(nvmlDevice_t, unsigned int*);

static HMODULE g_nvmlLib = NULL;
static nvmlDevice_t g_nvmlDevice = NULL;
static BOOL g_nvmlOk = FALSE;

static PFN_nvmlInit                      pNvmlInit = NULL;
static PFN_nvmlShutdown                  pNvmlShutdown = NULL;
static PFN_nvmlDeviceGetHandleByIndex    pNvmlGetHandle = NULL;
static PFN_nvmlDeviceGetUtilizationRates pNvmlGetUtil = NULL;
static PFN_nvmlDeviceGetMemoryInfo       pNvmlGetMem = NULL;
static PFN_nvmlDeviceGetTemperature      pNvmlGetTemp = NULL;
static PFN_nvmlDeviceGetPowerUsage       pNvmlGetPower = NULL;

// ===== Repli registre =====
static char gpuName[128] = "Unknown GPU";
static BOOL gpuAvailable = FALSE;

/*
 * GetGPUNameFromRegistry
 * Obtient le nom du GPU depuis le registre Windows (repli).
 */
static void GetGPUNameFromRegistry(void) {
    HKEY hKey;
    char buffer[256];
    DWORD size = sizeof(buffer);
    DWORD type;

    const char* regPaths[] = {
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000",
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0001",
        NULL
    };

    for (int i = 0; regPaths[i] != NULL; i++) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPaths[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "DriverDesc", NULL, &type, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                strncpy(gpuName, buffer, sizeof(gpuName) - 1);
                gpuName[sizeof(gpuName) - 1] = '\0';
                gpuAvailable = TRUE;
                RegCloseKey(hKey);
                return;
            }
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "HardwareInformation.AdapterString", NULL, &type, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                strncpy(gpuName, buffer, sizeof(gpuName) - 1);
                gpuName[sizeof(gpuName) - 1] = '\0';
                gpuAvailable = TRUE;
                RegCloseKey(hKey);
                return;
            }
            RegCloseKey(hKey);
        }
    }

    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(DISPLAY_DEVICEA);
    if (EnumDisplayDevicesA(NULL, 0, &dd, 0)) {
        if (strlen(dd.DeviceString) > 0) {
            strncpy(gpuName, dd.DeviceString, sizeof(gpuName) - 1);
            gpuName[sizeof(gpuName) - 1] = '\0';
            gpuAvailable = TRUE;
        }
    }
}

/*
 * TryLoadNVML
 * Charge nvml.dll et resout les fonctions. Retourne TRUE si tout est pret.
 */
static BOOL TryLoadNVML(void) {
    // nvml.dll moderne est dans System32 ; on tente aussi le chemin NVSMI.
    const char* paths[] = {
        "nvml.dll",
        "C:\\Program Files\\NVIDIA Corporation\\NVSMI\\nvml.dll",
        NULL
    };
    for (int i = 0; paths[i] != NULL && !g_nvmlLib; i++) {
        g_nvmlLib = LoadLibraryA(paths[i]);
    }
    if (!g_nvmlLib) return FALSE;

    pNvmlInit      = (PFN_nvmlInit)(void*)GetProcAddress(g_nvmlLib, "nvmlInit_v2");
    if (!pNvmlInit) pNvmlInit = (PFN_nvmlInit)(void*)GetProcAddress(g_nvmlLib, "nvmlInit");
    pNvmlShutdown  = (PFN_nvmlShutdown)(void*)GetProcAddress(g_nvmlLib, "nvmlShutdown");
    pNvmlGetHandle = (PFN_nvmlDeviceGetHandleByIndex)(void*)GetProcAddress(g_nvmlLib, "nvmlDeviceGetHandleByIndex_v2");
    if (!pNvmlGetHandle) pNvmlGetHandle = (PFN_nvmlDeviceGetHandleByIndex)(void*)GetProcAddress(g_nvmlLib, "nvmlDeviceGetHandleByIndex");
    pNvmlGetUtil   = (PFN_nvmlDeviceGetUtilizationRates)(void*)GetProcAddress(g_nvmlLib, "nvmlDeviceGetUtilizationRates");
    pNvmlGetMem    = (PFN_nvmlDeviceGetMemoryInfo)(void*)GetProcAddress(g_nvmlLib, "nvmlDeviceGetMemoryInfo");
    pNvmlGetTemp   = (PFN_nvmlDeviceGetTemperature)(void*)GetProcAddress(g_nvmlLib, "nvmlDeviceGetTemperature");
    pNvmlGetPower  = (PFN_nvmlDeviceGetPowerUsage)(void*)GetProcAddress(g_nvmlLib, "nvmlDeviceGetPowerUsage");

    if (!pNvmlInit || !pNvmlShutdown || !pNvmlGetHandle) return FALSE;
    if (pNvmlInit() != NVML_SUCCESS) return FALSE;
    if (pNvmlGetHandle(0, &g_nvmlDevice) != NVML_SUCCESS) {
        pNvmlShutdown();
        return FALSE;
    }
    return TRUE;
}

/*
 * gpu_init
 */
static void gpu_init(void) {
    GetGPUNameFromRegistry();   // toujours, pour le repli
    g_nvmlOk = TryLoadNVML();
}

/*
 * gpu_update
 * Affiche les metriques NVML si dispo, sinon le nom du GPU.
 */
static void gpu_update(MetricData* data) {
    if (g_nvmlOk) {
        nvmlUtilization_t util = {0, 0};
        nvmlMemory_t mem = {0, 0, 0};
        unsigned int temp = 0;
        unsigned int powerMw = 0;

        BOOL hasUtil  = pNvmlGetUtil  && pNvmlGetUtil(g_nvmlDevice, &util) == NVML_SUCCESS;
        BOOL hasMem   = pNvmlGetMem   && pNvmlGetMem(g_nvmlDevice, &mem) == NVML_SUCCESS;
        BOOL hasTemp  = pNvmlGetTemp  && pNvmlGetTemp(g_nvmlDevice, NVML_TEMPERATURE_GPU, &temp) == NVML_SUCCESS;
        BOOL hasPower = pNvmlGetPower && pNvmlGetPower(g_nvmlDevice, &powerMw) == NVML_SUCCESS;

        // Ligne 1 : utilisation, temperature, conso
        char l1[256];
        int off = snprintf(l1, sizeof(l1), "GPU  ");
        if (hasUtil)  off += snprintf(l1 + off, sizeof(l1) - off, " %3u%%", util.gpu);
        if (hasTemp)  off += snprintf(l1 + off, sizeof(l1) - off, " %2u\xB0""C", temp);  // \xB0 = ° (Latin-1)
        if (hasPower) off += snprintf(l1 + off, sizeof(l1) - off, " %3uW", powerMw / 1000);
        strncpy(data->display_lines[0], l1, sizeof(data->display_lines[0]) - 1);
        data->display_lines[0][sizeof(data->display_lines[0]) - 1] = '\0';
        data->line_count = 1;

        // Ligne 2 : VRAM utilisee / totale
        if (hasMem) {
            double usedGB  = (double)mem.used  / (1024.0 * 1024.0 * 1024.0);
            double totalGB = (double)mem.total / (1024.0 * 1024.0 * 1024.0);
            snprintf(data->display_lines[1], sizeof(data->display_lines[1]),
                     "VRAM  %.1f/%.1fGB", usedGB, totalGB);
            data->line_count = 2;
        }

        data->value = hasUtil ? (float)util.gpu : 0.0f;

        // Couleur selon la temperature (alerte thermique).
        if (hasTemp && temp >= 83) {
            data->color = RGB(255, 100, 100);   // rouge
        } else if (hasTemp && temp >= 72) {
            data->color = RGB(255, 200, 100);   // orange
        } else {
            data->color = COLOR_TEXT_PRIMARY;
        }
        return;
    }

    // ----- Repli : nom du GPU -----
    data->value = 0;
    data->line_count = 1;

    char shortName[40];
    strncpy(shortName, gpuName, 36);
    shortName[36] = '\0';
    int len = (int)strlen(shortName);
    while (len > 0 && shortName[len-1] == ' ') shortName[--len] = '\0';

    snprintf(data->display_lines[0], sizeof(data->display_lines[0]), "GPU   %s", shortName);
    data->color = COLOR_TEXT_PRIMARY;
}

/*
 * gpu_cleanup
 */
static void gpu_cleanup(void) {
    if (g_nvmlOk && pNvmlShutdown) pNvmlShutdown();
    if (g_nvmlLib) { FreeLibrary(g_nvmlLib); g_nvmlLib = NULL; }
    g_nvmlOk = FALSE;
}

/*
 * gpu_is_available
 */
static BOOL gpu_is_available(void) {
    return g_nvmlOk || gpuAvailable;
}

// Declaration du plugin
MetricPlugin GPUPlugin = {
    .plugin_name = "GPU",
    .description = "Metriques GPU (NVML) ou nom du GPU",
    .init = gpu_init,
    .update = gpu_update,
    .cleanup = gpu_cleanup,
    .is_available = gpu_is_available,
    .next = NULL
};
