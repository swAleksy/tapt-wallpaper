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
