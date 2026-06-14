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
    Q_PROPERTY(int threadCount READ threadCount NOTIFY threadCountChanged)

public:
    explicit ProcessProvider(QObject *parent = nullptr) : MetricProvider(parent) {}

    int count() const { return m_count; }
    int threadCount() const { return m_threadCount; }

    void poll() override;

signals:
    void countChanged();
    void threadCountChanged();

private:
    int m_count = 0;
    int m_threadCount = 0;
};
