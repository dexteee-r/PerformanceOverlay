#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

// Lancement au démarrage Windows via la clé de registre
// HKCU\Software\Microsoft\Windows\CurrentVersion\Run (valeur "PerformanceOverlay").
// Singleton QML `Startup`.
class StartupService : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Startup)
    QML_SINGLETON

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit StartupService(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool on);

signals:
    void enabledChanged();

private:
    bool readEnabled() const;
    bool m_enabled = false;
};
