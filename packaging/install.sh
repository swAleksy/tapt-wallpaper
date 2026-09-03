#!/bin/bash
# Installs Tapt Wallpaper to ~/.local (no sudo needed).
#
#   ./packaging/install.sh          build + install
#   ./packaging/install.sh --uninstall   remove everything
#
# After install:
#   - GUI:  launch "Tapt Wallpaper" from Kickoff / Application Launcher
#   - Daemon: runs as a systemd user service, starts on login
#     (systemctl --user status taptwallpaper-daemond)

set -e

PROJECT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

PREFIX="$HOME/.local"
BIN_DIR="$PREFIX/bin"
DESKTOP_DIR="$PREFIX/share/applications"
ICON_DIR="$PREFIX/share/icons/hicolor/scalable/apps"
SERVICE_DIR="$HOME/.config/systemd/user"

# Qt 6 looks for QML modules in the paths listed in $QML_IMPORT_PATH.
# On Fedora this is ~/.local/lib64/qml, on Debian ~/.local/lib/qml, etc.
# We install to the first one that exists, falling back to lib64.
QML_DIR=""
for candidate in "$PREFIX/lib64/qml" "$PREFIX/lib/qml" "$PREFIX/lib/x86_64-linux-gnu/qml"; do
    if [ -d "$candidate" ] || [ -n "$QML_IMPORT_PATH" ]; then
        QML_DIR="$candidate"
        break
    fi
done
QML_DIR="${QML_DIR:-$PREFIX/lib64/qml}"
QML_MODULE_DIR="$QML_DIR/org/kde/taptwallpaper"

SERVICE_NAME="taptwallpaper-daemond.service"
DESKTOP_FILE="org.kde.taptwallpaper.desktop"
ICON_FILE="taptwallpaper.svg"

# ── uninstall ──────────────────────────────────────────────────────────
if [ "$1" = "--uninstall" ]; then
    echo "Uninstalling Tapt Wallpaper…"

    systemctl --user disable --now "$SERVICE_NAME" 2>/dev/null || true
    rm -f "$SERVICE_DIR/$SERVICE_NAME"
    rm -f "$BIN_DIR/tapt-wallpaper"
    rm -f "$BIN_DIR/taptwallpaper-daemond"
    rm -f "$DESKTOP_DIR/$DESKTOP_FILE"
    rm -f "$ICON_DIR/$ICON_FILE"
    for size in 16 22 32 48 64 128 256; do
        rm -f "$PREFIX/share/icons/hicolor/${size}x${size}/apps/taptwallpaper.png"
    done
    rm -rf "$QML_MODULE_DIR"

    systemctl --user daemon-reload
    update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
    gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" 2>/dev/null || true

    echo "Done."
    exit 0
fi

# ── build ──────────────────────────────────────────────────────────────
echo "Building…"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(nproc)"

# ── install files ──────────────────────────────────────────────────────
echo "Installing to $PREFIX …"

install -Dm755 "$BUILD_DIR/tapt-wallpaper"          "$BIN_DIR/tapt-wallpaper"
install -Dm755 "$BUILD_DIR/taptwallpaper-daemond"    "$BIN_DIR/taptwallpaper-daemond"
install -Dm644 "$PROJECT_DIR/packaging/$DESKTOP_FILE" "$DESKTOP_DIR/$DESKTOP_FILE"
install -Dm644 "$PROJECT_DIR/packaging/$ICON_FILE"   "$ICON_DIR/$ICON_FILE"
install -Dm644 "$PROJECT_DIR/packaging/$SERVICE_NAME" "$SERVICE_DIR/$SERVICE_NAME"

# Generate PNG icons at standard sizes from the SVG. Some icon themes
# (e.g. Papirus) don't fall back to the hicolor scalable/ directory for
# SVGs, so rasterized PNGs at 16–256px are needed for the launcher to
# show the icon.
SVG_SRC="$PROJECT_DIR/packaging/$ICON_FILE"
if command -v convert &>/dev/null; then
    for size in 16 22 32 48 64 128 256; do
        png_dir="$PREFIX/share/icons/hicolor/${size}x${size}/apps"
        mkdir -p "$png_dir"
        convert -background none -resize ${size}x${size} "$SVG_SRC" "$png_dir/taptwallpaper.png" 2>/dev/null || true
    done
fi

# Install the QML module (qmldir, .qmltypes, and .qml files) so the
# installed binary can find its types without the build directory.
mkdir -p "$QML_MODULE_DIR"
cp -r "$BUILD_DIR/org/kde/taptwallpaper/." "$QML_MODULE_DIR/"

# ── refresh system caches ──────────────────────────────────────────────
update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" 2>/dev/null || true
systemctl --user daemon-reload

# ── enable + start daemon ──────────────────────────────────────────────
# Restart if already running (picks up the new binary), start otherwise.
systemctl --user enable "$SERVICE_NAME"
systemctl --user restart "$SERVICE_NAME" 2>/dev/null || systemctl --user start "$SERVICE_NAME"

echo ""
echo "Done!"
echo "  GUI:    Launch \"Tapt Wallpaper\" from your application launcher"
echo "  Daemon: systemctl --user status $SERVICE_NAME"
echo "  Logs:   journalctl --user -u $SERVICE_NAME -f"
