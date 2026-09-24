import QtQuick
import QtQuick.Controls
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// AllPhotosPage
// ---------------------------------------------------------------------------
// Second tab: every media item in display-date order.
//
// `App.mediaModel` is already sorted by display_date_taken DESC in SQL, so the
// view never sorts in the GUI thread. The grid is day-section aware via a
// repeated SectionHeader driven by TimelineModel, but v0.1 keeps it simple: one
// flat grid plus a header that shows the total count.
// ---------------------------------------------------------------------------
Item {
    id: root

    signal itemActivated(int mediaIndex)

    Component.onCompleted: App.mediaModel.reload()

    Column {
        anchors.fill: parent

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
                text: qsTr("All photos")
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
                visible: App.mediaModel ? App.mediaModel.count > 0 : false
                text: {
                    const n = App.mediaModel ? App.mediaModel.count : 0;
                    return n === 1 ? qsTr("1 item") : qsTr("%1 items").arg(n);
                }
                font.pixelSize: Theme.fontCaption
                color: Theme.textSecondary
            }
        }

        // ---- empty state ----------------------------------------------------
        Item {
            width: parent.width
            height: parent.height - Theme.topBarHeight
            visible: !App.mediaModel || App.mediaModel.count === 0

            Column {
                anchors.centerIn: parent
                spacing: Theme.spacingSm

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "\u25A3" // ▣
                    font.pixelSize: 48
                    color: Theme.textDisabled
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("No photos yet")
                    font.pixelSize: Theme.fontHeading
                    color: Theme.textPrimary
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Photos and videos you add will appear here.")
                    font.pixelSize: Theme.fontCaption
                    color: Theme.textSecondary
                }
            }
        }

        // ---- photo grid -----------------------------------------------------
        PhotoGrid {
            id: grid

            width: parent.width
            height: parent.height - Theme.topBarHeight
            visible: App.mediaModel && App.mediaModel.count > 0

            model: App.mediaModel

            onItemActivated: function (index) { root.itemActivated(index) }
        }
    }
}