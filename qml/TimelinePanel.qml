import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: rootTimeline
    anchors.fill: parent

    implicitHeight: 140
    Layout.minimumHeight: 140

    // Trzyma wybrany tryb z ComboBoxa: 0 = Random, 1 = Time of the day
    property int currentMode: 0

    readonly property int playlistCount: repeaterTimeline.count

    // ════════════════════════════════════════════════════════════════════════
    //  DOLNY PASEK NARZĘDZIOWY (pod obszarem podglądu tapet)
    // ════════════════════════════════════════════════════════════════════════
    RowLayout {
        id: toolbar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        spacing: 8

        Item { width: 4 }

        Button {
            text: qsTr("Configure")
            icon.name: "settings-configure"
            onClicked: configPopup.open()
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
        }

        Item { width: 4 }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  POPUP KONFIGURACYJNY
    // ════════════════════════════════════════════════════════════════════════
    Popup {
        id: configPopup
        y: toolbar.y - height - 8
        x: 8
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
                    model: [qsTr("Random"), qsTr("Time of the day")]
                    currentIndex: rootTimeline.currentMode
                    onActivated: rootTimeline.currentMode = currentIndex
                }
            }

            Kirigami.Separator { Layout.fillWidth: true }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: modeCombo.currentIndex

                ColumnLayout {
                    Label { text: qsTr("Random settings...") }
                    CheckBox { text: qsTr("Include previously used") }
                    Item { Layout.fillHeight: true }
                }

                ColumnLayout {
                    Label { text: qsTr("Time of the day settings...") }
                    CheckBox { text: qsTr("Blend transitions smoothly") }
                    Item { Layout.fillHeight: true }
                }
            }
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  PRZESTRZEŃ GŁÓWNA - STOS WIDOKÓW
    // ════════════════════════════════════════════════════════════════════════
    StackLayout {
        anchors.top: parent.top
        anchors.bottom: toolbar.top
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: rootTimeline.currentMode

        // --------------------------------------------------------------------
        // TRYB 0: RANDOM (Prosta lista Drag & Drop)
        // --------------------------------------------------------------------
        ListView {
            id: randomList
            orientation: ListView.Horizontal
            spacing: 8
            // left/right margins handle horizontal spacing from the edges of the screen
            leftMargin: 8
            rightMargin: 8
            clip: true

            model: DelegateModel {
                id: visualModel
                model: TimelineViewModel.queueModel

                delegate: DropArea {
                    id: delegateRoot
                    required property var model
                    required property int index

                    width: 120
                    // Fix 1: Emulate top and bottom container padding
                    height: randomList.height - 16
                    y: 8

                    keys: ["randomItem"]
                    onEntered: (drag) => {
                        visualModel.items.move(drag.source.visualIndex, delegateRoot.DelegateModel.itemsIndex)
                    }

                    property int visualIndex: DelegateModel.itemsIndex

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
                                ParentChange { target: itemRect; parent: randomList }
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

                            // Fix 2: Keep image inside the border bounds so they don't overlap
                            anchors.topMargin: itemRect.border.width
                            anchors.leftMargin: itemRect.border.width
                            anchors.rightMargin: itemRect.border.width
                            height: (parent.height * 0.7) - itemRect.border.width

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

                            // Fix 2 (cont): Keep label inside the border bounds
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
                    }
                }
            }
        }

        // --------------------------------------------------------------------
        // TRYB 1: TIME OF THE DAY (Skalowalna oś czasu 24h)
        // --------------------------------------------------------------------
        Item {
            id: timeContainer
            clip: true

            // Wymuszenie minimalnej wysokości w StackLayout/SplitView:
            // 20px (nagłówek godzin) + 40px (minimalna ścieżka kafelków) + 8px marginesów = 68px min.
            Layout.fillWidth: true
            Layout.fillHeight: true
            implicitHeight: 68
            Layout.minimumHeight: 68

            property real pxPerMin: width / 1440.0
            property int minSlotMinutes: 30
            property var slots: []

            function snapTo5(val) { return Math.round(val / 5) * 5; }

            function evenlyDistribute() {
                const count = repeaterTimeline.count;
                if (count === 0) {
                    slots = [];
                    return;
                }
                const newSlots = [];
                for (let i = 0; i < count; ++i) {
                    const start = Math.round(i * 1440 / count);
                    const end = (i === count - 1) ? 1440 : Math.round((i + 1) * 1440 / count);
                    newSlots.push({ startMin: start, endMin: end });
                }
                slots = newSlots;
            }

            function moveDivider(i, newMin) {
                const left = slots[i];
                const right = slots[i + 1];
                if (!left || !right) return;

                let clamped = snapTo5(newMin);
                clamped = Math.max(left.startMin + minSlotMinutes,
                                   Math.min(clamped, right.endMin - minSlotMinutes));

                const updated = slots.slice();
                updated[i] = { startMin: left.startMin, endMin: clamped };
                updated[i + 1] = { startMin: clamped, endMin: right.endMin };
                slots = updated;
            }

            function exportTimeSlots() {
                const result = [];
                for (let i = 0; i < repeaterTimeline.count; ++i) {
                    const item = repeaterTimeline.itemAt(i);
                    if (!item) continue;
                    result.push({
                        id: item.model.id,
                        name: item.model.name,
                        startMin: item.startMin,
                        endMin: item.endMin
                    });
                }
                return result;
            }

            Component.onCompleted: evenlyDistribute()

            // ── 1. Główka z etykietami godzin (Stała wysokość) ──────────────────
            Item {
                id: labelsRow
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 20

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

            // ── 2. Obszar kafelków i linii (Wypełnia resztę miejsca, min 40px) ───
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
                    onCountChanged: timeContainer.evenlyDistribute()

                    DropArea {
                        id: delegateRoot
                        required property var model
                        required property int index

                        readonly property int startMin: timeContainer.slots.length > index
                            ? timeContainer.slots[index].startMin : 0
                        readonly property int endMin: timeContainer.slots.length > index
                            ? timeContainer.slots[index].endMin : 0

                        y: 0
                        height: trackContent.height
                        x: startMin * timeContainer.pxPerMin
                        width: (endMin - startMin) * timeContainer.pxPerMin

                        keys: ["timelineItem"]
                        onEntered: (drag) => {
                            if (drag.source && drag.source.index !== delegateRoot.index)
                                TimelineViewModel.moveItem(drag.source.index, delegateRoot.index)
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
                        }
                    }
                }

                // Uchwyty granic między kafelkami
                Repeater {
                    id: dividerRepeater
                    model: Math.max(0, repeaterTimeline.count - 1)

                    Rectangle {
                        id: divider
                        required property int index

                        width: 4
                        height: parent.height
                        x: (timeContainer.slots.length > index
                            ? timeContainer.slots[index].endMin : 0) * timeContainer.pxPerMin - width / 2
                        z: 10
                        radius: 2
                        color: dividerArea.containsMouse || dividerArea.pressed
                            ? Kirigami.Theme.highlightColor
                            : Qt.rgba(1,1,1,0.25)

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
                                initialBoundary = timeContainer.slots[divider.index].endMin
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var mapped = mapToItem(trackContent, mouse.x, mouse.y)
                                    var deltaPx = mapped.x - startDragX
                                    var deltaMin = timeContainer.snapTo5(deltaPx / timeContainer.pxPerMin)
                                    timeContainer.moveDivider(divider.index, initialBoundary + deltaMin)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
