import QtQuick
import org.kde.taptwallpaper

Item {
    id: itemRenderer

    required property var model
    property string outputDir: ""
    property size renderSize: Qt.size(1920, 1080)

    signal rendered(string id, string path)
    signal failed(string id, string reason)

    width: renderSize.width
    height: renderSize.height

    // Musi zostawać "visible: true", żeby w ogóle trafiać do scene graph
    // (a to jest warunkiem koniecznym, żeby PreviewImage.MultiEffect miało
    // co renderować do tekstury) — chowamy go tylko przez opacity/enabled.
    visible: true
    opacity: 0.01   // 0.0 bywa agresywnie odcinane przez niektóre backendy QSG
    enabled: false
    z: -1000

    PreviewImage {
        id: preview
        anchors.fill: parent
        source: model.sourcePath
        fillMode: Image.PreserveAspectCrop   // tapeta wypełnia ekran, bez czarnych pasów
        hue: model.hue
        brightness: model.brightness
        saturation: model.saturation
        flipped: model.flipped
        lutPath: model.lutPath
        cache: false
        sourceSize: Qt.size(itemRenderer.width, itemRenderer.height)

        onStatusChanged: settleTimer.restart()
        onLutStatusChanged: settleTimer.restart()
    }

    // Heurystyka: po tym jak obraz źródłowy (i LUT, jeśli jest) są Ready,
    // dajemy łańcuchowi MultiEffect/ShaderEffect chwilę na faktyczne
    // wyrenderowanie klatki, zanim złapiemy piksele — grabToImage() łapie
    // to, co ostatnio narysowane, a nie gwarantowaną świeżą klatkę.
    // Jeśli zauważysz na eksportach brak LUT-a/koloru, zwiększ interval.
    Timer {
        id: settleTimer
        interval: 50
        repeat: false
        onTriggered: {
            if (preview.isError) {
                itemRenderer.failed(model.id, "nie udało się wczytać obrazu źródłowego");
                return;
            }
            if (preview.status !== Image.Ready)
                return;
            if (preview.lutPath.length > 0 && preview.lutStatus !== Image.Ready)
                return;
            grabTimer.start();
        }
    }

    Timer {
        id: grabTimer
        interval: 16   // ~1 klatka zapasu
        repeat: false
        onTriggered: itemRenderer.grabToImage(function (result) {
            const path = outputDir + "/" + model.id + ".png";
            if (result.saveToFile(path))
                itemRenderer.rendered(model.id, path);
            else
                itemRenderer.failed(model.id, "saveToFile nie powiodło się");
        })
    }

    Component.onCompleted: settleTimer.restart()
}
