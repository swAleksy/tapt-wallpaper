import QtQuick
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
        onAccepted: {
            galleryViewModel.loadFolder(folderDialog.selectedFolder)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Button {
            text: "Otwórz folder i skanuj"
            Layout.margins: 10
            onClicked: folderDialog.open()
        }

        // Komunikat błędu — binding na viewModel.errorMessage
        Label {
            visible: galleryViewModel.errorMessage !== ""
            text: galleryViewModel.errorMessage
            Layout.fillWidth: true
            Layout.margins: 10
            horizontalAlignment: Text.AlignHCenter
            color: Kirigami.Theme.negativeTextColor
        }

        // Placeholder gdy nic nie załadowano i nie ma błędu
        Label {
            visible: !galleryViewModel.hasFolder && !galleryViewModel.isLoading
            text: "Wybierz folder żeby zobaczyć zdjęcia"
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: Kirigami.Theme.disabledTextColor
        }

        Label {
            visible: galleryViewModel.isEmpty && galleryViewModel.errorMessage === ""
            text: "Brak zdjęć w wybranym folderze"
            Layout.fillWidth: true
            Layout.fillHeight: true
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

        // Footer — usunęliśmy paginację więc zostaje tylko status
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "transparent"

            Label {
                anchors.centerIn: parent
                text: galleryViewModel.imageCount > 0 ? "Załadowano: " + galleryViewModel.imageCount + " zdjęć ": ""
            }
        }
    }
}