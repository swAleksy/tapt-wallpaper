import QtQuick
import QtQuick.Controls
import org.kde.taptwallpaper
import org.kde.kirigami as Kirigami

ScrollView {
    id: gridScroll
    clip: true


    GridView {
        id: imageGrid
        anchors.fill: parent
        anchors.margins: 10

        onContentYChanged: {
            if (count === 0) return
            if (contentY + height >= contentHeight - cellHeight * 2) {
                GalleryController.loadNextBatch()
            }
        }
            
        model: GalleryController.imageModel

        cellWidth: 130  
        cellHeight: 130

        delegate: Item {
            width: imageGrid.cellWidth
            height: imageGrid.cellHeight

            Rectangle {
                anchors.fill: parent
                anchors.margins: 3
                color: Kirigami.Theme.alternateBackgroundColor
                radius: 4
                border.color: Kirigami.Theme.disabledTextColor

                Image {
                    id: galleryImage // Nadajemy unikalne ID
                    anchors.fill: parent
                    anchors.margins: 2
                    fillMode: Image.PreserveAspectCrop
                    source: imageUrl
                    
                    sourceSize.width: imageGrid.cellWidth * 2   // 2x dla ekranów HiDPI
                    sourceSize.height: imageGrid.cellHeight * 2
                    asynchronous: true
                }

                // Pasek z nazwą pliku na dole obrazka
                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24
                    color: Qt.rgba(0, 0, 0, 0.6) // Czarne, półprzezroczyste tło
                    radius: 2

                    Text {
                        anchors.fill: parent
                        anchors.margins: 2
                        text: imageName
                        color: "white" // Biały tekst, żeby był widoczny na ciemnym tle
                        font.pointSize: 9
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    // Teraz bezpiecznie sprawdzamy status obrazka po jego ID
                    running: galleryImage.status === Image.Loading
                }
            }
        }
    }
}