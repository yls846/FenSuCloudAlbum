#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "core/model/AlbumItem.h"

class DatabaseManager;

// ---------------------------------------------------------------------------
// AlbumModel
//
// Lists albums (== WebDAV folders) for the "Albums" tab.
// This model is intentionally *not* a tree: the D version shows the album list
// flat, but every entry keeps its full relativePath so navigation stays
// hierarchy-aware and nothing is ever flattened on disk or in the sync layer.
// ---------------------------------------------------------------------------
class AlbumModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        RelativePathRole,
        CoverMediaIdRole,
        MediaCountRole,
        UpdatedAtRole,
    };
    Q_ENUM(Roles)

    explicit AlbumModel(QObject *parent = nullptr);

    void setDatabase(DatabaseManager *database);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void clear();

    int count() const { return m_albums.size(); }

signals:
    void countChanged();

private:
    QVector<AlbumItem> m_albums;
    DatabaseManager *m_database = nullptr;
};