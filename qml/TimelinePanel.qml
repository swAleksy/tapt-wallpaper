import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

Item {
    id: rootTimeline
    anchors.fill: parent

    // Trzyma wybrany tryb z ComboBoxa: 0 = Random, 1 = Time of the day
    property int currentMode: 0

    // ════════════════════════════════════════════════════════════════════════
    //  GÓRNY PASEK NARZĘDZIOWY
    // ════════════════════════════════════════════════════════════════════════
    RowLayout {
        id: toolbar
        anchors.top: parent.top
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
        y: toolbar.height + 4
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
        anchors.top: toolbar.bottom
        anchors.bottom: parent.bottom
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
            leftMargin: 8
            rightMargin: 8
            topMargin: 8
            bottomMargin: 8
            clip: true

            model: DelegateModel {
                id: visualModel
                model: TimelineViewModel.queueModel

                delegate: DropArea {
                    id: delegateRoot
                    width: 120
                    height: randomList.height - 16

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
                                ParentChange {
                                    target: itemRect
                                    parent: randomList
                                }
                                PropertyChanges {
                                    target: itemRect
                                    opacity: 0.8
                                    anchors.fill: undefined
                                    width: delegateRoot.width
                                    height: delegateRoot.height
                                }
                            }
                        ]

                        Image {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: parent.height * 0.7
                            source: model.sourcePath || ""
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                        }

                        Label {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: parent.height * 0.3
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

            // Zmienna kluczowa - ile pikseli na 1 minutę przy aktualnej szerokości okna
            property real pxPerMin: width / 1440.0

            // Tło osi czasu z podziałką co 1 godzinę
            Row {
                anchors.fill: parent
                Repeater {
                    model: 24
                    Rectangle {
                        width: timeContainer.width / 24
                        height: timeContainer.height
                        color: "transparent"
                        border.color: Qt.rgba(1,1,1,0.05)
                        border.width: 1

                        Label {
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.margins: 4
                            text: index + ":00"
                            color: Kirigami.Theme.disabledTextColor
                            font.pointSize: 8
                        }
                    }
                }
            }

            Repeater {
                model: TimelineViewModel.queueModel

                Item {
                    id: timeItem
                    height: parent.height * 0.6
                    y: parent.height * 0.2

                    // Stan przedziału czasowego dla danego elementu
                    property int startMin: index * 120
                    property int endMin: startMin + 120
                    property int durationMin: endMin - startMin

                    // Sztywne powiązanie szerokości i x do kontenera rodzica
                    x: startMin * timeContainer.pxPerMin
                    width: durationMin * timeContainer.pxPerMin

                    function snapTo5(val) { return Math.round(val / 5) * 5; }

                    Rectangle {
                        id: timeRectVisual
                        anchors.fill: parent
                        color: Kirigami.Theme.alternateBackgroundColor
                        border.color: Qt.rgba(1,1,1,0.2)
                        radius: 4
                        clip: true

                        Image {
                            anchors.fill: parent
                            source: model.sourcePath || ""
                            fillMode: Image.PreserveAspectCrop
                            opacity: 0.5
                            asynchronous: true
                        }

                        Label {
                            anchors.centerIn: parent
                            text: model.name || qsTr("Unknown")
                            font.bold: true
                            style: Text.Outline
                            styleColor: "black"
                            color: "white"
                        }

                        // Środek - Przesuwanie na osi czasu
                        MouseArea {
                            anchors.fill: parent
                            anchors.margins: 10
                            cursorShape: Qt.SizeAllCursor

                            property real startDragX: 0
                            property int initialStartMin: 0

                            onPressed: (mouse) => {
                                // mapToItem pobiera absolutną pozycję względem głównego kontenera (timeContainer)
                                // uniemożliwia to skakanie gdy obiekt X się zmienia podczas Drag & Drop.
                                var mapped = mapToItem(timeContainer, mouse.x, mouse.y)
                                startDragX = mapped.x
                                initialStartMin = timeItem.startMin
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var mapped = mapToItem(timeContainer, mouse.x, mouse.y)
                                    var deltaPx = mapped.x - startDragX
                                    var deltaMin = snapTo5(deltaPx / timeContainer.pxPerMin)
                                    var newStart = initialStartMin + deltaMin

                                    // Limity krawędzi 00:00 - 23:59
                                    if (newStart < 0) newStart = 0
                                    if (newStart + timeItem.durationMin > 1440) newStart = 1440 - timeItem.durationMin

                                    timeItem.startMin = newStart
                                    timeItem.endMin = newStart + timeItem.durationMin
                                }
                            }
                        }

                        // LEWY CHWYT - Zmiana startu
                        MouseArea {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 10
                            cursorShape: Qt.SizeHorCursor

                            property real startDragX: 0
                            property int initialStartMin: 0

                            onPressed: (mouse) => {
                                var mapped = mapToItem(timeContainer, mouse.x, mouse.y)
                                startDragX = mapped.x
                                initialStartMin = timeItem.startMin
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var mapped = mapToItem(timeContainer, mouse.x, mouse.y)
                                    var deltaPx = mapped.x - startDragX
                                    var deltaMin = snapTo5(deltaPx / timeContainer.pxPerMin)
                                    var newStart = initialStartMin + deltaMin

                                    if (newStart < 0) newStart = 0
                                    if (newStart > timeItem.endMin - 5) newStart = timeItem.endMin - 5

                                    timeItem.startMin = newStart
                                }
                            }
                        }

                        // PRAWY CHWYT - Zmiana końca
                        MouseArea {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: 10
                            cursorShape: Qt.SizeHorCursor

                            property real startDragX: 0
                            property int initialEndMin: 0

                            onPressed: (mouse) => {
                                var mapped = mapToItem(timeContainer, mouse.x, mouse.y)
                                startDragX = mapped.x
                                initialEndMin = timeItem.endMin
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    var mapped = mapToItem(timeContainer, mouse.x, mouse.y)
                                    var deltaPx = mapped.x - startDragX
                                    var deltaMin = snapTo5(deltaPx / timeContainer.pxPerMin)
                                    var newEnd = initialEndMin + deltaMin

                                    if (newEnd > 1440) newEnd = 1440
                                    if (newEnd < timeItem.startMin + 5) newEnd = timeItem.startMin + 5

                                    timeItem.endMin = newEnd
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
