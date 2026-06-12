#include "network_provider.h"

#include <QNetworkInterface>

namespace {
constexpr qint64 kRefreshMs = 30000;   // l'IP change rarement
}

NetworkProvider::NetworkProvider(QObject *parent) : MetricProvider(parent)
{
    m_ip = resolveLocalIp();
    m_since.start();
}

void NetworkProvider::poll()
{
    if (m_since.isValid() && m_since.elapsed() < kRefreshMs)
        return;
    m_since.restart();

    const QString ip = resolveLocalIp();
    if (ip != m_ip) {
        m_ip = ip;
        emit ipAddressChanged();
    }
}

QString NetworkProvider::resolveLocalIp() const
{
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        const auto flags = iface.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp)
            || !flags.testFlag(QNetworkInterface::IsRunning)
            || flags.testFlag(QNetworkInterface::IsLoopBack))
            continue;

        const auto entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            const QHostAddress ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            const QString s = ip.toString();
            if (s.startsWith(QStringLiteral("169.254.")) || ip.isLoopback())
                continue;
            return s;
        }
    }
    return QStringLiteral("No Network");
}
