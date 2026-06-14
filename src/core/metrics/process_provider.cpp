#include "process_provider.h"

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>

void ProcessProvider::poll()
{
    // GetPerformanceInfo : compteurs système (processus ET threads) en un appel.
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);
    if (!GetPerformanceInfo(&pi, sizeof(pi)))
        return;

    const int procs = static_cast<int>(pi.ProcessCount);
    const int threads = static_cast<int>(pi.ThreadCount);

    if (procs != m_count) {
        m_count = procs;
        emit countChanged();
    }
    if (threads != m_threadCount) {
        m_threadCount = threads;
        emit threadCountChanged();
    }
}
