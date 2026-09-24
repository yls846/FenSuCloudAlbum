#pragma once

#include <QObject>
#include <QString>

class QFile;

// ---------------------------------------------------------------------------
// CacheManager
//
// Owns the local cache tree:
//
//   <cacheRoot>/thumbnails/...   small previews
//   <cacheRoot>/tmp/...          in-flight partial downloads
//   <cacheRoot>/logs/...         rotating diagnostic logs
//
// Everything here is platform neutral. Android paths are supplied by
// AndroidPlatform, but this class never calls Android APIs directly.
//
// Large file rule: helper methods stream in fixed-size chunks and never load a
// whole file into a QByteArray. A 5 GB upload must survive this class.
// ---------------------------------------------------------------------------
class CacheManager : public QObject
{
    Q_OBJECT

public:
    explicit CacheManager(QObject *parent = nullptr);
    ~CacheManager() override;

    // Creates the cache sub-tree below the given root and validates write
    // access. cacheRoot is normally provided by AndroidPlatform / QStandardPaths.
    bool initialize(const QString &cacheRoot);

    QString cacheRoot() const { return m_cacheRoot; }
    QString thumbnailsDir() const { return m_thumbnailsDir; }
    QString tempDir() const { return m_tempDir; }
    QString logsDir() const { return m_logsDir; }

    bool isReady() const { return m_ready; }
    QString lastError() const { return m_lastError; }

    // ---- chunked streaming helpers ------------------------------------------
    // Copies `length` bytes starting at `offset` from `sourcePath` into
    // `targetPath`. Uses a bounded buffer, safe for multi-gigabyte inputs.
    bool copyRange(const QString &sourcePath,
                   const QString &targetPath,
                   qint64 offset,
                   qint64 length,
                   int chunkSize = 2 * 1024 * 1024);

    // Appends one chunk to `targetPath`. Returns the number of bytes written,
    // or -1 on failure. Used by the (future) resumable uploader.
    qint64 appendChunk(const QString &targetPath, const QByteArray &chunk);

    // Total bytes currently held under tempDir, used before starting a sync.
    qint64 tempUsageBytes() const;

    // Removes stale files in tempDir older than maxAgeMinutes.
    int clearTempOlderThan(int maxAgeMinutes);

private:
    bool ensureDir(const QString &path);

    QString m_cacheRoot;
    QString m_thumbnailsDir;
    QString m_tempDir;
    QString m_logsDir;
    QString m_lastError;
    bool m_ready = false;
};