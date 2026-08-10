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
    title: qsTr("TapT")

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
        function onItemEditApplied(id, hue, brightness, saturation, flipped, lutPath) {
            TimelineViewModel.updateItem(id, hue, brightness, saturation, flipped, lutPath)
        }
    }

    Connections {
        target: DetailViewModel
        function onImageAdded(sourcePath, name, hue, brightness, saturation, flipped, lutPath) {
            const id = TimelineViewModel.addItem(sourcePath, name, hue, brightness, saturation, flipped, lutPath);
            DetailViewModel.setEditingItemId(id);
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
                SplitView.preferredWidth: 340
                SplitView.minimumWidth: 300
                color: Kirigami.Theme.alternateBackgroundColor
                visible: DetailViewModel.hasImage
                DetailView {
                    anchors.fill: parent
                }
            }
        }

        Rectangle {
            id: timeline
            // timelinePanel.playlistCount to
            // property wystawiona z roota TimelinePanel.qml
            // reaktywnie śledzi TimelineViewModel.queueModel.
            visible: timelinePanel.playlistCount > 0

            // Minimalna wysokość pochodzi teraz bezpośrednio z TimelinePanel
            // (rootTimeline.implicitHeight = toolbar + minimalna treść), więc
            // SplitView nigdy nie ściśnie panelu poniżej tego, czego faktycznie
            //
            SplitView.preferredHeight: Math.round(root.height * 0.20)
            SplitView.minimumHeight: timelinePanel.implicitHeight
            SplitView.maximumHeight: Math.round(root.height * 0.45)
            color: Kirigami.Theme.backgroundColor

            TimelinePanel {
                id: timelinePanel
                anchors.fill: parent
            }
        }
    }
}
