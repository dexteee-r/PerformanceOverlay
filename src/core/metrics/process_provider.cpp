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
    DWORD pids[1024];
    DWORD bytesReturned = 0;
    int c = 0;
    if (EnumProcesses(pids, sizeof(pids), &bytesReturned))
        c = static_cast<int>(bytesReturned / sizeof(DWORD));

    if (c != m_count) {
        m_count = c;
        emit countChanged();
    }
}
