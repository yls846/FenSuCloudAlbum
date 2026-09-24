#include "core/model/TimelineModel.h"
#include "core/database/DatabaseManager.h"
#include "core/model/MediaItem.h"

#include <QLocale>
#include <QMap>

#include <algorithm>

namespace {

// Deterministic, locale-independent cache key for grouping.
QDate displayDateOf(const MediaItem &item)
{
    if (item.displayDateTaken.isValid())
        return item.displayDateTaken.toLocalTime().date();
    if (item.originalDateTaken.isValid())
        return item.originalDateTaken.toLocalTime().date();

    // Undated media must still show up somewhere: park it on the epoch date.
    return QDate(1970, 1, 1);
}

} // namespace

TimelineModel::TimelineModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void TimelineModel::setDatabase(DatabaseManager *database)
{
    m_database = database;
    reload();
}

int TimelineModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_sections.size();
}

QVariant TimelineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_sections.size())
        return {};

    const DaySection &section = m_sections.at(index.row());

    switch (role) {
    case DateRole:      return section.date;
    case TitleRole:     return section.title;
    case SubtitleRole:  return QStringLiteral("%1 项").arg(section.itemCount);
    case ItemCountRole: return section.itemCount;
    case CoverPathRole: return section.coverPath;
    case IsTodayRole:   return section.isToday;
    default:            return {};
    }
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    return {
        { DateRole,      "sectionDate" },
        { TitleRole,     "title" },
        { SubtitleRole,  "subtitle" },
        { ItemCountRole, "itemCount" },
        { CoverPathRole, "coverPath" },
        { IsTodayRole,   "isToday" },
    };
}

void TimelineModel::reload()
{
    beginResetModel();
    m_sections.clear();

    if (m_database && m_database->isOpen()) {
        const QVector<MediaItem> items = m_database->fetchAllMedia();

        // QMap keeps the dates sorted ascending; we reverse at the end so the
        // newest day appears first, matching Google Photos behaviour.
        QMap<QDate, DaySection> grouped;

        for (const MediaItem &item : items) {
            const QDate date = displayDateOf(item);
            DaySection &section = grouped[date];
            if (!section.date.isValid()) {
                section.date = date;
                section.title = QLocale::system().toString(date, QLocale::LongFormat);
                section.isToday = (date == QDate::currentDate());
            }
            section.itemCount += 1;
            if (section.coverPath.isEmpty() && !item.thumbnailPath.isEmpty())
                section.coverPath = item.thumbnailPath;
        }

        m_sections.reserve(grouped.size());
        for (auto it = grouped.constEnd(); it != grouped.constBegin();) {
            --it;
            m_sections.push_back(it.value());
        }
    }

    endResetModel();
    emit countChanged();
}

void TimelineModel::clear()
{
    beginResetModel();
    m_sections.clear();
    endResetModel();

    emit countChanged();
}