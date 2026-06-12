#include "cpu_provider.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

namespace {

// Combine un FILETIME (100 ns depuis 1601) en compteur 64 bits.
inline quint64 toU64(const FILETIME &ft)
{
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    return u.QuadPart;
}

// Fréquence nominale du CPU lue dans le registre (~MHz → GHz). Statique : ne
// change pas pendant l'exécution, mais peu coûteux à relire.
double readFrequencyGhz()
{
    HKEY hKey;
    DWORD mhz = 0;
    DWORD size = sizeof(mhz);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hKey, "~MHz", nullptr, nullptr,
                         reinterpret_cast<LPBYTE>(&mhz), &size);
        RegCloseKey(hKey);
    }
    return mhz / 1000.0;
}

} // namespace

CpuProvider::CpuProvider(QObject *parent) : MetricProvider(parent)
{
    // Amorce la baseline : le premier poll() mesurera l'intervalle écoulé.
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        m_lastIdle = toU64(idle);
        m_lastKernel = toU64(kernel);
        m_lastUser = toU64(user);
    }
}

void CpuProvider::poll()
{
    FILETIME idleFt, kernelFt, userFt;
    if (!GetSystemTimes(&idleFt, &kernelFt, &userFt))
        return;

    const quint64 idle = toU64(idleFt);
    const quint64 kernel = toU64(kernelFt);   // inclut le temps idle
    const quint64 user = toU64(userFt);

    const quint64 idleDiff = idle - m_lastIdle;
    const quint64 totalDiff = (kernel - m_lastKernel) + (user - m_lastUser);

    m_lastIdle = idle;
    m_lastKernel = kernel;
    m_lastUser = user;

    double pct = 0.0;
    if (totalDiff != 0)
        pct = static_cast<double>(totalDiff - idleDiff) * 100.0
              / static_cast<double>(totalDiff);

    if (pct < 0.0) pct = 0.0;
    if (pct > 100.0) pct = 100.0;

    setUsagePercent(pct);
    setFrequencyGhz(readFrequencyGhz());

    m_history.push(pct);
    emit usageHistoryChanged();
}

void CpuProvider::setUsagePercent(double v)
{
    if (m_usagePercent == v)
        return;
    m_usagePercent = v;
    emit usagePercentChanged();
}

void CpuProvider::setFrequencyGhz(double v)
{
    if (m_frequencyGhz == v)
        return;
    m_frequencyGhz = v;
    emit frequencyGhzChanged();
}
