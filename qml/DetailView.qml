import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: detailRoot

    Label {
        anchors.centerIn: parent
        visible: !galleryViewModel.detail.hasImage
        text: qsTr("Kliknij zdjęcie")
        color: Kirigami.Theme.disabledTextColor
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        visible: galleryViewModel.detail.hasImage
        spacing: 8

        // 1. Image Container (Fixed Aspect Ratio Box)
        Rectangle {
            id: imageContainer
            Layout.fillWidth: true
            // Keeps the image area a nice, proportional rectangle
            Layout.preferredHeight: width * 0.6 
            color: Kirigami.Theme.backgroundColor
            radius: 4
            layer.enabled: true // Smooths out the corners

            Image {
                id: selectedWallpaper
                anchors.fill: parent
                fillMode: Image.PreserveAspectCrop
                // Accessing your model data (assuming it provides a single 'imageItem')
                source: galleryViewModel.detail.imageUrl
                asynchronous: true
                
                // Fallback placeholder text while loading or if empty
                Label {
                    anchors.centerIn: parent
                    text: qsTr("No Image Selected")
                    visible: parent.status !== Image.Ready
                    color: Kirigami.Theme.disabledTextColor
                }
            }
        }

        // 2. Image Name / Title
        Label {
            id: imageNameLabel
            Layout.fillWidth: true
            text: galleryViewModel.detail.imageName !== ""
                ? galleryViewModel.detail.imageName
                : qsTr("Unknown Artwork")
            font.family: Kirigami.Theme.defaultFont.family
            font.weight: Font.Bold
            font.pointSize: 12
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
        }

        // 3. Spacer to push future buttons/options upwards
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        // 4. Placeholder for your future buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            // You can drop your buttons here later!
            // For now, it stays tucked neatly at the bottom because of the spacer above.
        }
    }
}