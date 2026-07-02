# TapTWallpaper

A **Qt 6**/**QML** application for managing a wallpaper gallery and editing wallpapers. Thanks to its hybrid architecture, the application provides smooth real-time previews without putting unnecessary load on the CPU.

<img width="1487" height="761" alt="Image" src="https://github.com/user-attachments/assets/411ea3d1-55d7-4922-ba69-e3ef61f23870" />

## Main Features

* **Smooth Image Gallery:** Multithreaded folder scanning (`QtConcurrent`) with asynchronous thumbnail loading.
* **3D LUT Filters:** Apply professional color lookup tables directly on the GPU using a dedicated shader (`.qsb`). *(TODO)*
* **Real-Time Color Adjustments:** Instantly preview changes to brightness, saturation, hue, and horizontal mirroring. *(TODO)*
* **System Integration:** Quickly set the processed image as the desktop wallpaper and manage wallpaper playlists. *(TODO)*

## Architecture & Technology

* **C++ (Backend):** Manages the application state using the MVVM architecture.
* **QML & Kirigami (Frontend):** User interface built with the **KDE Kirigami** framework.

## Requirements & Build

* **Environment:** Qt 6.5+ (with the `Quick` and `QuickEffects` modules)
* **Framework:** KDE Kirigami
* **Build System:** CMake 3.20+

```bash
mkdir build && cd build
cmake ..
cmake --build .
./taptwallpaper
```
