import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

// Generic horizontal schedule track: a labelled axis with draggable image
// tiles separated by draggable dividers. It knows nothing about *what* the
// axis represents (minutes of a day, days of a week, ...) — all of that is
// injected through the API below.
//
// Assumes the TimelineViewModel singleton (queueModel, moveItem, editItem,
// removeItem) — it is not model-agnostic, just axis-agnostic. There's only
// ever one playlist queue in this app, so threading queueModel through as a
// separate injected property alongside the singleton-coupled callbacks below
// would add API surface without buying any real decoupling.
//
// The two concrete tracks (Time of the day in TimelinePanel.qml and
// DayOfWeekTrack.qml) differ only in:
//   - the axis scale (`unitCount`) and header labels (`labelsModel`),
//   - how a tile's start/end offset is decoded from its model row
//     (`startAt` / `endAt`),
//   - which C++ slots redistribute / move dividers (`distribute` / `moveDivider`).
Item {
    id: root
    clip: true

    Layout.fillWidth: true
    Layout.fillHeight: true
    implicitHeight: 68
    Layout.minimumHeight: 68

    // ── API ─────────────────────────────────────────────────────────────────

    // Total number of units spanning the full width (1440 minutes / 7 days).
    // Drives pxPerUnit, the px-per-unit scale used to place tiles & dividers.
    property int unitCount: 1

    // Header label texts. Its length also sets how many background separator
    // lines are drawn (24 hour marks / 7 day columns).
    property var labelsModel: []

    // How many leading queue items are actually scheduled/visible on the
    // track. Tiles with index >= visibleCount are hidden, as is the last
    // divider (a divider only sits *between* two visible tiles).
    property int visibleCount: 0

    // Drag key isolating this track's drag-and-drop from other tracks.
    property string dragKey: "scheduleItem"

    // Header label styling (time-of-day: bottom-left; day-of-week: centered/bold).
    property int labelHAlign: Text.AlignLeft
    property int labelVAlign: Text.AlignBottom
    property bool labelBold: false

    // Decode a row's start/end offset (in units) from its model. Both receive
    // the delegate's `model` so the implementation can touch the relevant role
    // (e.g. `model.scheduleStartMin` or `model.weekdayMask`) — reading that
    // role *inside the binding* is what makes the tile/divider position
    // reactive; a bare Q_INVOKABLE call would not register a dependency.
    property var startAt: function(model, index) { return 0 }
    property var endAt: function(model, index) { return 0 }

    // Move the divider at `index` so its boundary lands at `value` units
    // (raw/un-snapped; the C++ slot owns snapping & minimum widths).
    property var moveDivider: function(index, value) {}

    // Re-spread items evenly across the axis (called on completion & on count change).
    property var distribute: function() {}

    // ── Derived ──────────────────────────────────────────────────────────────
    readonly property real pxPerUnit: width / Math.max(1, unitCount)

    Component.onCompleted: root.distribute()

    // ── 1. Header with axis labels ───────────────────────────────────────────
    Item {
        id: labelsRow
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Math.min(20, root.height * 0.4)

        Row {
            anchors.fill: parent
            Repeater {
                model: root.labelsModel
                Item {
                    required property int index
                    required property var modelData
                    width: labelsRow.width / Math.max(1, root.labelsModel.length)
                    height: labelsRow.height
                    Label {
                        anchors.fill: parent
                        leftPadding: 4
                        bottomPadding: 4
                        text: parent.modelData
                        color: Kirigami.Theme.disabledTextColor
                        font.pointSize: 8
                        font.bold: root.labelBold
                        horizontalAlignment: root.labelHAlign
                        verticalAlignment: root.labelVAlign
                    }
                }
            }
        }
    }

    // ── 2. Tiles & dividers ──────────────────────────────────────────────────
    Item {
        id: trackContent
        anchors.top: labelsRow.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: 4
        anchors.bottomMargin: 4

        // Background with vertical separator lines
        Row {
            anchors.fill: parent
            Repeater {
                model: root.labelsModel.length
                Rectangle {
                    width: trackContent.width / Math.max(1, root.labelsModel.length)
                    height: trackContent.height
                    color: "transparent"
                    border.color: Qt.rgba(1,1,1,0.05)
                    border.width: 1
                }
            }
        }

        // Tiles — one per image
        Repeater {
            id: tileRepeater
            model: TimelineViewModel.queueModel
            onCountChanged: root.distribute()

            DropArea {
                id: delegateRoot
                required property var model
                required property int index

                visible: index >= 0 && index < root.visibleCount

                readonly property int startUnit: index >= 0 ? root.startAt(model, index) : 0
                readonly property int endUnit: index >= 0 ? root.endAt(model, index) : 0

                y: 0
                height: trackContent.height
                x: startUnit * root.pxPerUnit
                width: (endUnit - startUnit) * root.pxPerUnit

                keys: [root.dragKey]
                onEntered: (drag) => {
                    if (drag.source && drag.source.index !== delegateRoot.index
                        && delegateRoot.index < root.visibleCount) {
                        TimelineViewModel.moveItem(drag.source.index, delegateRoot.index)
                        root.distribute()
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
                    Drag.keys: [root.dragKey]
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

        // Dividers between adjacent tiles
        Repeater {
            id: dividerRepeater
            model: TimelineViewModel.queueModel

            Rectangle {
                id: divider
                required property int index
                required property var model

                visible: index >= 0 && index < root.visibleCount - 1

                width: 4
                height: parent.height

                // Read the role directly off this divider's own model row (via
                // endAt) rather than reaching into the tile Repeater with
                // itemAt() — itemAt() is a plain method call with no change
                // notification, so a binding built on it won't reliably refresh
                // after a drag-reorder.
                // Guard analogiczny do tile delegate wyżej (startUnit/endUnit
                // tam): index bywa przejściowo -1 (Qt Quick tak oznacza
                // delegat bez ważnego wiersza, np. w trakcie wymiany modelu
                // przy zmianie monitora) — bez tego guarda root.endAt(model,
                // index) próbuje odczytać rolę z modelu dla nieważnego
                // wiersza i dostaje undefined.
                readonly property int endUnit: index >= 0 ? root.endAt(model, index) : 0

                x: endUnit * root.pxPerUnit - width / 2
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
                        initialBoundary = divider.endUnit
                    }

                    onPositionChanged: (mouse) => {
                        if (pressed) {
                            var mapped = mapToItem(trackContent, mouse.x, mouse.y)
                            var deltaUnits = (mapped.x - startDragX) / root.pxPerUnit
                            root.moveDivider(divider.index, initialBoundary + deltaUnits)
                        }
                    }
                }
            }
        }
    }
}
