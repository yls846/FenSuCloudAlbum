import QtQuick
import QtQuick.Controls
import FenSuCloudAlbum

// ---------------------------------------------------------------------------
// BottomNavBar
// ---------------------------------------------------------------------------
// Google Photos style bottom navigation: three destinations (Albums / All /
// Memories). The selected item gets a pill behind its icon, exactly like the
// reference screenshots.
// ---------------------------------------------------------------------------
Item {
    id: root

    property int currentIndex: 0
    property var items: [
        { label: qsTr("Albums"),   icon: "\u25A6" }, // ▦
        { label: qsTr("All"),      icon: "\u25A3" }, // ▣
        { label: qsTr("Memories"), icon: "\u25CE" }  // ◎
    ]

    signal itemSelected(int index)

    implicitHeight: Theme.bottomBarHeight

    Rectangle {
        anchors.fill: parent
        color: Theme.surface

        // Hairline on top, matching Material 3 navigation bar.
        Rectangle {
            anchors { left: parent.left; right: parent.right; top: parent.top }
            height: 1
            color: Theme.outline
            opacity: 0.6
        }
    }

    Row {
        anchors.fill: parent

        Repeater {
            model: root.items

            delegate: Item {
                id: cell

                required property int index
                required property var modelData

                width: root.width / root.items.length
                height: root.height

                Column {
                    anchors.centerIn: parent
                    spacing: 2

                    Rectangle {
                        id: pill
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 56
                        height: 30
                        radius: 15
                        color: cell.index === root.currentIndex
                               ? Theme.selectedPill
                               : "transparent"

                        Behavior on color {
                            ColorAnimation { duration: 120 }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: cell.modelData.icon
                            font.pixelSize: 16
                            color: cell.index === root.currentIndex
                                   ? Theme.primary
                                   : Theme.textSecondary
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: cell.modelData.label
                        font.pixelSize: Theme.fontCaption
                        font.weight: cell.index === root.currentIndex
                                     ? Font.DemiBold
                                     : Font.Normal
                        color: cell.index === root.currentIndex
                               ? Theme.primary
                               : Theme.textSecondary
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.itemSelected(cell.index)
                }
            }
        }
    }
}