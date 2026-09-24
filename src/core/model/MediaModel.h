#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "core/model/MediaItem.h"

class DatabaseManager;

// ---------------------------------------------------------------------------
// MediaModel
//
// Exposes the flat "All media" list to QML. Sorting is delegated to the
// database (display_date_taken DESC) so a large library does not have to be
// re-sorted in the GUI thread.
// ---------------------------------------------------------------------------
class MediaModel : public QAbstractListModel
{
    Q_OBJECT

    // Exposed as a property (not just a method) because the controller hands
    // this model to QML as a plain QObject*, and QML needs `count` to be a
    // property to write `App.mediaModel.count` in bindings.
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AlbumIdRole,
        FileNameRole,
        DisplayNameRole,
        RelativePathRole,
        MimeTypeRole,
        FileSizeRole,
        WidthRole,
        HeightRole,
        DurationMsRole,
        OriginalDateTakenRole,
        DisplayDateTakenRole,
        FavoriteRole,
        IsVideoRole,
        ThumbnailPathRole,
        FileSizeTextRole,
    };
    Q_ENUM(Roles)

    explicit MediaModel(QObject *parent = nullptr);

    void setDatabase(DatabaseManager *database);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString fileSizeText(qint64 bytes) const;

    // Removes the given id from the in-memory model only. The database delete
    // is performed by the caller so the UI can stay optimistic.
    Q_INVOKABLE bool removeById(qint64 id);

    int count() const { return m_items.size(); }

signals:
    void countChanged();

private:
    static QString formatFileSize(qint64 bytes);

    QVector<MediaItem> m_items;
    DatabaseManager *m_database = nullptr;
};