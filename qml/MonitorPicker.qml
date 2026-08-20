import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Button {
    id: monitorPickerButton

    readonly property var screens: Qt.application.screens
    readonly property var currentScreen: {
        for (const s of screens) {
            if (s.name === TimelineViewModel.currentMonitorId)
                return s;
        }
        return screens.length > 0 ? screens[0] : null;
    }

    icon.name: "video-display"
    text: currentScreen
        ? (currentScreen.manufacturer ? (currentScreen.manufacturer + " " + currentScreen.model) : currentScreen.name)
        : qsTr("No monitor")

    onClicked: monitorPickerPopup.open()

    Popup {
        id: monitorPickerPopup
        y: monitorPickerButton.height + 4
        modal: true
        focus: true
        padding: 16
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        readonly property var screens: monitorPickerButton.screens

        readonly property real virtualLeft: {
            let min = Infinity;
            for (const s of screens) min = Math.min(min, s.virtualX);
            return isFinite(min) ? min : 0;
        }
        readonly property real virtualTop: {
            let min = Infinity;
            for (const s of screens) min = Math.min(min, s.virtualY);
            return isFinite(min) ? min : 0;
        }
        readonly property real virtualRight: {
            let max = -Infinity;
            for (const s of screens) max = Math.max(max, s.virtualX + s.width);
            return isFinite(max) ? max : 1;
        }
        readonly property real virtualBottom: {
            let max = -Infinity;
            for (const s of screens) max = Math.max(max, s.virtualY + s.height);
            return isFinite(max) ? max : 1;
        }
        readonly property real virtualWidth: Math.max(1, virtualRight - virtualLeft)
        readonly property real virtualHeight: Math.max(1, virtualBottom - virtualTop)


        readonly property real maxLayoutWidth: 460
        readonly property real maxLayoutHeight: 220
        readonly property real scaleFactor: Math.min(
            maxLayoutWidth / virtualWidth,
            maxLayoutHeight / virtualHeight)

        contentItem: ColumnLayout {
            spacing: 12

            Label {
                text: qsTr("Choose a monitor")
                font.bold: true
            }

            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: monitorPickerPopup.virtualWidth * monitorPickerPopup.scaleFactor
                Layout.preferredHeight: monitorPickerPopup.virtualHeight * monitorPickerPopup.scaleFactor

                Repeater {
                    model: monitorPickerPopup.screens

                    delegate: Rectangle {
                        id: tile
                        required property var modelData

                        readonly property bool isSelected: modelData.name === TimelineViewModel.currentMonitorId

                        x: (modelData.virtualX - monitorPickerPopup.virtualLeft) * monitorPickerPopup.scaleFactor
                        y: (modelData.virtualY - monitorPickerPopup.virtualTop) * monitorPickerPopup.scaleFactor
                        width: modelData.width * monitorPickerPopup.scaleFactor
                        height: modelData.height * monitorPickerPopup.scaleFactor

                        radius: 6
                        color: Kirigami.Theme.backgroundColor
                        border.width: 2
                        border.color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor

                        ColumnLayout {
                            anchors.centerIn: parent
                            width: parent.width - 12
                            spacing: 2

                            Label {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                elide: Text.ElideRight
                                font.pixelSize: 11
                                text: modelData.manufacturer
                                    ? (modelData.manufacturer + " " + modelData.model)
                                    : modelData.name
                            }
                            Label {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                opacity: 0.7
                                font.pixelSize: 10
                                text: "(%1 × %2)".arg(modelData.width).arg(modelData.height)
                            }
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 4
                            radius: 2
                            color: tile.isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                TimelineViewModel.currentMonitorId = tile.modelData.name;
                                monitorPickerPopup.close();
                            }
                        }
                    }
                }
            }
        }
    }
}
