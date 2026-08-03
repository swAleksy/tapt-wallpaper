import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: galleryRoot
    Kirigami.Theme.inherit: true

    FolderDialog {
        id: folderDialog
        title: qsTr("Select image folder")
        currentFolder: GalleryViewModel.defaultDir
        onAccepted: GalleryViewModel.loadFolder(folderDialog.selectedFolder)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: 30
            Layout.margins: 10

            Button {
                text: qsTr("Open folder and scan")
                onClicked: folderDialog.open()
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: GalleryViewModel.imageCount > 0 ? qsTr("Loaded: %1 images").arg(GalleryViewModel.imageCount) : ""
            }
        }

        Label {
            visible: GalleryViewModel.errorMessage !== ""
            text: GalleryViewModel.errorMessage
            Layout.fillWidth: true
            Layout.margins: 10
            horizontalAlignment: Text.AlignHCenter
            color: Kirigami.Theme.negativeTextColor
        }

        Label {
            visible: (!GalleryViewModel.hasFolder && !GalleryViewModel.isLoading) || (GalleryViewModel.isEmpty && GalleryViewModel.errorMessage === "")
            text: !GalleryViewModel.hasFolder ? qsTr("Select a folder to view images") : qsTr("No images in the selected folder")
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.disabledTextColor
        }

        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: GalleryViewModel.isLoading && GalleryViewModel.imageCount === 0
            visible: GalleryViewModel.isLoading && GalleryViewModel.imageCount === 0
            running: visible
        }

        GalleryGrid {
            Layout.fillWidth: true
            Layout.fillHeight: GalleryViewModel.imageCount > 0
            visible: GalleryViewModel.imageCount > 0
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(0, 0, 0, 0.1)
        }
    }
}
