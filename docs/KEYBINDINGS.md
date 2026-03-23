# uxwm Keybindings

`Mod` means the Super/Windows key (`Mod4Mask`).

## Launchers

| Binding | Action |
|---------|--------|
| `Mod + Return` | Launch `st` |
| `Mod + p` | Launch `dmenu_run` |
| `Mod + w` | Launch `firefox` |

## Window Control

| Binding | Action |
|---------|--------|
| `Mod + j` | Focus next visible window |
| `Mod + k` | Focus previous visible window |
| `Mod + Shift + Return` | Promote focused window to master |
| `Mod + Shift + c` | Close focused window |
| `Mod + Shift + Space` | Toggle floating for focused window |
| `Mod + Shift + f` | Toggle fullscreen for focused window |
| `Mod + b` | Toggle the bar |

## Layout And Sizing

| Binding | Action |
|---------|--------|
| `Mod + t` | Tiling layout |
| `Mod + f` | Floating layout |
| `Mod + m` | Monocle layout |
| `Mod + Space` | Cycle layouts |
| `Mod + h` | Shrink master area |
| `Mod + l` | Grow master area |
| `Mod + i` | Increase master count |
| `Mod + d` | Decrease master count |
| `Mod + g` | Toggle gaps |

## Tags

| Binding | Action |
|---------|--------|
| `Mod + 1..9` | View tag |
| `Mod + Ctrl + 1..9` | Toggle tag in current view |
| `Mod + Shift + 1..9` | Send focused window to tag |
| `Mod + Ctrl + Shift + 1..9` | Toggle focused window on tag |
| `Mod + 0` | View all tags |
| `Mod + Shift + 0` | Put focused window on all tags |
| `Mod + Tab` | Jump to previous tag view |

## Monitors

| Binding | Action |
|---------|--------|
| `Mod + ,` | Focus previous monitor |
| `Mod + .` | Focus next monitor |
| `Mod + Shift + ,` | Send focused window to previous monitor |
| `Mod + Shift + .` | Send focused window to next monitor |

## Media And System Keys

| Binding | Action |
|---------|--------|
| `Print` | Screenshot via `scrot` |
| `Shift + Print` | Interactive screenshot via `scrot -s` |
| `XF86AudioRaiseVolume` | Volume up |
| `XF86AudioLowerVolume` | Volume down |
| `XF86AudioMute` | Toggle mute |
| `XF86MonBrightnessUp` | Brightness up |
| `XF86MonBrightnessDown` | Brightness down |
| `Mod + Shift + q` | Quit uxwm |

## Mouse

| Binding | Action |
|---------|--------|
| `Mod + Button1` | Move focused window |
| `Mod + Button2` | Toggle floating |
| `Mod + Button3` | Resize focused window |

## Bar Clicks

| Click | Action |
|-------|--------|
| Left click layout symbol | Cycle layouts |
| Right click layout symbol | Switch to monocle |
| Middle click title | Promote focused window to master |
| Left click tag | View tag |
| Right click tag | Toggle tag in view |
| `Mod + Left click` tag | Send focused window to tag |
| `Mod + Right click` tag | Toggle focused window on tag |
| Middle click status text | Launch terminal |

## Notes

- `uxwm` supports runtime gaps; `Mod + g` toggles them on and off.
- Fullscreen works both from client EWMH requests and from `Mod + Shift + f`.
- `uxmenu_run` is installed as an optional helper, but the default binding still uses `dmenu_run`.
