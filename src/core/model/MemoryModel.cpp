#include "core/model/MemoryModel.h"
#include "core/database/DatabaseManager.h"
#include "core/model/MediaItem.h"

#include <QLocale>
#include <QMap>

MemoryModel::MemoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void MemoryModel::setDatabase(DatabaseManager *database)
{
    m_database = database;
    reload();
}

int MemoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.size();
}

QVariant MemoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const MemoryEntry &entry = m_entries.at(index.row());

    switch (role) {
    case TitleRole:     return entry.title;
    case DateRole:      return entry.date;
    case YearOffsetRole:return entry.yearOffset;
    case ItemCountRole: return entry.itemCount;
    case CoverPathRole: return entry.coverPath;
    default:            return {};
    }
}

QHash<int, QByteArray> MemoryModel::roleNames() const
{
    return {
        { TitleRole,      "title" },
        { DateRole,       "memoryDate" },
        { YearOffsetRole, "yearOffset" },
        { ItemCountRole,  "itemCount" },
        { CoverPathRole,  "coverPath" },
    };
}

void MemoryModel::reload()
{
    beginResetModel();
    m_entries.clear();

    if (m_database && m_database->isOpen()) {
        const QVector<MediaItem> items = m_database->fetchAllMedia();
        const QDate today = QDate::currentDate();

        QMap<QDate, MemoryEntry> grouped;

        for (const MediaItem &item : items) {
            QDate date = item.displayDateTaken.isValid()
                             ? item.displayDateTaken.toLocalTime().date()
                             : item.originalDateTaken.toLocalTime().date();
            if (!date.isValid())
                continue;

            MemoryEntry &entry = grouped[date];
            if (entry.date.isNull()) {
                entry.date = date;
                entry.yearOffset = today.year() - date.year();
                entry.title = entry.yearOffset == 0
                                  ? QLocale::system().toString(date, QLocale::LongFormat)
                                  : QStringLiteral("%1 年 · %2")
                                        .arg(entry.yearOffset)
                                        .arg(QLocale::system().toString(date, QLocale::LongFormat));
            }
            entry.itemCount += 1;
            if (entry.coverPath.isEmpty() && !item.thumbnailPath.isEmpty())
                entry.coverPath = item.thumbnailPath;
        }

        // Newest first, and drop the year offset == 0 entries only if there is
        // nothing else to show, so the tab is never completely empty.
        m_entries.reserve(grouped.size());
        for (auto it = grouped.constEnd(); it != grouped.constBegin();) {
            --it;
            m_entries.push_back(it.value());
        }
    }

    endResetModel();
    emit countChanged();
}

void MemoryModel::clear()
{
    beginResetModel();
    m_entries.clear();
    endResetModel();

    emit countChanged();
}