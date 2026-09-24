import QtQuick
import QtQuick.Controls
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// AlbumsPage
// ---------------------------------------------------------------------------
// First tab. Shows the album list (one album == one WebDAV folder).
//
// The page is a pure view over `App.albums`. It never flattens anything: the
// card carries the album's relativePath so a later release can push a sub
// folder view without changing the model.
// ---------------------------------------------------------------------------
Item {
    id: root

    // Emitted when the user opens an album. The Main page owns navigation.
    signal albumActivated(string relativePath, string albumName)

    Component.onCompleted: App.refreshAlbums()

    Column {
        anchors.fill: parent
        spacing: 0

        // ---- title bar ------------------------------------------------------
        Item {
            width: parent.width
            height: Theme.topBarHeight

            Text {
                anchors {
                    left: parent.left
                    leftMargin: Theme.spacingMd
                    verticalCenter: parent.verticalCenter
                }
                text: qsTr("Albums")
                font.pixelSize: Theme.fontTitle
                font.weight: Font.DemiBold
                color: Theme.textPrimary
            }

            Text {
                anchors {
                    right: parent.right
                    rightMargin: Theme.spacingMd
                    verticalCenter: parent.verticalCenter
                }
                visible: App.albums ? App.albums.count > 0 : false
                text: App.albums ? App.albums.count : 0
                font.pixelSize: Theme.fontCaption
                color: Theme.textSecondary
            }
        }

        // ---- empty state ----------------------------------------------------
        Item {
            width: parent.width
            height: parent.height - Theme.topBarHeight
            visible: !App.albums || App.albums.count === 0

            Column {
                anchors.centerIn: parent
                spacing: Theme.spacingSm

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "\u25A6" // ▦
                    font.pixelSize: 48
                    color: Theme.textDisabled
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("No albums yet")
                    font.pixelSize: Theme.fontHeading
                    color: Theme.textPrimary
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: App.ready
                          ? qsTr("Import photos or connect a WebDAV account to get started.")
                          : qsTr("Preparing local storage\u2026")
                    font.pixelSize: Theme.fontCaption
                    color: Theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // ---- album grid -----------------------------------------------------
        GridView {
            id: albumGrid

            width: parent.width
            height: parent.height - Theme.topBarHeight
            visible: App.albums && App.albums.count > 0

            clip: true
            // Two columns on phones, matching the Google Photos album grid.
            cellWidth: (width - Theme.spacingMd * 3) / 2
            cellHeight: cellWidth + 44
            topMargin: Theme.spacingSm
            bottomMargin: Theme.spacingMd
            cacheBuffer: cellHeight * 3

            model: App.albums

            delegate: Item {
                id: cell

                required property int index
                required property string name
                required property string relativePath
                required property int mediaCount
                required property var coverMediaId

                width: albumGrid.cellWidth
                height: albumGrid.cellHeight

                AlbumCard {
                    anchors {
                        fill: parent
                        margins: Theme.spacingSm
                    }
                    name: cell.name
                    relativePath: cell.relativePath
                    mediaCount: cell.mediaCount
                    // The D version has no thumbnail pipeline yet, so the cover
                    // stays empty and AlbumCard shows its placeholder glyph.
                    // The hook is already wired to coverMediaId for v0.2.
                    coverPath: ""
                    onClicked: root.albumActivated(cell.relativePath, cell.name)
                }
            }
        }
    }
}