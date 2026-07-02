import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: root

    GridView {
        id: imageGrid

        anchors.fill: parent
        anchors.margins: 10
        clip: true

        model: GalleryViewModel.imagesModel

        cacheBuffer: cellHeight * 2

        ScrollBar.vertical: ScrollBar {
            id: vScrollBar
            policy: ScrollBar.AlwaysOn
            interactive: true
            contentItem: Rectangle {
                implicitWidth: 6
                radius: 3
                color: Qt.rgba(1, 1, 1, 0.4)
            }
            background: Item {}
        }

        readonly property int scrollBarWidth: vScrollBar.implicitWidth

        property int stableWidth: 0
        Component.onCompleted: stableWidth = width

        Timer {
            id: resizeDebounceTimer
            interval: 150
            repeat: false
            onTriggered: imageGrid.stableWidth = imageGrid.width
        }

        onWidthChanged: resizeDebounceTimer.restart()

        readonly property int targetCellSize: Math.max(130, Math.min(280, Math.round(Screen.width * 0.10)))
        readonly property int columns: Math.max(1, Math.floor((stableWidth - scrollBarWidth) / targetCellSize))
        readonly property int actualCellSize: Math.floor((stableWidth - scrollBarWidth) / columns)

        cellWidth: actualCellSize
        cellHeight: actualCellSize
        rightMargin: scrollBarWidth

        Timer {
            id: loadDebounce
            interval: 250
            repeat: false
            onTriggered: GalleryViewModel.loadNextBatch()
        }

        onCountChanged: {
            if (count > 0 && contentHeight <= height + cellHeight) {
                loadDebounce.restart();
            }
        }

        delegate: Item {
            id: delegateRoot
            required property int index
            required property string imageUrl
            required property string imageName

            width: imageGrid.cellWidth
            height: imageGrid.cellHeight

            readonly property bool isSelected: GalleryViewModel.selectedIndex === index

            MouseArea {
                id: cellMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: GalleryViewModel.selectImage(delegateRoot.index)

                Rectangle {
                    id: glowRect
                    anchors.centerIn: parent
                    width: parent.width - 4
                    height: parent.height - 4
                    radius: 10
                    color: Kirigami.Theme.highlightColor
                    // visible: cellMouseArea.containsMouse  // skip paint when hidden
                    opacity: cellMouseArea.containsMouse ? 0.3 : 0.0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }

                    layer.enabled: opacity > 0
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

                        //sourceSize.width: 256
                        //.height: 256

                        cache: true

                        opacity: status === Image.Ready ? 1.0 : 0.0
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 200
                            }
                        }
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

                    // Timer {
                    //     id: indicatorDelay
                    //     interval: 150
                    //     running: galleryImage.status === Image.Loading
                    // }

                    // BusyIndicator {
                    //     anchors.centerIn: parent
                    //     running: indicatorDelay.running // Bind to the timer, not the image directly
                    //     visible: running
                    // }
                    BusyIndicator {
                        anchors.centerIn: parent
                        running: galleryImage.status === Image.Loading
                        opacity: running ? 1.0 : 0.0

                        Behavior on opacity {
                            NumberAnimation {
                                duration: 150
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            }
        }

        readonly property bool approachingEnd: count > 0 && (contentY + height >= contentHeight - cellHeight * 3)

        onApproachingEndChanged: {
            if (approachingEnd) {
                loadDebounce.restart();
            }
        }
    }
}
