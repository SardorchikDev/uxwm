# uxwm Setup Checklist

## Build

- [ ] Install X11 build deps: `libx11`, `libxinerama`, `libxft`, `fontconfig`
- [ ] Install launcher dependency: `dmenu`
- [ ] Run `make clean && make`
- [ ] Run `sudo make install` or `make PREFIX="$HOME/.local" install`

## Session

- [ ] Run `./setup.sh` if you want a sample `autostart` hook
- [ ] Create `~/.xinitrc` with `exec ux-session` or `exec uxwm`
- [ ] Confirm `uxwm`, `ux-session`, and `uxmenu_run` are in `PATH`

## Xephyr Smoke Test

```sh
Xephyr :1 -screen 1280x720 &
DISPLAY=:1 uxwm
```

- [ ] uxwm starts without crashing
- [ ] The bar appears
- [ ] `Mod + Return` opens a terminal
- [ ] `Mod + p` opens the launcher
- [ ] `Mod + t / f / m` switch layouts
- [ ] `Mod + g` toggles gaps
- [ ] `Mod + Shift + f` toggles fullscreen
- [ ] `Mod + 1..9` changes tags

## Real Session

- [ ] `startx` launches uxwm from a TTY
- [ ] `Mod + Shift + c` closes a focused window
- [ ] `Mod + Shift + q` exits uxwm
- [ ] `Print` and `Shift + Print` work if `scrot` is installed
- [ ] Audio and brightness keys work if `pamixer` and `brightnessctl` are installed

## Customization

- [ ] Review `wm/config.h`
- [ ] Change terminal / browser / launcher commands if needed
- [ ] Rebuild after local config changes
- [ ] Add background apps to `~/.config/uxwm/autostart`
- [ ] Add an external bar updater to `~/.config/uxwm/status.sh` only if you do not want the built-in clock
