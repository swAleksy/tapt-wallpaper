import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

ScrollView {
    id: gridScroll
    clip: true

    GridView {
        id: imageGrid
        anchors.fill: parent
        anchors.margins: 10

        model: galleryViewModel.imagesModel

        cellWidth: 130
        cellHeight: 130

        onContentYChanged: {
            if (count === 0) return
            if (contentY + height >= contentHeight - cellHeight * 2) {
                galleryViewModel.loadNextBatch()
            }
        }

        delegate: Item {
            required property int    index
            required property string imageUrl  
            required property string imageName 

            width: imageGrid.cellWidth
            height: imageGrid.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: 5
                color: Kirigami.Theme.alternateBackgroundColor
                radius: 4
                border.color: Kirigami.Theme.disabledTextColor
                MouseArea {
                    anchors.fill: parent

                    Image {
                        id: galleryImage
                        anchors.fill: parent
                        anchors.margins: 2
                        fillMode: Image.PreserveAspectCrop
                        source: imageUrl
                        asynchronous: true
                        sourceSize.width: imageGrid.cellWidth * 2
                        sourceSize.height: imageGrid.cellHeight * 2
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 24
                        color: Qt.rgba(0, 0, 0, 0.6)
                        radius: 2

                        Text {
                            anchors.fill: parent
                            anchors.margins: 2
                            text: imageName
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
                    }

                    onClicked: {
                        galleryViewModel.selectImage(index) 
                    }
                }
            }
        }
    }
}