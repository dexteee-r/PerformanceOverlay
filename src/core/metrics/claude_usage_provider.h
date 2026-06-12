#pragma once

#include <QtQml/qqmlregistration.h>
#include <QElapsedTimer>
#include "metric_provider.h"

// Provider ClaudeUsage : conso de tokens Claude Code locale, en % de TA moyenne.
// Scanne ~/.claude/projects/<session>/*.jsonl, agrège les tokens par jour sur
// 28 jours (dédup requestId+message.id), et calcule :
//   dayPercent  = tokens_aujourdhui / (somme_28j / 28) * 100
//   weekPercent = tokens_7j        / (somme_28j / 4)  * 100
// Re-scan throttlé à 1×/minute. Reflète CETTE machine, pas le quota Anthropic.
class ClaudeUsageProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(bool hasData READ hasData NOTIFY changed)
    Q_PROPERTY(double dayPercent READ dayPercent NOTIFY changed)
    Q_PROPERTY(double weekPercent READ weekPercent NOTIFY changed)

public:
    explicit ClaudeUsageProvider(QObject *parent = nullptr) : MetricProvider(parent) {}

    bool hasData() const { return m_hasData; }
    double dayPercent() const { return m_dayPercent; }
    double weekPercent() const { return m_weekPercent; }

    void poll() override;

signals:
    void changed();

private:
    void scan();

    bool m_hasData = false;
    double m_dayPercent = 0.0;
    double m_weekPercent = 0.0;

    QElapsedTimer m_since;
    bool m_firstScan = true;
};
