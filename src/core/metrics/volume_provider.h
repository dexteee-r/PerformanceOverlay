#pragma once

#include <QtQml/qqmlregistration.h>
#include <memory>
#include "metric_provider.h"

// Provider Volume : niveau du périphérique de sortie par défaut (0-100) + mute.
// Porté de plugin_volume.c (Windows Core Audio). L'état COM est en PIMPL pour
// garder mmdeviceapi/endpointvolume hors de l'en-tête.
class VolumeProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(double level READ level NOTIFY levelChanged)   // 0..100
    Q_PROPERTY(bool muted READ muted NOTIFY mutedChanged)

public:
    explicit VolumeProvider(QObject *parent = nullptr);
    ~VolumeProvider() override;

    bool available() const { return m_available; }
    double level() const { return m_level; }
    bool muted() const { return m_muted; }

    void poll() override;

signals:
    void availableChanged();
    void levelChanged();
    void mutedChanged();

private:
    struct Com;                       // PIMPL : interfaces Core Audio
    std::unique_ptr<Com> m_com;

    bool m_available = false;
    double m_level = 0.0;
    bool m_muted = false;
};
