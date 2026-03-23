#!/bin/sh
# setup.sh - create sample user config for uxwm

set -eu

CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/uxwm"

mkdir -p "$CONFIG_DIR"

if [ ! -f "$CONFIG_DIR/autostart" ]; then
    cat >"$CONFIG_DIR/autostart" <<'EOF'
#!/bin/sh
# Start background programs here.
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

printf 'uxwm setup complete.\n'
printf 'Config dir: %s\n' "$CONFIG_DIR"
