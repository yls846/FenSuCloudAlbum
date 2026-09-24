// FenSu Cloud Album
// D版 (first release) entry point.
//
// Responsibilities of this file are intentionally minimal:
//   - create the QGuiApplication / QQmlApplicationEngine
//   - instantiate the AppController (application layer)
//   - expose it to QML
//   - load the root QML component
//
// All real logic lives in the app/ and core/ layers.

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>
#include <QFont>
#include <QStandardPaths>
#include <QDirIterator>
#include <QDebug>

#include "app/AppController.h"
#include "platform/android/AndroidPlatform.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("FenSu Cloud Album"));
    app.setOrganizationName(QStringLiteral("FenSu"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    // Hand the platform layer the real sandbox path. Qt can only resolve this
    // after QGuiApplication has set up the Android JNI bridge, which is why it
    // happens here and not in a static initialiser.
    const QString appDataRoot =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataRoot.isEmpty())
        AndroidPlatform::setPlatformRoot(appDataRoot);

    // Material 3 baseline style. The actual colors are driven by
    // Theme.qml so we only pick the control style here.
    QQuickStyle::setStyle(QStringLiteral("Material"));

    AppController controller;

    QQmlApplicationEngine engine;

    // Expose the controller as a context property for the first stage.
    // Later stages may migrate to a proper singleton / qml type registration.
    engine.rootContext()->setContextProperty(QStringLiteral("App"), &controller);

    // The QML module is registered with
    //   URI              FenSuCloudAlbum
    //   RESOURCE_PREFIX  /qt/qml
    //   QML_FILES        src/ui/qml/Main.qml   (path as written in CMakeLists)
    // which places the file at exactly this URL.
    //
    // The path inside the module is whatever was written in QML_FILES, so the
    // repository relative path is part of the URL. If this URL is ever wrong
    // the engine fails to create a root object and the list of resource paths
    // that do exist is printed below.
    const QUrl url(QStringLiteral("qrc:/qt/qml/FenSuCloudAlbum/src/ui/qml/Main.qml"));

    // Fail with a useful message instead of a silent black screen.
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (obj || url != objUrl)
                return;
            qCritical() << "Failed to load root QML component:" << objUrl;
            qCritical() << "QML files present in the resource system:";
            QDirIterator it(QStringLiteral(":/qt/qml"),
                            QStringList() << QStringLiteral("*.qml"),
                            QDir::Files,
                            QDirIterator::Subdirectories);
            while (it.hasNext())
                qCritical() << "   " << it.next();
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "No root objects were created, aborting.";
        return -1;
    }

    return app.exec();
}
