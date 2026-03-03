/* uxwm/wm/x11.c — X11 event handling, WM lifecycle */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <time.h>
#include "wm.h"

/* defined in wm.c — writes current time into stext */
void updatestatustext(void);

/* ── event handler forward declarations ─────────────────────────────────── */
static void on_buttonpress(XEvent *e);
static void on_clientmessage(XEvent *e);
static void on_configurerequest(XEvent *e);
static void on_configurenotify(XEvent *e);
static void on_destroynotify(XEvent *e);
static void on_enternotify(XEvent *e);
static void on_expose(XEvent *e);
static void on_focusin(XEvent *e);
static void on_keypress(XEvent *e);
static void on_mappingnotify(XEvent *e);
static void on_maprequest(XEvent *e);
static void on_motionnotify(XEvent *e);
static void on_propertynotify(XEvent *e);
static void on_unmapnotify(XEvent *e);

static void (*handler[LASTEvent])(XEvent *) = {
    [ButtonPress]      = on_buttonpress,
    [ClientMessage]    = on_clientmessage,
    [ConfigureRequest] = on_configurerequest,
    [ConfigureNotify]  = on_configurenotify,
    [DestroyNotify]    = on_destroynotify,
    [EnterNotify]      = on_enternotify,
    [Expose]           = on_expose,
    [FocusIn]          = on_focusin,
    [KeyPress]         = on_keypress,
    [MappingNotify]    = on_mappingnotify,
    [MapRequest]       = on_maprequest,
    [MotionNotify]     = on_motionnotify,
    [PropertyNotify]   = on_propertynotify,
    [UnmapNotify]      = on_unmapnotify,
};

/* ── error handling ──────────────────────────────────────────────────────── */
static int (*xerrorxlib)(Display *, XErrorEvent *);

int
xerrordummy(Display *d, XErrorEvent *ee)
{
    (void)d; (void)ee;
    return 0;
}

int
xerror(Display *d, XErrorEvent *ee)
{
    (void)d;
    if (ee->error_code == BadWindow
    || (ee->request_code == X_SetInputFocus     && ee->error_code == BadMatch)
    || (ee->request_code == X_PolyText8         && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment       && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow   && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabButton        && ee->error_code == BadAccess)
    || (ee->request_code == X_GrabKey           && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea          && ee->error_code == BadDrawable))
        return 0;
    fprintf(stderr, "uxwm: X error: request=%d error=%d\n",
        ee->request_code, ee->error_code);
    return xerrorxlib(d, ee);
}

static int
xerrorstart(Display *d, XErrorEvent *ee)
{
    (void)d; (void)ee;
    die("uxwm: another window manager is already running");
    return -1;
}

/* ── monitor management ──────────────────────────────────────────────────── */
Monitor *
createmon(void)
{
    Monitor *m = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact   = mfact;
    m->nmaster = nmaster;
    m->showbar = showbar;
    m->topbar  = topbar;
    m->lt[0]   = &layouts[0];
    m->lt[1]   = &layouts[1 % num_layouts];
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
    return m;
}

void
cleanupmon(Monitor *mon)
{
    Monitor *m;
    if (mon == mons) {
        mons = mons->next;
    } else {
        for (m = mons; m && m->next != mon; m = m->next);
        m->next = mon->next;
    }
    XUnmapWindow(dpy, mon->barwin);
    XDestroyWindow(dpy, mon->barwin);
    free(mon);
}

Monitor *
dirtomon(int dir)
{
    Monitor *m = NULL;
    if (dir > 0) {
        if (!(m = selmon->next)) m = mons;
    } else if (selmon == mons) {
        for (m = mons; m->next; m = m->next);
    } else {
        for (m = mons; m->next != selmon; m = m->next);
    }
    return m;
}

/* ── geometry helpers ────────────────────────────────────────────────────── */
int
getrootptr(int *x, int *y)
{
    int di; unsigned int dui; Window dummy;
    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

Monitor *
recttomon(int x, int y, int w, int h)
{
    Monitor *m, *r = selmon;
    int a, area = 0;
    for (m = mons; m; m = m->next)
        if ((a = INTERSECT(x, y, w, h, m)) > area) { area = a; r = m; }
    return r;
}

/* ── window lookup ───────────────────────────────────────────────────────── */
Client *
wintoclient(Window w)
{
    Client *c; Monitor *m;
    for (m = mons; m; m = m->next)
        for (c = m->clients; c; c = c->next)
            if (c->win == w) return c;
    return NULL;
}

Monitor *
wintomon(Window w)
{
    int x, y; Client *c; Monitor *m;
    if (w == root && getrootptr(&x, &y)) return recttomon(x, y, 1, 1);
    for (m = mons; m; m = m->next)
        if (w == m->barwin) return m;
    if ((c = wintoclient(w))) return c->mon;
    return selmon;
}

/* ── numlock ─────────────────────────────────────────────────────────────── */
void
updatenumlockmask(void)
{
    unsigned int i, j;
    XModifierKeymap *modmap = XGetModifierMapping(dpy);
    numlockmask = 0;
    for (i = 0; i < 8; i++)
        for (j = 0; j < modmap->max_keypermod; j++)
            if (modmap->modifiermap[i * modmap->max_keypermod + j]
                == XKeysymToKeycode(dpy, XK_Num_Lock))
                numlockmask = 1u << i;
    XFreeModifiermap(modmap);
}

/* ── multi-monitor geometry ──────────────────────────────────────────────── */
#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
        &&  unique[n].width == info->width  && unique[n].height == info->height)
            return 0;
    return 1;
}
#endif

int
updategeom(void)
{
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int i, j, n, nn;
        Client *c; Monitor *m;
        XineramaScreenInfo *info   = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo *unique = ecalloc(nn, sizeof *unique);

        for (n = 0, m = mons; m; m = m->next, n++);
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof *unique);
        XFree(info);
        nn = j;

        for (i = n; i < nn; i++) {         /* add new monitors */
            for (m = mons; m && m->next; m = m->next);
            if (m) m->next = createmon(); else mons = createmon();
        }
        for (i = 0, m = mons; i < nn && m; m = m->next, i++) {
            if (i >= n
            || unique[i].x_org != m->mx || unique[i].y_org != m->my
            || unique[i].width != m->mw || unique[i].height != m->mh) {
                dirty = 1; m->num = i;
                m->mx = m->wx = unique[i].x_org;
                m->my = m->wy = unique[i].y_org;
                m->mw = m->ww = unique[i].width;
                m->mh = m->wh = unique[i].height;
                updatebarpos(m);
            }
        }
        for (i = nn; i < n; i++) {         /* remove surplus monitors */
            for (m = mons; m && m->next; m = m->next);
            while ((c = m->clients)) {
                dirty = 1; m->clients = c->next;
                detachstack(c); c->mon = mons;
                attach(c); attachstack(c);
            }
            if (m == selmon) selmon = mons;
            cleanupmon(m);
        }
        free(unique);
    } else
#endif
    {
        if (!mons) mons = createmon();
        if (mons->mw != sw || mons->mh != sh) {
            dirty = 1;
            mons->mw = mons->ww = sw;
            mons->mh = mons->wh = sh;
            updatebarpos(mons);
        }
    }
    if (dirty) { selmon = mons; selmon = wintomon(root); }
    return dirty;
}

/* ── startup ─────────────────────────────────────────────────────────────── */
void
checkotherwm(void)
{
    xerrorxlib = XSetErrorHandler(xerrorstart);
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);
}

void
scan(void)
{
    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (!XQueryTree(dpy, root, &d1, &d2, &wins, &num)) return;

    for (i = 0; i < num; i++) {    /* non-transient first */
        if (!XGetWindowAttributes(dpy, wins[i], &wa)
        || wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
            continue;
        if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
            manage(wins[i], &wa);
    }
    for (i = 0; i < num; i++) {    /* transient second */
        if (!XGetWindowAttributes(dpy, wins[i], &wa)) continue;
        if (XGetTransientForHint(dpy, wins[i], &d1)
        && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
            manage(wins[i], &wa);
    }
    if (wins) XFree(wins);
}

/* ── event handlers ──────────────────────────────────────────────────────── */
static void
on_buttonpress(XEvent *e)
{
    unsigned int i, x, click;
    Arg arg = {0};
    Client *c; Monitor *m;
    XButtonPressedEvent *ev = &e->xbutton;

    click = ClkRootWin;
    if ((m = wintomon(ev->window)) && m != selmon) {
        unfocus(selmon->sel, 1);
        selmon = m;
        focus(NULL);
    }
    if (ev->window == selmon->barwin) {
        i = x = 0;
        do x += TEXTW(tags[i]);
        while (ev->x >= (int)x && ++i < (unsigned int)num_tags);

        if (i < (unsigned int)num_tags) {
            click = ClkTagBar; arg.ui = 1u << i;
        } else if (ev->x < (int)(x + TEXTW(selmon->ltsymbol))) {
            click = ClkLtSymbol;
        } else if (ev->x > selmon->ww - (int)TEXTW(stext)) {
            click = ClkStatusText;
        } else {
            click = ClkWinTitle;
        }
    } else if ((c = wintoclient(ev->window))) {
        focus(c);
        restack(selmon);
        XAllowEvents(dpy, ReplayPointer, CurrentTime);
        click = ClkClientWin;
    }
    for (i = 0; i < (unsigned int)num_buttons; i++)
        if (click == buttons[i].click && buttons[i].func
        && buttons[i].button == ev->button
        && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
            buttons[i].func(
                click == ClkTagBar && buttons[i].arg.i == 0
                ? &arg : &buttons[i].arg);
}

static void
on_clientmessage(XEvent *e)
{
    XClientMessageEvent *cme = &e->xclient;
    Client *c = wintoclient(cme->window);
    if (!c) return;
    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == (long)netatom[NetWMFullscreen]
        ||  cme->data.l[2] == (long)netatom[NetWMFullscreen])
            setfullscreen(c, cme->data.l[0] == 1
                || (cme->data.l[0] == 2 && !c->isfullscreen));
    } else if (cme->message_type == netatom[NetActiveWindow]) {
        if (c != selmon->sel && !c->isurgent) seturgent(c, 1);
    }
}

static void
on_configurenotify(XEvent *e)
{
    XConfigureEvent *ev = &e->xconfigure;
    Client *c; Monitor *m;
    int dirty;
    if (ev->window != root) return;
    dirty = (sw != ev->width || sh != ev->height);
    sw = ev->width; sh = ev->height;
    if (updategeom() || dirty) {
        drw_resize(drw, sw, bh);
        updatebars();
        for (m = mons; m; m = m->next) {
            for (c = m->clients; c; c = c->next)
                if (c->isfullscreen)
                    resizeclient(c, m->mx, m->my, m->mw, m->mh);
            XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, m->ww, bh);
        }
        focus(NULL);
        arrange(NULL);
    }
}

static void
on_configurerequest(XEvent *e)
{
    Client *c; Monitor *m;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges wc;

    if ((c = wintoclient(ev->window))) {
        if (ev->value_mask & CWBorderWidth) {
            c->bw = ev->border_width;
        } else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
            m = c->mon;
            if (ev->value_mask & CWX)      { c->oldx = c->x; c->x = m->mx + ev->x; }
            if (ev->value_mask & CWY)      { c->oldy = c->y; c->y = m->my + ev->y; }
            if (ev->value_mask & CWWidth)  { c->oldw = c->w; c->w = ev->width;      }
            if (ev->value_mask & CWHeight) { c->oldh = c->h; c->h = ev->height;     }
            if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
                c->x = m->mx + (m->mw / 2 - WIDTH(c)  / 2);
            if ((c->y + c->h) > m->my + m->mh && c->isfloating)
                c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2);
            if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
                configure(c);
            if (ISVISIBLE(c))
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
        } else {
            configure(c);
        }
    } else {
        wc.x = ev->x; wc.y = ev->y;
        wc.width = ev->width; wc.height = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling = ev->above; wc.stack_mode = ev->detail;
        XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    }
    XSync(dpy, False);
}

static void
on_destroynotify(XEvent *e)
{
    Client *c;
    if ((c = wintoclient(e->xdestroywindow.window))) unmanage(c, 1);
}

static void
on_enternotify(XEvent *e)
{
    Client *c; Monitor *m;
    XCrossingEvent *ev = &e->xcrossing;
    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior)
    && ev->window != root) return;
    c = wintoclient(ev->window);
    m = c ? c->mon : wintomon(ev->window);
    if (m != selmon) { unfocus(selmon->sel, 1); selmon = m; }
    else if (!c || c == selmon->sel) return;
    focus(c);
}

static void
on_expose(XEvent *e)
{
    Monitor *m;
    if (e->xexpose.count == 0 && (m = wintomon(e->xexpose.window)))
        drawbar(m);
}

static void
on_focusin(XEvent *e)
{
    /* enforce focus — some clients steal it */
    if (selmon->sel && e->xfocus.window != selmon->sel->win)
        setfocus(selmon->sel);
}

static void
on_keypress(XEvent *e)
{
    unsigned int i;
    KeySym ks = XKeycodeToKeysym(dpy, (KeyCode)e->xkey.keycode, 0);
    for (i = 0; i < (unsigned int)num_keys; i++)
        if (ks == keys[i].keysym
        && CLEANMASK(keys[i].mod) == CLEANMASK(e->xkey.state)
        && keys[i].func)
            keys[i].func(&keys[i].arg);
}

static void
on_mappingnotify(XEvent *e)
{
    XRefreshKeyboardMapping(&e->xmapping);
    if (e->xmapping.request == MappingKeyboard) grabkeys();
}

static void
on_maprequest(XEvent *e)
{
    static XWindowAttributes wa;
    XMapRequestEvent *ev = &e->xmaprequest;
    if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect) return;
    if (!wintoclient(ev->window)) manage(ev->window, &wa);
}

static void
on_motionnotify(XEvent *e)
{
    static Monitor *mon = NULL;
    Monitor *m;
    if (e->xmotion.window != root) return;
    if ((m = recttomon(e->xmotion.x_root, e->xmotion.y_root, 1, 1)) != mon && mon) {
        unfocus(selmon->sel, 1); selmon = m; focus(NULL);
    }
    mon = m;
}

static void
on_propertynotify(XEvent *e)
{
    Client *c; Window trans;
    XPropertyEvent *ev = &e->xproperty;

    if (ev->window == root && ev->atom == XA_WM_NAME) {
        updatestatus(); /* falls back to built-in clock if root name is empty */
        return;
    }
    if (ev->state == PropertyDelete || !(c = wintoclient(ev->window))) return;

    switch (ev->atom) {
    case XA_WM_TRANSIENT_FOR:
        if (!c->isfloating && XGetTransientForHint(dpy, c->win, &trans)
        && (c->isfloating = (wintoclient(trans) != NULL)))
            arrange(c->mon);
        break;
    case XA_WM_NORMAL_HINTS: c->hintsvalid = 0;         break;
    case XA_WM_HINTS:        updatewmhints(c); drawbars(); break;
    default: break;
    }
    if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
        updatetitle(c);
        if (c == c->mon->sel) drawbar(c->mon);
    }
    if (ev->atom == netatom[NetWMWindowType]) updatewindowtype(c);
}

static void
on_unmapnotify(XEvent *e)
{
    Client *c;
    XUnmapEvent *ev = &e->xunmap;
    if ((c = wintoclient(ev->window))) {
        if (ev->send_event) setclientstate(c, WithdrawnState);
        else                unmanage(c, 0);
    }
}

/* ── setup ───────────────────────────────────────────────────────────────── */
void
setup(void)
{
    int i;
    Atom utf8string;
    XSetWindowAttributes wa;
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);
    while (waitpid(-1, NULL, WNOHANG) > 0);

    screen = DefaultScreen(dpy);
    sw     = DisplayWidth(dpy, screen);
    sh     = DisplayHeight(dpy, screen);
    root   = RootWindow(dpy, screen);

    drw = drw_create(dpy, screen, root, sw, sh);
    if (!drw_fontset_create(drw, fonts, num_fonts))
        die("uxwm: no fonts could be loaded");
    lrpad = drw->fonts->h;
    bh    = drw->fonts->h + 2;

    updategeom();

    utf8string                     = XInternAtom(dpy, "UTF8_STRING",                False);
    wmatom[WMProtocols]            = XInternAtom(dpy, "WM_PROTOCOLS",               False);
    wmatom[WMDelete]               = XInternAtom(dpy, "WM_DELETE_WINDOW",           False);
    wmatom[WMState]                = XInternAtom(dpy, "WM_STATE",                   False);
    wmatom[WMTakeFocus]            = XInternAtom(dpy, "WM_TAKE_FOCUS",              False);
    netatom[NetActiveWindow]       = XInternAtom(dpy, "_NET_ACTIVE_WINDOW",         False);
    netatom[NetSupported]          = XInternAtom(dpy, "_NET_SUPPORTED",             False);
    netatom[NetWMName]             = XInternAtom(dpy, "_NET_WM_NAME",               False);
    netatom[NetWMState]            = XInternAtom(dpy, "_NET_WM_STATE",              False);
    netatom[NetWMCheck]            = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK",   False);
    netatom[NetWMFullscreen]       = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN",   False);
    netatom[NetWMWindowType]       = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE",        False);
    netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList]         = XInternAtom(dpy, "_NET_CLIENT_LIST",           False);

    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove]   = drw_cur_create(drw, XC_fleur);

    scheme = ecalloc(num_colors, sizeof *scheme);
    for (i = 0; i < num_colors; i++)
        scheme[i] = drw_scm_create(drw, colors[i], 3);

    updatebars();
    updatestatustext();   /* seed clock before first event */
    drawbars();

    wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char *)&wmcheckwin, 1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
        PropModeReplace, (unsigned char *)"uxwm", 4);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
        PropModeReplace, (unsigned char *)&wmcheckwin, 1);
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
        PropModeReplace, (unsigned char *)netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);

    wa.cursor     = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask
                  | ButtonPressMask          | PointerMotionMask
                  | EnterWindowMask          | LeaveWindowMask
                  | StructureNotifyMask      | PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);

    grabkeys();
    focus(NULL);
}

/* ── main event loop ─────────────────────────────────────────────────────── */
void
run(void)
{
    XEvent ev;
    int xfd = ConnectionNumber(dpy);

    XSync(dpy, False);

    while (running) {
        /* drain all pending events */
        while (XPending(dpy)) {
            XNextEvent(dpy, &ev);
            if (handler[ev.type]) handler[ev.type](&ev);
            if (!running) return;
        }

        /* sleep up to 1 s; tick the clock on timeout */
        fd_set fds;
        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
        FD_ZERO(&fds);
        FD_SET(xfd, &fds);
        if (select(xfd + 1, &fds, NULL, NULL, &tv) == 0) {
            updatestatustext();
            drawbars();
        }
    }
}

/* ── cleanup ─────────────────────────────────────────────────────────────── */
void
cleanup(void)
{
    Arg    a   = { .ui = ~0u };
    Layout foo = { "", NULL };
    Monitor *m;
    size_t i;

    view(&a);
    selmon->lt[selmon->sellt] = &foo;
    for (m = mons; m; m = m->next)
        while (m->stack) unmanage(m->stack, 0);

    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    while (mons) cleanupmon(mons);

    for (i = 0; i < CurLast; i++)              drw_cur_free(drw, cursor[i]);
    for (i = 0; i < (size_t)num_colors; i++)   drw_scm_free(drw, scheme[i], 3);
    free(scheme);

    XDestroyWindow(dpy, wmcheckwin);
    drw_free(drw);
    XSync(dpy, False);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}
