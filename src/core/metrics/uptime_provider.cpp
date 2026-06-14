#include "uptime_provider.h"

#include <QTimer>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

UptimeProvider::UptimeProvider(QObject *parent) : MetricProvider(parent)
{
    poll();
    // Tic à la seconde, indépendant de l'intervalle de polling (2 s) → l'uptime
    // affiche des secondes qui avancent en continu au lieu de sauter.
    auto *clock = new QTimer(this);
    clock->setInterval(1000);
    connect(clock, &QTimer::timeout, this, &UptimeProvider::poll);
    clock->start();
}

void UptimeProvider::poll()
{
    const qint64 s = static_cast<qint64>(GetTickCount64() / 1000ULL);
    if (s != m_seconds) {
        m_seconds = s;
        emit secondsChanged();
    }
}
