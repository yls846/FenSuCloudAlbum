#pragma once

#include <QObject>
#include <QString>

// ---------------------------------------------------------------------------
// AndroidPlatform
// ---------------------------------------------------------------------------
// Thin wrapper around the handful of Android specific things the application
// layer needs. It is compiled ONLY on Android (see CMakeLists.txt). On other
// platforms the same API is provided by the generic fallback below, so the
// application layer can call it unconditionally.
//
// Keep this class deliberately boring: it must never contain media, sync or
// database logic. It only answers "where do I put files" and "what does the
// system think about X".
// ---------------------------------------------------------------------------
class AndroidPlatform : public QObject
{
    Q_OBJECT

public:
    explicit AndroidPlatform(QObject *parent = nullptr);
    ~AndroidPlatform() override;

    // Root directory the application may write to.
    //   Android : /storage/emulated/0/Android/data/<pkg>/files
    //             (see setPlatformRoot, called from main.cpp after the
    //              Activity has been created)
    //   Desktop : QStandardPaths::AppDataLocation
    QString defaultStorageRoot() const;

    // Public album root that is visible through a file manager:
    //   Android : <external>/FenSuCloudAlbum
    //   Desktop : <home>/FenSuCloudAlbum
    QString publicAlbumRoot() const;

    // Best effort free space of the volume backing `path`, in bytes.
    // Returns -1 when the platform cannot tell us.
    qint64 availableBytes(const QString &path) const;

    // Human readable device + OS description, used by the About screen.
    QString deviceDescription() const;

    // True when the app currently has the "all files" / storage permission.
    // On Android 13+ this is READ_MEDIA_IMAGES / READ_MEDIA_VIDEO.
    bool hasStoragePermission() const;

    // Whether the platform has a persistent storage concept at all.
    bool isAndroid() const;

    // Overrides the app-private root. main.cpp receives the real path from
    // QStandardPaths once Qt has initialised the Android JNI bridge.
    static void setPlatformRoot(const QString &root);
};
