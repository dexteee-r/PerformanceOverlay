#include "task_killer_model.h"

#include <algorithm>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

namespace {
// Processus critiques à ne JAMAIS tuer (BSOD / instabilité).
const char *kCritical[] = {
    "csrss.exe", "lsass.exe", "wininit.exe", "services.exe", "smss.exe",
    "svchost.exe", "dwm.exe", "winlogon.exe", "System", "Registry",
    "Memory Compression", "audiodg.exe", "fontdrvhost.exe", "conhost.exe",
    nullptr
};
}

TaskKillerModel::TaskKillerModel(QObject *parent) : QAbstractListModel(parent)
{
    refresh();
}

int TaskKillerModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

QVariant TaskKillerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
        return {};
    const ProcRow &r = m_rows.at(index.row());
    switch (role) {
    case NameRole:     return r.name;
    case PidRole:      return r.pid;
    case MemoryRole:   return r.memoryMb;
    case CriticalRole: return r.critical;
    default:           return {};
    }
}

QHash<int, QByteArray> TaskKillerModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {PidRole, "pid"},
        {MemoryRole, "memoryMb"},
        {CriticalRole, "critical"},
    };
}

bool TaskKillerModel::isCritical(const QString &name, quint32 pid) const
{
    if (pid == 0 || pid == 4)
        return true;
    for (int i = 0; kCritical[i] != nullptr; ++i)
        if (name.compare(QString::fromLatin1(kCritical[i]), Qt::CaseInsensitive) == 0)
            return true;
    return false;
}

void TaskKillerModel::refresh()
{
    QList<ProcRow> rows;

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe;
        pe.dwSize = sizeof(pe);
        if (Process32FirstW(snap, &pe)) {
            do {
                const quint32 pid = pe.th32ProcessID;
                if (pid == 0 || pid == 4)
                    continue;

                ProcRow row;
                row.pid = pid;
                row.name = QString::fromWCharArray(pe.szExeFile);
                row.critical = isCritical(row.name, pid);

                HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                                       FALSE, pid);
                if (h) {
                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc)))
                        row.memoryMb = static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
                    CloseHandle(h);
                }
                rows.append(row);
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }

    std::sort(rows.begin(), rows.end(),
              [](const ProcRow &a, const ProcRow &b) { return a.memoryMb > b.memoryMb; });

    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
    emit countChanged();
}

QString TaskKillerModel::kill(int pid)
{
    // Cherche la ligne pour vérifier la protection.
    auto it = std::find_if(m_rows.begin(), m_rows.end(),
                           [pid](const ProcRow &r) { return r.pid == static_cast<quint32>(pid); });
    if (it == m_rows.end()) {
        setLastMessage(QStringLiteral("Processus introuvable"));
        return m_lastMessage;
    }
    if (it->critical || pid == 0 || pid == 4) {
        setLastMessage(QStringLiteral("%1 : processus critique (protégé)").arg(it->name));
        return m_lastMessage;
    }

    const QString name = it->name;
    HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, static_cast<DWORD>(pid));
    if (!h) {
        setLastMessage(QStringLiteral("%1 : accès refusé").arg(name));
        return m_lastMessage;
    }
    if (!TerminateProcess(h, 1)) {
        CloseHandle(h);
        setLastMessage(QStringLiteral("%1 : échec (protégé ?)").arg(name));
        return m_lastMessage;
    }
    const DWORD wait = WaitForSingleObject(h, 3000);
    CloseHandle(h);

    if (wait == WAIT_OBJECT_0) {
        setLastMessage(QStringLiteral("%1 : terminé").arg(name));
        refresh();
    } else if (wait == WAIT_TIMEOUT) {
        setLastMessage(QStringLiteral("%1 : timeout (peut se relancer)").arg(name));
    } else {
        setLastMessage(QStringLiteral("%1 : erreur").arg(name));
    }
    return m_lastMessage;
}

void TaskKillerModel::setLastMessage(const QString &m)
{
    if (m_lastMessage == m)
        return;
    m_lastMessage = m;
    emit lastMessageChanged();
}
