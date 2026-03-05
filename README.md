# uxwm - unifined x11 window manager

A minimal X11 window manager. Tiling, floating, and monocle layouts.
Built-in status bar. 9 workspaces. No dependencies beyond Xlib, Xft, and Xinerama.

## Build
```sh
# Arch
sudo pacman -S base-devel libx11 libxinerama libxft fontconfig

# Debian / Ubuntu
sudo apt install build-essential libx11-dev libxinerama-dev libxft-dev libfontconfig-dev

make
sudo make install
```

## Start
```sh
echo "exec uxwm" >> ~/.xinitrc
startx
```

To test without replacing your current WM:
```sh
Xephyr :1 -screen 1280x720 &
DISPLAY=:1 uxwm
```

## Keybindings

| Key | Action |
|-----|--------|
| `Super + Return` | Terminal (`st`) |
| `Super + p` | Launcher (`dmenu_run`) |
| `Super + [1–9]` | Switch workspace |
| `Super + Shift + [1–9]` | Move window to workspace |
| `Super + j / k` | Focus next / prev window |
| `Super + h / l` | Shrink / grow master |
| `Super + t / f / m` | Tile / float / monocle layout |
| `Super + Shift + c` | Close window |
| `Super + b` | Toggle bar |
| `Super + Tab` | Last workspace |
| `Super + Shift + e` | Quit |

Mouse: `Super + drag` moves, `Super + right-drag` resizes.

## Configure

Edit `wm/config.h`, then rebuild:
```sh
make clean && make && sudo make install
```

Key options:
```c
unsigned int borderpx = 2;        // border width in pixels
unsigned int gappx    = 10;       // gap between windows
const float  mfact    = 0.55;     // master area fraction
const char  *fonts[]  = { "monospace:size=10" };

// colours: bg, fg, border
const char *colors[][3] = {
    [SchemeNorm] = { "#a9b1d6", "#1a1b26", "#414868" },
    [SchemeSel]  = { "#c0caf5", "#7aa2f7", "#7aa2f7" },
};
```

## Status bar

The bar updates its clock every second with no external scripts required.
To push custom status text (e.g. battery, volume):
```sh
xsetroot -name " vol: 80%  2024-01-15 14:23 "
```

If the root name is empty, the built-in clock takes over automatically.

## File layout
```
wm/
├── uxwm.c      entry point
├── wm.c        core logic, bar drawing, client management
├── x11.c       X11 event loop and WM lifecycle
├── input.c     keybindings, mouse, desktop switching
├── layout.c    tile, monocle
├── drw.c/h     Xft drawing primitives
├── util.c/h    die(), ecalloc()
└── config.h    all user configuration
```

## License

MIT
