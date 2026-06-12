#pragma once

#include <QtQml/qqmlregistration.h>
#include <QVariantList>
#include "metric_provider.h"

// Provider Disk : utilisation des disques fixes.
// `disks` = liste d'objets { name: "C:", usagePercent: 73.4 } consommable en QML
// (Repeater / modèle). `primaryUsagePercent` = raccourci sur le premier disque.
class DiskProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(QVariantList disks READ disks NOTIFY disksChanged)
    Q_PROPERTY(double primaryUsagePercent READ primaryUsagePercent NOTIFY disksChanged)

public:
    explicit DiskProvider(QObject *parent = nullptr) : MetricProvider(parent) {}

    QVariantList disks() const { return m_disks; }
    double primaryUsagePercent() const { return m_primary; }

    void poll() override;

signals:
    void disksChanged();

private:
    QVariantList m_disks;
    double m_primary = 0.0;
};
