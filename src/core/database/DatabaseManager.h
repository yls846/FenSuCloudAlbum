#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include "core/model/MediaItem.h"
#include "core/model/AlbumItem.h"

// ---------------------------------------------------------------------------
// DatabaseManager
//
// Owns the single QSqlDatabase connection and all SQL statements used by the
// Core layer. It is deliberately free of any platform specific code so that it
// keeps working on Linux / Windows / Android unchanged.
//
// Design notes:
//   * One connection per DatabaseManager instance (named connection).
//   * Query helpers never expose raw QSqlQuery to callers, so column layout
//     stays an implementation detail of this file + DatabaseSchema.
//   * All byte/offset values travel as qint64.
// ---------------------------------------------------------------------------
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager() override;

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    // Opens (and creates/upgrades) the database at the given absolute file
    // path. Returns false and sets lastError() on failure.
    bool open(const QString &databasePath);

    void close();

    bool isOpen() const;
    QString databasePath() const { return m_databasePath; }
    QString lastError() const { return m_lastError; }

    // ---- media -----------------------------------------------------------
    bool insertOrUpdateMedia(MediaItem &item);
    bool deleteMedia(qint64 mediaId);
    QVector<MediaItem> fetchAllMedia();
    QVector<MediaItem> fetchMediaForAlbum(qint64 albumId);
    qint64 countMedia() const;

    // ---- albums ----------------------------------------------------------
    bool insertOrUpdateAlbum(AlbumItem &item);
    QVector<AlbumItem> fetchAllAlbums();

    // Flattens the stored folder tree back into the full relative path list.
    // Used by the sync engine, not by the UI.
    QStringList storedRelativePaths() const;

signals:
    void databaseChanged();

private:
    bool applySchema();
    bool execSimple(const QString &sql);
    void setError(const QString &error);
    void clearError();

    QString m_databasePath;
    QString m_connectionName;
    QString m_lastError;
    QSqlDatabase m_db;
};