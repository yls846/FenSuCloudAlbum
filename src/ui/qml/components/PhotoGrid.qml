import QtQuick
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// PhotoGrid
// ---------------------------------------------------------------------------
// Uniform square thumbnail grid used by the Albums / All photos / Memories
// pages. It is a pure view: it renders whatever list model it is given and
// emits taps. No database access, no platform calls.
// ---------------------------------------------------------------------------
GridView {
    id: root

    // Roles the delegate reads. Kept explicit so the component works with
    // any model that exposes these names (MediaModel today, a future
    // sectioned model tomorrow).
    property string coverRole: "thumbnailPath"
    property string nameRole: "displayName"

    clip: true
    cellWidth: (width - Theme.gridSpacing) / 3
    cellHeight: cellWidth
    cacheBuffer: cellHeight * 4

    signal itemActivated(int index)

    delegate: Item {
        id: cell

        required property int index
        required property string thumbnailPath
        required property bool isVideo

        width: root.cellWidth
        height: root.cellHeight

        Rectangle {
            anchors.fill: parent
            anchors.margins: Theme.gridSpacing / 2
            color: Theme.surfaceVariant
            clip: true

            Image {
                id: thumb
                anchors.fill: parent
                source: cell.thumbnailPath
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                visible: status === Image.Ready
            }

            Text {
                anchors.centerIn: parent
                visible: !thumb.visible
                text: "\u25A3"
                font.pixelSize: 28
                color: Theme.textDisabled
            }

            // Video badge: a small corner marker so the grid stays readable
            // without loading the video itself.
            Rectangle {
                visible: cell.isVideo
                anchors { right: parent.right; bottom: parent.bottom; margins: 6 }
                width: 18
                height: 18
                radius: 9
                color: Theme.scrim

                Text {
                    anchors.centerIn: parent
                    text: "\u25B6" // ▶
                    font.pixelSize: 9
                    color: "#FFFFFF"
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.itemActivated(cell.index)
        }
    }
}