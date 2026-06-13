#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QVariantList>
#include <QElapsedTimer>
#include "metric_provider.h"
#include "history.h"

// Provider Network : IPv4 locale + débit instantané ↓/↑ (octets/s).
// IP rafraîchie ≤ 1×/30 s (QNetworkInterface) ; débit échantillonné à chaque
// tick (GetIfTable2 : octets cumulés in/out → delta / temps écoulé).
class NetworkProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(QString ipAddress READ ipAddress NOTIFY ipAddressChanged)
    Q_PROPERTY(double downBytesPerSec READ downBytesPerSec NOTIFY ratesChanged)
    Q_PROPERTY(double upBytesPerSec READ upBytesPerSec NOTIFY ratesChanged)
    Q_PROPERTY(QVariantList downHistory READ downHistory NOTIFY ratesChanged)

public:
    explicit NetworkProvider(QObject *parent = nullptr);

    QString ipAddress() const { return m_ip; }
    double downBytesPerSec() const { return m_down; }
    double upBytesPerSec() const { return m_up; }
    QVariantList downHistory() const { return m_downHist.toVariantList(); }

    void poll() override;

signals:
    void ipAddressChanged();
    void ratesChanged();

private:
    QString resolveLocalIp() const;
    void sampleThroughput();

    QString m_ip = QStringLiteral("N/A");
    QElapsedTimer m_since;

    double m_down = 0.0;
    double m_up = 0.0;
    History m_downHist;
    quint64 m_lastIn = 0;
    quint64 m_lastOut = 0;
    bool m_firstRate = true;
    QElapsedTimer m_rateTimer;
};
