import QtQuick
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// SectionHeader
// ---------------------------------------------------------------------------
// Sticky-free section title used on the Timeline / Memories pages, e.g.
// "Today" or "August 2025". Kept as its own component so a later release can
// turn it into a pinned header without editing every page.
// ---------------------------------------------------------------------------
Item {
    id: root

    property string title: ""
    property string subtitle: ""
    property int itemCount: 0

    implicitHeight: content.implicitHeight + Theme.spacingMd

    Column {
        id: content
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: Theme.spacingMd
            rightMargin: Theme.spacingMd
        }
        spacing: 2

        Row {
            spacing: Theme.spacingSm

            Text {
                id: titleText
                text: root.title
                font.pixelSize: Theme.fontHeading
                font.weight: Font.DemiBold
                color: Theme.textPrimary
            }

            Text {
                visible: root.itemCount > 0
                anchors.baseline: titleText.baseline
                text: root.itemCount
                font.pixelSize: Theme.fontCaption
                color: Theme.textSecondary
            }
        }

        Text {
            visible: root.subtitle.length > 0
            text: root.subtitle
            font.pixelSize: Theme.fontCaption
            color: Theme.textSecondary
        }
    }
}