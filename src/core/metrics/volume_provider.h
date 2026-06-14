#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QVariantList>
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
    Q_PROPERTY(double peakLevel READ peakLevel NOTIFY peakLevelChanged)   // 0..1 (VU sortie)
    Q_PROPERTY(double micLevel READ micLevel NOTIFY micLevelChanged)      // 0..1 (niveau micro)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)

public:
    explicit VolumeProvider(QObject *parent = nullptr);
    ~VolumeProvider() override;

    bool available() const { return m_available; }
    double level() const { return m_level; }
    bool muted() const { return m_muted; }
    double peakLevel() const { return m_peak; }
    double micLevel() const { return m_mic; }
    QString deviceName() const { return m_deviceName; }

    // Liste des micros actifs : [{ id, name }]. Pour le sélecteur des Réglages.
    Q_INVOKABLE QVariantList inputDevices() const;
    // Choisit le micro qui anime la sphère (id vide = défaut Windows).
    Q_INVOKABLE void useMicDevice(const QString &id);
    QString currentMicId() const { return m_micId; }

    void poll() override;

signals:
    void availableChanged();
    void levelChanged();
    void mutedChanged();
    void peakLevelChanged();
    void micLevelChanged();
    void deviceNameChanged();
    void micDeviceChanged();

private:
    void samplePeak();                // VU-mètre : lu à ~16 Hz par un timer interne

    struct Com;                       // PIMPL : interfaces Core Audio
    std::unique_ptr<Com> m_com;

    bool m_available = false;
    double m_level = 0.0;
    bool m_muted = false;
    double m_peak = 0.0;
    double m_mic = 0.0;
    QString m_deviceName;
    QString m_micId;          // id du micro courant ("" = défaut)
};
