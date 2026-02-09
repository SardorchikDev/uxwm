#ifndef WM_H
#define WM_H

#include <X11/Xlib.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define LENGTH(X) (sizeof(X) / sizeof(X[0]))

typedef struct Client Client;
typedef struct Monitor Monitor;

struct Client {
    Window win;
    int x, y, w, h;
    int oldx, oldy, oldw, oldh;
    int basew, baseh, incw, inch;
    int maxw, maxh, minw, minh;
    int bw;
    int isfloating;
    int isfullscreen;
    int isvisible;
    int isurgent;
    unsigned int tags;  /* ADDED: Tag membership */
    Client *next;
    Client *snext;
};

struct Monitor {
    int mx, my, mw, mh;
    int wx, wy, ww, wh;
    int nmaster;
    float mfact;
    unsigned int seltags;
    unsigned int sellt;
    unsigned int tagset[2];
    const void *lt[2];
    Client *clients;
    Client *sel;
    Client *stack;
};

extern Display *dpy;
extern Window root;
extern Monitor *mon;
extern int running;
extern int screen;
extern int sw, sh;
extern int enablegaps;

void manage(Window w);
void unmanage(Client *c, int destroyed);
void focus(Client *c);
void unfocus(Client *c, int setfocus);
void arrange(Monitor *m);
void resize(Client *c, int x, int y, int w, int h, int interact);
void restack(Monitor *m);
void setfullscreen(Client *c, int fullscreen);
void togglefloating(const void *arg);
void killclient(const void *arg);
void focusstack(const void *arg);
void incnmaster(const void *arg);
void setmfact(const void *arg);
void zoom(const void *arg);
void setlayout(const void *arg);
void view(const void *arg);      /* ADDED: Switch workspace */
void tag(const void *arg);       /* ADDED: Move window to workspace */
void updateclientlist(void);
void updatewmhints(Client *c);
void updatesizehints(Client *c);
void configure(Client *c);
void setborder(Client *c, int focused);

#endif
