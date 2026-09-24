import QtQuick
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// AlbumCard
// ---------------------------------------------------------------------------
// One album (== one WebDAV folder) in the Albums grid. Shows a cover, the
// folder name and the media count. `relativePath` is carried through so the
// page can navigate into sub-folders without ever losing the hierarchy.
// ---------------------------------------------------------------------------
Item {
    id: root

    property string name: ""
    property string relativePath: ""
    property int mediaCount: 0
    property string coverPath: ""

    signal clicked()

    implicitHeight: column.implicitHeight

    Column {
        id: column
        anchors { left: parent.left; right: parent.right; top: parent.top }
        spacing: Theme.spacingSm

        Rectangle {
            id: coverFrame
            width: parent.width
            height: width
            radius: Theme.radiusMd
            color: Theme.surfaceVariant
            clip: true

            Image {
                id: cover
                anchors.fill: parent
                source: root.coverPath
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                cache: true
                visible: status === Image.Ready
            }

            // Placeholder shown while there is no cover yet (fresh install,
            // or a folder that only contains files the D version cannot read).
            Text {
                anchors.centerIn: parent
                visible: !cover.visible
                text: "\u25A3" // ▣
                font.pixelSize: 32
                color: Theme.textDisabled
            }
        }

        Text {
            width: parent.width
            text: root.name
            font.pixelSize: Theme.fontBody
            font.weight: Font.Medium
            color: Theme.textPrimary
            elide: Text.ElideRight
        }

        Text {
            width: parent.width
            text: root.mediaCount === 1
                  ? qsTr("1 item")
                  : qsTr("%1 items").arg(root.mediaCount)
            font.pixelSize: Theme.fontCaption
            color: Theme.textSecondary
            elide: Text.ElideRight
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}