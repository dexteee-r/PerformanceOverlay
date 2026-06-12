#pragma once

#include <QList>
#include <QVariantList>

// Tampon circulaire de doubles pour les historiques de métriques (sparklines).
// Capacité fixe : pousse une valeur par tick, évince la plus ancienne.
class History
{
public:
    explicit History(int capacity = 60) : m_capacity(capacity) {}

    void push(double v)
    {
        m_data.append(v);
        while (m_data.size() > m_capacity)
            m_data.removeFirst();
    }

    // Exposé au QML (le Sparkline lit cette liste de nombres).
    QVariantList toVariantList() const
    {
        QVariantList list;
        list.reserve(m_data.size());
        for (double v : m_data)
            list.append(v);
        return list;
    }

private:
    int m_capacity;
    QList<double> m_data;
};
