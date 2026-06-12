#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QElapsedTimer>
#include "metric_provider.h"

// Provider Network : adresse IPv4 locale principale (hors loopback / APIPA).
// Rafraîchie au plus toutes les 30 s (l'IP change rarement).
class NetworkProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY ipAddressChanged)

public:
    explicit NetworkProvider(QObject *parent = nullptr);

    QString ipAddress() const { return m_ip; }

    void poll() override;

signals:
    void ipAddressChanged();

private:
    QString resolveLocalIp() const;

    QString m_ip = QStringLiteral("N/A");
    QElapsedTimer m_since;
};
