/* uxwm - window manager header */
#ifndef WM_H
#define WM_H

#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/XF86keysym.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include <X11/Xft/Xft.h>
#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & \
                                (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)   (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1u << num_tags) - 1u)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)

/* enums */
enum { CurNormal, CurResize, CurMove, CurLast };
enum { SchemeNorm, SchemeSel };
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetWMFullscreen, NetActiveWindow, NetWMWindowType,
       NetWMWindowTypeDialog, NetWMWindowTypeUtility,
       NetWMWindowTypeToolbar, NetWMWindowTypeSplash,
       NetCloseWindow, NetClientList, NetClientListStacking,
       NetCurrentDesktop, NetNumberOfDesktops, NetDesktopNames,
       NetWMDesktop, NetLast };
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast };
enum { ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast };

/* types */
typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;

struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;

struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;
	int mx, my, mw, mh;
	int wx, wy, ww, wh;
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	const Layout *lt[2];
};

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} Rule;

/* globals (defined in wm.c) */
extern Display *dpy;
extern Window root, wmcheckwin;
extern Monitor *mons, *selmon;
extern Drw *drw;
extern Clr **scheme;
extern Cur *cursor[CurLast];
extern Atom wmatom[WMLast], netatom[NetLast];
extern int running, screen, sw, sh, bh, lrpad;
extern unsigned int numlockmask;
extern char stext[256];

/* config (defined in wm.c via config.h) */
extern const char broken[];
extern unsigned int borderpx;
extern unsigned int gappx;
extern unsigned int snap;
extern int enablegaps;
extern int showbar;
extern int topbar;
extern const char *fonts[];
extern const char dmenufont[];
extern const char *colors[][3];
extern const char *tags[];
extern const Rule rules[];
extern const float mfact;
extern const int nmaster;
extern const int resizehints;
extern const int lockfullscreen;
extern const int refreshrate;
extern const Layout layouts[];
extern char dmenumon[2];
extern const char *dmenucmd[];
extern const char *termcmd[];
extern const Key keys[];
extern const Button buttons[];

/* config array lengths (needed by other TUs that can't use LENGTH()) */
extern int num_fonts;
extern int num_colors;
extern int num_tags;
extern int num_layouts;
extern int num_keys;
extern int num_buttons;
extern int num_rules;

/* functions - wm.c */
void applyrules(Client *c);
int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
void arrange(Monitor *m);
void arrangemon(Monitor *m);
void attach(Client *c);
void attachstack(Client *c);
void configure(Client *c);
Client *nexttiled(Client *c);
void pop(Client *c);
void detach(Client *c);
void detachstack(Client *c);
void drawbar(Monitor *m);
void drawbars(void);
void focus(Client *c);
void unfocus(Client *c, int setfocus);
Atom getatomprop(Client *c, Atom prop);
long getstate(Window w);
int gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabbuttons(Client *c, int focused);
void manage(Window w, XWindowAttributes *wa);
int propcontainsatom(Client *c, Atom prop, Atom atom);
void unmanage(Client *c, int destroyed);
void resize(Client *c, int x, int y, int w, int h, int interact);
void resizeclient(Client *c, int x, int y, int w, int h);
void restack(Monitor *m);
void sendmon(Client *c, Monitor *m);
void setclientstate(Client *c, long state);
int sendevent(Client *c, Atom proto);
void setfocus(Client *c);
void setfullscreen(Client *c, int fullscreen);
void seturgent(Client *c, int urg);
void showhide(Client *c);
void updatebars(void);
void updatebarpos(Monitor *m);
void updateclientlist(void);
void updateclientdesktop(Client *c);
void updateclientliststacking(void);
void updatecurrentdesktop(void);
void updatedesktopnames(Atom utf8string);
void updatenumdesktops(void);
void updatesizehints(Client *c);
void updatestatus(void);
void updatetitle(Client *c);
void updatewindowtype(Client *c);
void updatewmhints(Client *c);

/* functions - x11.c */
void checkotherwm(void);
void scan(void);
void setup(void);
void run(void);
void cleanup(void);
int xerrordummy(Display *d, XErrorEvent *ee);
int xerror(Display *d, XErrorEvent *ee);
Monitor *createmon(void);
void cleanupmon(Monitor *m);
Monitor *dirtomon(int dir);
int getrootptr(int *x, int *y);
Monitor *recttomon(int x, int y, int w, int h);
int updategeom(void);
void updatenumlockmask(void);
Client *wintoclient(Window w);
Monitor *wintomon(Window w);

/* functions - input.c */
void grabkeys(void);
void focusmon(const Arg *arg);
void focusstack(const Arg *arg);
void incnmaster(const Arg *arg);
void killclient(const Arg *arg);
void movemouse(const Arg *arg);
void quit(const Arg *arg);
void resizemouse(const Arg *arg);
void setlayout(const Arg *arg);
void setmfact(const Arg *arg);
void spawn(const Arg *arg);
void tag(const Arg *arg);
void tagmon(const Arg *arg);
void togglebar(const Arg *arg);
void togglefloating(const Arg *arg);
void togglegaps(const Arg *arg);
void togglefullscreen(const Arg *arg);
void toggletag(const Arg *arg);
void toggleview(const Arg *arg);
void view(const Arg *arg);
void zoom(const Arg *arg);

/* functions - layout.c */
void tile(Monitor *m);
void monocle(Monitor *m);

#endif
