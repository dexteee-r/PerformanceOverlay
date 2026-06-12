#include "gpu_provider.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>
#include <cstring>

namespace {
constexpr double kBytesPerGb = 1024.0 * 1024.0 * 1024.0;

// ===== Déclarations minimales de l'API NVML (évite de dépendre du SDK) =====
using nvmlReturn_t = int;             // NVML_SUCCESS = 0
using nvmlDevice_t = void *;

struct nvmlUtilization_t { unsigned int gpu; unsigned int memory; };
struct nvmlMemory_t { unsigned long long total; unsigned long long free; unsigned long long used; };

constexpr int kNvmlSuccess = 0;
constexpr int kNvmlTemperatureGpu = 0;

using PFN_nvmlInit = nvmlReturn_t (*)(void);
using PFN_nvmlShutdown = nvmlReturn_t (*)(void);
using PFN_nvmlGetHandle = nvmlReturn_t (*)(unsigned int, nvmlDevice_t *);
using PFN_nvmlGetUtil = nvmlReturn_t (*)(nvmlDevice_t, nvmlUtilization_t *);
using PFN_nvmlGetMem = nvmlReturn_t (*)(nvmlDevice_t, nvmlMemory_t *);
using PFN_nvmlGetTemp = nvmlReturn_t (*)(nvmlDevice_t, int, unsigned int *);
using PFN_nvmlGetPower = nvmlReturn_t (*)(nvmlDevice_t, unsigned int *);

// Nom du GPU via le registre / EnumDisplayDevices (repli si pas de NVML).
QString readGpuNameFromRegistry()
{
    HKEY hKey;
    char buffer[256];
    DWORD size;
    DWORD type;

    const char *regPaths[] = {
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000",
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0001",
        nullptr
    };

    for (int i = 0; regPaths[i] != nullptr; ++i) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPaths[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            for (const char *value : {"DriverDesc", "HardwareInformation.AdapterString"}) {
                size = sizeof(buffer);
                if (RegQueryValueExA(hKey, value, nullptr, &type,
                                     reinterpret_cast<LPBYTE>(buffer), &size) == ERROR_SUCCESS) {
                    buffer[sizeof(buffer) - 1] = '\0';
                    RegCloseKey(hKey);
                    return QString::fromLocal8Bit(buffer);
                }
            }
            RegCloseKey(hKey);
        }
    }

    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(dd);
    if (EnumDisplayDevicesA(nullptr, 0, &dd, 0) && dd.DeviceString[0] != '\0')
        return QString::fromLocal8Bit(dd.DeviceString);

    return QStringLiteral("Unknown GPU");
}

} // namespace

// État NVML encapsulé (PIMPL).
struct GpuProvider::Nvml {
    HMODULE lib = nullptr;
    nvmlDevice_t device = nullptr;
    bool ok = false;

    PFN_nvmlInit init = nullptr;
    PFN_nvmlShutdown shutdown = nullptr;
    PFN_nvmlGetHandle getHandle = nullptr;
    PFN_nvmlGetUtil getUtil = nullptr;
    PFN_nvmlGetMem getMem = nullptr;
    PFN_nvmlGetTemp getTemp = nullptr;
    PFN_nvmlGetPower getPower = nullptr;

    bool load()
    {
        const char *paths[] = {
            "nvml.dll",
            "C:\\Program Files\\NVIDIA Corporation\\NVSMI\\nvml.dll",
            nullptr
        };
        for (int i = 0; paths[i] != nullptr && !lib; ++i)
            lib = LoadLibraryA(paths[i]);
        if (!lib)
            return false;

        auto sym = [this](const char *primary, const char *fallback = nullptr) -> void * {
            void *p = reinterpret_cast<void *>(GetProcAddress(lib, primary));
            if (!p && fallback)
                p = reinterpret_cast<void *>(GetProcAddress(lib, fallback));
            return p;
        };

        init = reinterpret_cast<PFN_nvmlInit>(sym("nvmlInit_v2", "nvmlInit"));
        shutdown = reinterpret_cast<PFN_nvmlShutdown>(sym("nvmlShutdown"));
        getHandle = reinterpret_cast<PFN_nvmlGetHandle>(
            sym("nvmlDeviceGetHandleByIndex_v2", "nvmlDeviceGetHandleByIndex"));
        getUtil = reinterpret_cast<PFN_nvmlGetUtil>(sym("nvmlDeviceGetUtilizationRates"));
        getMem = reinterpret_cast<PFN_nvmlGetMem>(sym("nvmlDeviceGetMemoryInfo"));
        getTemp = reinterpret_cast<PFN_nvmlGetTemp>(sym("nvmlDeviceGetTemperature"));
        getPower = reinterpret_cast<PFN_nvmlGetPower>(sym("nvmlDeviceGetPowerUsage"));

        if (!init || !shutdown || !getHandle)
            return false;
        if (init() != kNvmlSuccess)
            return false;
        if (getHandle(0, &device) != kNvmlSuccess) {
            shutdown();
            return false;
        }
        ok = true;
        return true;
    }

    void unload()
    {
        if (ok && shutdown)
            shutdown();
        if (lib) {
            FreeLibrary(lib);
            lib = nullptr;
        }
        ok = false;
    }
};

GpuProvider::GpuProvider(QObject *parent)
    : MetricProvider(parent), m_nvml(std::make_unique<Nvml>())
{
    m_name = readGpuNameFromRegistry();   // toujours, pour le repli
    m_available = m_nvml->load();
}

GpuProvider::~GpuProvider()
{
    m_nvml->unload();
}

void GpuProvider::poll()
{
    if (!m_nvml->ok) {
        // Repli : pas de NVML, on garde juste le nom du GPU.
        setAvailable(false);
        m_history.push(0.0);
        emit usageHistoryChanged();
        return;
    }

    nvmlUtilization_t util = {0, 0};
    nvmlMemory_t mem = {0, 0, 0};
    unsigned int temp = 0;
    unsigned int powerMw = 0;

    if (m_nvml->getUtil && m_nvml->getUtil(m_nvml->device, &util) == kNvmlSuccess)
        setUsagePercent(util.gpu);
    if (m_nvml->getTemp && m_nvml->getTemp(m_nvml->device, kNvmlTemperatureGpu, &temp) == kNvmlSuccess)
        setTemperatureC(temp);
    if (m_nvml->getPower && m_nvml->getPower(m_nvml->device, &powerMw) == kNvmlSuccess)
        setPowerW(powerMw / 1000.0);
    if (m_nvml->getMem && m_nvml->getMem(m_nvml->device, &mem) == kNvmlSuccess) {
        setVramUsedGb(static_cast<double>(mem.used) / kBytesPerGb);
        setVramTotalGb(static_cast<double>(mem.total) / kBytesPerGb);
    }

    setAvailable(true);

    m_history.push(m_usagePercent);
    emit usageHistoryChanged();
}

void GpuProvider::setAvailable(bool v)
{
    if (m_available == v) return;
    m_available = v;
    emit availableChanged();
}

void GpuProvider::setName(const QString &v)
{
    if (m_name == v) return;
    m_name = v;
    emit nameChanged();
}

void GpuProvider::setUsagePercent(double v)
{
    if (m_usagePercent == v) return;
    m_usagePercent = v;
    emit usagePercentChanged();
}

void GpuProvider::setTemperatureC(double v)
{
    if (m_temperatureC == v) return;
    m_temperatureC = v;
    emit temperatureCChanged();
}

void GpuProvider::setPowerW(double v)
{
    if (m_powerW == v) return;
    m_powerW = v;
    emit powerWChanged();
}

void GpuProvider::setVramUsedGb(double v)
{
    if (m_vramUsedGb == v) return;
    m_vramUsedGb = v;
    emit vramUsedGbChanged();
}

void GpuProvider::setVramTotalGb(double v)
{
    if (m_vramTotalGb == v) return;
    m_vramTotalGb = v;
    emit vramTotalGbChanged();
}
