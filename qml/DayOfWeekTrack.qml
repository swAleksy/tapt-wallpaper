import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: root
    clip: true

    Layout.fillWidth: true
    Layout.fillHeight: true
    implicitHeight: 68
    Layout.minimumHeight: 68

    readonly property int dayCount: 7
    readonly property var dayNames: [
        qsTr("Mon"), qsTr("Tue"), qsTr("Wed"), qsTr("Thu"),
        qsTr("Fri"), qsTr("Sat"), qsTr("Sun")
    ]

    property real pxPerDay: width / dayCount
    readonly property int scheduledCount: Math.min(repeaterDay.count, dayCount)

    Component.onCompleted: TimelineViewModel.distributeWeekdaysEvenly()

    // ── 1. Główka z etykietami dni tygodnia (stała, 7 równych kolumn) ───────
    Item {
        id: labelsRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.min(20, root.height * 0.4)

        Row {
            anchors.fill: parent
            Repeater {
                model: root.dayCount
                Item {
                    width: labelsRow.width / root.dayCount
                    height: labelsRow.height
                    Label {
                        anchors.centerIn: parent
                        text: root.dayNames[index]
                        color: Kirigami.Theme.disabledTextColor
                        font.pointSize: 8
                        font.bold: true
                    }
                }
            }
        }
    }

    // ── 2. Obszar kafelków i linii ───────────────────────────────────────
    Item {
        id: trackContent
        anchors.top: labelsRow.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.bottomMargin: 4

        // Tło z pionowymi liniami rozdzielającymi 7 dni
        Row {
            anchors.fill: parent
            Repeater {
                model: root.dayCount
                Rectangle {
                    width: trackContent.width / root.dayCount
                    height: trackContent.height
                    color: "transparent"
                    border.color: Qt.rgba(1,1,1,0.05)
                    border.width: 1
                }
            }
        }

        // Kafelki — jeden na obraz, maks. 7
        Repeater {
            id: repeaterDay
            model: TimelineViewModel.queueModel
            onCountChanged: TimelineViewModel.distributeWeekdaysEvenly()

            DropArea {
                id: delegateRoot
                required property var model
                required property int index

                visible: index >= 0 && index < root.scheduledCount

                function getStartDay(mask) {
                        if (!mask) return 0;
                        let lo = 0; while (!(mask & (1 << lo))) ++lo; return lo;
                    }

                function getEndDay(mask) {
                    if (!mask) return 0;
                    let hi = 6; while (!(mask & (1 << hi))) --hi; return hi + 1;
                }

                readonly property int startDay: index >= 0 ? getStartDay(model.weekdayMask) : 0
                readonly property int endDay: index >= 0 ? getEndDay(model.weekdayMask) : 0

                y: 0
                height: trackContent.height
                x: startDay * root.pxPerDay
                width: (endDay - startDay) * root.pxPerDay

                keys: ["dayItem"]
                onEntered: (drag) => {
                    if (drag.source && drag.source.index !== delegateRoot.index
                        && delegateRoot.index < root.scheduledCount) {
                        TimelineViewModel.moveItem(drag.source.index, delegateRoot.index)
                        TimelineViewModel.distributeWeekdaysEvenly() // <-- Forces layout refresh
                    }
                }

                Rectangle {
                    id: itemRect
                    anchors.fill: parent
                    color: Kirigami.Theme.alternateBackgroundColor
                    border.color: dragArea.drag.active ? Kirigami.Theme.highlightColor : Qt.rgba(1,1,1,0.2)
                    border.width: dragArea.drag.active ? 2 : 1
                    radius: 4
                    clip: true

                    Drag.active: dragArea.drag.active
                    Drag.source: delegateRoot
                    Drag.keys: ["dayItem"]
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2

                    states: [
                        State {
                            when: itemRect.Drag.active
                            ParentChange { target: itemRect; parent: trackContent }
                            PropertyChanges {
                                target: itemRect
                                opacity: 0.85
                                anchors.fill: undefined
                                width: delegateRoot.width
                                height: delegateRoot.height
                            }
                        }
                    ]

                    PreviewImage {
                        id: squarePreview
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: itemRect.border.width + 4

                        height: Math.max(10, parent.height - (itemRect.border.width * 2) - 4)
                        width: height

                        source: model.sourcePath
                        fillMode: Image.PreserveAspectCrop
                        hue: model.hue
                        brightness: model.brightness
                        saturation: model.saturation
                        flipped: model.flipped
                        lutPath: model.lutPath
                        sourceSize: Qt.size(Math.round(height), Math.round(height))
                    }

                    Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 4

                        text: model.name || qsTr("Unknown")
                        font.pixelSize: 10
                        opacity: 0.6
                        color: "white"
                        style: Text.Outline
                        styleColor: "#80000000"
                    }

                    MouseArea {
                        id: dragArea
                        anchors.fill: parent
                        drag.target: itemRect
                        drag.axis: Drag.XAxis
                        cursorShape: dragArea.drag.active ? Qt.ClosedHandCursor : Qt.PointingHandCursor
                        onClicked: TimelineViewModel.editItem(model.id)
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
                            onClicked: TimelineViewModel.removeItem(model.id)
                        }
                    }
                }
            }
        }

        // Uchwyty granic między kafelkami
        Repeater {
            id: dividerRepeater
            model: TimelineViewModel.queueModel

            Rectangle {
                id: divider
                required property int index
                required property var model

                visible: index < root.scheduledCount - 1

                width: 4
                height: parent.height


                function getEndDay(mask) {
                    if (!mask) return 0;
                    let hi = 6; while (!(mask & (1 << hi))) --hi; return hi + 1;
                }
                readonly property int endDay: getEndDay(model.weekdayMask)

                x: endDay * root.pxPerDay - width / 2
                z: 10

                radius: 2
                color: dividerArea.containsMouse || dividerArea.pressed
                    ? Kirigami.Theme.highlightColor
                    : Kirigami.Theme.disabledTextColor
                opacity: dividerArea.containsMouse || dividerArea.pressed ? 1.0 : 0.5

                MouseArea {
                    id: dividerArea
                    anchors.fill: parent
                    anchors.margins: -4
                    hoverEnabled: true
                    cursorShape: Qt.SizeHorCursor

                    property real startDragX: 0
                    property int initialBoundary: 0

                    onPressed: (mouse) => {
                        var mapped = mapToItem(trackContent, mouse.x, mouse.y)
                        startDragX = mapped.x
                        initialBoundary = divider.endDay // <-- Update this too
                    }

                    onPositionChanged: (mouse) => {
                        if (pressed) {
                            var mapped = mapToItem(trackContent, mouse.x, mouse.y)
                            var deltaPx = mapped.x - startDragX
                            var deltaDay = deltaPx / root.pxPerDay
                            TimelineViewModel.moveWeekdayDivider(divider.index, initialBoundary + deltaDay)
                        }
                    }
                }
            }
        }
    }
}
