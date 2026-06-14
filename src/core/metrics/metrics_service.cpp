#include "metrics_service.h"

#include <QTimer>

MetricsService::MetricsService(QObject *parent)
    : QObject(parent)
    , m_cpu(new CpuProvider(this))
    , m_ram(new RamProvider(this))
    , m_gpu(new GpuProvider(this))
    , m_disk(new DiskProvider(this))
    , m_uptime(new UptimeProvider(this))
    , m_dateTime(new DateTimeProvider(this))
    , m_process(new ProcessProvider(this))
    , m_network(new NetworkProvider(this))
    , m_volume(new VolumeProvider(this))
    , m_prayer(new PrayerProvider(this))
    , m_claudeUsage(new ClaudeUsageProvider(this))
    , m_timer(new QTimer(this))
{
    m_providers = {m_cpu, m_ram, m_gpu, m_disk, m_uptime, m_dateTime,
                   m_process, m_network, m_volume, m_prayer, m_claudeUsage};

    connect(m_timer, &QTimer::timeout, this, &MetricsService::pollAll);
    m_timer->start(m_intervalMs);
    pollAll();   // valeurs immédiates (le CPU se cale au tick suivant)
}

void MetricsService::setIntervalMs(int ms)
{
    if (ms <= 0 || ms == m_intervalMs)
        return;
    m_intervalMs = ms;
    m_timer->setInterval(ms);
    emit intervalMsChanged();
}

void MetricsService::pollAll()
{
    for (MetricProvider *p : m_providers)
        p->poll();

    // Charge système = mélange CPU/GPU (le GPU retombe sur le CPU si indispo).
    const double gpu = m_gpu->available() ? m_gpu->usagePercent() : m_cpu->usagePercent();
    m_systemLoad = (m_cpu->usagePercent() * 0.6 + gpu * 0.4) / 100.0;
    m_loadHistory.push(m_systemLoad * 100.0);   // historique en 0..100 (comme cpu/gpu)

    // Historique long de la charge (240 pts ≈ 8 min) pour le LineChart du cockpit.
    m_loadHistoryLong.push(m_systemLoad * 100.0);

    emit systemLoadChanged();
}
