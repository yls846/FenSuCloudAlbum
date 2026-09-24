import QtQuick
import QtQuick.Controls
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// MemoriesPage
// ---------------------------------------------------------------------------
// Third tab. In the D version a "memory" is a past day that has media attached
// to it (see MemoryModel). Nothing is generated or uploaded; the entries are
// derived from the local database so the page works completely offline.
// ---------------------------------------------------------------------------
Item {
    id: root

    signal memoryActivated(int memoryIndex)

    Component.onCompleted: App.memoryModel.reload()

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
                text: qsTr("Memories")
                font.pixelSize: Theme.fontTitle
                font.weight: Font.DemiBold
                color: Theme.textPrimary
            }
        }

        // ---- empty state ----------------------------------------------------
        Item {
            width: parent.width
            height: parent.height - Theme.topBarHeight
            visible: !App.memoryModel || App.memoryModel.count === 0

            Column {
                anchors.centerIn: parent
                spacing: Theme.spacingSm

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "\u25CE" // ◎
                    font.pixelSize: 48
                    color: Theme.textDisabled
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("No memories yet")
                    font.pixelSize: Theme.fontHeading
                    color: Theme.textPrimary
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: root.width * 0.7
                    text: qsTr("Once you have photos from earlier days, they will show up here automatically.")
                    font.pixelSize: Theme.fontCaption
                    color: Theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                }
            }
        }

        // ---- memory list ----------------------------------------------------
        ListView {
            id: memoryList

            width: parent.width
            height: parent.height - Theme.topBarHeight
            visible: App.memoryModel && App.memoryModel.count > 0

            clip: true
            model: App.memoryModel
            spacing: Theme.spacingSm
            topMargin: Theme.spacingSm
            bottomMargin: Theme.spacingMd

            delegate: Item {
                id: cell

                required property int index
                required property string title
                required property var memoryDate
                required property int yearOffset
                required property int itemCount
                required property string coverPath

                width: memoryList.width
                height: 200

                Column {
                    anchors {
                        fill: parent
                        leftMargin: Theme.spacingMd
                        rightMargin: Theme.spacingMd
                    }
                    spacing: Theme.spacingSm

                    SectionHeader {
                        width: parent.width
                        title: cell.title
                        subtitle: cell.yearOffset === 0
                                  ? qsTr("This year")
                                  : qsTr("%1 year(s) ago").arg(cell.yearOffset)
                        itemCount: cell.itemCount
                    }

                    Rectangle {
                        width: parent.width
                        height: 144
                        radius: Theme.radiusLg
                        color: Theme.surfaceVariant
                        clip: true

                        Image {
                            id: cover
                            anchors.fill: parent
                            source: cell.coverPath
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: true
                            visible: status === Image.Ready
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: !cover.visible
                            text: "\u25CE"
                            font.pixelSize: 36
                            color: Theme.textDisabled
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.memoryActivated(cell.index)
                }
            }
        }
    }
}