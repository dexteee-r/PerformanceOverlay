#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QVariantList>
#include <memory>
#include "metric_provider.h"
#include "history.h"

// Provider GPU : utilisation (%), température (°C), conso (W), VRAM (Go).
// Porté de Widget-perf_overlay/src/plugins/plugin_gpu.c :
//  - NVIDIA : charge nvml.dll dynamiquement (livré avec le driver).
//  - Repli  : nom du GPU via le registre / EnumDisplayDevices (available=false).
// L'état NVML (HMODULE, pointeurs de fonctions) est encapsulé en PIMPL pour
// garder windows.h / NVML hors de l'en-tête.
class GpuProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(double usagePercent READ usagePercent NOTIFY usagePercentChanged)
    Q_PROPERTY(double temperatureC READ temperatureC NOTIFY temperatureCChanged)
    Q_PROPERTY(double powerW READ powerW NOTIFY powerWChanged)
    Q_PROPERTY(double vramUsedGb READ vramUsedGb NOTIFY vramUsedGbChanged)
    Q_PROPERTY(double vramTotalGb READ vramTotalGb NOTIFY vramTotalGbChanged)
    Q_PROPERTY(QVariantList usageHistory READ usageHistory NOTIFY usageHistoryChanged)

public:
    explicit GpuProvider(QObject *parent = nullptr);
    ~GpuProvider() override;

    bool available() const { return m_available; }
    QString name() const { return m_name; }
    double usagePercent() const { return m_usagePercent; }
    double temperatureC() const { return m_temperatureC; }
    double powerW() const { return m_powerW; }
    double vramUsedGb() const { return m_vramUsedGb; }
    double vramTotalGb() const { return m_vramTotalGb; }
    QVariantList usageHistory() const { return m_history.toVariantList(); }

    void poll() override;

signals:
    void availableChanged();
    void nameChanged();
    void usagePercentChanged();
    void temperatureCChanged();
    void powerWChanged();
    void vramUsedGbChanged();
    void vramTotalGbChanged();
    void usageHistoryChanged();

private:
    void setAvailable(bool v);
    void setName(const QString &v);
    void setUsagePercent(double v);
    void setTemperatureC(double v);
    void setPowerW(double v);
    void setVramUsedGb(double v);
    void setVramTotalGb(double v);

    struct Nvml;                      // PIMPL : détails NVML/Win32 dans le .cpp
    std::unique_ptr<Nvml> m_nvml;

    bool m_available = false;
    QString m_name = QStringLiteral("Unknown GPU");
    double m_usagePercent = 0.0;
    double m_temperatureC = 0.0;
    double m_powerW = 0.0;
    double m_vramUsedGb = 0.0;
    double m_vramTotalGb = 0.0;
    History m_history;
};
