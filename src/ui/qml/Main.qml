import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// Main.qml
// ---------------------------------------------------------------------------
// Application shell. Owns the three top level tabs and the bottom navigation
// bar. Pages are kept alive (StackLayout) so scroll positions and models
// survive tab switches, exactly like Google Photos.
//
// Data access goes through the C++ context property `App` (AppController):
//   App.ready / App.albums / App.mediaModel / App.memoryModel / ...
// ---------------------------------------------------------------------------
ApplicationWindow {
    id: window

    width: 412
    height: 915
    visible: true
    title: qsTr("FenSu Cloud Album")

    color: Theme.background

    // Kick off core initialisation as soon as the window exists.
    //
    // AppController::initialize() is synchronous and Q_INVOKABLE, so the call
    // returns a bool that says whether storage, the database and the models
    // are ready. Initialisation is idempotent, so re-creating the window
    // (rotation, for example) is harmless.
    //
    // The outcome is written to the Qt log so `adb logcat` shows it without a
    // debugger attached. That is the first thing to check when the app shows a
    // blank screen on a real device.
    Component.onCompleted: {
        let ok = App.ready;
        if (!ok)
            ok = App.initialize();

        startupSucceeded = ok;

        if (ok) {
            console.info("FenSuCloudAlbum: core ready"
                         + " platform=" + App.platformName
                         + " storage=" + App.storageRoot
                         + " database=" + App.databasePath);
        } else {
            console.warn("FenSuCloudAlbum: core NOT ready."
                         + " lastError=" + (App.lastError.length > 0
                                            ? App.lastError
                                            : "(none reported)"));
        }
    }

    // True once Component.onCompleted has run the initialisation and it
    // reported success. Declared along with the window so the overlay below
    // can bind to it.
    property bool startupSucceeded: false

    // -----------------------------------------------------------------------
    // Content
    // -----------------------------------------------------------------------
    // The two pages stay alive in a StackLayout so scroll position and model
    // state survive a tab switch, the way Google Photos behaves.
    StackLayout {
        id: stack

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: navBar.top
        }

        // One property drives both the layout and the navigation bar. The
        // navigation bar writes to it through onItemSelected below and the
        // layout reads it. Previously Main.qml set currentIndex to the literal
        // 0 on the bar and never handled itemSelected, so the bar's own
        // currentIndex was reset to 0 on every change and tapping a tab did
        // nothing at all.
        property int activeTab: 0

        currentIndex: activeTab
        onCurrentIndexChanged: activeTab = currentIndex

        AlbumsPage {
            onAlbumActivated: function (relativePath, albumName) {
                // The D version has no album detail screen yet. The hook exists
                // so the navigation model is already correct when that screen
                // lands: relativePath is carried through, never flattened.
                console.log("Open album:", albumName, "at", relativePath)
            }
        }

        AllPhotosPage {
            onItemActivated: function (mediaIndex) {
                console.log("Open media index:", mediaIndex)
            }
        }
    }

    // -----------------------------------------------------------------------
    // Bottom navigation
    // -----------------------------------------------------------------------
    BottomNavBar {
        id: navBar

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        // Two way link with the layout. onItemSelected is what was missing
        // before: the bar emitted the signal and Main.qml ignored it, so the
        // index never moved. currentIndex bound to the layout property keeps
        // the highlighted tab in step even if the layout is changed in code.
        currentIndex: stack.activeTab
        onItemSelected: function (index) {
            stack.activeTab = index
        }
    }

    // -----------------------------------------------------------------------
    // Global error surface
    // -----------------------------------------------------------------------
    // AppController mirrors any core failure into `lastError`. Showing it as a
    // small banner keeps storage / database problems visible instead of
    // silently leaving the user with an empty grid.
    Rectangle {
        id: errorBanner

        readonly property bool active: App.lastError.length > 0

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: active ? bannerText.implicitHeight + Theme.spacingMd * 2 : 0
        visible: height > 0
        color: "#B3261E"
        clip: true

        Behavior on height {
            NumberAnimation { duration: 160 }
        }

        Text {
            id: bannerText
            anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
                margins: Theme.spacingMd
            }
            text: App.lastError
            color: "#FFFFFF"
            font.pixelSize: Theme.fontCaption
            wrapMode: Text.WordWrap
        }
    }

    // -----------------------------------------------------------------------
    // Startup overlay
    // -----------------------------------------------------------------------
    // While core initialisation is running the pages would show "No albums
    // yet", which reads like data loss, so the screen is covered with a short
    // progress state.
    //
    // If initialisation fails, this overlay gets out of the way instead of
    // spinning forever. The error banner above is then the only thing on
    // screen and it says what went wrong, which the earlier revision did not:
    // the banner used to sit underneath a full-screen spinner.
    Rectangle {
        id: readinessOverlay

        anchors.fill: parent
        visible: !App.ready && !startupSucceeded && App.lastError.length === 0
        color: Theme.background

        Column {
            anchors.centerIn: parent
            width: parent.width * 0.8
            spacing: Theme.spacingMd

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                running: readinessOverlay.visible
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: qsTr("Preparing your library\u2026")
                font.pixelSize: Theme.fontBody
                color: Theme.textSecondary
            }
        }
    }
}