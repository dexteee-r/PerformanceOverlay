#include "datetime_provider.h"

#include <QTimer>

DateTimeProvider::DateTimeProvider(QObject *parent) : MetricProvider(parent)
{
    m_now = QDateTime::currentDateTime();
    // Horloge à la seconde, indépendante de l'intervalle de polling (2 s) →
    // les secondes avancent en continu au lieu de sauter par 2.
    auto *clock = new QTimer(this);
    clock->setInterval(1000);
    connect(clock, &QTimer::timeout, this, [this] {
        m_now = QDateTime::currentDateTime();
        emit nowChanged();
    });
    clock->start();
}

void DateTimeProvider::poll()
{
    m_now = QDateTime::currentDateTime();
    emit nowChanged();
}
