# uxwm

`uxwm` is a small X11 window manager written in C.

It is a `dwm`-style manager with:

- one tiling layout
- one floating mode
- one monocle layout
- 9 tags
- a simple Xft status bar
- mouse move/resize for floating windows
- Xinerama multi-monitor support

This repo is the window manager itself, not a full desktop environment.

## What It Actually Supports

- tiling, floating, and monocle layouts
- master/stack resizing
- moving windows between tags and monitors
- fullscreen via EWMH and keybinding
- runtime gaps toggle
- root-window status text via `xsetroot`
- built-in clock when no external status text is set
- basic EWMH state for:
  `_NET_ACTIVE_WINDOW`, `_NET_WM_STATE_FULLSCREEN`,
  `_NET_CURRENT_DESKTOP`, `_NET_WM_DESKTOP`,
  `_NET_CLIENT_LIST`, `_NET_CLIENT_LIST_STACKING`

## What It Does Not Try To Be

- not Wayland
- not a compositor
- not a panel system
- not a dynamic plugin system
- not runtime-configurable without rebuild

Current rough edges:

- config is edited in `wm/config.h` and requires rebuild
- default keybindings assume a few external tools exist
- no dock/strut handling yet
- no restart/reload command yet

## Build Dependencies

Arch:

```sh
sudo pacman -S base-devel libx11 libxinerama libxft fontconfig dmenu
```

Debian / Ubuntu:

```sh
sudo apt install build-essential libx11-dev libxinerama-dev libxft-dev libfontconfig-dev suckless-tools
```

Build:

```sh
make
sudo make install
```

Installed binaries:

- `uxwm`
- `ux-session`
- `uxmenu_run`

## Runtime Dependencies In The Default Config

The shipped `wm/config.h` uses:

- `st`
- `dmenu_run`
- `firefox`
- `pamixer`
- `brightnessctl`
- `scrot`

If you do not use those tools, change `wm/config.h`.

## Start

Minimal:

```sh
echo 'exec uxwm' > ~/.xinitrc
startx
```

With the helper session script:

```sh
echo 'exec ux-session' > ~/.xinitrc
startx
```

`ux-session` optionally runs:

- `~/.config/uxwm/autostart`
- `~/.config/uxwm/status.sh`

Create a sample autostart file with:

```sh
./setup.sh
```

## Test In Xephyr

```sh
Xephyr :1 -screen 1280x720 &
DISPLAY=:1 uxwm
```

## Default Keys

- `Super + Return`: terminal
- `Super + p`: launcher
- `Super + w`: browser
- `Super + j / k`: focus next / previous
- `Super + h / l`: shrink / grow master
- `Super + i / d`: increase / decrease master count
- `Super + t / f / m`: tile / floating / monocle
- `Super + Space`: cycle layouts
- `Super + g`: toggle gaps
- `Super + Shift + Space`: toggle floating
- `Super + Shift + f`: toggle fullscreen
- `Super + Shift + Return`: move focused window to master
- `Super + 1..9`: view tag
- `Super + Shift + 1..9`: send window to tag
- `Super + Tab`: previous tag view
- `Super + , / .`: focus previous / next monitor
- `Super + Shift + c`: close focused window
- `Super + Shift + q`: quit

Mouse:

- `Super + Button1`: move window
- `Super + Button2`: toggle floating
- `Super + Button3`: resize window

More detail: `docs/KEYBINDINGS.md`

## Configure

Edit `wm/config.h`, then rebuild:

```sh
make clean
make
sudo make install
```

Important defaults:

```c
unsigned int borderpx = 2;
unsigned int gappx    = 10;
unsigned int snap     = 32;
const float  mfact    = 0.55;
const int    nmaster  = 1;
```

## Status Bar

If you want external status text:

```sh
xsetroot -name " vol: 80%  Tue Mar 23  14:23 "
```

If root window name is empty, `uxwm` shows its own clock.

## Source Layout

```text
wm/
├── uxwm.c      main
├── x11.c       event loop and X11 lifecycle
├── wm.c        client management and bar logic
├── input.c     keybindings and mouse actions
├── layout.c    tile and monocle
├── drw.c/h     drawing helpers
├── util.c/h    small utilities
└── config.h    config
```

## License

MIT
