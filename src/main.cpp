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
// QDir::Files is passed to the QDirIterator constructor below. QDirIterator
// pulls QDir in today because its own signature needs it, but the dependency
// is stated here as well.
#include <QDir>
// QCoreApplication::exit() is used below. QGuiApplication derives from it.
#include <QCoreApplication>
// Translation loading: QTranslator, and QFile::exists() to test each candidate
// before letting QTranslator report the failure itself.
#include <QTranslator>
#include <QFile>
#include <QLocale>
#include <QDebug>
// Used directly in this file: Qt::QueuedConnection, QObject::connect and the
// QObject* parameter of the lambda. QGuiApplication happens to pull them in
// today, but relying on that is fragile.
#include <QtGlobal>
#include <QObject>
#include <Qt>

#include "app/AppController.h"
#include "platform/android/AndroidPlatform.h"

int main(int argc, char *argv[])
{
    // ---------------------------------------------------------------------
    // Earliest possible log line.
    // ---------------------------------------------------------------------
    // On Android the process can die before any QML is loaded: a missing Qt
    // library, a failing JNI call in the Qt bootstrap, or an exception inside
    // a static initialiser. Printing this before touching anything else makes
    // it possible to tell "never started" apart from "started and failed
    // later" using nothing but `adb logcat`.
    //
    // qInfo/qWarning are routed to the Android log by Qt's default message
    // handler, tag Qt, so the filter is:
    //     adb logcat -s Qt:V
    qInfo() << "FenSuCloudAlbum: main() entered, argc=" << argc;

    QGuiApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("FenSu Cloud Album"));
    app.setOrganizationName(QStringLiteral("FenSu"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    // -----------------------------------------------------------------------
    // Translations
    // -----------------------------------------------------------------------
    // The .qm file is embedded in the resource system by qt_add_translations
    // under the /i18n prefix. Nothing is translatable until a QTranslator is
    // installed on the application, which must happen before the QML engine
    // loads anything: qsTr() is evaluated once, at component creation, so a
    // translator installed afterwards has no effect on text already shown.
    //
    // The translator is kept alive for the lifetime of main(); a QTranslator
    // that goes out of scope is removed from the application and its
    // translations stop applying.
    QTranslator translator;

    const QStringList translationCandidates = {
        QStringLiteral(":/i18n/fensucloudalbum_zh_CN.qm"),
        QStringLiteral(":/i18n/FenSuCloudAlbum_zh_CN.qm"),
        QStringLiteral(":/i18n/fensucloudalbum_zh.qm"),
    };

    bool translatorLoaded = false;
    for (const QString &candidate : translationCandidates) {
        if (QFile::exists(candidate) && translator.load(candidate)) {
            translatorLoaded = app.installTranslator(&translator);
            qInfo() << "FenSuCloudAlbum: loaded translation" << candidate
                    << "installed=" << translatorLoaded;
            break;
        }
    }

    if (!translatorLoaded) {
        // Not fatal: the UI falls back to the English source strings. The
        // message says what was tried so a packaging problem is visible in
        // logcat rather than being mistaken for a missing translation.
        qWarning() << "FenSuCloudAlbum: no translation loaded, UI will be English."
                   << "Candidates tried:" << translationCandidates;

        // List what i18n resources actually exist, if any, to make the fix
        // obvious from the log alone.
        QDirIterator it(QStringLiteral(":/i18n"),
                        QStringList() << QStringLiteral("*.qm"),
                        QDir::Files,
                        QDirIterator::Subdirectories);
        int found = 0;
        while (it.hasNext()) {
            qWarning() << "FenSuCloudAlbum:     present:" << it.next();
            ++found;
        }
        qWarning() << "FenSuCloudAlbum: total .qm files in :/i18n =" << found;
    }

    qInfo() << "FenSuCloudAlbum: application locale ="
            << QLocale::system().name();

    qInfo() << "FenSuCloudAlbum: application object created"
            << "name=" << app.applicationName()
            << "org=" << app.organizationName()
            << "version=" << app.applicationVersion();

    // Hand the platform layer the real sandbox path. Qt can only resolve this
    // after QGuiApplication has set up the Android JNI bridge, which is why it
    // happens here and not in a static initialiser.
    const QString appDataRoot =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!appDataRoot.isEmpty())
        AndroidPlatform::setPlatformRoot(appDataRoot);

    qInfo() << "FenSuCloudAlbum: AppDataLocation ="
            << (appDataRoot.isEmpty() ? QStringLiteral("(EMPTY - this will break storage)")
                                      : appDataRoot);

    // Report the other location the platform layer consults, so a scoped
    // storage or permission problem is visible immediately.
    qInfo() << "FenSuCloudAlbum: GenericDataLocation ="
            << QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    // Material 3 baseline style. The actual colors are driven by
    // Theme.qml so we only pick the control style here.
    QQuickStyle::setStyle(QStringLiteral("Material"));

    qInfo() << "FenSuCloudAlbum: QQuickStyle set to Material";

    AppController controller;

    qInfo() << "FenSuCloudAlbum: AppController constructed, ready ="
            << controller.isReady();

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

    qInfo() << "FenSuCloudAlbum: loading root QML from" << url.toString();
    qInfo() << "FenSuCloudAlbum: QML import paths:";
    for (const QString &p : engine.importPathList())
        qInfo() << "    " << p;

    // Dump what is actually embedded, before loading anything. If the URL is
    // wrong this list says what the right one would have been, and it is
    // visible even when the engine goes on to create no root object.
    {
        QDirIterator it(QStringLiteral(":/qt/qml"),
                        QStringList() << QStringLiteral("*.qml"),
                        QDir::Files,
                        QDirIterator::Subdirectories);
        int found = 0;
        while (it.hasNext()) {
            qInfo() << "    embedded:" << it.next();
            ++found;
        }
        qInfo() << "FenSuCloudAlbum: total embedded QML files =" << found;
    }

    // Fail with a useful message instead of a silent black screen.
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (obj) {
                qInfo() << "FenSuCloudAlbum: root object created for" << objUrl.toString();
                return;
            }
            if (url != objUrl)
                return;
            qCritical() << "FenSuCloudAlbum: FAILED to create a root object for"
                        << objUrl.toString();
            qCritical() << "FenSuCloudAlbum: the QML errors above explain why;"
                        << "compare the URL with the 'embedded:' list printed at startup";
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "FenSuCloudAlbum: engine has no root objects, aborting";
        return -1;
    }

    qInfo() << "FenSuCloudAlbum: entering the event loop";

    return app.exec();
}
