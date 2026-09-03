#!/bin/bash
# Remote installer — safe to pipe from curl:
#
#   curl -fsSL https://raw.githubusercontent.com/swAleksy/tapt-wallpaper/main/packaging/remote-install.sh | bash
#
# Clones the repo to a temp directory, builds, installs to ~/.local, and
# cleans up. The daemon starts as a systemd user service on login.
#
# Prerequisites (Arch / CachyOS):
#   sudo pacman -S base-devel cmake qt6-base qt6-declarative qt6-shadertools \
#                  kf6-kirigami extra-cmake-modules
# Prerequisites (Fedora):
#   sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtdeclarative-devel \
#                    qt6-qtshadertools-devel kf6-kirigami-devel extra-cmake-modules
# Prerequisites (Debian/Ubuntu):
#   sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev \
#                    qt6-shadertools-dev kf6-kirigami-dev extra-cmake-modules

set -e

REPO_URL="https://github.com/swAleksy/tapt-wallpaper.git"
TEMP_DIR="$(mktemp -d)"

cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

echo "Cloning $REPO_URL …"
git clone --depth 1 "$REPO_URL" "$TEMP_DIR"

echo "Running installer …"
bash "$TEMP_DIR/packaging/install.sh"
