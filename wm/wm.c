
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <string.h>
#include "wm.h"
#include "atoms.h"
#include "util.h"
#include "config.h"
#include "layout.h"

Display *dpy;
Window root;
Monitor *mon;
int running = 1;
int screen;
int sw, sh;
int enablegaps = 1; /* FIXED: Start with gaps enabled */

void manage(Window w)
{
    Client *c;
    Window trans = None;
    XWindowAttributes wa;

    if (!XGetWindowAttributes(dpy, w, &wa) || wa.override_redirect || 
        XGetTransientForHint(dpy, w, &trans))
        return;

    c = ecalloc(1, sizeof(Client));
    c->win = w;
    c->x = c->oldx = wa.x;
    c->y = c->oldy = wa.y;
    c->w = c->oldw = wa.width;
    c->h = c->oldh = wa.height;
    c->bw = borderpx;
    c->isvisible = 1;
    c->tags = mon->tagset[mon->seltags]; /* Assign to current tag */

    updatesizehints(c);
    updatewmhints(c);

    if (trans != None || c->isfullscreen)
        c->isfloating = 1;

    /* Add to client list */
    c->next = mon->clients;
    mon->clients = c;

    /* Add to stack */
    c->snext = mon->stack;
    mon->stack = c;

    /* Set border - FIXED: Only if borderpx > 0 */
    if (borderpx > 0) {
        XSetWindowBorder(dpy, w, 0x333333);
        XSetWindowBorderWidth(dpy, w, c->bw);
    }

    configure(c);

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
                    PropModeReplace, (unsigned char[]){WithdrawnState, None}, 2);

    XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | 
                 PropertyChangeMask | StructureNotifyMask);

    Atom protocols[] = {wmatom[WMDelete], wmatom[WMTakeFocus]};
    XSetWMProtocols(dpy, c->win, protocols, 2);

    XMapWindow(dpy, w);
    
    setborder(c, 0);
    arrange(mon);
    focus(c);
    updateclientlist();
}

void unmanage(Client *c, int destroyed)
{
    Client **tc;

    /* Remove from client list */
    for (tc = &mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    if (*tc)
        *tc = c->next;

    /* Remove from stack */
    for (tc = &mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
    if (*tc)
        *tc = c->snext;

    if (!destroyed) {
        XGrabServer(dpy);
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        XSync(dpy, False);
        XUngrabServer(dpy);
    }

    free(c);
    focus(NULL);
    updateclientlist();
    arrange(mon);
}

void focus(Client *c)
{
    /* FIXED: Improved focus handling */
    if (!c || !c->isvisible)
        c = NULL;
    
    if (mon->sel && mon->sel != c)
        unfocus(mon->sel, 0);

    if (c) {
        /* Only focus if different from current */
        if (c != mon->sel) {
            /* Set border if enabled */
            if (borderpx > 0)
                XSetWindowBorder(dpy, c->win, 0x0088cc);
            
            /* Set input focus */
            XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
            
            /* Update EWMH active window */
            XChangeProperty(dpy, root, wmatom[NetActiveWindow], XA_WINDOW, 32,
                            PropModeReplace, (unsigned char *)&(c->win), 1);
            
            /* Raise window */
            XRaiseWindow(dpy, c->win);
            
            /* Send WM_TAKE_FOCUS if supported */
            XEvent ev;
            ev.type = ClientMessage;
            ev.xclient.window = c->win;
            ev.xclient.message_type = wmatom[WMProtocols];
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = wmatom[WMTakeFocus];
            ev.xclient.data.l[1] = CurrentTime;
            XSendEvent(dpy, c->win, False, NoEventMask, &ev);
        }
        mon->sel = c;
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, wmatom[NetActiveWindow]);
        mon->sel = NULL;
    }
    
    XSync(dpy, False);
}

void unfocus(Client *c, int setfocus)
{
    if (!c)
        return;
    setborder(c, 0);
    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, wmatom[NetActiveWindow]);
    }
}

void arrange(Monitor *m)
{
    /* Update window visibility based on tags */
    Client *c;
    for (c = m->clients; c; c = c->next)
        c->isvisible = c->tags & m->tagset[m->seltags];
    
    if (m->lt[m->sellt])
        ((Layout *)m->lt[m->sellt])->arrange(m);
    restack(m);
}

void resize(Client *c, int x, int y, int w, int h, int interact)
{
    (void)interact;
    
    if (resizehints) {
        if (c->incw)
            w -= w % c->incw;
        if (c->inch)
            h -= h % c->inch;
        
        w = MAX(w, c->minw);
        h = MAX(h, c->minh);
        
        if (c->maxw)
            w = MIN(w, c->maxw);
        if (c->maxh)
            h = MIN(h, c->maxh);
    }

    if (x > sw)
        x = sw - w - 2 * c->bw;
    if (y > sh)
        y = sh - h - 2 * c->bw;
    if (x + w + 2 * c->bw < 0)
        x = 0;
    if (y + h + 2 * c->bw < 0)
        y = 0;

    if (c->x != x || c->y != y || c->w != w || c->h != h) {
        c->x = x;
        c->y = y;
        c->w = w;
        c->h = h;
        XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        configure(c);
    }
}

void restack(Monitor *m)
{
    Client *c;
    XWindowChanges wc;

    if (!m->sel)
        return;

    if (m->sel->isfloating)
        XRaiseWindow(dpy, m->sel->win);

    wc.stack_mode = Below;
    wc.sibling = m->sel->win;
    for (c = m->stack; c; c = c->snext) {
        if (!c->isfloating && c->isvisible && c != m->sel) {
            XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
            wc.sibling = c->win;
        }
    }
    XSync(dpy, False);
}

void setfullscreen(Client *c, int fullscreen)
{
    if (fullscreen && !c->isfullscreen) {
        XChangeProperty(dpy, c->win, wmatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&wmatom[NetWMStateFullscreen], 1);
        c->isfullscreen = 1;
        c->oldx = c->x;
        c->oldy = c->y;
        c->oldw = c->w;
        c->oldh = c->h;
        resize(c, mon->mx, mon->my, mon->mw, mon->mh, 0);
        XRaiseWindow(dpy, c->win);
    } else if (!fullscreen && c->isfullscreen) {
        XChangeProperty(dpy, c->win, wmatom[NetWMState], XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)0, 0);
        c->isfullscreen = 0;
        resize(c, c->oldx, c->oldy, c->oldw, c->oldh, 0);
        arrange(mon);
    }
}

void configure(Client *c)
{
    XConfigureEvent ce;
    ce.type = ConfigureNotify;
    ce.display = dpy;
    ce.event = c->win;
    ce.window = c->win;
    ce.x = c->x;
    ce.y = c->y;
    ce.width = c->w;
    ce.height = c->h;
    ce.border_width = c->bw;
    ce.above = None;
    ce.override_redirect = False;
    XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void setborder(Client *c, int focused)
{
    /* FIXED: Only set border if borderpx > 0 */
    if (borderpx > 0)
        XSetWindowBorder(dpy, c->win, focused ? 0x0088cc : 0x333333);
}

void updateclientlist(void)
{
    Client *c;
    XDeleteProperty(dpy, root, wmatom[NetClientList]);
    for (c = mon->clients; c; c = c->next)
        XChangeProperty(dpy, root, wmatom[NetClientList], XA_WINDOW, 32,
                        PropModeAppend, (unsigned char *)&(c->win), 1);
}

void updatewmhints(Client *c)
{
    XWMHints *wmh = XGetWMHints(dpy, c->win);
    if (wmh) {
        if (c == mon->sel && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        } else
            c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
        XFree(wmh);
    }
}

void updatesizehints(Client *c)
{
    long msize;
    XSizeHints size;

    if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
        size.flags = PSize;

    if (size.flags & PBaseSize) {
        c->basew = size.base_width;
        c->baseh = size.base_height;
    } else if (size.flags & PMinSize) {
        c->basew = size.min_width;
        c->baseh = size.min_height;
    } else
        c->basew = c->baseh = 0;

    if (size.flags & PResizeInc) {
        c->incw = size.width_inc;
        c->inch = size.height_inc;
    } else
        c->incw = c->inch = 0;

    if (size.flags & PMaxSize) {
        c->maxw = size.max_width;
        c->maxh = size.max_height;
    } else
        c->maxw = c->maxh = 0;

    if (size.flags & PMinSize) {
        c->minw = size.min_width;
        c->minh = size.min_height;
    } else if (size.flags & PBaseSize) {
        c->minw = size.base_width;
        c->minh = size.base_height;
    } else
        c->minw = c->minh = 0;

    c->isfloating = size.flags & (PMaxSize | PMinSize) &&
                    size.min_width && size.min_height &&
                    size.max_width && size.max_height &&
                    size.min_width == size.max_width &&
                    size.min_height == size.max_height;
}

void togglefloating(const void *arg)
{
    (void)arg;
    if (!mon->sel)
        return;
    
    if (mon->sel->isfullscreen)
        return;
    
    mon->sel->isfloating = !mon->sel->isfloating;
    
    if (mon->sel->isfloating)
        resize(mon->sel, mon->sel->x, mon->sel->y, mon->sel->w, mon->sel->h, 0);
    
    arrange(mon);
}

void killclient(const void *arg)
{
    (void)arg;
    if (!mon->sel)
        return;
    
    XEvent ev;
    ev.type = ClientMessage;
    ev.xclient.window = mon->sel->win;
    ev.xclient.message_type = wmatom[WMProtocols];
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = wmatom[WMDelete];
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, mon->sel->win, False, NoEventMask, &ev);
}

void focusstack(const void *arg)
{
    Client *c = NULL, *i;
    int inc = *(const int *)arg;

    if (!mon->sel)
        return;

    /* FIXED: Better window cycling */
    if (inc > 0) {
        /* Focus next visible window */
        for (c = mon->sel->next; c && !c->isvisible; c = c->next);
        if (!c)
            for (c = mon->clients; c && !c->isvisible; c = c->next);
    } else {
        /* Focus previous visible window */
        for (i = mon->clients; i != mon->sel; i = i->next)
            if (i->isvisible)
                c = i;
        if (!c)
            for (; i; i = i->next)
                if (i->isvisible)
                    c = i;
    }

    if (c) {
        focus(c);
        restack(mon);
    }
}

void incnmaster(const void *arg)
{
    int inc = *(const int *)arg;
    mon->nmaster = MAX(mon->nmaster + inc, 0);
    arrange(mon);
}

void setmfact(const void *arg)
{
    float f = *(const float *)arg;
    
    if (f < 0.1 || f > 0.9)
        return;
    
    mon->mfact = mon->mfact + f;
    if (mon->mfact < 0.1)
        mon->mfact = 0.1;
    if (mon->mfact > 0.9)
        mon->mfact = 0.9;
    
    arrange(mon);
}

void zoom(const void *arg)
{
    (void)arg;
    Client *c = mon->sel;

    if (!c || c->isfloating)
        return;

    if (c == mon->clients) {
        if (!c->next || c->next->isfloating)
            return;
        c = c->next;
    }

    Client **tc;
    for (tc = &mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    if (*tc)
        *tc = c->next;

    c->next = mon->clients;
    mon->clients = c;

    focus(c);
    arrange(mon);
}

void setlayout(const void *arg)
{
    int i = *(const int *)arg;
    if (i >= 0 && (unsigned int)i < LENGTH(layouts))
        mon->lt[mon->sellt] = &layouts[i];
    arrange(mon);
}

void view(const void *arg)
{
    unsigned int newtags = *(const unsigned int *)arg;
    
    if (newtags == mon->tagset[mon->seltags])
        return;
    
    mon->seltags ^= 1; /* Toggle sel tagset */
    if (newtags != ~0U)  /* Use ~0U for unsigned comparison */
        mon->tagset[mon->seltags] = newtags;
    
    focus(NULL);
    arrange(mon);
}

void tag(const void *arg)
{
    if (mon->sel && arg) {
        mon->sel->tags = *(const unsigned int *)arg;
        focus(NULL);
        arrange(mon);
    }
}
