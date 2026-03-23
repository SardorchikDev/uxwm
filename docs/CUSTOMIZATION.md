# uxwm Customization

`uxwm` is configured in source. Edit [wm/config.h](/home/sadi/uxwm/wm/config.h), rebuild, and reinstall.

## Rebuild Cycle

```sh
make clean
make
sudo make install
```

## Appearance

These variables control borders, gaps, snapping, and fonts:

```c
unsigned int borderpx  = 2;
unsigned int gappx     = 10;
unsigned int snap      = 32;
int showbar            = 1;
int topbar             = 1;
const char *fonts[]    = { "JetBrainsMono Nerd Font Mono:size=12", "monospace:size=10" };
```

Color schemes are defined in `colors[][3]`:

```c
const char *colors[][3] = {
    [SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
    [SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};
```

## Layout Defaults

```c
const float mfact        = 0.55;
const int nmaster        = 1;
const int resizehints    = 0;
const int lockfullscreen = 1;
```

`Mod + g` toggles gaps at runtime. `gappx` sets the default gap width used when gaps are enabled.

## Tags

Tag labels live in `tags[]`:

```c
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };
```

Rules are matched against `WM_CLASS` and the window title:

```c
const Rule rules[] = {
    { "Gimp",        NULL, NULL, 0,      1, -1 },
    { "Firefox",     NULL, NULL, 1 << 8, 0, -1 },
    { "mpv",         NULL, NULL, 0,      1, -1 },
    { "Pavucontrol", NULL, NULL, 0,      1, -1 },
};
```

To inspect a window:

```sh
xprop WM_CLASS WM_NAME
```

## Commands

The default launcher, terminal, browser, media, brightness, and screenshot commands all live in `wm/config.h`.

Examples:

```c
const char *termcmd[]    = { "alacritty", NULL };
const char *browsercmd[] = { "librewolf", NULL };
const char *scrotcmd[]   = { "maim", "-u", NULL };
```

## Keybindings

Bindings are defined in the `keys[]` array. Example:

```c
{ MODKEY,           XK_Return,   spawn,           {.v = termcmd } },
{ MODKEY,           XK_g,        togglegaps,      {0} },
{ MODKEY|ShiftMask, XK_f,        togglefullscreen,{0} },
```

Rebind the launcher by editing `dmenucmd[]`. `uxmenu_run` is installed as an optional helper if you want a wrapper around `dmenu_run`.

## Session Hooks

If you start through `ux-session`, these optional user scripts are supported:

- `~/.config/uxwm/autostart`
- `~/.config/uxwm/status.sh`

`status.sh` can drive the bar with `xsetroot -name ...`. If that file does not exist, uxwm shows its built-in clock automatically.
