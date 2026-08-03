# TapT Wallpaper

TapT Wallpaper is a Qt-based application for managing dynamic wallpaper playlists and applying real-time image edits. It allows you to browse local galleries, apply visual effects using 3D LUT filters, and organize wallpapers on a timeline.

<img width="1960" height="1094" alt="Image" src="https://github.com/user-attachments/assets/b942fb69-e17e-42b6-9794-5409e84a3d03" />
---

## Features

* **Gallery View**: Browse local image folders with lazy loading.
* **Image Editing**: Adjust hue, brightness, and saturation, flip images, and apply 3D LUT filters.
* **Timeline Playlist**: Create and manage wallpaper queues with time-based transitions using drag-and-drop.
* **GPU Rendering**: Uses OpenGL and GLSL shaders for real-time effect previews and asynchronous image loading.
* **Linux Integration**: Built for Linux desktop environments using Qt Quick and KDE Kirigami.

---

## Tech Stack

* **Framework**: Qt 6 (Core, Gui, Qml, Quick, QuickControls2, Widgets, Concurrent) & KDE Frameworks 6 (Kirigami)
* **Language**: C++17
* **Graphics**: OpenGL / GLSL
* **Build System**: CMake 3.16+

---

## Architecture and Project Structure

The application follows the MVVM pattern:

* **QML Views**: User interface components (`Gallery.qml`, `DetailView.qml`, `TimelinePanel.qml`, `PreviewImage.qml`)
* **ViewModels**: Business logic (`GalleryViewModel`, `DetailViewModel`, `TimelineViewModel`)
* **Models**: Data structures (`ImagesModel`, `QueueModel`, `EditState`)
* **Services**: Backend operations (`GalleryService`, `LutService`)
* **Providers**: Image handling (`TaptImageProvider`, `LutImageProvider`)

```text
├── qml/             # QML UI components
├── src/             # C++ source files
│   ├── models/      # Data models
│   ├── services/    # Backend services
│   └── viewmodels/  # ViewModel classes
└── luts/            # LUT filter files
```

---

## Build Requirements

* C++17 compatible compiler
* CMake 3.16+
* Qt 6 (Core, Gui, Qml, Quick, QuickControls2, Widgets, Concurrent)
* KDE Frameworks 6 (Kirigami)

---

## WIP

* Automated Wallpaper Rotation: Implementation of a Linux service or background script to automatically cycle through wallpapers according to the active playlist schedule.
* Advanced Image Processing: Integration of further image transformation capabilities.

---
