import QtQuick
import QtQml
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami

Item {
    id: galleryRoot
    Kirigami.Theme.inherit: true

    FolderDialog {
        id: folderDialog
        title: "Wybierz folder ze zdjęciami"
        currentFolder: galleryViewModel.defaultDir
        onAccepted: galleryViewModel.loadFolder(folderDialog.selectedFolder)
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
                text: "Otwórz folder i skanuj"
                onClicked: folderDialog.open()
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: galleryViewModel.imageCount > 0 ? "Załadowano: " + galleryViewModel.imageCount + " zdjęć": ""
            }
        }

        Label {
            visible: galleryViewModel.errorMessage !== ""
            text: galleryViewModel.errorMessage
            Layout.fillWidth: true
            Layout.margins: 10
            horizontalAlignment: Text.AlignHCenter
            color: Kirigami.Theme.negativeTextColor
        }

        Label {
            visible: !galleryViewModel.hasFolder && !galleryViewModel.isLoading
            text: "Wybierz folder żeby zobaczyć zdjęcia"
            Layout.fillWidth: true
            Layout.fillHeight: visible
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.disabledTextColor
        }

        Label {
            visible: galleryViewModel.isEmpty && galleryViewModel.errorMessage === ""
            text: "Brak zdjęć w wybranym folderze"
            Layout.fillWidth: true
            Layout.fillHeight: visible
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.disabledTextColor
        }

        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            visible: galleryViewModel.isLoading
            running: galleryViewModel.isLoading
        }

        GalleryGrid {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: galleryViewModel.imageCount > 0
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Qt.rgba(0, 0, 0, 0.1)
        }
    }
}