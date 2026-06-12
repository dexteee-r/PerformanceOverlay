#include "disk_provider.h"

#include <QVariantMap>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

void DiskProvider::poll()
{
    QVariantList list;
    const DWORD drives = GetLogicalDrives();

    for (int i = 0; i < 26; ++i) {
        if (!(drives & (1u << i)))
            continue;

        const char root[4] = {static_cast<char>('A' + i), ':', '\\', '\0'};
        const UINT type = GetDriveTypeA(root);
        if (type != DRIVE_FIXED && type != DRIVE_RAMDISK)
            continue;

        ULARGE_INTEGER freeAvail, total, totalFree;
        if (GetDiskFreeSpaceExA(root, &freeAvail, &total, &totalFree)
            && total.QuadPart > 0) {
            const double usage = static_cast<double>(total.QuadPart - totalFree.QuadPart)
                                 / static_cast<double>(total.QuadPart) * 100.0;
            QVariantMap entry;
            entry.insert(QStringLiteral("name"), QString(QChar('A' + i)) + QChar(':'));
            entry.insert(QStringLiteral("usagePercent"), usage);
            list.append(entry);
        }
    }

    const double primary = list.isEmpty()
            ? 0.0
            : list.first().toMap().value(QStringLiteral("usagePercent")).toDouble();

    // La liste change rarement → on notifie seulement sur changement réel.
    if (list != m_disks) {
        m_disks = list;
        m_primary = primary;
        emit disksChanged();
    }
}
