import QtQuick
import QtQuick.Effects

// PreviewImage
// ─────────────────────────────────────────────────────────────────────────
// Współdzielony podgląd obrazu z korektami (barwa/jasność/nasycenie),
// odbiciem i filtrem LUT. Wydzielony z DetailView.qml, żeby TimelinePanel
// (i każdy kolejny widok) mógł stosować te same efekty shadera na
// miniaturkach bez duplikowania łańcucha MultiEffect → ShaderEffectSource →
// ShaderEffect.
//
//  Properties
//    source     : url    — surowa ścieżka systemowa ALBO gotowy URL. Jeśli
//                          nie zawiera schematu (np. "image://", "file://"),
//                          komponent sam dokleja prefiks providera
//                          "image://taptimage/" (dokładna odwrotność tego,
//                          co DetailViewModel::setImage() robi przy zapisie).
//                          Jedno miejsce budowania URL zamiast identycznej
//                          imageSourceFor() powielanej w każdym widoku.
//    fillMode   : int
//    hue, brightness, saturation : real   (−1.0 … +1.0, 0.0 = brak korekty)
//    flipped    : bool
//    lutPath    : string — ścieżka pliku LUT, "" = brak filtra
//    lutSize    : real   — rozmiar sześcianu LUT (domyślnie 33)
//    cache      : bool
//    sourceSize : size
//    status     : int (readonly, przekazywane z wewnętrznego Image)
//
Item {
    id: root

    property string source: ""
    property int fillMode: Image.PreserveAspectFit
    property real brightness: 0.0
    property real saturation: 0.0
    property real hue: 0.0
    property bool flipped: false
    property string lutPath: ""
    property real lutSize: 33.0
    property bool cache: true
    property size sourceSize: Qt.size(0, 0)

    readonly property int status: baseImage.status
    readonly property int lutStatus: lutImg.status

    readonly property bool isError: root.status === Image.Error

    // Czy trzeba w ogóle uruchamiać łańcuch efektów GPU.
    readonly property bool needsEffects: !isError && (hue !== 0.0 || brightness !== 0.0 || saturation !== 0.0 || lutPath.length > 0)

    readonly property string resolvedSource: {
        var s = source;
        if (s.length === 0)
            return "";
        return (s.indexOf("://") !== -1) ? s : "image://taptimage/" + encodeURIComponent(s);
    }

    Image {
        id: baseImage
        anchors.fill: parent
        source: root.resolvedSource
        fillMode: root.fillMode
        asynchronous: true
        mirror: root.flipped
        mipmap: true
        cache: root.cache

        // Wyświetlamy bazowy obraz tylko wtedy, gdy nie używamy efektów
        // I JEDNOCZEŚNIE nie ma błędu (aby nie pokazywać "zepsutej" ikonki systemowej)
        visible: !root.needsEffects && !root.isError
        sourceSize: (root.sourceSize.width > 0 && root.sourceSize.height > 0) ? root.sourceSize : undefined
    }

    MultiEffect {
        id: colorEffect
        source: baseImage
        anchors.fill: baseImage
        brightness: root.brightness
        saturation: root.saturation
        visible: true
    }

    ShaderEffectSource {
        id: effectSource
        sourceItem: colorEffect
        hideSource: true
        mipmap: true

        // Zamiast sztywnego true, uaktywniamy "live" tylko gdy efekty są włączone.
        live: root.needsEffects
    }

    ShaderEffect {
        anchors.fill: parent
        visible: root.needsEffects
        property variant sourceImage: effectSource
        property variant lutTexture: lutImg

        property real lutSize: lutImg.status === Image.Ready ? lutImg.sourceSize.height : root.lutSize
        property real filterMix: root.lutPath.length > 0 ? 1.0 : 0.0
        property real hue: root.hue
        fragmentShader: "qrc:/shaders/lut_filters.frag.qsb"

        Image {
            id: lutImg
            visible: false
            asynchronous: true
            source: root.lutPath.length > 0 ? "image://lut/" + encodeURIComponent(root.lutPath) : ""
        }
    }

    Rectangle {
        id: errorOverlay
        anchors.fill: parent
        color: "#3B0E0F"
        visible: root.isError

        Text {
            anchors.centerIn: parent
            text: qsTr("No media found")
            color: "#9E2828"

            // Skaluje się razem z rozmiarem okna/elementu.
            font.pixelSize: Math.max(8, Math.min(parent.width, parent.height) * 0.08)

            font.bold: true
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
