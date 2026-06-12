#pragma once

#include <QObject>
#include <QList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include "history.h"
#include "cpu_provider.h"
#include "ram_provider.h"
#include "gpu_provider.h"
#include "disk_provider.h"
#include "uptime_provider.h"
#include "datetime_provider.h"
#include "process_provider.h"
#include "network_provider.h"
#include "volume_provider.h"
#include "prayer_provider.h"
#include "claude_usage_provider.h"

class QTimer;

// Façade exposée au QML : possède tous les providers et pilote leur polling via
// un QTimer unique. Singleton accessible en QML sous le nom `Metrics` :
//   Metrics.cpu.usagePercent, Metrics.ram.usedGb, Metrics.gpu.temperatureC,
//   Metrics.disk.disks, Metrics.uptime.seconds, Metrics.dateTime.now,
//   Metrics.process.count, Metrics.network.ipAddress, Metrics.volume.level,
//   Metrics.prayer.nextName, Metrics.claudeUsage.weekPercent, …
class MetricsService : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Metrics)
    QML_SINGLETON

    Q_PROPERTY(CpuProvider *cpu READ cpu CONSTANT)
    Q_PROPERTY(RamProvider *ram READ ram CONSTANT)
    Q_PROPERTY(GpuProvider *gpu READ gpu CONSTANT)
    Q_PROPERTY(DiskProvider *disk READ disk CONSTANT)
    Q_PROPERTY(UptimeProvider *uptime READ uptime CONSTANT)
    Q_PROPERTY(DateTimeProvider *dateTime READ dateTime CONSTANT)
    Q_PROPERTY(ProcessProvider *process READ process CONSTANT)
    Q_PROPERTY(NetworkProvider *network READ network CONSTANT)
    Q_PROPERTY(VolumeProvider *volume READ volume CONSTANT)
    Q_PROPERTY(PrayerProvider *prayer READ prayer CONSTANT)
    Q_PROPERTY(ClaudeUsageProvider *claudeUsage READ claudeUsage CONSTANT)
    Q_PROPERTY(double systemLoad READ systemLoad NOTIFY systemLoadChanged)               // 0..1
    Q_PROPERTY(QVariantList systemLoadHistory READ systemLoadHistory NOTIFY systemLoadChanged)
    Q_PROPERTY(int intervalMs READ intervalMs WRITE setIntervalMs NOTIFY intervalMsChanged)

public:
    explicit MetricsService(QObject *parent = nullptr);

    CpuProvider *cpu() const { return m_cpu; }
    RamProvider *ram() const { return m_ram; }
    GpuProvider *gpu() const { return m_gpu; }
    DiskProvider *disk() const { return m_disk; }
    UptimeProvider *uptime() const { return m_uptime; }
    DateTimeProvider *dateTime() const { return m_dateTime; }
    ProcessProvider *process() const { return m_process; }
    NetworkProvider *network() const { return m_network; }
    VolumeProvider *volume() const { return m_volume; }
    PrayerProvider *prayer() const { return m_prayer; }
    ClaudeUsageProvider *claudeUsage() const { return m_claudeUsage; }

    double systemLoad() const { return m_systemLoad; }
    QVariantList systemLoadHistory() const { return m_loadHistory.toVariantList(); }

    int intervalMs() const { return m_intervalMs; }
    void setIntervalMs(int ms);

signals:
    void intervalMsChanged();
    void systemLoadChanged();

private:
    void pollAll();

    CpuProvider *m_cpu;
    RamProvider *m_ram;
    GpuProvider *m_gpu;
    DiskProvider *m_disk;
    UptimeProvider *m_uptime;
    DateTimeProvider *m_dateTime;
    ProcessProvider *m_process;
    NetworkProvider *m_network;
    VolumeProvider *m_volume;
    PrayerProvider *m_prayer;
    ClaudeUsageProvider *m_claudeUsage;

    QList<MetricProvider *> m_providers;
    QTimer *m_timer;
    int m_intervalMs = 2000;   // défaut config.ini (refresh 2000 ms)

    double m_systemLoad = 0.0;
    History m_loadHistory;
};
