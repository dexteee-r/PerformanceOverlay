#pragma once

#include <QtQml/qqmlregistration.h>
#include <QVariantList>
#include "metric_provider.h"
#include "history.h"

// Provider CPU : utilisation globale (%) et fréquence (GHz).
// Porté de Widget-perf_overlay/src/plugins/plugin_cpu.c (GetSystemTimes +
// registre ~MHz). Créé uniquement par MetricsService → non instanciable en QML.
class CpuProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(double usagePercent READ usagePercent NOTIFY usagePercentChanged)
    Q_PROPERTY(double frequencyGhz READ frequencyGhz NOTIFY frequencyGhzChanged)
    Q_PROPERTY(QVariantList usageHistory READ usageHistory NOTIFY usageHistoryChanged)

public:
    explicit CpuProvider(QObject *parent = nullptr);

    double usagePercent() const { return m_usagePercent; }
    double frequencyGhz() const { return m_frequencyGhz; }
    QVariantList usageHistory() const { return m_history.toVariantList(); }

    void poll() override;

signals:
    void usagePercentChanged();
    void frequencyGhzChanged();
    void usageHistoryChanged();

private:
    void setUsagePercent(double v);
    void setFrequencyGhz(double v);

    // État pour le calcul différentiel (ticks cumulés du dernier échantillon).
    quint64 m_lastIdle = 0;
    quint64 m_lastKernel = 0;
    quint64 m_lastUser = 0;

    double m_usagePercent = 0.0;
    double m_frequencyGhz = 0.0;
    History m_history;
};
