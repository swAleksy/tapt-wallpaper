import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: rootTimeline
    anchors.fill: parent

    readonly property int toolbarHeight: 40
    readonly property int contentMinHeight: 92
    implicitHeight: toolbarHeight + contentMinHeight
    Layout.minimumHeight: implicitHeight

    readonly property int playlistCount: repeaterTimeline.count

    // ════════════════════════════════════════════════════════════════════════
    //  DOLNY PASEK NARZĘDZIOWY
    // ════════════════════════════════════════════════════════════════════════
    RowLayout {
        id: toolbar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: rootTimeline.toolbarHeight
        spacing: 8

        Item { width: 4 }

        Button {
            text: qsTr("Configure")
            icon.name: "settings-configure"
            onClicked: configPopup.open()
        }

        Button {
            text: qsTr("Clear playlist")
            icon.name: "edit-clear-all"
            enabled: rootTimeline.playlistCount > 0
            onClicked: clearPlaylistDialog.open()
        }

        Item { Layout.fillWidth: true }

        Button {
            text: qsTr("Cancel")
            icon.name: "dialog-cancel"
        }
        Button {
            text: qsTr("Save playlist")
            highlighted: true
            icon.name: "document-save"
            onClicked: {
                const path = StandardPaths.writableLocation(StandardPaths.AppDataLocation) + "/playlist.json"
                if (!TimelineViewModel.exportPlaylist(path))
                    console.warn("Nie udało się zapisać playlisty:", path)
            }
        }

        Item { width: 4 }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  POPUP KONFIGURACYJNY
    // ════════════════════════════════════════════════════════════════════════
    Popup {
        id: configPopup
        parent: toolbar
        x: 0
        y: -height - 8

        width: 320
        leftMargin: 12
        rightMargin: 12
        topMargin: 12
        bottomMargin: 12
        modal: true
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            border.color: Kirigami.Theme.highlightColor
            border.width: 1
            radius: 6
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Label {
                    text: qsTr("Change wallpaper:")
                    font.bold: true
                }
                ComboBox {
                    id: modeCombo
                    Layout.fillWidth: true
                    model: [
                        qsTr("Time of the day"),
                        qsTr("When logging in"),
                        qsTr("On a timer"),
                        qsTr("Day of week")
                    ]

                    currentIndex: TimelineViewModel.currentMode

                    onActivated: (index) => {
                        TimelineViewModel.switchMode(index);
                    }
                }

            }

            Kirigami.Separator { Layout.fillWidth: true }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: modeCombo.currentIndex

                ColumnLayout {
                    Label { text: qsTr("Time of the day settings...") }
                    CheckBox { text: qsTr("Blend transitions smoothly") }
                    Item { Layout.fillHeight: true }
                }

                ColumnLayout {
                    Label { text: qsTr("When logging in settings...") }
                    RowLayout {
                        Label { text: qsTr("Order:") }
                        RadioButton {
                            text: qsTr("Random")
                            checked: TimelineViewModel.loginOrderMode === 0
                            onToggled: if (checked) TimelineViewModel.loginOrderMode = 0
                        }
                        RadioButton {
                            text: qsTr("Ordered")
                            checked: TimelineViewModel.loginOrderMode === 1
                            onToggled: if (checked) TimelineViewModel.loginOrderMode = 1
                        }
                    }
                    Item { Layout.fillHeight: true }
                }

                ColumnLayout {
                    Label { text: qsTr("On a timer settings...") }
                    RowLayout {
                        Label { text: qsTr("Order:") }
                        RadioButton {
                            text: qsTr("Random")
                            checked: TimelineViewModel.timerOrderMode === 0
                            onToggled: if (checked) TimelineViewModel.timerOrderMode = 0
                        }
                        RadioButton {
                            text: qsTr("Ordered")
                            checked: TimelineViewModel.timerOrderMode === 1
                            onToggled: if (checked) TimelineViewModel.timerOrderMode = 1
                        }
                    }
                    RowLayout {
                        Label { text: qsTr("Change every:") }
                        SpinBox {
                            id: timerIntervalSpin
                            from: 1
                            to: 999
                            value: TimelineViewModel.timerIntervalValue
                            onValueModified: TimelineViewModel.timerIntervalValue = value
                        }
                        ComboBox {
                            id: timerUnitCombo
                            model: [qsTr("Minutes"), qsTr("Hours")]
                            currentIndex: TimelineViewModel.timerIntervalUnit
                            onActivated: TimelineViewModel.timerIntervalUnit = currentIndex
                        }
                    }
                    Item { Layout.fillHeight: true }
                }

                ColumnLayout {
                    Label { text: qsTr("Day of week settings...") }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("The week is split evenly among the images in the queue (up to 7 — one per day). Drag the handles on the track to adjust how many days each image gets.")
                        wrapMode: Text.WordWrap
                        opacity: 0.7
                        font.pointSize: 9
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }

    Dialog {
        id: clearPlaylistDialog
        title: qsTr("Clear playlist?")
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.No

        Label {
            width: 260
            wrapMode: Text.WordWrap
            text: qsTr("This removes all %1 wallpapers from the playlist. This can't be undone.")
                .arg(rootTimeline.playlistCount)
        }

        onAccepted: TimelineViewModel.clearPlaylist()
    }

    // ════════════════════════════════════════════════════════════════════════
    //  PRZESTRZEŃ GŁÓWNA - STOS WIDOKÓW
    // ════════════════════════════════════════════════════════════════════════
    StackLayout {
        anchors.top: parent.top
        anchors.bottom: toolbar.top
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: TimelineViewModel.currentMode

        // --------------------------------------------------------------------
        // TRYB 0: TIME OF THE DAY
        // --------------------------------------------------------------------
        Item {
            id: timeContainer
            clip: true

            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 68
            Layout.minimumHeight: 68

            property real pxPerMin: width / 1440.0

            Component.onCompleted: TimelineViewModel.distributeTimeSlotsEvenly()

            // ── 1. Główka z etykietami godzin ──────────────────────────────────
            Item {
                id: labelsRow
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                height: Math.min(20, timeContainer.height * 0.4)

                Row {
                    anchors.fill: parent
                    Repeater {
                        model: 24
                        Item {
                            width: labelsRow.width / 24
                            height: labelsRow.height
                            Label {
                                anchors.bottom: parent.bottom
                                anchors.left: parent.left
                                anchors.margins: 4
                                text: index + ":00"
                                color: Kirigami.Theme.disabledTextColor
                                font.pointSize: 8
                            }
                        }
                    }
                }
            }

            // ── 2. Obszar kafelków i linii ─────────────────────────────────────
            Item {
                id: trackContent
                anchors.top: labelsRow.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.topMargin: 4
                anchors.bottomMargin: 4

                // Tło osi czasu z pionowymi liniami co 1 godzinę
                Row {
                    anchors.fill: parent
                    Repeater {
                        model: 24
                        Rectangle {
                            width: trackContent.width / 24
                            height: trackContent.height
                            color: "transparent"
                            border.color: Qt.rgba(1,1,1,0.05)
                            border.width: 1
                        }
                    }
                }

                // Kafelki
                Repeater {
                    id: repeaterTimeline
                    model: TimelineViewModel.queueModel
                    onCountChanged: TimelineViewModel.distributeTimeSlotsEvenly()

                    DropArea {
                        id: delegateRoot
                        required property var model
                        required property int index

                        readonly property int startMin: index >= 0 ? model.scheduleStartMin : 0
                        readonly property int endMin: index >= 0 ? model.scheduleEndMin : 0

                        y: 0
                        height: trackContent.height
                        x: startMin * timeContainer.pxPerMin
                        width: (endMin - startMin) * timeContainer.pxPerMin

                        keys: ["timelineItem"]
                        onEntered: (drag) => {
                            if (drag.source && drag.source.index !== delegateRoot.index) {
                                TimelineViewModel.moveItem(drag.source.index, delegateRoot.index)
                                TimelineViewModel.distributeTimeSlotsEvenly()
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
                            Drag.keys: ["timelineItem"]
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

                        visible: index < repeaterTimeline.count - 1

                        width: 4
                        height: parent.height

                        // Bind to the reactive delegate instead of the non-reactive Q_INVOKABLE
                        property var targetDelegate: repeaterTimeline.itemAt(index)
                        readonly property int endMin: model.scheduleEndMin

                        x: endMin * timeContainer.pxPerMin - width / 2
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
                                initialBoundary = divider.endMin
                            }

                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var mapped = mapToItem(trackContent, mouse.x, mouse.y)
                                    var deltaMin = (mapped.x - startDragX) / timeContainer.pxPerMin
                                    TimelineViewModel.moveTimeSlotDivider(divider.index, initialBoundary + deltaMin)
                                }
                            }
                        }
                    }
                }
            }
        }

        // --------------------------------------------------------------------
        // TRYB 1: WHEN LOGGING IN
        // --------------------------------------------------------------------
        SimpleQueueList {
            id: loginQueueList
        }

        // --------------------------------------------------------------------
        // TRYB 2: ON A TIMER
        // --------------------------------------------------------------------
        SimpleQueueList {
            id: timerQueueList
        }

        // --------------------------------------------------------------------
        // TRYB 3: DAY OF WEEK
        // --------------------------------------------------------------------
        DayOfWeekTrack {
            id: dayOfWeekTrack
        }
    }
}
