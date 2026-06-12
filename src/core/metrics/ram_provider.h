#pragma once

#include <QtQml/qqmlregistration.h>
#include "metric_provider.h"

// Provider RAM : charge (%), Go utilisés et Go totaux.
// Porté de Widget-perf_overlay/src/plugins/plugin_ram.c (GlobalMemoryStatusEx).
class RamProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(double usagePercent READ usagePercent NOTIFY usagePercentChanged)
    Q_PROPERTY(double usedGb READ usedGb NOTIFY usedGbChanged)
    Q_PROPERTY(double totalGb READ totalGb NOTIFY totalGbChanged)

public:
    explicit RamProvider(QObject *parent = nullptr);

    double usagePercent() const { return m_usagePercent; }
    double usedGb() const { return m_usedGb; }
    double totalGb() const { return m_totalGb; }

    void poll() override;

signals:
    void usagePercentChanged();
    void usedGbChanged();
    void totalGbChanged();

private:
    void setUsagePercent(double v);
    void setUsedGb(double v);
    void setTotalGb(double v);

    double m_usagePercent = 0.0;
    double m_usedGb = 0.0;
    double m_totalGb = 0.0;
};
