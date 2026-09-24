#include "core/database/DatabaseManager.h"
#include "core/database/DatabaseSchema.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUuid>
#include <QVariant>
#include <QDebug>

namespace {

// Column indexes are resolved once per query below and never hard-coded in a
// way that would silently break if the schema grows.

QDateTime fromEpochMs(qint64 ms)
{
    if (ms <= 0)
        return QDateTime();
    return QDateTime::fromMSecsSinceEpoch(ms, Qt::UTC);
}

qint64 toEpochMs(const QDateTime &dt)
{
    if (!dt.isValid())
        return 0;
    return dt.toUTC().toMSecsSinceEpoch();
}

MediaItem mediaFromQuery(const QSqlQuery &q)
{
    MediaItem m;
    m.id                = q.value(QStringLiteral("id")).toLongLong();
    m.albumId           = q.value(QStringLiteral("album_id")).toLongLong();
    m.fileName          = q.value(QStringLiteral("file_name")).toString();
    m.displayName       = q.value(QStringLiteral("display_name")).toString();
    m.relativePath      = q.value(QStringLiteral("relative_path")).toString();
    m.mimeType          = q.value(QStringLiteral("mime_type")).toString();
    m.fileSize          = q.value(QStringLiteral("file_size")).toLongLong();
    m.width             = q.value(QStringLiteral("width")).toLongLong();
    m.height            = q.value(QStringLiteral("height")).toLongLong();
    m.durationMs        = q.value(QStringLiteral("duration_ms")).toLongLong();
    m.originalDateTaken = fromEpochMs(q.value(QStringLiteral("original_date_taken")).toLongLong());
    m.displayDateTaken  = fromEpochMs(q.value(QStringLiteral("display_date_taken")).toLongLong());
    m.favorite          = q.value(QStringLiteral("favorite")).toInt() != 0;
    m.isVideo           = q.value(QStringLiteral("is_video")).toInt() != 0;
    m.thumbnailPath     = q.value(QStringLiteral("thumbnail_path")).toString();

    // Reserved columns, kept for a future release.
    m.latitude        = q.value(QStringLiteral("latitude")).toDouble();
    m.longitude       = q.value(QStringLiteral("longitude")).toDouble();
    m.locationName    = q.value(QStringLiteral("location_name")).toString();
    m.locationSource  = q.value(QStringLiteral("location_source")).toString();
    return m;
}

AlbumItem albumFromQuery(const QSqlQuery &q)
{
    AlbumItem a;
    a.id           = q.value(QStringLiteral("id")).toLongLong();
    a.name         = q.value(QStringLiteral("name")).toString();
    a.relativePath = q.value(QStringLiteral("relative_path")).toString();
    a.coverMediaId = q.value(QStringLiteral("cover_media_id")).toLongLong();
    a.mediaCount   = q.value(QStringLiteral("media_count")).toLongLong();
    a.createdAt    = fromEpochMs(q.value(QStringLiteral("created_at")).toLongLong());
    a.updatedAt    = fromEpochMs(q.value(QStringLiteral("updated_at")).toLongLong());
    return a;
}

// Shared column list so SELECTs and INSERTs never drift apart.
constexpr auto kMediaColumns =
    "id, album_id, file_name, display_name, relative_path, mime_type, "
    "file_size, width, height, duration_ms, original_date_taken, "
    "display_date_taken, favorite, is_video, thumbnail_path, "
    "latitude, longitude, location_name, location_source";

constexpr auto kAlbumColumns =
    "id, name, relative_path, cover_media_id, media_count, created_at, updated_at";

} // namespace

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
    , m_connectionName(QStringLiteral("fensu_cloud_album_%1")
                           .arg(QUuid::createUuid().toString(QUuid::WithoutBraces)))
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &databasePath)
{
    clearError();

    if (databasePath.isEmpty()) {
        setError(QStringLiteral("Database path is empty."));
        return false;
    }

    const QFileInfo info(databasePath);
    QDir dir = info.absoluteDir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        setError(QStringLiteral("Cannot create database directory: %1").arg(dir.absolutePath()));
        return false;
    }

    if (m_db.isOpen())
        m_db.close();

    m_db = QSqlDatabase::contains(m_connectionName)
               ? QSqlDatabase::database(m_connectionName, false)
               : QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);

    m_db.setDatabaseName(databasePath);

    if (!m_db.open()) {
        setError(QStringLiteral("Cannot open database: %1").arg(m_db.lastError().text()));
        return false;
    }

    // Per-connection pragmas. WAL keeps the UI readable while a sync writes.
    QSqlQuery pragma(m_db);
    pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    pragma.exec(QStringLiteral("PRAGMA foreign_keys=ON"));
    pragma.exec(QStringLiteral("PRAGMA synchronous=NORMAL"));

    if (!applySchema()) {
        m_db.close();
        return false;
    }

    m_databasePath = databasePath;
    qInfo() << "[DatabaseManager] opened" << databasePath;
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen())
        m_db.close();
    m_db = QSqlDatabase();

    if (QSqlDatabase::contains(m_connectionName))
        QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

bool DatabaseManager::applySchema()
{
    const QStringList creates = DatabaseSchema::creationStatements();
    for (const QString &sql : creates) {
        if (!execSimple(sql))
            return false;
    }

    const QStringList indexes = DatabaseSchema::indexStatements();
    for (const QString &sql : indexes) {
        if (!execSimple(sql))
            return false;
    }
    return true;
}

bool DatabaseManager::execSimple(const QString &sql)
{
    QSqlQuery query(m_db);
    if (!query.exec(sql)) {
        setError(QStringLiteral("SQL failed: %1\nStatement: %2")
                     .arg(query.lastError().text(), sql));
        return false;
    }
    return true;
}

void DatabaseManager::setError(const QString &error)
{
    m_lastError = error;
    qWarning() << "[DatabaseManager]" << error;
}

void DatabaseManager::clearError()
{
    m_lastError.clear();
}

// ---------------------------------------------------------------------------
// media
// ---------------------------------------------------------------------------

bool DatabaseManager::insertOrUpdateMedia(MediaItem &item)
{
    clearError();

    if (item.fileName.isEmpty() || item.relativePath.isEmpty()) {
        setError(QStringLiteral("insertOrUpdateMedia: fileName and relativePath are required."));
        return false;
    }

    QSqlQuery query(m_db);

    if (item.id >= 0) {
        query.prepare(QStringLiteral(
            "UPDATE media SET album_id=:album_id, file_name=:file_name, "
            "display_name=:display_name, relative_path=:relative_path, "
            "mime_type=:mime_type, file_size=:file_size, width=:width, "
            "height=:height, duration_ms=:duration_ms, "
            "original_date_taken=:original_date_taken, "
            "display_date_taken=:display_date_taken, favorite=:favorite, "
            "is_video=:is_video, thumbnail_path=:thumbnail_path, "
            "latitude=:latitude, longitude=:longitude, "
            "location_name=:location_name, location_source=:location_source "
            "WHERE id=:id"));
        query.bindValue(QStringLiteral(":id"), item.id);
    } else {
        query.prepare(QStringLiteral(
            "INSERT INTO media (album_id, file_name, display_name, relative_path, "
            "mime_type, file_size, width, height, duration_ms, "
            "original_date_taken, display_date_taken, favorite, is_video, "
            "thumbnail_path, latitude, longitude, location_name, location_source) "
            "VALUES (:album_id, :file_name, :display_name, :relative_path, "
            ":mime_type, :file_size, :width, :height, :duration_ms, "
            ":original_date_taken, :display_date_taken, :favorite, :is_video, "
            ":thumbnail_path, :latitude, :longitude, :location_name, :location_source)"));
    }

    query.bindValue(QStringLiteral(":album_id"), item.albumId);
    query.bindValue(QStringLiteral(":file_name"), item.fileName);
    query.bindValue(QStringLiteral(":display_name"), item.displayName);
    query.bindValue(QStringLiteral(":relative_path"), item.relativePath);
    query.bindValue(QStringLiteral(":mime_type"), item.mimeType);
    query.bindValue(QStringLiteral(":file_size"), item.fileSize); // qint64
    query.bindValue(QStringLiteral(":width"), item.width);
    query.bindValue(QStringLiteral(":height"), item.height);
    query.bindValue(QStringLiteral(":duration_ms"), item.durationMs);
    query.bindValue(QStringLiteral(":original_date_taken"), toEpochMs(item.originalDateTaken));
    query.bindValue(QStringLiteral(":display_date_taken"), toEpochMs(item.displayDateTaken));
    query.bindValue(QStringLiteral(":favorite"), item.favorite ? 1 : 0);
    query.bindValue(QStringLiteral(":is_video"), item.isVideo ? 1 : 0);
    query.bindValue(QStringLiteral(":thumbnail_path"), item.thumbnailPath);
    query.bindValue(QStringLiteral(":latitude"), item.latitude);
    query.bindValue(QStringLiteral(":longitude"), item.longitude);
    query.bindValue(QStringLiteral(":location_name"), item.locationName);
    query.bindValue(QStringLiteral(":location_source"), item.locationSource);

    if (!query.exec()) {
        setError(QStringLiteral("insertOrUpdateMedia failed: %1").arg(query.lastError().text()));
        return false;
    }

    if (item.id < 0)
        item.id = query.lastInsertId().toLongLong();

    emit databaseChanged();
    return true;
}

bool DatabaseManager::deleteMedia(qint64 mediaId)
{
    clearError();

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM media WHERE id=:id"));
    query.bindValue(QStringLiteral(":id"), mediaId);

    if (!query.exec()) {
        setError(QStringLiteral("deleteMedia failed: %1").arg(query.lastError().text()));
        return false;
    }

    emit databaseChanged();
    return true;
}

QVector<MediaItem> DatabaseManager::fetchAllMedia()
{
    QVector<MediaItem> result;

    QSqlQuery query(m_db);
    const QString sql = QStringLiteral("SELECT %1 FROM media "
                                       "ORDER BY display_date_taken DESC, id DESC")
                            .arg(QLatin1String(kMediaColumns));
    if (!query.exec(sql)) {
        setError(QStringLiteral("fetchAllMedia failed: %1").arg(query.lastError().text()));
        return result;
    }

    while (query.next())
        result.push_back(mediaFromQuery(query));

    return result;
}

QVector<MediaItem> DatabaseManager::fetchMediaForAlbum(qint64 albumId)
{
    QVector<MediaItem> result;

    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("SELECT %1 FROM media WHERE album_id=:album_id "
                                 "ORDER BY display_date_taken DESC, id DESC")
                      .arg(QLatin1String(kMediaColumns)));
    query.bindValue(QStringLiteral(":album_id"), albumId);

    if (!query.exec()) {
        setError(QStringLiteral("fetchMediaForAlbum failed: %1").arg(query.lastError().text()));
        return result;
    }

    while (query.next())
        result.push_back(mediaFromQuery(query));

    return result;
}

qint64 DatabaseManager::countMedia() const
{
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT COUNT(*) FROM media")) || !query.next())
        return 0;
    return query.value(0).toLongLong();
}

// ---------------------------------------------------------------------------
// albums
// ---------------------------------------------------------------------------

bool DatabaseManager::insertOrUpdateAlbum(AlbumItem &item)
{
    clearError();

    if (item.name.isEmpty() && item.relativePath.isEmpty()) {
        setError(QStringLiteral("insertOrUpdateAlbum: name/relativePath are required."));
        return false;
    }

    QSqlQuery query(m_db);

    if (item.id >= 0) {
        query.prepare(QStringLiteral(
            "UPDATE albums SET name=:name, relative_path=:relative_path, "
            "cover_media_id=:cover_media_id, media_count=:media_count, "
            "updated_at=:updated_at WHERE id=:id"));
        query.bindValue(QStringLiteral(":id"), item.id);
    } else {
        query.prepare(QStringLiteral(
            "INSERT INTO albums (name, relative_path, cover_media_id, media_count, "
            "created_at, updated_at) VALUES (:name, :relative_path, "
            ":cover_media_id, :media_count, :created_at, :updated_at)"));
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        query.bindValue(QStringLiteral(":created_at"),
                        item.createdAt.isValid() ? toEpochMs(item.createdAt) : now);
    }

    query.bindValue(QStringLiteral(":name"), item.name);
    query.bindValue(QStringLiteral(":relative_path"), item.relativePath);
    query.bindValue(QStringLiteral(":cover_media_id"), item.coverMediaId);
    query.bindValue(QStringLiteral(":media_count"), item.mediaCount);
    query.bindValue(QStringLiteral(":updated_at"),
                    item.updatedAt.isValid() ? toEpochMs(item.updatedAt)
                                             : QDateTime::currentMSecsSinceEpoch());

    if (!query.exec()) {
        setError(QStringLiteral("insertOrUpdateAlbum failed: %1").arg(query.lastError().text()));
        return false;
    }

    if (item.id < 0)
        item.id = query.lastInsertId().toLongLong();

    emit databaseChanged();
    return true;
}

QVector<AlbumItem> DatabaseManager::fetchAllAlbums()
{
    QVector<AlbumItem> result;

    QSqlQuery query(m_db);
    const QString sql = QStringLiteral("SELECT %1 FROM albums ORDER BY relative_path ASC")
                            .arg(QLatin1String(kAlbumColumns));
    if (!query.exec(sql)) {
        setError(QStringLiteral("fetchAllAlbums failed: %1").arg(query.lastError().text()));
        return result;
    }

    while (query.next())
        result.push_back(albumFromQuery(query));

    return result;
}

QStringList DatabaseManager::storedRelativePaths() const
{
    QStringList paths;

    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("SELECT relative_path FROM media"))) {
        return paths;
    }

    while (query.next())
        paths << query.value(0).toString();

    return paths;
}