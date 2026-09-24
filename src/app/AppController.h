#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include <memory>

class DatabaseManager;
class MediaModel;
class AlbumModel;
class MemoryModel;
class TimelineModel;
class CacheManager;
class NetworkMonitor;

// ---------------------------------------------------------------------------
// AppController
// ---------------------------------------------------------------------------
// Application layer object exposed to QML as `App`.
//
// It owns the core services and provides a stable, UI friendly surface.
// The controller does NOT contain business rules itself, it coordinates
// the core layer objects and translates their state into QML friendly
// properties / signals.
// ---------------------------------------------------------------------------
class AppController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("AppController is provided by C++ as context property `App`.")

    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_PROPERTY(QString storageRoot READ storageRoot NOTIFY storageRootChanged)
    Q_PROPERTY(QString databasePath READ databasePath NOTIFY databasePathChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool online READ isOnline NOTIFY onlineChanged)
    Q_PROPERTY(QString platformName READ platformName CONSTANT)

    // List models exposed to QML. They are stable for the whole lifetime
    // of the controller and become valid after initialize() succeeds.
    Q_PROPERTY(QObject *albums READ albums NOTIFY modelsChanged)
    Q_PROPERTY(QObject *mediaModel READ mediaModel NOTIFY modelsChanged)
    Q_PROPERTY(QObject *timelineModel READ timelineModel NOTIFY modelsChanged)
    Q_PROPERTY(QObject *memoryModel READ memoryModel NOTIFY modelsChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    // Initialise core services. Returns true on success.
    // Safe to call multiple times, subsequent calls are no-ops.
    Q_INVOKABLE bool initialize();

    // Shut down core services and flush any pending database work.
    Q_INVOKABLE void shutdown();

    // Reload the album list from the database.
    Q_INVOKABLE void refreshAlbums();

    // Human readable application info for the About dialog.
    Q_INVOKABLE QString applicationInfo() const;

    bool isReady() const { return m_ready; }
    QString storageRoot() const { return m_storageRoot; }
    QString databasePath() const { return m_databasePath; }
    QString lastError() const { return m_lastError; }
    bool isOnline() const { return m_online; }
    QString platformName() const;

    // QML facing model accessors (QObject* so QML sees the derived model type).
    QObject *albums() const { return m_albumModel; }
    QObject *mediaModel() const { return m_mediaModel; }
    QObject *timelineModel() const { return m_timelineModel; }
    QObject *memoryModel() const { return m_memoryModel; }

    // Core service accessors. They return nullptr before initialize().
    DatabaseManager *database() const { return m_database.get(); }
    AlbumModel *albumListModel() const { return m_albumModel; }
    MediaModel *mediaListModel() const { return m_mediaModel; }
    MemoryModel *memoryListModel() const { return m_memoryModel; }
    TimelineModel *timelineListModel() const { return m_timelineModel; }
    CacheManager *cache() const { return m_cache.get(); }
    NetworkMonitor *network() const { return m_network.get(); }

signals:
    void readyChanged();
    void storageRootChanged();
    void databasePathChanged();
    void lastErrorChanged();
    void onlineChanged();

    // Emitted once the list models have been created and wired to the database.
    void modelsChanged();

    // Emitted after a successful refreshAlbums().
    void albumsRefreshed();

private:
    void setLastError(const QString &error);
    void setOnline(bool online);
    bool prepareStorage();

    std::unique_ptr<DatabaseManager> m_database;
    std::unique_ptr<CacheManager> m_cache;
    std::unique_ptr<NetworkMonitor> m_network;

    MediaModel *m_mediaModel = nullptr;
    AlbumModel *m_albumModel = nullptr;
    MemoryModel *m_memoryModel = nullptr;
    TimelineModel *m_timelineModel = nullptr;

    bool m_ready = false;
    bool m_online = false;
    QString m_storageRoot;
    QString m_databasePath;
    QString m_lastError;
};