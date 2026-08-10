import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

//  ─────────────────────────────────────────────────────────────────────────────
//  READ Properties
//    hasImage          : bool
//    imageUrl          : url
//    imageName         : string
//    hue               : real   // hueMin … hueMax (−1.0 … +1.0)
//    brightness        : real   // brightnessMin … brightnessMax (−0.5 … +0.5)
//    saturation        : real   // saturationMin … saturationMax (−0.9 … +0.9)
//    flipped           : bool
//    activeFilterIndex : int    // −1 = brak
//    filtersModel      : model  // role: name (string)
//    hueMin/Max        : real   // limity suwaka barwy
//    brightnessMin/Max : real   // limity suwaka jasności
//    saturationMin/Max : real   // limity suwaka nasycenia
//
//  Signals
//    imageLoaded()
//
//  Invokables
//    applyChanges(hue, brightness, saturation, flipped, filterIndex)
//      — wołane automatycznie z debounce 500 ms po zmianie suwaka/filtra/
//        odbicia (patrz autoApplyTimer poniżej); przycisk "Zastosuj" tylko
//        pomija to opóźnienie.
//    revertChanges()
//      — czyści korekty i filtr do zera (nie do ostatnio zatwierdzonych
//        wartości).
//    setAsWallpaper()
//    addToPlaylist()

Item {
    id: detailRoot

    // ── Lokalny stan podglądu (niezatwierdzony w VM) ─────────────────────────
    property real previewHue: DetailViewModel.hue
    property real previewBrightness: DetailViewModel.brightness
    property real previewSaturation: DetailViewModel.saturation
    property bool previewFlipped: DetailViewModel.flipped
    property int previewFilterIndex: DetailViewModel.activeFilterIndex

    // PreviewImage nie zna pojęcia "indeksu w liście filtrów" (to szczegół UI
    // tego widoku) — przyjmuje gotowe lutPath/lutSize, więc rozwiązanie
    // indeksu robimy tutaj, przy pomocy tego samego lutFiltersListModel.
    readonly property string previewLutPath: (previewFilterIndex >= 0 && DetailViewModel.lutFiltersListModel)
        ? DetailViewModel.lutFiltersListModel.lutPath(previewFilterIndex)
        : ""
    readonly property real previewLutSize: (previewFilterIndex >= 0 && DetailViewModel.lutFiltersListModel)
        ? DetailViewModel.lutFiltersListModel.filterSize(previewFilterIndex)
        : 33.0

    // Funkcja pomocnicza do synchronizacji stanu
    function syncLocalState() {
        previewHue = DetailViewModel.hue;
        previewBrightness = DetailViewModel.brightness;
        previewSaturation = DetailViewModel.saturation;
        previewFlipped = DetailViewModel.flipped;
        previewFilterIndex = DetailViewModel.activeFilterIndex;
    }

    // ── Auto-zastosuj ─────────────────────────────────────────────────────
    // Po każdej zmianie suwaka/filtra/odbicia nie trzeba już klikać
    // "Zastosuj" — applyChanges() woła się sama po 100 ms bezczynności.
    // Debounce (a nie wywołanie przy każdym pikselu przeciągania) chroni
    // przed zalewaniem TimelineViewModel::updateItem() dziesiątkami wywołań
    // podczas jednego przeciągnięcia suwaka.
    Timer {
        id: autoApplyTimer
        interval: 100
        repeat: false
        onTriggered: DetailViewModel.applyChanges(
            previewHue, previewBrightness, previewSaturation,
            previewFlipped, previewFilterIndex)
    }

    // Natychmiastowe zatwierdzenie z pominięciem odczekiwania — używane
    // tam, gdzie VM musi mieć aktualny stan już teraz (np. przed dodaniem
    // do playlisty), a nie dopiero za 100 ms.
    function flushPendingApply() {
        autoApplyTimer.stop();
        DetailViewModel.applyChanges(
            previewHue, previewBrightness, previewSaturation,
            previewFlipped, previewFilterIndex);
    }

    Connections {
        target: DetailViewModel
        function onImageLoaded() {
            syncLocalState();
        }
        function onStateReverted() {
            syncLocalState();
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  Popup – pełnoekranowy podgląd
    // ════════════════════════════════════════════════════════════════════════
    Popup {
        id: previewPopup
        parent: Overlay.overlay
        width: Math.round(Overlay.overlay.width * 0.82)
        height: Math.round(Overlay.overlay.height * 0.82)
        x: Math.round((Overlay.overlay.width - width) / 2)
        y: Math.round((Overlay.overlay.height - height) / 2)
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 6

        enter: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 180
                easing.type: Easing.OutCubic
            }
        }
        exit: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 140
                easing.type: Easing.InCubic
            }
        }

        background: Rectangle {
            color: "#0e0f14"
            radius: 10
            border.color: Qt.rgba(1, 1, 1, 0.06)
            border.width: 1
        }

        PreviewImage {
            id: popupBase
            anchors.fill: parent
            source: DetailViewModel.imageUrl
            fillMode: Image.PreserveAspectFit
            brightness: previewBrightness
            saturation: previewSaturation
            hue: previewHue
            flipped: previewFlipped
            lutPath: previewLutPath
            lutSize: previewLutSize
            sourceSize: Qt.size(Math.round(width), Math.round(height))
            cache: false
        }


        ToolButton {
            anchors {
                top: parent.top
                right: parent.right
                margins: 8
            }
            icon.name: "window-close-symbolic"
            onClicked: previewPopup.close()
            ToolTip.text: qsTr("Close (Esc)")
            ToolTip.visible: hovered
            ToolTip.delay: 500
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  Stan pusty
    // ════════════════════════════════════════════════════════════════════════
    Column {
        anchors.centerIn: parent
        visible: !DetailViewModel.hasImage
        spacing: 10

        Kirigami.Icon {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "image-x-generic-symbolic"
            implicitWidth: 48
            implicitHeight: 48
            color: Kirigami.Theme.disabledTextColor
            opacity: 0.4
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Select an image\nto view details")
            horizontalAlignment: Text.AlignHCenter
            color: Kirigami.Theme.disabledTextColor
            font.pointSize: 10
        }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  Główny panel boczny
    // ════════════════════════════════════════════════════════════════════════
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        visible: DetailViewModel.hasImage
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            width: parent.width
            spacing: 0

            // ── Miniatura ────────────────────────────
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 12
                Layout.topMargin: 14
                Layout.preferredHeight: Math.round(width * 0.5625)

                radius: 7
                color: "#08090c"
                //layer.enabled: true
                clip: true

                MouseArea {
                    id: thumbHoverArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: previewPopup.open()
                }

                PreviewImage {
                    id: thumbBase
                    anchors.fill: parent
                    source: DetailViewModel.imageUrl
                    fillMode: Image.PreserveAspectCrop
                    brightness: previewBrightness
                    saturation: previewSaturation
                    hue: previewHue
                    flipped: previewFlipped
                    lutPath: previewLutPath
                    lutSize: previewLutSize
                }

                Label {
                    anchors.centerIn: parent
                    visible: thumbBase.status !== Image.Ready
                    text: thumbBase.status === Image.Loading ? qsTr("Loading...") : qsTr("No image")
                    color: Kirigami.Theme.disabledTextColor
                    font.pointSize: 9
                }

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: parent.radius
                    border.color: Qt.rgba(1, 1, 1, 0.07)
                    border.width: 1
                }

                Item {
                    anchors {
                        top: parent.top
                        right: parent.right
                        margins: 7
                    }
                    width: 28
                    height: 28
                    opacity: thumbHoverArea.containsMouse ? 0.92 : 0.0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 160
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: Qt.rgba(0, 0, 0, 0.55)
                        border.color: Qt.rgba(1, 1, 1, 0.18)
                        border.width: 1
                    }

                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        source: "zoom-in"
                        color: "white"
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: previewPopup.open()
                        ToolTip.text: qsTr("Full preview")
                        ToolTip.visible: containsMouse
                        ToolTip.delay: 500
                    }
                }
            }

            // ── Nazwa obrazu ──────────────────────────────────────────────
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 14
                Layout.rightMargin: 14
                Layout.topMargin: 8
                Layout.bottomMargin: 2
                text: DetailViewModel.imageName || qsTr("Unknown wallpaper")
                font.bold: true
                font.pointSize: 10
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.textColor
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.topMargin: 12
            }

            // ── KOREKTY ──────────────────────────────────────────────
            Label {
                Layout.leftMargin: 14
                Layout.topMargin: 8
                Layout.bottomMargin: 4
                text: qsTr("ADJUSTMENTS")
                font.pointSize: 7
                font.letterSpacing: 1.2
                font.bold: true
                color: Kirigami.Theme.disabledTextColor
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 14
                Layout.rightMargin: 14
                Layout.bottomMargin: 4
                spacing: 1

                // Barwa
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        text: qsTr("Hue")
                        font.pointSize: 9
                        Layout.preferredWidth: 72
                        color: Kirigami.Theme.textColor
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: DetailViewModel.hueMin
                        to: DetailViewModel.hueMax
                        value: previewHue
                        onMoved: {
                            previewHue = value;
                            autoApplyTimer.restart();
                        }
                    }
                    Label {
                        text: (previewHue > 0 ? "+" : "") + Math.round(previewHue * 180) + "°"
                        font.pointSize: 9
                        font.family: "monospace"
                        color: previewHue !== 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                        Layout.preferredWidth: 42
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // Jasność
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        text: qsTr("Brightness")
                        font.pointSize: 9
                        Layout.preferredWidth: 72
                        color: Kirigami.Theme.textColor
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: DetailViewModel.brightnessMin
                        to: DetailViewModel.brightnessMax
                        value: previewBrightness
                        onMoved: {
                            previewBrightness = value;
                            autoApplyTimer.restart();
                        }
                    }
                    Label {
                        text: (previewBrightness > 0 ? "+" : "") + Math.round(previewBrightness * 100) + "%"
                        font.pointSize: 9
                        font.family: "monospace"
                        color: previewBrightness !== 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                        Layout.preferredWidth: 42
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // Nasycenie
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        text: qsTr("Saturation")
                        font.pointSize: 9
                        Layout.preferredWidth: 72
                        color: Kirigami.Theme.textColor
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: DetailViewModel.saturationMin
                        to: DetailViewModel.saturationMax
                        value: previewSaturation
                        onMoved: {
                            previewSaturation = value;
                            autoApplyTimer.restart();
                        }
                    }
                    Label {
                        text: (previewSaturation > 0 ? "+" : "") + Math.round(previewSaturation * 100) + "%"
                        font.pointSize: 9
                        font.family: "monospace"
                        color: previewSaturation !== 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                        Layout.preferredWidth: 42
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }

            CheckBox {
                Layout.leftMargin: 10
                Layout.topMargin: 4
                Layout.bottomMargin: 2
                text: qsTr("Flip horizontally")
                font.pointSize: 9
                checked: previewFlipped
                onToggled: {
                    previewFlipped = checked;
                    autoApplyTimer.restart();
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.topMargin: 10
            }

            // ── FILTRY ──────────────────────────────────────────────
            Item {
                id: filtersHeader
                Layout.fillWidth: true
                Layout.leftMargin: 14
                Layout.rightMargin: 14
                Layout.topMargin: 8
                Layout.bottomMargin: 6
                implicitHeight: headerRow.implicitHeight
                property bool sectionExpanded: false

                RowLayout {
                    id: headerRow
                    anchors.fill: parent

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        text: qsTr("Filters")
                        font.bold: true
                        font.pointSize: 9
                        color: Kirigami.Theme.textColor
                    }

                    Rectangle {
                        Layout.alignment: Qt.AlignVCenter
                        visible: previewFilterIndex >= 0
                        implicitWidth: badge.implicitWidth + 12
                        implicitHeight: 18
                        radius: 9
                        color: "transparent"
                        border.color: Kirigami.Theme.highlightColor

                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            color: Kirigami.Theme.highlightColor
                            opacity: 0.22
                        }
                        Label {
                            id: badge
                            anchors.centerIn: parent
                            font.pointSize: 7
                            color: Kirigami.Theme.highlightColor
                            elide: Text.ElideRight
                            maximumLineCount: 1
                            text: previewFilterIndex >= 0 ? DetailViewModel.lutFiltersListModel.filterName(previewFilterIndex) : ""
                        }
                    }

                    Kirigami.Icon {
                        Layout.alignment: Qt.AlignVCenter
                        source: filtersHeader.sectionExpanded ? "arrow-up-symbolic" : "arrow-down-symbolic"
                        implicitWidth: 12
                        implicitHeight: 12
                        color: Kirigami.Theme.disabledTextColor
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: filtersHeader.sectionExpanded = !filtersHeader.sectionExpanded
                }
            }

            // Rozwijana lista
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12

                color: Qt.rgba(0, 0, 0, 0.25)
                border.color: Qt.rgba(1, 1, 1, 0.05)
                border.width: 1
                radius: 6
                clip: true

                visible: Layout.preferredHeight > 0
                Layout.preferredHeight: filtersHeader.sectionExpanded ? Math.min(filterList.contentHeight + 8, 220) : 0

                Behavior on Layout.preferredHeight {
                    NumberAnimation {
                        duration: 220
                        easing.type: Easing.OutCubic
                    }
                }

                ListView {
                    id: filterList
                    anchors.fill: parent
                    anchors.margins: 4
                    interactive: true
                    spacing: 2
                    model: DetailViewModel.lutFiltersListModel

                    delegate: ItemDelegate {
                        width: filterList.width
                        required property int index
                        required property var model

                        highlighted: previewFilterIndex === index
                        text: model.name || qsTr("Filter")
                        onClicked: {
                            previewFilterIndex = previewFilterIndex === index ? -1 : index;
                            autoApplyTimer.restart();
                        }
                    }
                }
            }

            // Wypełniacz wypychający elementy do góry
            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: 16
            }

            // ── PRZYCISKI AKCJI ───────────────────────────────────────
            Kirigami.Separator {
                Layout.fillWidth: true
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.margins: 12
                Layout.topMargin: 10
                Layout.bottomMargin: 16
                columns: 2
                columnSpacing: 6
                rowSpacing: 6

                Button {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: qsTr("Restore")
                    icon.name: "edit-undo-symbolic"
                    onClicked: {
                        autoApplyTimer.stop();
                        DetailViewModel.revertChanges();
                    }
                    ToolTip.text: qsTr("Clear adjustments and remove filter")
                    ToolTip.visible: hovered
                    ToolTip.delay: 600
                }

                Button {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: qsTr("Set as wallpaper")
                    icon.name: "preferences-desktop-wallpaper-symbolic"
                    onClicked: {
                        detailRoot.flushPendingApply();
                        DetailViewModel.setAsWallpaper();
                    }
                }

                Button {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: qsTr("Add to playlist")
                    icon.name: "media-playlist-append-symbolic"

                    readonly property bool dayOfWeekFull:
                        TimelineViewModel.currentMode === 3
                        && TimelineViewModel.queueModel.count >= TimelineViewModel.maxDayOfWeekItems

                    enabled: !dayOfWeekFull
                    onClicked: {
                        detailRoot.flushPendingApply();
                        DetailViewModel.addToPlaylist();
                    }

                    ToolTip.text: qsTr("Day of week only supports up to %1 wallpapers (one per day). Remove one or switch mode first.")
                        .arg(TimelineViewModel.maxDayOfWeekItems)
                    ToolTip.visible: dayOfWeekFull && hovered
                    ToolTip.delay: 600
                }
            }
        }
    }
}
