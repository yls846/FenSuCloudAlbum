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

    // No QML_ELEMENT / QML_UNCREATABLE here on purpose.
    //
    // This object is handed to QML through
    //     engine.rootContext()->setContextProperty("App", &controller)
    // and QML only ever reaches it as `App.<something>`. It is never
    // instantiated from QML and never referenced as a type, so registering it
    // with the QML type system buys nothing.
    //
    // Registering it also costs: Qt then generates a qmltyperegistrations
    // translation unit containing
    //     #include <AppController.h>
    // without a path. The header lives at src/app/AppController.h, so that
    // include does not resolve and the build fails with
    //     fatal error: 'AppController.h' file not found

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

    // QML facing model accessors.
    //
    // The return type is QObject* so QML can read the properties the models
    // expose (count, for example) without Qt having to know the concrete type.
    // The bodies live in the .cpp: this header only forward declares the model
    // classes, and a forward declaration is not enough to convert a
    // MediaModel* (or any of the others) to a QObject*, because at this point
    // the compiler does not yet know they derive from QObject.
    QObject *albums() const;
    QObject *mediaModel() const;
    QObject *timelineModel() const;
    QObject *memoryModel() const;

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