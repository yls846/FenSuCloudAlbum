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

    // Kick off core initialisation as soon as the window exists. The call is
    // idempotent, so a re-creation of the window (rotation) is harmless.
    Component.onCompleted: {
        if (!App.ready)
            App.initialize();
    }

    // -----------------------------------------------------------------------
    // Content
    // -----------------------------------------------------------------------
    StackLayout {
        id: stack

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: navBar.top
        }

        currentIndex: navBar.currentIndex

        AlbumsPage {
            onAlbumActivated: function (relativePath, albumName) {
                // v0.1 has no album detail screen yet. The hook exists so the
                // navigation model (relativePath preserved, never flattened)
                // is already correct when the sub folder view lands.
                console.log("Open album:", albumName, "at", relativePath);
            }
        }

        AllPhotosPage {
            onItemActivated: function (mediaIndex) {
                console.log("Open media index:", mediaIndex);
            }
        }

        MemoriesPage {
            onMemoryActivated: function (memoryIndex) {
                console.log("Open memory index:", memoryIndex);
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

        currentIndex: 0
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
    // Storage readiness overlay
    // -----------------------------------------------------------------------
    // While the database is being created / migrated the pages would show
    // "No albums yet", which reads like data loss. Cover the screen with a
    // short progress state instead.
    Rectangle {
        anchors.fill: parent
        visible: !App.ready
        color: Theme.background

        Column {
            anchors.centerIn: parent
            spacing: Theme.spacingMd

            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter
                running: parent.parent.visible
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Preparing your library\u2026")
                font.pixelSize: Theme.fontBody
                color: Theme.textSecondary
            }
        }
    }
}