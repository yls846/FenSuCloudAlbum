#include "core/cache/CacheManager.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace {

// Fixed streaming window. Never grow this to "the whole file" - the whole
// point is that a 5 GB upload only ever holds a few MB of RAM.
constexpr int kDefaultChunkSize = 2 * 1024 * 1024; // 2 MiB

} // namespace

CacheManager::CacheManager(QObject *parent)
    : QObject(parent)
{
}

CacheManager::~CacheManager() = default;

bool CacheManager::initialize(const QString &cacheRoot)
{
    m_ready = false;
    m_lastError.clear();

    if (cacheRoot.isEmpty()) {
        m_lastError = QStringLiteral("Cache root path is empty.");
        return false;
    }

    m_cacheRoot = cacheRoot;
    m_thumbnailsDir = m_cacheRoot + QStringLiteral("/thumbnails");
    m_tempDir = m_cacheRoot + QStringLiteral("/tmp");
    m_logsDir = m_cacheRoot + QStringLiteral("/logs");

    if (!ensureDir(m_cacheRoot) || !ensureDir(m_thumbnailsDir)
        || !ensureDir(m_tempDir) || !ensureDir(m_logsDir)) {
        return false;
    }

    // Verify we can really write here; a read-only cache would fail much later.
    const QString probe = m_tempDir + QStringLiteral("/.write_probe");
    QFile probeFile(probe);
    if (!probeFile.open(QIODevice::WriteOnly)) {
        m_lastError = QStringLiteral("Cache directory is not writable: %1").arg(m_tempDir);
        return false;
    }
    probeFile.write("ok");
    probeFile.close();
    QFile::remove(probe);

    m_ready = true;
    qInfo() << "[CacheManager] ready at" << m_cacheRoot;
    return true;
}

bool CacheManager::ensureDir(const QString &path)
{
    QDir dir;
    if (dir.exists(path))
        return true;
    if (dir.mkpath(path))
        return true;

    m_lastError = QStringLiteral("Cannot create directory: %1").arg(path);
    return false;
}

bool CacheManager::copyRange(const QString &sourcePath,
                             const QString &targetPath,
                             qint64 offset,
                             qint64 length,
                             int chunkSize)
{
    if (length < 0 || offset < 0) {
        m_lastError = QStringLiteral("copyRange: negative offset/length.");
        return false;
    }

    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly)) {
        m_lastError = QStringLiteral("copyRange: cannot open source %1").arg(sourcePath);
        return false;
    }

    if (!source.seek(offset)) {
        m_lastError = QStringLiteral("copyRange: cannot seek to %1").arg(offset);
        return false;
    }

    QFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        m_lastError = QStringLiteral("copyRange: cannot open target %1").arg(targetPath);
        return false;
    }

    const int window = chunkSize > 0 ? chunkSize : kDefaultChunkSize;
    qint64 remaining = length;

    while (remaining > 0) {
        const qint64 want = qMin<qint64>(window, remaining);
        const QByteArray chunk = source.read(want);
        if (chunk.isEmpty()) {
            m_lastError = QStringLiteral("copyRange: unexpected EOF after %1 bytes")
                              .arg(length - remaining);
            return false;
        }
        if (target.write(chunk) != chunk.size()) {
            m_lastError = QStringLiteral("copyRange: short write to %1").arg(targetPath);
            return false;
        }
        remaining -= chunk.size();
    }

    target.flush();
    target.close();
    source.close();
    return true;
}

qint64 CacheManager::appendChunk(const QString &targetPath, const QByteArray &chunk)
{
    if (chunk.isEmpty())
        return 0;

    QFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly | QIODevice::Append)) {
        m_lastError = QStringLiteral("appendChunk: cannot open %1").arg(targetPath);
        return -1;
    }

    const qint64 written = target.write(chunk);
    target.flush();
    target.close();

    if (written != chunk.size()) {
        m_lastError = QStringLiteral("appendChunk: short write to %1").arg(targetPath);
        return -1;
    }

    return written;
}

qint64 CacheManager::tempUsageBytes() const
{
    qint64 total = 0;

    QDirIterator it(m_tempDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        total += it.fileInfo().size();
    }

    return total;
}

int CacheManager::clearTempOlderThan(int maxAgeMinutes)
{
    if (maxAgeMinutes < 0)
        maxAgeMinutes = 0;

    const QDateTime cutoff = QDateTime::currentDateTime().addSecs(-60LL * maxAgeMinutes);
    int removed = 0;

    QDirIterator it(m_tempDir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().lastModified() < cutoff) {
            if (QFile::remove(it.filePath()))
                ++removed;
        }
    }

    qInfo() << "[CacheManager] cleared" << removed << "stale temp files";
    return removed;
}