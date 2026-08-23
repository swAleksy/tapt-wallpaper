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

    readonly property int playlistCount: TimelineViewModel.queueModel.count

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

                    currentIndex: TimelineViewModel.monitorState.currentMode

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
                    OrderModeSelector {
                        value: TimelineViewModel.monitorState.loginOrderMode
                        onValueModified: TimelineViewModel.monitorState.loginOrderMode = value
                    }
                    Item { Layout.fillHeight: true }
                }

                ColumnLayout {
                    Label { text: qsTr("On a timer settings...") }
                    OrderModeSelector {
                        value: TimelineViewModel.monitorState.timerOrderMode
                        onValueModified: TimelineViewModel.monitorState.timerOrderMode = value
                    }
                    RowLayout {
                        Label { text: qsTr("Change every:") }
                        SpinBox {
                            id: timerIntervalSpin
                            from: 1
                            to: 999
                            value: TimelineViewModel.monitorState.timerIntervalValue
                            onValueModified: TimelineViewModel.monitorState.timerIntervalValue = value
                        }
                        ComboBox {
                            id: timerUnitCombo
                            model: [qsTr("Minutes"), qsTr("Hours")]
                            currentIndex: TimelineViewModel.monitorState.timerIntervalUnit
                            onActivated: TimelineViewModel.monitorState.timerIntervalUnit = currentIndex
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
        currentIndex: TimelineViewModel.monitorState.currentMode

        // --------------------------------------------------------------------
        // TRYB 0: TIME OF THE DAY
        // --------------------------------------------------------------------
        ScheduleTrack {
            id: timeTrack

            unitCount: 1440            // minutes in a day
            visibleCount: TimelineViewModel.queueModel.count
            dragKey: "timelineItem"

            // 24 hour marks ("0:00" .. "23:00"), bottom-left aligned (default style).
            labelsModel: {
                var out = []
                for (var i = 0; i < 24; ++i)
                    out.push(i + ":00")
                return out
            }

            // Reading `model.scheduleStartMin/EndMin` inside these callbacks is
            // what makes tile/divider positions react to schedule edits.
            startAt: (model, index) => model.scheduleStartMin
            endAt: (model, index) => model.scheduleEndMin
            moveDivider: (index, value) => TimelineViewModel.moveTimeSlotDivider(index, value)
            distribute: () => TimelineViewModel.distributeTimeSlotsEvenly()
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
