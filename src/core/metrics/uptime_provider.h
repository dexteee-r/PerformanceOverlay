#pragma once

#include <QtQml/qqmlregistration.h>
#include "metric_provider.h"

// Provider Uptime : temps écoulé depuis le démarrage système, en secondes.
// Le formatage (jours/heures/min) est laissé au QML.
class UptimeProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(qint64 seconds READ seconds NOTIFY secondsChanged)

public:
    explicit UptimeProvider(QObject *parent = nullptr) : MetricProvider(parent) {}

    qint64 seconds() const { return m_seconds; }

    void poll() override;

signals:
    void secondsChanged();

private:
    qint64 m_seconds = 0;
};
