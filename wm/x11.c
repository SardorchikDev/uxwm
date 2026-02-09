#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/cursorfont.h>
#include <stdio.h>
#include <stdlib.h>
#include "wm.h"
#include "x11.h"
#include "atoms.h"
#include "util.h"
#include "config.h"
#include "layout.h"
#include "input.h"

static Cursor cursor[3];

static int xerror(Display *d, XErrorEvent *ee)
{
    (void)d;
    if (ee->error_code == BadWindow
        || (ee->request_code == 42 && ee->error_code == BadMatch)
        || (ee->request_code == 74 && ee->error_code == BadDrawable)
        || (ee->request_code == 70 && ee->error_code == BadDrawable)
        || (ee->request_code == 66 && ee->error_code == BadDrawable)
        || (ee->request_code == 12 && ee->error_code == BadMatch)
        || (ee->request_code == 28 && ee->error_code == BadAccess)
        || (ee->request_code == 33 && ee->error_code == BadAccess)
        || (ee->request_code == 62 && ee->error_code == BadDrawable))
        return 0;
    
    fprintf(stderr, "uxwm: fatal error: request code=%d, error code=%d\n",
            ee->request_code, ee->error_code);
    return 0;
}

static int xerrorstart(Display *d, XErrorEvent *ee)
{
    (void)d;
    (void)ee;
    die("uxwm: another window manager is already running");
    return -1;
}

static void scan(void)
{
    unsigned int num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (unsigned int i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa)
                || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
                continue;
            if (wa.map_state == IsViewable)
                manage(wins[i]);
        }
        if (wins)
            XFree(wins);
    }
}

void setup(void)
{
    XSetWindowAttributes wa;

    dpy = XOpenDisplay(NULL);
    if (!dpy)
        die("uxwm: cannot open display");

    XSetErrorHandler(xerrorstart);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);
    sw = DisplayWidth(dpy, screen);
    sh = DisplayHeight(dpy, screen);

    initatoms();

    cursor[0] = XCreateFontCursor(dpy, XC_left_ptr);
    cursor[1] = XCreateFontCursor(dpy, XC_sizing);
    cursor[2] = XCreateFontCursor(dpy, XC_fleur);

    mon = ecalloc(1, sizeof(Monitor));
    
    /* FIXED: Set work area equal to monitor area - NO BAR OFFSET */
    mon->mx = mon->wx = 0;
    mon->my = mon->wy = 0;
    mon->mw = mon->ww = sw;
    mon->mh = mon->wh = sh;  /* Full height, no bar reservation */
    
    mon->nmaster = nmaster;
    mon->mfact = mfact;
    mon->sellt = 0;
    mon->tagset[0] = mon->tagset[1] = 1;
    mon->lt[0] = &layouts[0];
    mon->lt[1] = &layouts[1];

    wa.cursor = cursor[0];
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
                    ButtonPressMask | PointerMotionMask | EnterWindowMask |
                    LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
    
    XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);

    grabkeys();

    scan();
}

void run(void)
{
    XEvent ev;
    
    XSync(dpy, False);
    
    while (running && !XNextEvent(dpy, &ev)) {
        switch (ev.type) {
        case MapRequest:
            manage(ev.xmaprequest.window);
            break;
            
        case UnmapNotify: {
            Client *c;
            for (c = mon->clients; c; c = c->next) {
                if (c->win == ev.xunmap.window) {
                    unmanage(c, 0);
                    break;
                }
            }
        } break;
            
        case DestroyNotify: {
            Client *c;
            for (c = mon->clients; c; c = c->next) {
                if (c->win == ev.xdestroywindow.window) {
                    unmanage(c, 1);
                    break;
                }
            }
        } break;
            
        case ConfigureRequest: {
            XWindowChanges wc;
            Client *c;
            
            for (c = mon->clients; c; c = c->next)
                if (c->win == ev.xconfigurerequest.window)
                    break;
            
            if (c) {
                if (ev.xconfigurerequest.value_mask & CWBorderWidth)
                    c->bw = ev.xconfigurerequest.border_width;
                else if (c->isfloating) {
                    if (ev.xconfigurerequest.value_mask & CWX)
                        c->x = ev.xconfigurerequest.x;
                    if (ev.xconfigurerequest.value_mask & CWY)
                        c->y = ev.xconfigurerequest.y;
                    if (ev.xconfigurerequest.value_mask & CWWidth)
                        c->w = ev.xconfigurerequest.width;
                    if (ev.xconfigurerequest.value_mask & CWHeight)
                        c->h = ev.xconfigurerequest.height;
                    
                    if ((c->x + c->w) > mon->mx + mon->mw && c->isfloating)
                        c->x = mon->mx + (mon->mw / 2 - c->w / 2);
                    if ((c->y + c->h) > mon->my + mon->mh && c->isfloating)
                        c->y = mon->my + (mon->mh / 2 - c->h / 2);
                    
                    if ((ev.xconfigurerequest.value_mask & (CWX|CWY)) &&
                        !(ev.xconfigurerequest.value_mask & (CWWidth|CWHeight)))
                        configure(c);
                    
                    XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
                } else
                    configure(c);
            } else {
                wc.x = ev.xconfigurerequest.x;
                wc.y = ev.xconfigurerequest.y;
                wc.width = ev.xconfigurerequest.width;
                wc.height = ev.xconfigurerequest.height;
                wc.border_width = ev.xconfigurerequest.border_width;
                wc.sibling = ev.xconfigurerequest.above;
                wc.stack_mode = ev.xconfigurerequest.detail;
                XConfigureWindow(dpy, ev.xconfigurerequest.window,
                                 ev.xconfigurerequest.value_mask, &wc);
            }
            XSync(dpy, False);
        } break;
            
        case EnterNotify: {
            Client *c;
            if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior)
                break;
            
            for (c = mon->clients; c; c = c->next)
                if (c->win == ev.xcrossing.window)
                    break;
            
            focus(c);
        } break;
            
        case FocusIn:
            if (mon->sel && ev.xfocus.window != mon->sel->win)
                XSetInputFocus(dpy, mon->sel->win, RevertToPointerRoot, CurrentTime);
            break;
            
        case PropertyNotify: {
            Client *c;
            if (ev.xproperty.state == PropertyDelete)
                break;
            
            for (c = mon->clients; c; c = c->next) {
                if (c->win == ev.xproperty.window) {
                    if (ev.xproperty.atom == XA_WM_HINTS) {
                        updatewmhints(c);
                    } else if (ev.xproperty.atom == XA_WM_NORMAL_HINTS) {
                        updatesizehints(c);
                    }
                    break;
                }
            }
        } break;
            
        case ClientMessage:
            if (ev.xclient.message_type == wmatom[NetActiveWindow]) {
                Client *c;
                for (c = mon->clients; c; c = c->next)
                    if (c->win == ev.xclient.window)
                        break;
                if (c)
                    focus(c);
            }
            break;
            
        case KeyPress:
            handle_keypress(&ev);
            break;
        }
    }
}

void cleanup(void)
{
    Client *c;
    
    while (mon->clients) {
        c = mon->clients;
        mon->clients = c->next;
        XUnmapWindow(dpy, c->win);
        free(c);
    }
    
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    XFreeCursor(dpy, cursor[0]);
    XFreeCursor(dpy, cursor[1]);
    XFreeCursor(dpy, cursor[2]);
    XSync(dpy, False);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, wmatom[NetActiveWindow]);
    
    free(mon);
    XCloseDisplay(dpy);
}
