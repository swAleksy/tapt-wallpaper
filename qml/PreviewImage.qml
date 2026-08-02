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
// Uwaga API: komponent celowo przyjmuje gotowe lutPath/lutSize zamiast
// filterIndex + referencji do LutFiltersListModel — nie musi nic wiedzieć
// o "indeksie w liście filtrów", co jest pojęciem specyficznym dla UI
// DetailView. Rozwiązanie indeks → (lutPath, lutSize) zostaje po stronie
// wywołującego (DetailView), a TimelinePanel może po prostu podać lutPath
// zapisany bezpośrednio w EditState/QueueModel.
//
// Wydajność:
//   • Gdy edycja jest "tożsamościowa" (hue=0, brightness=0, saturation=0,
//     brak lutPath), pomijamy wizualnie cały łańcuch efektów i pokazujemy
//     zwykły Image. Odwrócenie (flipped) samo w sobie NIE wymaga łańcucha
//     efektów — obsługuje je Image.mirror.
//   • ShaderEffectSource ma live:false. Automatyczny re-render co klatkę
//     (live:true) ma sens przy 1–2 instancjach w DetailView, ale przy
//     niezwirtualizowanym Repeaterze w trybie "Time of the day" (potencjalnie
//     kilkanaście–kilkadziesiąt instancji naraz) byłby to ciągły koszt GPU
//     nawet gdy obraz się wizualnie nie zmienia. Zamiast tego wywołujemy
//     scheduleUpdate() ręcznie, tylko wtedy, gdy coś faktycznie się zmieniło
//     (parametry, wynik ładowania obrazu, rozmiar).
Item {
    id: root

    property url source: ""
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

    // Czy trzeba w ogóle uruchamiać łańcuch efektów GPU.
    readonly property bool needsEffects: hue !== 0.0 || brightness !== 0.0 || saturation !== 0.0 || lutPath.length > 0

    readonly property url resolvedSource: {
        var s = source.toString();
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
        // Gdy nie trzeba żadnych korekt, ten Image jest wyświetlany wprost —
        // reszta łańcucha poniżej zostaje bez efektu wizualnego i bez pracy.
        visible: !root.needsEffects
        sourceSize: (root.sourceSize.width > 0 && root.sourceSize.height > 0) ? root.sourceSize : undefined

        onStatusChanged: effectSource.scheduleUpdate()
        onWidthChanged: effectSource.scheduleUpdate()
        onHeightChanged: effectSource.scheduleUpdate()
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
        live: false
        mipmap: true
        visible: false

        Component.onCompleted: scheduleUpdate()
    }

    ShaderEffect {
        anchors.fill: parent
        visible: root.needsEffects
        property variant sourceImage: effectSource
        property variant lutTexture: Image {
            asynchronous: true
            source: root.lutPath.length > 0 ? "image://lut/" + encodeURIComponent(root.lutPath) : ""
        }
        property real lutSize: root.lutSize
        property real filterMix: root.lutPath.length > 0 ? 1.0 : 0.0
        property real hue: root.hue
        fragmentShader: "qrc:/shaders/lut_filters.frag.qsb"
    }

    onHueChanged: effectSource.scheduleUpdate()
    onBrightnessChanged: effectSource.scheduleUpdate()
    onSaturationChanged: effectSource.scheduleUpdate()
    onLutPathChanged: effectSource.scheduleUpdate()
    onLutSizeChanged: effectSource.scheduleUpdate()
}
