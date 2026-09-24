#include "platform/android/AndroidPlatform.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSysInfo>
#include <QStorageInfo>
#include <QtGlobal>

namespace {
// Set once by main.cpp. Empty means "ask QStandardPaths".
QString g_platformRoot;
}

AndroidPlatform::AndroidPlatform(QObject *parent)
    : QObject(parent)
{
}

AndroidPlatform::~AndroidPlatform() = default;

void AndroidPlatform::setPlatformRoot(const QString &root)
{
    g_platformRoot = root;
}

QString AndroidPlatform::defaultStorageRoot() const
{
    if (!g_platformRoot.isEmpty())
        return g_platformRoot;

    // AppDataLocation is backed by the app sandbox on Android and by
    // ~/.local/share/<app> on desktop. Both are writable without extra
    // permissions, which keeps the first launch working before the user
    // grants storage access.
    const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!root.isEmpty())
        return root;

    return QDir::homePath() + QStringLiteral("/.fensucloudalbum");
}

QString AndroidPlatform::publicAlbumRoot() const
{
#if defined(Q_OS_ANDROID)
    // On Android the user visible album folder lives under the shared
    // external storage. QStandardPaths::GenericDataLocation is the location
    // that maps to shared storage on every platform; the Qt 6.7 documentation
    // lists it as the segment of the Android paths that belongs to the user
    // rather than to the app sandbox.
    //
    // There is no QStandardPaths::GenericExternalStorageLocation: the enum
    // does not exist, and naming it was a mistake. QStandardPaths only offers
    // the locations listed in its StandardLocation enum.
    QString external = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if (external.isEmpty()) {
        // Documented fallback for devices where the shared storage path is not
        // resolvable (rare, but GenericDataLocation may be empty on a device
        // with no external volume mounted).
        external = QStringLiteral("/storage/emulated/0");
    }
    return external + QStringLiteral("/FenSuCloudAlbum");
#else
    return QDir::homePath() + QStringLiteral("/FenSuCloudAlbum");
#endif
}

qint64 AndroidPlatform::availableBytes(const QString &path) const
{
    const QString probe = QFileInfo(path).absolutePath();
    QStorageInfo storage(probe);
    if (!storage.isValid() || !storage.isReady())
        return -1;
    return static_cast<qint64>(storage.bytesAvailable());
}

QString AndroidPlatform::deviceDescription() const
{
    const QString product = QSysInfo::prettyProductName();
    const QString arch = QSysInfo::currentCpuArchitecture();
    return QStringLiteral("%1 (%2)").arg(product, arch);
}

bool AndroidPlatform::hasStoragePermission() const
{
#if defined(Q_OS_ANDROID)
    // D version v0.1 reads and writes only inside the app sandbox plus the
    // public album folder, which Android grants through the scoped storage
    // APIs. A future release that touches the system gallery will query
    // READ_MEDIA_IMAGES here. Until then report what actually matters:
    // whether the sandbox root is writable.
    const QString root = defaultStorageRoot();
    return QFileInfo(root).isWritable() || QDir().mkpath(root);
#else
    return true;
#endif
}

bool AndroidPlatform::isAndroid() const
{
#if defined(Q_OS_ANDROID)
    return true;
#else
    return false;
#endif
}
