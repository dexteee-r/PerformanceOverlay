#include "datetime_provider.h"

void DateTimeProvider::poll()
{
    // Toujours notifier : l'heure change à chaque tick.
    m_now = QDateTime::currentDateTime();
    emit nowChanged();
}
