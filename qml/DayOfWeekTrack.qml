import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

// DayOfWeekTrack
// ─────────────────────────────────────────────────────────────────────────
// Widok dla trybu "Day of week": ta sama mechanika co "Time of the day"
// (timeContainer w TimelinePanel.qml), tylko oś to nie 1440 minut tylko
// 7 jednostek-dni (Pon..Nd), a granice między kafelkami przeciąga się
// z zaokrągleniem do PEŁNYCH dni (bez podziałów typu "pół dnia").
//
// 7 dni jest zawsze w całości "zajętych" — dzielone równo między
// obrazy w kolejce (ta sama prosta formuła co evenlyDistribute() w Time of
// the day, tylko total=7 zamiast total=1440),
//
// Maks. 7 obrazów — jeśli w kolejce jest więcej, nadwyżka nie jest tu
// widoczna ani planowana (nadal istnieje w kolejce i jest dostępna w
// pozostałych trybach).
//
// Wynikowy podział jest zapisywany w QueueModel jako weekdayMask (bit i =
// dzień i) przez setWeekdayMask() — bez żadnego skomplikowanego

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
    property var slots: [] // [{startDay, endDay}, ...] w pełnych dniach (0..7)


    readonly property int scheduledCount: Math.min(repeaterDay.count, dayCount)

    function evenlyDistribute() {
        // Celowo NIE root.scheduledCount tutaj: to osobna właściwość, której
        // powiązanie może jeszcze nie zdążyć się przeliczyć w momencie, gdy
        // odpalany jest ten handler (onCountChanged) — dawało to "spóźnioną
        // o jedno" wartość (0 przy dodaniu 1. obrazu, 1 przy dodaniu 2. itd.).
        // repeaterDay.count jest tą samą właściwością, której zmiana
        // wywołała ten handler, więc czytana bezpośrednio jest zawsze
        // aktualna — tak samo jak w TimelinePanel::evenlyDistribute().
        const count = Math.min(repeaterDay.count, root.dayCount);
        if (count === 0) {
            slots = [];
            return;
        }
        const newSlots = [];
        for (let i = 0; i < count; ++i) {
            const start = Math.round(i * root.dayCount / count);
            const end = (i === count - 1) ? root.dayCount : Math.round((i + 1) * root.dayCount / count);
            newSlots.push({ startDay: start, endDay: end });
        }
        slots = newSlots;
        applyMasks();
    }

    function moveDivider(i, newDay) {
        const left = slots[i];
        const right = slots[i + 1];
        if (!left || !right) return;

        let clamped = Math.round(newDay);
        clamped = Math.max(left.startDay + 1, Math.min(clamped, right.endDay - 1));

        const updated = slots.slice();
        updated[i] = { startDay: left.startDay, endDay: clamped };
        updated[i + 1] = { startDay: clamped, endDay: right.endDay };
        slots = updated;
        applyMasks();
    }

    // Zapisuje bieżący podział jako weekdayMask na każdym zaplanowanym
    // obrazie (i czyści maskę na obrazach poza pierwszymi 7 pozycjami
    function applyMasks() {
        // Uwaga: id bierzemy bezpośrednio z modelu (QueueModel::idAt), a nie
        // z repeaterDay.itemAt(i) — tuż po wstawieniu wiersza delegat może
        // jeszcze nie istnieć, więc itemAt() potrafi chwilowo zwrócić null i
        // gubić przypisanie maski dla świeżo dodanego obrazu.
        const model = TimelineViewModel.queueModel;
        for (let i = 0; i < repeaterDay.count; ++i) {
            const id = model.idAt(i);
            if (!id) continue;

            if (i < slots.length) {
                let mask = 0;
                for (let d = slots[i].startDay; d < slots[i].endDay; ++d)
                    mask |= (1 << d);
                model.setWeekdayMask(id, mask);
            } else {
                model.setWeekdayMask(id, 0);
            }
        }
    }

    Component.onCompleted: evenlyDistribute()

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

        // Kafelki — jeden na obraz, maks. 7 (reszta ukryta, patrz scheduledCount)
        Repeater {
            id: repeaterDay
            model: TimelineViewModel.queueModel
            onCountChanged: root.evenlyDistribute()

            DropArea {
                id: delegateRoot
                required property var model
                required property int index

                visible: index >= 0 && index < root.scheduledCount

                // index >= 0: tuż po usunięciu elementu z modelu delegat, który jest w trakcie
                // niszczenia, na chwilę dostaje index === -1. Sam warunek
                // "length > index" go nie łapie , a root.slots[-1] zwraca undefined

                readonly property int startDay: (index >= 0 && root.slots.length > index)
                    ? root.slots[index].startDay : 0
                readonly property int endDay: (index >= 0 && root.slots.length > index)
                    ? root.slots[index].endDay : 0

                y: 0
                height: trackContent.height
                x: startDay * root.pxPerDay
                width: (endDay - startDay) * root.pxPerDay

                keys: ["dayItem"]
                onEntered: (drag) => {
                    if (drag.source && drag.source.index !== delegateRoot.index
                        && delegateRoot.index < root.scheduledCount) {
                        TimelineViewModel.moveItem(drag.source.index, delegateRoot.index)

                        // Fix #3: beginMoveRows/endMoveRows nie zmienia count, więc
                        // onCountChanged nie wywoła applyMasks(). Wywołujemy je bezpośrednio,
                        // bez evenlyDistribute(), by zachować ręcznie przesunięte granice.

                        root.applyMasks()
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

                    // Hover przez HoverHandler, żeby nie kolidować z dragArea
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

        // Uchwyty granic między kafelkami — przeciąganie zaokrąglone do
        // pełnych dni (bez podziałów ułamkowych, min. 1 dzień na kafelek).
        Repeater {
            id: dividerRepeater
            model: Math.max(0, root.scheduledCount - 1)

            Rectangle {
                id: divider
                required property int index

                width: 4
                height: parent.height
                x: (root.slots.length > index
                    ? root.slots[index].endDay : 0) * root.pxPerDay - width / 2
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
                        initialBoundary = root.slots[divider.index].endDay
                    }
                    onPositionChanged: (mouse) => {
                        if (pressed) {
                            var mapped = mapToItem(trackContent, mouse.x, mouse.y)
                            var deltaPx = mapped.x - startDragX
                            var deltaDay = Math.round(deltaPx / root.pxPerDay)
                            root.moveDivider(divider.index, initialBoundary + deltaDay)
                        }
                    }
                }
            }
        }
    }
}
