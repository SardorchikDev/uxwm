/* uxwm - layout functions */
#include "wm.h"

void
tile(Monitor *m)
{
	unsigned int i, n, h, mw, my, ty;
	unsigned int mastercount;
	int gap, split, wx, wy, ww, wh;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0) return;

	gap = enablegaps ? (int)gappx : 0;
	wx = m->wx + gap;
	wy = m->wy + gap;
	ww = MAX(m->ww - 2 * gap, 1);
	wh = MAX(m->wh - 2 * gap, 1);
	mastercount = MIN(n, (unsigned int)m->nmaster);
	split = n > (unsigned int)m->nmaster && m->nmaster ? gap : 0;

	if (n > (unsigned int)m->nmaster)
		mw = m->nmaster ? (ww - split) * m->mfact : 0;
	else
		mw = ww;

	for (i = my = ty = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < mastercount) {
			h = (wh - my - gap * (mastercount - i - 1)) / (mastercount - i);
			resize(c, wx, wy + my, mw - (2 * c->bw), h - (2 * c->bw), 0);
			my += HEIGHT(c) + gap;
		} else {
			h = (wh - ty - gap * (n - i - 1)) / (n - i);
			resize(c, wx + mw + split, wy + ty, ww - mw - split - (2 * c->bw), h - (2 * c->bw), 0);
			ty += HEIGHT(c) + gap;
		}
}

void
monocle(Monitor *m)
{
	unsigned int n = 0;
	int gap = enablegaps ? (int)gappx : 0;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c)) n++;
	if (n > 0)
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx + gap, m->wy + gap,
			m->ww - 2 * gap - 2 * c->bw,
			m->wh - 2 * gap - 2 * c->bw, 0);
}
