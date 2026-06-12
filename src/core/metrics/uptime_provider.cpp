#include "uptime_provider.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

void UptimeProvider::poll()
{
    const qint64 s = static_cast<qint64>(GetTickCount64() / 1000ULL);
    if (s != m_seconds) {
        m_seconds = s;
        emit secondsChanged();
    }
}
