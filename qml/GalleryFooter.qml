import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: footerRoot
    color: "transparent" // Inherits styling naturally

    // Expose properties to interact with Gallery.qml
    property int currentPage: 1
    property int totalPages: 1

    // Define custom signals that Gallery.qml can listen to
    signal nextPageClicked()
    signal prevPageClicked()

    RowLayout {
        anchors.centerIn: parent
        spacing: 10
        Label {
            text: "footer"
            font.bold: true
            verticalAlignment: Text.AlignVCenter
        }
    }
}