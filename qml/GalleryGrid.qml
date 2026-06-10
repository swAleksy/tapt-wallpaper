import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import org.kde.kirigami as Kirigami

Item {
    id: root

    GridView {
        id: imageGrid

        anchors.fill: parent
        anchors.margins: 10
        clip: true
        
        model: galleryViewModel.imagesModel

        readonly property int scrollBarWidth: 16
        readonly property int targetCellSize: Math.max(130, Math.min(280, Math.round(Screen.width * 0.10)))
        readonly property int columns: Math.max(1, Math.floor((width - scrollBarWidth) / targetCellSize))
        readonly property int actualCellSize: Math.floor((width - scrollBarWidth) / columns)

        cellWidth: actualCellSize
        cellHeight: actualCellSize

        rightMargin: scrollBarWidth

        maximumFlickVelocity: 2500
        flickDeceleration: 1000

        cacheBuffer: cellHeight * 4

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            interactive: true
            contentItem: Rectangle {
                implicitWidth: 6
                radius: 3
                color: Qt.rgba(1, 1, 1, 0.4)
            }
            background: Rectangle {
                color: "transparent" 
            }
        }

        Timer {
            id: loadDebounce
            interval: 100
            repeat: false
            onTriggered: galleryViewModel.loadNextBatch()
        }

        onCountChanged: {
            if (count === 0) return
            if (contentHeight <= height + cellHeight) {
                loadDebounce.restart()
            }
        }

        onContentYChanged: {
            if (count === 0) return
            if (contentY + height >= contentHeight - cellHeight * 2) {
                loadDebounce.restart()
            }
        }

        delegate: Item {
            id: delegateRoot
            required property int index
            required property string imageUrl
            required property string imageName

            width: imageGrid.cellWidth
            height: imageGrid.cellHeight

            readonly property bool isSelected: galleryViewModel.selectedIndex === index

            MouseArea {
                id: cellMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: galleryViewModel.selectImage(delegateRoot.index)

                // Glow bg 
                Rectangle {
                    id: glowRect
                    anchors.centerIn: parent
                    width: parent.width - 4
                    height: parent.height - 4
                    radius: 10
                    color: Kirigami.Theme.highlightColor
                    visible: cellMouseArea.containsMouse  // skip paint when hidden
                    opacity: cellMouseArea.containsMouse ? 0.3 : 0.0

                    Behavior on opacity {
                        NumberAnimation { duration: 150 }
                    }

                    layer.enabled: cellMouseArea.containsMouse
                    layer.effect: MultiEffect {
                        blurEnabled: true
                        blur: 1.0
                        blurMax: 28
                        paddingRect: Qt.rect(-20, -20, 40, 40)
                    }
                }

                // Card
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 5
                    color: Kirigami.Theme.alternateBackgroundColor
                    radius: 4
                    clip: true

                    border.color: delegateRoot.isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                    border.width: delegateRoot.isSelected ? 2 : 1

                    Image {
                        id: galleryImage
                        anchors.fill: parent
                        anchors.margins: 2
                        fillMode: Image.PreserveAspectCrop
                        source: delegateRoot.imageUrl
                        asynchronous: true

                        sourceSize.width: imageGrid.cellWidth
                        sourceSize.height: imageGrid.cellHeight

                        cache: true
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: delegateRoot.isSelected ? 2 : 1  // ← match border.width
                        height: 24
                        color: Qt.rgba(0, 0, 0, 0.6)
                        radius: 4

                        Text {
                            anchors.fill: parent
                            anchors.margins: 2
                            text: delegateRoot.imageName
                            color: "white"
                            font.pointSize: 9
                            elide: Text.ElideRight
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: galleryImage.status === Image.Loading
                        visible: running
                    }
                }
            }
        }
    }
}
