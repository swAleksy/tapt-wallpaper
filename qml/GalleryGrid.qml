import QtQuick
import QtQuick.Controls
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
                color: Kirigami.Theme.disabledTextColor
                opacity: 0.6
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
            // FIX: Dodano sprawdzanie GalleryViewModel.hasMoreImages, aby zapobiec pętli nieskończonej
            if (count > 0 && contentHeight <= height + cellHeight && GalleryViewModel.hasMoreImages) {
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

                // FIX: Zastąpiono MultiEffect prostym prostokątem podświetlenia
                Rectangle {
                    id: hoverHighlight
                    anchors.fill: parent
                    // Wypełnia całą komórkę delegata, jest nieco większy niż wewnętrzna karta
                    anchors.margins: 0
                    radius: 6
                    color: Kirigami.Theme.highlightColor
                    opacity: cellMouseArea.containsMouse ? 0.35 : 0.0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 150
                        }
                    }
                }

                // Card
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
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

                        // Wystarczy czyste cellWidth / cellHeight
                        sourceSize.width: imageGrid.cellWidth
                        sourceSize.height: imageGrid.cellHeight

                        smooth: true
                        mipmap: true
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
                        anchors.margins: delegateRoot.isSelected ? 2 : 1
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
            // FIX: Dodano sprawdzanie GalleryViewModel.hasMoreImages
            if (approachingEnd && GalleryViewModel.hasMoreImages) {
                loadDebounce.restart();
            }
        }
    }
}
