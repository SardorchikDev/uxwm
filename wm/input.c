#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <unistd.h>
#include <stdlib.h>
#include "wm.h"
#include "atoms.h"
#include "util.h"
#include "config.h"
#include "layout.h"

extern Display *dpy;
extern Window root;
extern Monitor *mon;
extern int running;

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const void *);
    const void *arg;
} Key;

/* FIXED: Use uxterm instead of alacritty */
static const char *termcmd[] = { "alacritty", NULL };
static const char *dmenucmd[] = { "dmenu_run", "-fn", "JetBrainsMono Nerd Font-12",
                                  "-nb", "#111111", "-nf", "#ffffff",
                                  "-sb", "#D3D3D3", "-sf", "#ffffff", NULL };
static const char *upvol[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+5%", NULL };
static const char *downvol[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-5%", NULL };
static const char *mutevol[] = { "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL };
static const char *brightup[] = { "brightnessctl", "set", "10%+", NULL };
static const char *brightdown[] = { "brightnessctl", "set", "10%-", NULL };

static void spawn(const void *arg)
{
    const char **cmd = (const char **)arg;
    if (!cmd || !cmd[0])
        return;
    
    if (fork() == 0) {
        if (dpy)
            close(ConnectionNumber(dpy));
        setsid();
        execvp(cmd[0], (char *const *)cmd);
        die("uxwm: execvp %s failed", cmd[0]);
    }
}

static void quit(const void *arg)
{
    (void)arg;
    running = 0;
}

static void togglegaps(const void *arg)
{
    (void)arg;
    enablegaps = !enablegaps;
    arrange(mon);
}

static Key keys[] = {
    /* modifier         key             function        argument */
    { MOD,              XK_Return,      spawn,          termcmd },
    { MOD,              XK_p,           spawn,          dmenucmd },
    { MOD|ShiftMask,    XK_q,           killclient,     NULL },
    { MOD|ShiftMask,    XK_e,           quit,           NULL },
    
    /* Window focus - FIXED */
    { MOD,              XK_j,           focusstack,     &(int){+1} },
    { MOD,              XK_k,           focusstack,     &(int){-1} },
    { MOD,              XK_Tab,         focusstack,     &(int){+1} },
    { MOD|ShiftMask,    XK_Tab,         focusstack,     &(int){-1} },
    
    /* Layout switching */
    { MOD,              XK_t,           setlayout,      &(int){0} },
    { MOD,              XK_m,           setlayout,      &(int){1} },
    { MOD,              XK_f,           setlayout,      &(int){2} },
    { MOD,              XK_space,       togglefloating, NULL },
    { MOD,              XK_g,           togglegaps,     NULL },
    
    /* Master area control */
    { MOD,              XK_i,           incnmaster,     &(int){+1} },
    { MOD,              XK_d,           incnmaster,     &(int){-1} },
    { MOD,              XK_h,           setmfact,       &(float){-0.05} },
    { MOD,              XK_l,           setmfact,       &(float){+0.05} },
    { MOD|ShiftMask,    XK_Return,      zoom,           NULL },
    
    /* Media keys */
    { 0,                XF86XK_AudioRaiseVolume, spawn, upvol },
    { 0,                XF86XK_AudioLowerVolume, spawn, downvol },
    { 0,                XF86XK_AudioMute,        spawn, mutevol },
    { 0,                XF86XK_MonBrightnessUp,  spawn, brightup },
    { 0,                XF86XK_MonBrightnessDown,spawn, brightdown },
};

void grabkeys(void)
{
    unsigned int i;
    KeyCode code;

    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    
    for (i = 0; i < LENGTH(keys); i++) {
        if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
            XGrabKey(dpy, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
    }
}

void handle_keypress(XEvent *e)
{
    unsigned int i;
    KeySym keysym = XLookupKeysym(&e->xkey, 0);

    for (i = 0; i < LENGTH(keys); i++) {
        if (keysym == keys[i].keysym && 
            keys[i].mod == e->xkey.state &&
            keys[i].func) {
            keys[i].func(keys[i].arg);
            return;
        }
    }
}
