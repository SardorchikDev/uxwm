#!/bin/sh
# install.sh - build and install uxwm plus helper scripts

set -eu

PREFIX="${PREFIX:-/usr/local}"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/uxwm"
missing=""

say() {
    printf '%s\n' "$1"
}

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || missing="$missing $1"
}

require_pkg() {
    pkg-config --exists "$1" 2>/dev/null || missing="$missing $1"
}

require_cmd cc
require_cmd make
require_cmd pkg-config
require_pkg x11
require_pkg xinerama
require_pkg xft
require_pkg fontconfig

if [ -n "$missing" ]; then
    say "Missing build dependencies:$missing"
    say "Arch: sudo pacman -S base-devel libx11 libxinerama libxft fontconfig dmenu"
    say "Debian/Ubuntu: sudo apt install build-essential libx11-dev libxinerama-dev libxft-dev libfontconfig-dev suckless-tools"
    exit 1
fi

if [ "$PREFIX" = "/usr/local" ] && [ "$(id -u)" -ne 0 ]; then
    PREFIX="$HOME/.local"
    say "Installing without root; using PREFIX=$PREFIX"
fi

say "Building uxwm..."
make clean
make

say "Installing to $PREFIX/bin..."
make PREFIX="$PREFIX" install

mkdir -p "$CONFIG_DIR"

if [ ! -f "$CONFIG_DIR/autostart" ]; then
    cat >"$CONFIG_DIR/autostart" <<'EOF'
#!/bin/sh
# Start background applications here.
# Example:
# nm-applet &
# feh --bg-scale ~/Pictures/wallpaper.jpg &
EOF
    chmod +x "$CONFIG_DIR/autostart"
fi

if [ ! -f "$HOME/.xinitrc" ]; then
    cat >"$HOME/.xinitrc" <<'EOF'
#!/bin/sh
exec ux-session
EOF
    chmod +x "$HOME/.xinitrc"
fi

say "Install complete."
say "Runtime tools used by the default config: st, firefox, pamixer, brightnessctl, scrot, dmenu."
