#include "network_provider.h"

#include <QNetworkInterface>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2ipdef.h>
#include <windows.h>
#include <iphlpapi.h>
#include <netioapi.h>   // GetIfTable2 / MIB_IF_TABLE2 (pas tirés par iphlpapi.h sur MinGW)

namespace {
constexpr qint64 kIpRefreshMs = 30000;   // l'IP change rarement
}

NetworkProvider::NetworkProvider(QObject *parent) : MetricProvider(parent)
{
    m_ip = resolveLocalIp();
    m_since.start();
}

void NetworkProvider::poll()
{
    sampleThroughput();   // débit : à chaque tick

    if (!m_since.isValid() || m_since.elapsed() >= kIpRefreshMs) {
        m_since.restart();
        const QString ip = resolveLocalIp();
        if (ip != m_ip) {
            m_ip = ip;
            emit ipAddressChanged();
        }
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

void NetworkProvider::sampleThroughput()
{
    // Somme des octets in/out de toutes les interfaces actives non-loopback.
    quint64 totalIn = 0, totalOut = 0;
    PMIB_IF_TABLE2 table = nullptr;
    if (GetIfTable2(&table) == NO_ERROR && table) {
        for (ULONG i = 0; i < table->NumEntries; ++i) {
            const MIB_IF_ROW2 &r = table->Table[i];
            if (r.OperStatus != IfOperStatusUp)
                continue;
            if (r.Type == IF_TYPE_SOFTWARE_LOOPBACK)
                continue;
            totalIn += r.InOctets;
            totalOut += r.OutOctets;
        }
    }
    if (table)
        FreeMibTable(table);

    if (m_firstRate) {
        m_lastIn = totalIn;
        m_lastOut = totalOut;
        m_rateTimer.start();
        m_firstRate = false;
        return;
    }

    const double secs = qMax(0.001, m_rateTimer.restart() / 1000.0);
    const double down = static_cast<double>(totalIn - m_lastIn) / secs;
    const double up = static_cast<double>(totalOut - m_lastOut) / secs;
    m_lastIn = totalIn;
    m_lastOut = totalOut;

    m_down = down;
    m_up = up;
    m_downHist.push(down);
    emit ratesChanged();
}
