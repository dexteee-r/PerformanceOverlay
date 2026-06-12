#include "startup_service.h"

#include <QCoreApplication>
#include <QDir>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

namespace {
const wchar_t *kRunKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const wchar_t *kValueName = L"PerformanceOverlay";
}

StartupService::StartupService(QObject *parent) : QObject(parent)
{
    m_enabled = readEnabled();
}

bool StartupService::readEnabled() const
{
    HKEY hKey;
    bool present = false;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        present = (RegQueryValueExW(hKey, kValueName, nullptr, nullptr, nullptr, nullptr)
                   == ERROR_SUCCESS);
        RegCloseKey(hKey);
    }
    return present;
}

void StartupService::setEnabled(bool on)
{
    if (on == m_enabled)
        return;

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (on) {
            const QString path = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
            const std::wstring value = QStringLiteral("\"%1\"").arg(path).toStdWString();
            RegSetValueExW(hKey, kValueName, 0, REG_SZ,
                           reinterpret_cast<const BYTE *>(value.c_str()),
                           static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
        } else {
            RegDeleteValueW(hKey, kValueName);
        }
        RegCloseKey(hKey);
    }

    m_enabled = readEnabled();   // relit pour confirmer l'état réel
    emit enabledChanged();
}
