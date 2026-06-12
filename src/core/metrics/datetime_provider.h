#pragma once

#include <QtQml/qqmlregistration.h>
#include <QDateTime>
#include "metric_provider.h"

// Provider DateTime : horodatage courant. Le formatage (jour/mois localisés,
// couleur selon l'heure) est laissé au QML (QLocale / Qt.formatDateTime).
class DateTimeProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(QDateTime now READ now NOTIFY nowChanged)

public:
    explicit DateTimeProvider(QObject *parent = nullptr) : MetricProvider(parent) {}

    QDateTime now() const { return m_now; }

    void poll() override;

signals:
    void nowChanged();

private:
    QDateTime m_now;
};
