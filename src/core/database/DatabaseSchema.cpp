#include "core/database/DatabaseSchema.h"

namespace DatabaseSchema
{

QStringList creationStatements()
{
    return {
        // -------------------------------------------------------------------
        // albums
        // -------------------------------------------------------------------
        // One row per WebDAV folder. `remote_path` is the absolute path on
        // the WebDAV server, relative to the configured Photos root, e.g.
        //   "旅行/日本/东京"
        // `parent_id` is NULL for the top level albums inside Photos/.
        // `cover_media_id` may point at any media row of this album.
        // `item_count` and `total_size` are denormalised counters refreshed
        // by the album model after a scan.
        // -------------------------------------------------------------------
        QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS albums (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    name            TEXT    NOT NULL,
    remote_path     TEXT    NOT NULL UNIQUE,
    parent_id       INTEGER REFERENCES albums(id) ON DELETE CASCADE,
    cover_media_id  INTEGER,
    item_count      INTEGER NOT NULL DEFAULT 0,
    total_size      INTEGER NOT NULL DEFAULT 0,
    created_at      INTEGER NOT NULL DEFAULT 0,
    updated_at      INTEGER NOT NULL DEFAULT 0
)
)SQL"),

        // -------------------------------------------------------------------
        // media
        // -------------------------------------------------------------------
        // Unified photo + video record.
        //   media_type  : 0 = unknown, 1 = photo, 2 = video
        //   sync_status : see SyncStatus enum in MediaItem.h
        // All *_time columns are epoch milliseconds.
        // All size columns are bytes and MUST stay 64 bit.
        // The location_* columns are reserved for a future release and are
        // never written by D version UI.
        // -------------------------------------------------------------------
        QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS media (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    album_id            INTEGER REFERENCES albums(id) ON DELETE SET NULL,
    file_name           TEXT    NOT NULL,
    local_path          TEXT,
    remote_path         TEXT,
    file_size           INTEGER NOT NULL DEFAULT 0,
    mime_type           TEXT,
    media_type          INTEGER NOT NULL DEFAULT 0,
    modified_time       INTEGER NOT NULL DEFAULT 0,
    original_date_taken INTEGER NOT NULL DEFAULT 0,
    display_date_taken  INTEGER NOT NULL DEFAULT 0,
    width               INTEGER NOT NULL DEFAULT 0,
    height              INTEGER NOT NULL DEFAULT 0,
    duration            INTEGER NOT NULL DEFAULT 0,
    thumbnail_path      TEXT,
    sync_status         INTEGER NOT NULL DEFAULT 0,
    etag                TEXT,
    is_favorite         INTEGER NOT NULL DEFAULT 0,
    is_archived         INTEGER NOT NULL DEFAULT 0,
    is_trashed          INTEGER NOT NULL DEFAULT 0,
    latitude            REAL,
    longitude           REAL,
    location_name       TEXT,
    location_source     TEXT,
    created_at          INTEGER NOT NULL DEFAULT 0,
    updated_at          INTEGER NOT NULL DEFAULT 0
)
)SQL"),

        // -------------------------------------------------------------------
        // sync_tasks
        // -------------------------------------------------------------------
        // Queued / in flight transfer tasks. One row per file transfer.
        // `total_bytes` and `transferred_bytes` are 64 bit by necessity,
        // single files up to (and beyond) 5 GB must be representable.
        //   state     : 0 queued, 1 running, 2 paused, 3 done, 4 failed, 5 cancelled
        //   direction : 0 download, 1 upload
        // -------------------------------------------------------------------
        QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS sync_tasks (
    id                  INTEGER PRIMARY KEY AUTOINCREMENT,
    media_id            INTEGER REFERENCES media(id) ON DELETE CASCADE,
    album_id            INTEGER REFERENCES albums(id) ON DELETE CASCADE,
    direction           INTEGER NOT NULL DEFAULT 0,
    state               INTEGER NOT NULL DEFAULT 0,
    total_bytes         INTEGER NOT NULL DEFAULT 0,
    transferred_bytes   INTEGER NOT NULL DEFAULT 0,
    retry_count         INTEGER NOT NULL DEFAULT 0,
    last_error          TEXT,
    priority            INTEGER NOT NULL DEFAULT 0,
    created_at          INTEGER NOT NULL DEFAULT 0,
    updated_at          INTEGER NOT NULL DEFAULT 0
)
)SQL"),

        // -------------------------------------------------------------------
        // sync_records
        // -------------------------------------------------------------------
        // Incremental sync bookkeeping, one row per known file.
        // Used to compare local vs remote without re-downloading everything.
        // -------------------------------------------------------------------
        QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS sync_records (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    remote_path     TEXT    NOT NULL UNIQUE,
    file_name       TEXT    NOT NULL,
    file_size       INTEGER NOT NULL DEFAULT 0,
    modified_time   INTEGER NOT NULL DEFAULT 0,
    etag            TEXT,
    last_synced_at  INTEGER NOT NULL DEFAULT 0,
    state           INTEGER NOT NULL DEFAULT 0
)
)SQL"),

        // -------------------------------------------------------------------
        // memories
        // -------------------------------------------------------------------
        // Locally generated memories. D version generates them from dates
        // and album membership only, no AI service is involved.
        //   kind : 0 on this day, 1 time range, 2 album based
        // `media_ids` stores a JSON array of media ids for the first release.
        // -------------------------------------------------------------------
        QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS memories (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    title           TEXT    NOT NULL,
    subtitle        TEXT,
    kind            INTEGER NOT NULL DEFAULT 0,
    cover_media_id  INTEGER,
    media_ids       TEXT    NOT NULL DEFAULT '[]',
    start_time      INTEGER NOT NULL DEFAULT 0,
    end_time        INTEGER NOT NULL DEFAULT 0,
    generated_at    INTEGER NOT NULL DEFAULT 0
)
)SQL"),

        // -------------------------------------------------------------------
        // settings
        // -------------------------------------------------------------------
        // Simple key/value store for application preferences.
        // WebDAV credentials are NOT stored here in plain text; on Android
        // they go through the Keystore backed secure storage instead.
        // -------------------------------------------------------------------
        QStringLiteral(R"SQL(
CREATE TABLE IF NOT EXISTS settings (
    key     TEXT PRIMARY KEY,
    value   TEXT
)
)SQL"),
    };
}

QStringList indexStatements()
{
    return {
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_media_album ON media(album_id)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_media_display_date ON media(display_date_taken DESC)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_media_remote_path ON media(remote_path)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_media_local_path ON media(local_path)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_media_type ON media(media_type)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_media_sync_status ON media(sync_status)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_albums_parent ON albums(parent_id)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_sync_tasks_state ON sync_tasks(state)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_sync_records_path ON sync_records(remote_path)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_memories_kind ON memories(kind)"),
    };
}

} // namespace DatabaseSchema