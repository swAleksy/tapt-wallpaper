import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import org.kde.kirigami as Kirigami
import org.kde.taptwallpaper

//  ─────────────────────────────────────────────────────────────────────────────
//  READ Properties
//    hasImage          : bool
//    imageUrl          : url
//    imageName         : string
//    hue               : real   // −1.0 … +1.0  (brak live preview — patrz komentarz)
//    brightness        : real   // −1.0 … +1.0  (live preview przez MultiEffect)
//    saturation        : real   // −1.0 … +1.0  (live preview przez MultiEffect)
//    flipped           : bool
//    activeFilterIndex : int    // −1 = brak
//    filtersModel      : model  // role: name (string), previewUrl (url)
//
//  Signals
//    imageLoaded()             ← emitowany przez loadImage() w C++ po zmianie obrazu
//
//  Invokables
//    applyChanges(hue, brightness, saturation, flipped, filterIndex)
//    revertChanges()
//    setAsWallpaper()
//    addToPlaylist()

Item {
    id: detailRoot

    // ── Lokalny stan podglądu (niezatwierdzony w VM) ─────────────────────────
    //
    //  Tylko brightness i saturation mają live GPU-preview przez MultiEffect.
    //  Hue jest przechowywane lokalnie i wysyłane do C++ przez applyChanges().
    //  Live preview hue: zaimplementuj layer.effect + custom .qsb shader lub
    //  niech ViewModel przetworzy obraz i zaktualizuje imageUrl po każdym ruchu.
    // ─────────────────────────────────────────────────────────────────────────
    property real previewHue: DetailViewModel.hue
    property real previewBrightness: DetailViewModel.brightness
    property real previewSaturation: DetailViewModel.saturation
    property bool previewFlipped: DetailViewModel.flipped
    property int previewFilterIndex: DetailViewModel.activeFilterIndex

    Connections {
        target: DetailViewModel

        // Nowy obraz załadowany → reset CAŁEGO lokalnego stanu naraz
        function onImageLoaded() {
            previewHue = DetailViewModel.hue;
            previewBrightness = DetailViewModel.brightness;
            previewSaturation = DetailViewModel.saturation;
            previewFlipped = DetailViewModel.flipped;
            previewFilterIndex = DetailViewModel.activeFilterIndex;
        }

        // Sync po revertChanges() (poszczególne właściwości)
        function onStateReverted() {
            previewHue = DetailViewModel.hue;
            previewBrightness = DetailViewModel.brightness;
            previewSaturation = DetailViewModel.saturation;
            previewFlipped = DetailViewModel.flipped;
            previewFilterIndex = DetailViewModel.activeFilterIndex;
        }
        // function onBrightnessChanged() {
        // }
        // function onSaturationChanged() {
        // }
        // function onFlippedChanged() {
        // }
        // function onActiveFilterIndexChanged() {
        // }
    }

    // ════════════════════════════════════════════════════════════════════════
    //  Popup – pełnoekranowy / 80% podgląd
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
        padding: 0

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

        Item {
            anchors.fill: parent
            anchors.margins: 6

            Image {
                id: popupBase
                anchors.fill: parent
                source: DetailViewModel.imageUrl
                fillMode: Image.PreserveAspectFit
                asynchronous: true
                mirror: previewFlipped
                visible: false
            }

            // Qt6 native – jeden efekt zamiast łańcucha
            MultiEffect {
                id: colorEffect
                source: popupBase
                anchors.fill: popupBase
                brightness: previewBrightness
                saturation: previewSaturation
                visible: false // ZMIANA: Ukrywamy, bo ma być tylko wejściem dla LUT
            }

            // 3. Przechwycenie wyniku MultiEffect jako nowej tekstury
            ShaderEffectSource {
                id: effectSource
                sourceItem: colorEffect
                hideSource: true // Automatycznie ukrywa źródło, jeśli zapomnisz dać visible: false
                live: true       // Odświeża się na bieżąco przy ruszaniu suwakami
            }

            // 4. Drugi etap: Nakładamy Twój autorski filtr LUT
            ShaderEffect {
                anchors.fill: popupBase
                // ZMIANA: Zamiast thumbBase, podajemy wygenerowaną teksturę z efektami
                property variant sourceImage: effectSource
                property variant lutTexture: Image {
                    source: previewFilterIndex >= 0 ? "image://lut/" + DetailViewModel.lutFiltersListModel.lutPath(previewFilterIndex) : ""
                }
                property real lutSize: 33.0
                property real filterMix: previewFilterIndex >= 0 ? 1.0 : 0.0
                fragmentShader: "qrc:/shaders/lut_filters.frag.qsb"
            }
        }

        ToolButton {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 8
            icon.name: "window-close-symbolic"
            z: 10
            onClicked: previewPopup.close()
            ToolTip.text: qsTr("Zamknij  (Esc)")
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
            text: qsTr("Wybierz obraz\naby zobaczyć szczegóły")
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

            // ── Miniatura z real-time efektami ────────────────────────────
            Rectangle {
                id: thumbContainer
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 14
                Layout.preferredHeight: Math.round(width * 0.5625)  // 16:9

                radius: 7
                color: "#08090c"
                layer.enabled: true  // clip do zaokrąglonych rogów

                // Obraz źródłowy (widoczny – MultiEffect renderuje się na wierzchu)
                // 1. Obraz bazowy - ukrywamy go, bo nie chcemy go bezpośrednio widzieć
                Image {
                    id: thumbBase
                    anchors.fill: parent
                    source: DetailViewModel.imageUrl
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    mirror: previewFlipped
                    visible: false // ZMIANA: Ukrywamy oryginał!
                }

                // 2. Pierwszy etap: Jasność i Nasycenie
                MultiEffect {
                    id: thumbColorEffect
                    source: thumbBase
                    anchors.fill: thumbBase
                    brightness: previewBrightness
                    saturation: previewSaturation
                    visible: false // ZMIANA: Ukrywamy, bo ma być tylko wejściem dla LUT
                }

                // 3. Przechwycenie wyniku MultiEffect jako nowej tekstury
                ShaderEffectSource {
                    id: thumbEffectSource
                    sourceItem: thumbColorEffect
                    hideSource: true // Automatycznie ukrywa źródło, jeśli zapomnisz dać visible: false
                    live: true       // Odświeża się na bieżąco przy ruszaniu suwakami
                }

                // 4. Drugi etap: Nakładamy Twój autorski filtr LUT
                ShaderEffect {
                    anchors.fill: thumbBase
                    // ZMIANA: Zamiast thumbBase, podajemy wygenerowaną teksturę z efektami
                    property variant sourceImage: thumbEffectSource
                    property variant lutTexture: Image {
                        source: previewFilterIndex >= 0 ? "image://lut/" + DetailViewModel.lutFiltersListModel.lutPath(previewFilterIndex) : ""
                    }
                    property real lutSize: 33.0
                    property real filterMix: previewFilterIndex >= 0 ? 1.0 : 0.0
                    fragmentShader: "qrc:/shaders/lut_filters.frag.qsb"
                }

                // Placeholder ładowania (ponad efektem)
                Label {
                    anchors.centerIn: parent
                    visible: thumbBase.status !== Image.Ready
                    z: 2
                    text: thumbBase.status === Image.Loading ? qsTr("Ładowanie…") : qsTr("Brak obrazu")
                    color: Kirigami.Theme.disabledTextColor
                    font.pointSize: 9
                }

                // Ramka
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: parent.radius
                    border.color: Qt.rgba(1, 1, 1, 0.07)
                    border.width: 1
                }

                // Przycisk expand
                RoundButton {
                    id: expandBtn
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 7
                    width: 28
                    height: 28
                    icon.name: "view-fullscreen-symbolic"
                    padding: 4
                    z: 5
                    opacity: thumbHoverArea.containsMouse ? 0.92 : 0.0
                    Behavior on opacity {
                        NumberAnimation {
                            duration: 160
                        }
                    }

                    background: Rectangle {
                        radius: expandBtn.width / 2
                        color: Qt.rgba(0, 0, 0, 0.55)
                        border.color: Qt.rgba(1, 1, 1, 0.18)
                        border.width: 1
                    }

                    onClicked: previewPopup.open()
                    ToolTip.text: qsTr("Pełny podgląd  ·  lub dwuklik")
                    ToolTip.visible: hovered
                    ToolTip.delay: 500
                }

                MouseArea {
                    id: thumbHoverArea
                    anchors.fill: parent
                    hoverEnabled: true
                    z: 4
                    onDoubleClicked: previewPopup.open()
                }
            }

            // ── Nazwa obrazu ──────────────────────────────────────────────
            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 14
                Layout.rightMargin: 14
                Layout.topMargin: 8
                Layout.bottomMargin: 2
                text: DetailViewModel.imageName || qsTr("Nieznana tapeta")
                font.bold: true
                font.pointSize: 10
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.textColor
            }

            // ════════════════════════════════════════════════════════════
            //  SEKCJA: Korekty kolorów
            // ════════════════════════════════════════════════════════════
            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.topMargin: 12
            }

            Label {
                Layout.leftMargin: 14
                Layout.topMargin: 8
                Layout.bottomMargin: 4
                text: qsTr("KOREKTY")
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

                // ── Barwa (Hue) ───────────────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Barwa")
                        font.pointSize: 9
                        Layout.preferredWidth: 72
                        color: Kirigami.Theme.textColor
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: -1.0
                        to: 1.0
                        stepSize: 0.005
                        value: previewHue
                        // onMoved zamiast onValueChanged → unika feedback loop
                        onMoved: previewHue = value

                        ToolTip.text: qsTr("Live preview barwy wymaga custom shadera (.qsb)\nWartość zostanie zastosowana przez Apply")
                        ToolTip.visible: pressed
                    }
                    Label {
                        readonly property string sign: previewHue >= 0 ? "+" : ""
                        text: sign + Math.round(previewHue * 180) + "°"
                        font.pointSize: 9
                        font.family: "monospace"
                        color: previewHue !== 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                        Layout.preferredWidth: 42
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // ── Jasność (Brightness) ──────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Jasność")
                        font.pointSize: 9
                        Layout.preferredWidth: 72
                        color: Kirigami.Theme.textColor
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: -1.0
                        to: 1.0
                        stepSize: 0.005
                        value: previewBrightness
                        onMoved: previewBrightness = value
                    }
                    Label {
                        readonly property string sign: previewBrightness >= 0 ? "+" : ""
                        text: sign + Math.round(previewBrightness * 100) + "%"
                        font.pointSize: 9
                        font.family: "monospace"
                        color: previewBrightness !== 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                        Layout.preferredWidth: 42
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // ── Nasycenie (Saturation) ────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: qsTr("Nasycenie")
                        font.pointSize: 9
                        Layout.preferredWidth: 72
                        color: Kirigami.Theme.textColor
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: -1.0
                        to: 1.0
                        stepSize: 0.005
                        value: previewSaturation
                        onMoved: previewSaturation = value
                    }
                    Label {
                        readonly property string sign: previewSaturation >= 0 ? "+" : ""
                        text: sign + Math.round(previewSaturation * 100) + "%"
                        font.pointSize: 9
                        font.family: "monospace"
                        color: previewSaturation !== 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                        Layout.preferredWidth: 42
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }

            // ── Odwróć poziomo ────────────────────────────────────────────
            CheckBox {
                Layout.leftMargin: 10
                Layout.topMargin: 4
                Layout.bottomMargin: 2
                text: qsTr("Odwróć poziomo")
                font.pointSize: 9
                checked: previewFlipped
                onToggled: previewFlipped = checked
            }

            // ════════════════════════════════════════════════════════════
            //  SEKCJA: Filtry / LUT  (zwijana)
            // ════════════════════════════════════════════════════════════
            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.topMargin: 10
            }

            ItemDelegate {
                id: filtersHeader
                Layout.fillWidth: true
                implicitHeight: 42
                property bool sectionExpanded: false

                contentItem: RowLayout {
                    spacing: 8
                    anchors {
                        fill: parent
                        leftMargin: 6
                        rightMargin: 8
                    }

                    Kirigami.Icon {
                        source: "color-management-symbolic"
                        implicitWidth: 16
                        implicitHeight: 16
                        color: Kirigami.Theme.textColor
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Filtry / LUT")
                        font.bold: true
                        font.pointSize: 9
                        color: Kirigami.Theme.textColor
                    }
                    // Odznaka aktywnego filtra
                    // Odznaka aktywnego filtra
                    Rectangle {
                        visible: previewFilterIndex >= 0
                        implicitWidth: badge.implicitWidth + 12
                        implicitHeight: 18
                        radius: 9

                        // Główny kontener jest przezroczysty, ma tylko ramkę
                        color: "transparent"
                        border.color: Kirigami.Theme.highlightColor
                        border.width: 1

                        // Ten wewnętrzny prostokąt robi za półprzezroczyste tło (alpha = 0.22)
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
                        source: filtersHeader.sectionExpanded ? "arrow-up-symbolic" : "arrow-down-symbolic"
                        implicitWidth: 12
                        implicitHeight: 12
                        color: Kirigami.Theme.disabledTextColor
                    }
                }

                onClicked: sectionExpanded = !sectionExpanded
            }

            // Siatka filtrów (animowane rozwijanie)
            Item {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                clip: true
                height: filtersHeader.sectionExpanded ? filterGrid.implicitHeight + 10 : 0
                Behavior on height {
                    NumberAnimation {
                        duration: 220
                        easing.type: Easing.OutCubic
                    }
                }

                GridView {
                    id: filterGrid
                    anchors {
                        top: parent.top
                        topMargin: 4
                        left: parent.left
                        right: parent.right
                    }
                    interactive: false
                    implicitHeight: contentHeight
                    cellWidth: Math.floor(width / 3)
                    cellHeight: Math.floor(cellWidth * 0.68) + 22

                    model: DetailViewModel.lutFiltersListModel

                    delegate: Item {
                        width: filterGrid.cellWidth
                        height: filterGrid.cellHeight

                        readonly property bool isActive: previewFilterIndex === index

                        readonly property color sepColor: Kirigami.Theme.separatorColor

                        Rectangle {
                            anchors {
                                fill: parent
                                margins: 3
                            }
                            radius: 5
                            color: "transparent"
                            border.width: isActive ? 2 : 1
                            border.color: isActive ? Kirigami.Theme.highlightColor : Qt.rgba(sepColor.r, sepColor.g, sepColor.b, 0.5)

                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                color: Kirigami.Theme.highlightColor
                                opacity: isActive ? 0.18 : 0.0
                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 120
                                    }
                                }
                            }

                            ColumnLayout {
                                anchors {
                                    fill: parent
                                    margins: 4
                                }
                                spacing: 3

                                Item {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    clip: true

                                    Image {
                                        anchors.fill: parent
                                        source: model.previewUrl || ""
                                        fillMode: Image.PreserveAspectCrop
                                        asynchronous: true
                                        visible: status === Image.Ready
                                    }
                                    Rectangle {
                                        anchors.fill: parent
                                        visible: parent.children[0].status !== Image.Ready
                                        color: Kirigami.Theme.alternateBackgroundColor
                                        radius: 3
                                        Label {
                                            anchors.centerIn: parent
                                            text: "◧"
                                            font.pointSize: 13
                                            color: Kirigami.Theme.disabledTextColor
                                            opacity: 0.5
                                        }
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: model.name || qsTr("Filtr")
                                    font.pointSize: 7
                                    elide: Text.ElideRight
                                    horizontalAlignment: Text.AlignHCenter
                                    color: isActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                // Toggle: klik na aktywny odznacza go
                                onClicked: previewFilterIndex = isActive ? -1 : index
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: 16
            }

            // ════════════════════════════════════════════════════════════
            //  Przyciski akcji
            // ════════════════════════════════════════════════════════════
            Kirigami.Separator {
                Layout.fillWidth: true
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12
                Layout.topMargin: 10
                Layout.bottomMargin: 16
                columns: 2
                columnSpacing: 6
                rowSpacing: 6

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Zastosuj")
                    highlighted: true
                    icon.name: "dialog-ok-apply-symbolic"
                    onClicked: DetailViewModel.applyChanges(previewHue, previewBrightness, previewSaturation, previewFlipped, previewFilterIndex)
                    ToolTip.text: qsTr("Zatwierdź korekty")
                    ToolTip.visible: hovered
                    ToolTip.delay: 600
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Przywróć")
                    icon.name: "edit-undo-symbolic"
                    onClicked: DetailViewModel.revertChanges()
                    ToolTip.text: qsTr("Cofnij do ostatnio zatwierdzonych wartości")
                    ToolTip.visible: hovered
                    ToolTip.delay: 600
                }

                Button {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: qsTr("Ustaw jako tapetę")
                    icon.name: "preferences-desktop-wallpaper-symbolic"
                    onClicked: DetailViewModel.setAsWallpaper()
                }

                Button {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: qsTr("Dodaj do playlisty")
                    icon.name: "media-playlist-append-symbolic"
                    onClicked: DetailViewModel.addToPlaylist()
                }
            }
        }
    }
}
