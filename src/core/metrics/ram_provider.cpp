#include "ram_provider.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

namespace {
constexpr double kBytesPerGb = 1024.0 * 1024.0 * 1024.0;
}

RamProvider::RamProvider(QObject *parent) : MetricProvider(parent) {}

void RamProvider::poll()
{
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    if (!GlobalMemoryStatusEx(&mem))
        return;

    setTotalGb(static_cast<double>(mem.ullTotalPhys) / kBytesPerGb);
    setUsedGb(static_cast<double>(mem.ullTotalPhys - mem.ullAvailPhys) / kBytesPerGb);
    setUsagePercent(static_cast<double>(mem.dwMemoryLoad));
}

void RamProvider::setUsagePercent(double v)
{
    if (m_usagePercent == v)
        return;
    m_usagePercent = v;
    emit usagePercentChanged();
}

void RamProvider::setUsedGb(double v)
{
    if (m_usedGb == v)
        return;
    m_usedGb = v;
    emit usedGbChanged();
}

void RamProvider::setTotalGb(double v)
{
    if (m_totalGb == v)
        return;
    m_totalGb = v;
    emit totalGbChanged();
}
