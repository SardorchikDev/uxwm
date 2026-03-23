# uxwm Installation

## Build Dependencies

Arch:

```sh
sudo pacman -S base-devel libx11 libxinerama libxft fontconfig dmenu
```

Debian / Ubuntu:

```sh
sudo apt install build-essential libx11-dev libxinerama-dev libxft-dev libfontconfig-dev suckless-tools
```

## Quick Install

Scripted install:

```sh
./install.sh
```

Manual install:

```sh
make clean
make
sudo make install
```

User-local install:

```sh
make clean
make
make PREFIX="$HOME/.local" install
```

`make install` installs `uxwm`, `ux-session`, and `uxmenu_run`.

## First Session

Direct launch:

```sh
echo 'exec uxwm' > ~/.xinitrc
startx
```

Session wrapper with user hooks:

```sh
echo 'exec ux-session' > ~/.xinitrc
startx
```

`ux-session` optionally runs:

- `~/.config/uxwm/autostart`
- `~/.config/uxwm/status.sh`

You can create a sample autostart hook with:

```sh
./setup.sh
```

If you want an external status updater instead of the built-in clock, create `~/.config/uxwm/status.sh` yourself.

## Xephyr Test

```sh
Xephyr :1 -screen 1280x720 &
DISPLAY=:1 uxwm
```

## Display Manager Entry

If you use a display manager, create `/usr/share/xsessions/uxwm.desktop`:

```ini
[Desktop Entry]
Name=uxwm
Comment=Minimal X11 window manager
Exec=ux-session
Type=Application
DesktopNames=uxwm
```

## Runtime Notes

The default config expects these commands:

- `st`
- `dmenu_run`
- `firefox`
- `pamixer`
- `brightnessctl`
- `scrot`

If you use different tools, edit [wm/config.h](/home/sadi/uxwm/wm/config.h) and rebuild.
