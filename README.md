# TapT Wallpaper

TapT Wallpaper is a Qt-based application for managing dynamic wallpaper playlists and applying real-time image edits. It allows you to browse local galleries, apply visual effects using 3D LUT filters, and organize wallpapers on a timeline.

<img width="1960" height="1094" alt="Image" src="https://github.com/user-attachments/assets/b942fb69-e17e-42b6-9794-5409e84a3d03" />

---

## Features

* **Gallery**: Browse local image folders with lazy loading.
* **Image Editing**: Adjust hue, brightness, saturation, flip, and apply 3D LUT filters.
* **Timeline Playlist**: Schedule wallpapers by time of day, day of week, login, or timer — per monitor.
* **GPU Rendering**: OpenGL/GLSL shaders for real-time previews and offline export.
* **KDE Plasma Integration**: A background daemon watches the playlist and applies wallpapers via D-Bus.

---

## How It Works

The app has two parts:

1. **GUI** (`tapt-wallpaper`) — Browse images, edit them, build a playlist, and export. On export, each image is rendered to a PNG with all edits baked in, and the playlist is saved as JSON.
2. **Daemon** (`taptwallpaper-daemond`) — Runs as a systemd user service. Watches the playlist file for changes and applies the right wallpaper to each screen based on the current mode and schedule.

---

## Install

### One-liner (from GitHub)

```bash
curl -fsSL https://raw.githubusercontent.com/swAleksy/tapt-wallpaper/main/packaging/remote-install.sh | bash
```

This clones the repo, builds, installs to `~/.local`, and enables the daemon service.

### From a local clone

```bash
git clone https://github.com/swAleksy/tapt-wallpaper.git
cd tapt-wallpaper
./packaging/install.sh
```

### Uninstall

```bash
./packaging/install.sh --uninstall
```

---

## Dependencies

**Arch / CachyOS:**

```bash
sudo pacman -S base-devel cmake qt6-base qt6-declarative qt6-shadertools \
    kf6-kirigami extra-cmake-modules
```

**Fedora:**

```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtdeclarative-devel \
    qt6-qtshadertools-devel kf6-kirigami-devel extra-cmake-modules
```

**Debian / Ubuntu:**

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev \
    qt6-shadertools-dev kf6-kirigami-dev extra-cmake-modules
```

---

## Usage

1. Launch **Tapt Wallpaper** from your application launcher.
2. Add images to the playlist from the gallery.
3. Pick a mode (time of day, day of week, on login, or on a timer) and adjust the schedule.
4. Click **Save playlist** — images are rendered and the daemon picks them up automatically.

The daemon starts on login via systemd. Check its status with:

```bash
systemctl --user status taptwallpaper-daemond
journalctl --user -u taptwallpaper-daemond -f
```

---

## Project Structure

```text
├── qml/             # QML UI components
├── src/             # C++ source
│   ├── models/      # Data models (QueueModel, MonitorPlaylistState, ...)
│   ├── services/    # Playlist I/O, gallery, LUT service
│   └── viewmodels/  # ViewModels (Timeline, Gallery, Detail)
├── daemon/          # Background daemon (watches playlist, applies wallpapers)
├── packaging/       # Install scripts, .desktop, .service, icon
├── shaders/         # GLSL fragment shaders
└── luts/            # 3D LUT filter files
```

---

## Tech Stack

* C++17, Qt 6, KDE Frameworks 6 (Kirigami)
* OpenGL / GLSL shaders
* CMake 3.16+
