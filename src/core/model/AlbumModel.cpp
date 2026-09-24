#include "core/model/AlbumModel.h"
#include "core/database/DatabaseManager.h"

AlbumModel::AlbumModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void AlbumModel::setDatabase(DatabaseManager *database)
{
    m_database = database;
    reload();
}

int AlbumModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_albums.size();
}

QVariant AlbumModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_albums.size())
        return {};

    const AlbumItem &album = m_albums.at(index.row());

    switch (role) {
    case IdRole:            return album.id;
    case NameRole:          return album.name;
    case RelativePathRole:  return album.relativePath;
    case CoverMediaIdRole:  return album.coverMediaId;
    case MediaCountRole:    return album.mediaCount;
    case UpdatedAtRole:     return album.updatedAt;
    default:                return {};
    }
}

QHash<int, QByteArray> AlbumModel::roleNames() const
{
    return {
        { IdRole,           "albumId" },
        { NameRole,         "name" },
        { RelativePathRole, "relativePath" },
        { CoverMediaIdRole, "coverMediaId" },
        { MediaCountRole,   "mediaCount" },
        { UpdatedAtRole,    "updatedAt" },
    };
}

void AlbumModel::reload()
{
    beginResetModel();
    m_albums.clear();
    if (m_database && m_database->isOpen())
        m_albums = m_database->fetchAllAlbums();
    endResetModel();

    emit countChanged();
}

void AlbumModel::clear()
{
    beginResetModel();
    m_albums.clear();
    endResetModel();

    emit countChanged();
}