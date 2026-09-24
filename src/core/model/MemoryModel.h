#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QVector>

class DatabaseManager;

// ---------------------------------------------------------------------------
// MemoryModel
//
// "Memories" tab. In the D version memories are derived, not AI generated:
// a memory is a past day that has media attached to it. The table already
// exists in the schema (memories), so a later release can persist richer
// memories without a migration.
// ---------------------------------------------------------------------------
class MemoryModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        DateRole,
        YearOffsetRole,
        ItemCountRole,
        CoverPathRole,
    };
    Q_ENUM(Roles)

    struct MemoryEntry {
        QString title;
        QDate date;
        int yearOffset = 0; // 0 = this year, 1 = "1 年前的今天", ...
        qint64 itemCount = 0;
        QString coverPath;
    };

    explicit MemoryModel(QObject *parent = nullptr);

    void setDatabase(DatabaseManager *database);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void clear();

    int count() const { return m_entries.size(); }

signals:
    void countChanged();

private:
    QVector<MemoryEntry> m_entries;
    DatabaseManager *m_database = nullptr;
};