#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Liste des processus (nom, PID, mémoire) triés par mémoire décroissante, avec
// kill robuste (protection des processus critiques). Singleton QML `TaskKiller`.
// Porté de Widget-perf_overlay/src/taskkiller.c (CreateToolhelp32Snapshot +
// GetProcessMemoryInfo + TerminateProcess/WaitForSingleObject).
class TaskKillerModel : public QAbstractListModel
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TaskKiller)
    QML_SINGLETON

    Q_PROPERTY(QString lastMessage READ lastMessage NOTIFY lastMessageChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles { NameRole = Qt::UserRole + 1, PidRole, MemoryRole, CriticalRole };

    explicit TaskKillerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString lastMessage() const { return m_lastMessage; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString kill(int pid);

signals:
    void lastMessageChanged();
    void countChanged();

private:
    struct ProcRow {
        QString name;
        quint32 pid = 0;
        double memoryMb = 0.0;
        bool critical = false;
    };

    bool isCritical(const QString &name, quint32 pid) const;
    void setLastMessage(const QString &m);

    QList<ProcRow> m_rows;
    QString m_lastMessage;
};
