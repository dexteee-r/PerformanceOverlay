#pragma once

#include <QObject>

// Interface commune à tous les providers de métriques.
// Un provider encapsule UNE source (CPU, RAM, GPU…) : il collecte la donnée
// brute (Win32 / NVML) et l'expose via des Q_PROPERTY + signaux NOTIFY.
// Aucune logique d'affichage ici — uniquement de l'acquisition.
class MetricProvider : public QObject
{
    Q_OBJECT
public:
    explicit MetricProvider(QObject *parent = nullptr) : QObject(parent) {}

    // Rafraîchit les valeurs depuis la source système et émet les NOTIFY
    // pour les propriétés qui ont changé. Appelé par MetricsService.
    virtual void poll() = 0;
};
