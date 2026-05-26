import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: galleryRoot

    Kirigami.Theme.inherit: true

    property int imageCount: -1
    

    FolderDialog {
        id: folderDialog
        title: "Select Folder"
        options: FolderDialog.ShowDirsOnly
        onAccepted: {
            GalleryController.requestScan(folderDialog.selectedFolder)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Button {
            text: "Open folder and scan"
            Layout.margins: 10
            onClicked: folderDialog.open()
        }

        // 1. Siatka zdjęć (Plik: GalleryGrid.qml)
        
        GalleryGrid {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: galleryRoot.imageCount !== 0
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: galleryRoot.imageCount === 0
            text: "No images found"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: galleryRoot.imageCount === -1
            text: "---"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        // Linia podziału
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(0, 0, 0, 0.1)
        }

        GalleryFooter {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
        }
    }

    Connections {
    target: GalleryController
        function onScanFinished(count) {
            galleryRoot.imageCount = count
        }
    }
}