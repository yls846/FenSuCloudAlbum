#include "core/model/MediaModel.h"
#include "core/database/DatabaseManager.h"

#include <QLocale>
#include <QDebug>

MediaModel::MediaModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void MediaModel::setDatabase(DatabaseManager *database)
{
    m_database = database;
    reload();
}

int MediaModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant MediaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const MediaItem &item = m_items.at(index.row());

    switch (role) {
    case IdRole:                return item.id;
    case AlbumIdRole:           return item.albumId;
    case FileNameRole:          return item.fileName;
    case DisplayNameRole:       return item.displayName.isEmpty() ? item.fileName : item.displayName;
    case RelativePathRole:      return item.relativePath;
    case MimeTypeRole:          return item.mimeType;
    case FileSizeRole:          return item.fileSize;
    case WidthRole:             return item.width;
    case HeightRole:            return item.height;
    case DurationMsRole:        return item.durationMs;
    case OriginalDateTakenRole: return item.originalDateTaken;
    case DisplayDateTakenRole:  return item.displayDateTaken;
    case FavoriteRole:          return item.favorite;
    case IsVideoRole:           return item.isVideo;
    case ThumbnailPathRole:     return item.thumbnailPath;
    case FileSizeTextRole:      return formatFileSize(item.fileSize);
    default:                    return {};
    }
}

QHash<int, QByteArray> MediaModel::roleNames() const
{
    return {
        { IdRole,                "mediaId" },
        { AlbumIdRole,           "albumId" },
        { FileNameRole,          "fileName" },
        { DisplayNameRole,       "displayName" },
        { RelativePathRole,      "relativePath" },
        { MimeTypeRole,          "mimeType" },
        { FileSizeRole,          "fileSize" },
        { WidthRole,             "width" },
        { HeightRole,            "height" },
        { DurationMsRole,        "durationMs" },
        { OriginalDateTakenRole, "originalDateTaken" },
        { DisplayDateTakenRole,  "displayDateTaken" },
        { FavoriteRole,          "favorite" },
        { IsVideoRole,           "isVideo" },
        { ThumbnailPathRole,     "thumbnailPath" },
        { FileSizeTextRole,      "fileSizeText" },
    };
}

void MediaModel::reload()
{
    beginResetModel();
    m_items.clear();
    if (m_database && m_database->isOpen())
        m_items = m_database->fetchAllMedia();
    endResetModel();

    emit countChanged();
}

void MediaModel::clear()
{
    beginResetModel();
    m_items.clear();
    endResetModel();

    emit countChanged();
}

bool MediaModel::removeById(qint64 id)
{
    for (int row = 0; row < m_items.size(); ++row) {
        if (m_items.at(row).id != id)
            continue;
        beginRemoveRows(QModelIndex(), row, row);
        m_items.remove(row);
        endRemoveRows();
        emit countChanged();
        return true;
    }
    return false;
}

QString MediaModel::fileSizeText(qint64 bytes) const
{
    return formatFileSize(bytes);
}

QString MediaModel::formatFileSize(qint64 bytes)
{
    // 64-bit safe formatter: 5 GB+ files must print correctly.
    constexpr qint64 kKiB = 1024;
    constexpr qint64 kMiB = kKiB * 1024;
    constexpr qint64 kGiB = kMiB * 1024;
    constexpr qint64 kTiB = kGiB * 1024;

    const qint64 abs = bytes < 0 ? -bytes : bytes;

    if (abs < kKiB)
        return QStringLiteral("%1 B").arg(bytes);
    if (abs < kMiB)
        return QStringLiteral("%1 KB").arg(QLocale::system().toString(double(bytes) / kKiB, 'f', 1));
    if (abs < kGiB)
        return QStringLiteral("%1 MB").arg(QLocale::system().toString(double(bytes) / kMiB, 'f', 1));
    if (abs < kTiB)
        return QStringLiteral("%1 GB").arg(QLocale::system().toString(double(bytes) / kGiB, 'f', 2));

    return QStringLiteral("%1 TB").arg(QLocale::system().toString(double(bytes) / kTiB, 'f', 2));
}