import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Kirigami.ApplicationWindow {
    id: root
    width: Math.round(Screen.width * 0.666)
    height: Math.round(Screen.height * 0.666)
    minimumWidth: 800
    minimumHeight: 600
    title: qsTr("Tapt")

    Connections {
        target: GalleryViewModel
        function onImageSelected(url, name) {
            DetailViewModel.setImage(url, name);
        }
    }

    Connections {
        target: TimelineViewModel
        function onItemRequestedForEditing(id, sourcePath, name, hue, brightness, saturation, flipped, lutPath) {
            DetailViewModel.loadForEditing(id, sourcePath, name, hue, brightness, saturation, flipped, lutPath);
        }
    }

    Connections {
        target: DetailViewModel
        function onImageAdded(sourcePath, name, hue, brightness, saturation, flipped, lutPath) {
            TimelineViewModel.addItem(sourcePath, name, hue, brightness, saturation, flipped, lutPath);
        }
    }

    SplitView {
        id: mainSplit
        anchors.fill: parent
        orientation: Qt.Vertical

        SplitView {
            id: topSplit
            SplitView.fillHeight: true
            orientation: Qt.Horizontal

            Rectangle {
                id: p1
                SplitView.fillWidth: true
                SplitView.minimumWidth: 200
                color: Kirigami.Theme.backgroundColor

                Gallery {
                    anchors.fill: parent
                }
            }

            Rectangle {
                id: detailsWindow
                SplitView.preferredWidth: 220
                SplitView.minimumWidth: 120
                color: Kirigami.Theme.alternateBackgroundColor
                visible: DetailViewModel.hasImage
                DetailView {
                    anchors.fill: parent
                }
            }
        }

        Rectangle {
            id: timeline
            SplitView.preferredHeight: Math.round(root.height * 0.15)
            SplitView.minimumHeight: Math.round(root.height * 0.08)
            SplitView.maximumHeight: Math.round(root.height * 0.35)
            color: Kirigami.Theme.backgroundColor

            TimelinePanel {
                anchors.fill: parent
            }
            // ← drop your P3 component here
            // Label {
            //     anchors.centerIn: parent
            //     text: "Timeline"
            //     font.pointSize: 18
            // }
        }
    }
}
