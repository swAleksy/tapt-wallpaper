import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

// Pozioma lista kolejki z drag & drop (miniaturki do zmiany kolejności).
// Działa w trybach "When logging in" i "On a timer" — różnica: czy/jak używana jest kolejność (losowo / sekwencyjnie).

// Fix #1: model powiązany bezpośrednio z TimelineViewModel.queueModel.
// Drag & drop wywołuje TimelineViewModel.moveItem(), więc zmienia kolejność w współdzielonym QueueModel.
// Poprzednio DelegateModel.items.move() zmieniał tylko lokalny widok ListView — bez zapisu do modelu,
// więc kolejność znikała przy zmianach i nie trafiała do eksportu.

ListView {
    id: root
    orientation: ListView.Horizontal
    spacing: 8
    // left/right margins handle horizontal spacing from the edges of the screen
    leftMargin: 8
    rightMargin: 8
    clip: true

    readonly property int cardTopPadding: 16
    readonly property int cardBottomPadding: 8

    ScrollBar.horizontal: ScrollBar {
        policy: ScrollBar.AsNeeded
        visible: root.contentWidth > root.width
    }

    model: TimelineViewModel.queueModel

    delegate: DropArea {
        id: delegateRoot
        required property var model
        required property int index

        width: 120
        height: root.height - root.cardTopPadding - root.cardBottomPadding
        y: root.cardTopPadding

        keys: ["randomItem"]
        // @param {DragEvent} drag - drag.source: DropArea (delegateRoot innego kafelka),
        //   drag.source.index: int - jego aktualna pozycja w QueueModel
        onEntered: (drag) => {
            if (drag.source && drag.source.index !== delegateRoot.index)
                TimelineViewModel.moveItem(drag.source.index, delegateRoot.index)
        }

        Rectangle {
            id: itemRect
            anchors.fill: parent
            color: Kirigami.Theme.alternateBackgroundColor
            border.color: dragArea.drag.active ? Kirigami.Theme.highlightColor : Qt.rgba(1,1,1,0.1)
            border.width: dragArea.drag.active ? 2 : 1
            radius: 6
            clip: true

            Drag.active: dragArea.drag.active
            Drag.source: delegateRoot
            Drag.keys: ["randomItem"]
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            states: [
                State {
                    when: itemRect.Drag.active
                    ParentChange { target: itemRect; parent: root }
                    PropertyChanges {
                        target: itemRect
                        opacity: 0.8
                        anchors.fill: undefined
                        width: delegateRoot.width
                        height: delegateRoot.height
                    }
                }
            ]

            PreviewImage {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                anchors.topMargin: itemRect.border.width + 4
                anchors.leftMargin: itemRect.border.width
                anchors.rightMargin: itemRect.border.width
                height: (parent.height * 0.7) - itemRect.border.width - 4

                source: model.sourcePath
                fillMode: Image.PreserveAspectCrop

                hue: delegateRoot.model.hue
                brightness: delegateRoot.model.brightness
                saturation: delegateRoot.model.saturation
                flipped: delegateRoot.model.flipped
                lutPath: delegateRoot.model.lutPath

                sourceSize: Qt.size(width, height)
            }

            Label {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                anchors.bottomMargin: itemRect.border.width
                anchors.leftMargin: itemRect.border.width
                anchors.rightMargin: itemRect.border.width
                height: (parent.height * 0.3) - itemRect.border.width

                text: model.name || qsTr("Unknown")
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                font.pointSize: 9
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                drag.target: parent
                cursorShape: Qt.OpenHandCursor
                onPressed: cursorShape = Qt.ClosedHandCursor
                onReleased: cursorShape = Qt.OpenHandCursor
                onClicked: TimelineViewModel.editItem(delegateRoot.model.id)
            }

            HoverHandler {
                id: hoverHandler
            }

            Rectangle {
                id: deleteButton
                width: 18
                height: 18
                radius: 9
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 4
                z: 10
                color: deleteArea.containsMouse ? "#e74c3c" : Qt.rgba(0, 0, 0, 0.6)
                visible: hoverHandler.hovered && !dragArea.drag.active

                Label {
                    anchors.centerIn: parent
                    text: "✕"
                    color: "white"
                    font.pixelSize: 10
                    font.bold: true
                }

                MouseArea {
                    id: deleteArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: TimelineViewModel.removeItem(delegateRoot.model.id)
                }
            }
        }
    }
}
