/* uxwm - keyboard and mouse input commands */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wm.h"

/* ── key grabbing ────────────────────────────────────────────────────────── */
void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < (unsigned int)num_keys; i++) {
			if (!(code = XKeysymToKeycode(dpy, keys[i].keysym)))
				continue;
			for (j = 0; j < LENGTH(modifiers); j++)
				XGrabKey(dpy, code,
					keys[i].mod | modifiers[j],
					root, True,
					GrabModeAsync, GrabModeAsync);
		}
	}
}

/* ── spawn ───────────────────────────────────────────────────────────────── */
void
spawn(const Arg *arg)
{
	struct sigaction sa;

	if (arg->v == (const void *)dmenucmd)
		dmenumon[0] = '0' + selmon->num;
	if (fork() == 0) {
		if (dpy) close(ConnectionNumber(dpy));
		setsid();
		sigemptyset(&sa.sa_mask);
		sa.sa_flags   = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);
		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("uxwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

/* ── quit ────────────────────────────────────────────────────────────────── */
void
quit(const Arg *arg)
{
	(void)arg;
	running = 0;
}

/* ── kill client ─────────────────────────────────────────────────────────── */
void
killclient(const Arg *arg)
{
	(void)arg;
	if (!selmon->sel) return;
	if (!sendevent(selmon->sel, wmatom[WMDelete])) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void
togglefullscreen(const Arg *arg)
{
	(void)arg;
	if (!selmon->sel) return;
	setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}

/* ── focus stack ─────────────────────────────────────────────────────────── */
void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;

	if (!selmon->sel || (selmon->sel->isfullscreen && lockfullscreen))
		return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i)) c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i)) c = i;
	}
	if (c) {
		focus(c);
		restack(selmon);
	}
}

/* ── focus monitor ───────────────────────────────────────────────────────── */
void
focusmon(const Arg *arg)
{
	Monitor *m;
	if (!mons->next) return;
	if ((m = dirtomon(arg->i)) == selmon) return;
	unfocus(selmon->sel, 0);
	selmon = m;
	focus(NULL);
}

/* ── inc nmaster ─────────────────────────────────────────────────────────── */
void
incnmaster(const Arg *arg)
{
	selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

/* ── set mfact ───────────────────────────────────────────────────────────── */
void
setmfact(const Arg *arg)
{
	float f;
	if (!arg || !selmon->lt[selmon->sellt]->arrange) return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95) return;
	selmon->mfact = f;
	arrange(selmon);
}

/* ── set layout ──────────────────────────────────────────────────────────── */
void
setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = (Layout *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel) arrange(selmon);
	else             drawbar(selmon);
}

/* ── toggle bar ──────────────────────────────────────────────────────────── */
void
togglebar(const Arg *arg)
{
	(void)arg;
	selmon->showbar = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bh);
	arrange(selmon);
}

/* ── toggle floating ─────────────────────────────────────────────────────── */
void
togglefloating(const Arg *arg)
{
	(void)arg;
	if (!selmon->sel) return;
	if (selmon->sel->isfullscreen) return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if (selmon->sel->isfloating)
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
			selmon->sel->w, selmon->sel->h, 0);
	arrange(selmon);
}

void
togglegaps(const Arg *arg)
{
	(void)arg;
	enablegaps = !enablegaps;
	arrange(selmon);
}

/* ── view (switch desktop) ───────────────────────────────────────────────── */
/*
 * FIX: The original logic used a simple XOR on seltags which caused a
 * "last visited" toggle — but it broke when switching desktops in a
 * non-alternating order (e.g. 1->3->2->3 would jump back to 1 instead of
 * staying on 3, because seltags[0] still held tag 1).
 *
 * The correct approach:
 *   - Always write the new tagset into the CURRENT seltags slot.
 *   - Toggle seltags so the previous slot is preserved for Mod+Tab "last
 *     visited" behaviour.
 *   - Only bail out early when the tag is already active (no-op).
 */
void
view(const Arg *arg)
{
	unsigned int newtagset = arg->ui & TAGMASK;

	/* Mod+Tab / view({0}) means "go back to previous tag" */
	if (newtagset == 0) {
		selmon->seltags ^= 1;
		focus(NULL);
		arrange(selmon);
		return;
	}

	/* Already on this exact desktop — nothing to do */
	if (newtagset == selmon->tagset[selmon->seltags])
		return;

	/* Save current as "previous", flip slot, write new tagset */
	selmon->seltags ^= 1;
	selmon->tagset[selmon->seltags] = newtagset;

	focus(NULL);
	arrange(selmon);
}

void
toggleview(const Arg *arg)
{
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);
	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL);
		arrange(selmon);
	}
}

/* ── tag (move focused window to desktop) ────────────────────────────────── */
void
tag(const Arg *arg)
{
	if (selmon->sel && (arg->ui & TAGMASK)) {
		selmon->sel->tags = arg->ui & TAGMASK;
		updateclientdesktop(selmon->sel);
		focus(NULL);
		arrange(selmon);
	}
}

void
toggletag(const Arg *arg)
{
	unsigned int newtags;
	if (!selmon->sel) return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		updateclientdesktop(selmon->sel);
		focus(NULL);
		arrange(selmon);
	}
}

void
tagmon(const Arg *arg)
{
	if (!selmon->sel || !mons->next) return;
	sendmon(selmon->sel, dirtomon(arg->i));
}

/* ── zoom (swap with master) ─────────────────────────────────────────────── */
void
zoom(const Arg *arg)
{
	(void)arg;
	Client *c = selmon->sel;
	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating) return;
	if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next))) return;
	pop(c);
}

/* ── mouse move ──────────────────────────────────────────────────────────── */
void
movemouse(const Arg *arg)
{
	(void)arg;
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel)) return;
	if (c->isfullscreen) return;
	restack(selmon);
	ocx = c->x; ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y)) return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			if (ev.type == Expose) {
				Monitor *em;
				if (ev.xexpose.count == 0 && (em = wintomon(ev.xexpose.window)))
					drawbar(em);
			} else if (ev.type == MapRequest) {
				static XWindowAttributes wa;
				if (!XGetWindowAttributes(dpy, ev.xmaprequest.window, &wa) || wa.override_redirect)
					break;
				if (!wintoclient(ev.xmaprequest.window))
					manage(ev.xmaprequest.window, &wa);
			}
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (unsigned long)(1000 / refreshrate))
				continue;
			lasttime = ev.xmotion.time;
			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < (int)snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < (int)snap)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < (int)snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < (int)snap)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
			&& (abs(nx - c->x) > (int)snap || abs(ny - c->y) > (int)snap))
				togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

/* ── mouse resize ────────────────────────────────────────────────────────── */
void
resizemouse(const Arg *arg)
{
	(void)arg;
	int ocx, ocy, nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel)) return;
	if (c->isfullscreen) return;
	restack(selmon);
	ocx = c->x; ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			if (ev.type == Expose) {
				Monitor *em;
				if (ev.xexpose.count == 0 && (em = wintomon(ev.xexpose.window)))
					drawbar(em);
			}
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (unsigned long)(1000 / refreshrate))
				continue;
			lasttime = ev.xmotion.time;
			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&&  c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh) {
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > (int)snap || abs(nh - c->h) > (int)snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}
