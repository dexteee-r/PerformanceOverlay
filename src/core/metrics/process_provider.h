#pragma once

#include <QtQml/qqmlregistration.h>
#include "metric_provider.h"

// Provider Process : nombre de processus actifs.
class ProcessProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ProcessProvider(QObject *parent = nullptr) : MetricProvider(parent) {}

    int count() const { return m_count; }

    void poll() override;

signals:
    void countChanged();

private:
    int m_count = 0;
};
