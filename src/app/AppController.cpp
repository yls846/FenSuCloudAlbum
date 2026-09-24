#include "app/AppController.h"

#include "core/database/DatabaseManager.h"
#include "core/model/MediaModel.h"
#include "core/model/AlbumModel.h"
#include "core/model/MemoryModel.h"
#include "core/model/TimelineModel.h"
#include "core/cache/CacheManager.h"
#include "core/network/NetworkMonitor.h"
#include "platform/android/AndroidPlatform.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

AppController::~AppController()
{
    // Core objects are owned by unique_ptr / parent-child ownership.
    // Nothing special has to be done here besides ensuring the database
    // is closed cleanly, which DatabaseManager does in its destructor.
}

// ---------------------------------------------------------------------------
// QML facing model accessors
// ---------------------------------------------------------------------------
// These live here rather than in the header because AppController.h only
// forward declares the model classes. Converting a MediaModel* to a QObject*
// requires the compiler to know that MediaModel derives from QObject, and a
// forward declaration does not say that. All four models are fully included
// at the top of this file.
QObject *AppController::albums() const
{
    return m_albumModel;
}

QObject *AppController::mediaModel() const
{
    return m_mediaModel;
}

QObject *AppController::timelineModel() const
{
    return m_timelineModel;
}

QObject *AppController::memoryModel() const
{
    return m_memoryModel;
}

QString AppController::platformName() const
{
#if defined(FSCA_PLATFORM_ANDROID)
    return QStringLiteral("Android");
#elif defined(FSCA_PLATFORM_WINDOWS)
    return QStringLiteral("Windows");
#else
    return QStringLiteral("Linux");
#endif
}

bool AppController::prepareStorage()
{
    // Storage layout (all under the platform specific writable location):
    //
    //   <root>/
    //   ├── FensuCloudAlbum.db
    //   └── Cache/
    //       ├── thumbnails/
    //       ├── tmp/
    //       └── logs/
    //
    QString root;

    // The platform layer is the single source of truth for "where may I
    // write". On Android it returns the app sandbox (no runtime permission
    // needed); on desktop it returns the standard app data location.
    AndroidPlatform platform;
    root = platform.defaultStorageRoot();

    if (root.isEmpty()) {
        setLastError(tr("Unable to resolve a writable application data location."));
        return false;
    }

    QDir rootDir(root);
    if (!rootDir.exists() && !rootDir.mkpath(QStringLiteral("."))) {
        setLastError(tr("Unable to create application data directory: %1").arg(root));
        return false;
    }

    m_storageRoot = root;
    m_databasePath = rootDir.filePath(QStringLiteral("FensuCloudAlbum.db"));

    // The cache sub-tree (thumbnails/tmp/logs) is owned and created by
    // CacheManager itself so the directory naming lives in exactly one place.
    const QString cacheRoot = rootDir.filePath(QStringLiteral("Cache"));
    if (m_cache && !m_cache->initialize(cacheRoot)) {
        setLastError(m_cache->lastError());
        return false;
    }

    emit storageRootChanged();
    emit databasePathChanged();
    return true;
}

bool AppController::initialize()
{
    if (m_ready) {
        return true;
    }

    m_cache = std::make_unique<CacheManager>(this);
    m_network = std::make_unique<NetworkMonitor>(this);

    connect(m_network.get(), &NetworkMonitor::onlineChanged,
            this, [this]() { setOnline(m_network->isOnline()); });

    // start() is written to degrade gracefully: if Qt has no network
    // information backend at all it reports "online" instead of crashing.
    m_network->start();
    setOnline(m_network->isOnline());

    if (!prepareStorage()) {
        return false;
    }

    m_database = std::make_unique<DatabaseManager>(this);
    if (!m_database->open(m_databasePath)) {
        setLastError(m_database->lastError());
        m_database.reset();
        return false;
    }

    // Core models. They are QObject children of the controller so QML
    // can safely reference them for the whole application lifetime.
    m_albumModel = new AlbumModel(this);
    m_mediaModel = new MediaModel(this);
    m_timelineModel = new TimelineModel(this);
    m_memoryModel = new MemoryModel(this);

    m_albumModel->setDatabase(m_database.get());
    m_mediaModel->setDatabase(m_database.get());
    m_timelineModel->setDatabase(m_database.get());
    m_memoryModel->setDatabase(m_database.get());

    // Expose the list models to QML. QML reaches them through the
    // App.albums / App.mediaModel / App.timelineModel / App.memoryModel
    // properties, so nothing else has to be registered here.
    emit modelsChanged();

    // Try an initial load; failure here is not fatal for startup,
    // the UI can still request a refresh later.
    m_albumModel->reload();
    m_mediaModel->reload();
    m_timelineModel->reload();
    m_memoryModel->reload();

    m_ready = true;
    emit readyChanged();

    qInfo() << "FenSu Cloud Album initialised."
            << "platform=" << platformName()
            << "storage=" << m_storageRoot
            << "db=" << m_databasePath;

    return true;
}

void AppController::shutdown()
{
    if (!m_ready) {
        return;
    }

    if (m_database) {
        m_database->close();
    }

    m_ready = false;
    emit readyChanged();

    qInfo() << "FenSu Cloud Album shut down.";
}

void AppController::refreshAlbums()
{
    if (m_albumModel) {
        m_albumModel->reload();
    }
    if (m_mediaModel) {
        m_mediaModel->reload();
    }
    if (m_timelineModel) {
        m_timelineModel->reload();
    }
    if (m_memoryModel) {
        m_memoryModel->reload();
    }
    emit albumsRefreshed();
}

QString AppController::applicationInfo() const
{
    const QString version = QCoreApplication::applicationVersion();
    return tr("FenSu Cloud Album %1\n"
              "Platform: %2\n"
              "Storage: %3\n"
              "Database: %4")
        .arg(version,
             platformName(),
             m_storageRoot.isEmpty() ? tr("(not initialised)") : m_storageRoot,
             m_databasePath.isEmpty() ? tr("(not initialised)") : m_databasePath);
}

void AppController::setLastError(const QString &error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    qWarning() << "AppController error:" << error;
    emit lastErrorChanged();
}

void AppController::setOnline(bool online)
{
    if (m_online == online) {
        return;
    }
    m_online = online;
    emit onlineChanged();
}