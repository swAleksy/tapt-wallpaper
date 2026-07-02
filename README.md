# TaptWallpaper

Aplikacja galerii i tapet dla KDE Plasma (Qt6/QML). Pozwala przeglądać zdjęcia
z wybranego folderu, edytować je (jasność, nasycenie, barwa, filtry LUT) i
ustawiać jako tapetę pulpitu.

## Funkcje

- Przeglądanie folderu ze zdjęciami w siatce miniaturek (ładowanie kafelkami)
- Podgląd szczegółowy zdjęcia z edycją hue / brightness / saturation
- Filtry kolorystyczne oparte na plikach LUT (`.cube`)
- Ustawianie wybranego zdjęcia jako tapety

## Stack

- Qt6 / QML — MVVM, view modele jako `QML_SINGLETON`
- C++ — skanowanie folderów, przetwarzanie obrazów, provider tekstur LUT
- Custom shader (GLSL `.frag` → `.qsb`) do próbkowania filtrów LUT na GPU

## Status

Projekt w budowie. Podgląd LUT na żywo oraz akcje w widoku szczegółów
(*Zastosuj*, *Ustaw jako tapetę*) są jeszcze w trakcie dopinania.
