#include <X11/Xlib.h>
#include "wm.h"
#include "layout.h"
#include "config.h"

void tile(Monitor *m)
{
    Client *c;
    int n = 0, i = 0;
    int mw, my, ty;
    int gap = enablegaps ? gappx : 0;

    for (c = m->clients; c; c = c->next)
        if (!c->isfloating && c->isvisible)
            n++;

    if (n == 0)
        return;

    /* Calculate master width with gaps */
    if (n > m->nmaster)
        mw = m->nmaster ? (m->ww - gap) * m->mfact : 0;
    else
        mw = m->ww - 2 * gap;

    /* Start positions with gap from edges */
    my = ty = m->wy + gap;

    for (c = m->clients; c; c = c->next) {
        if (!c->isvisible || c->isfloating)
            continue;

        if (i < m->nmaster) {
            /* Master window(s) - left side */
            int h = (m->wh - my - gap) / (MIN(n, m->nmaster) - i);
            resize(c, m->wx + gap, my, 
                   mw - gap, 
                   h - gap, 0);
            my += h;
        } else {
            /* Stack window(s) - right side */
            int h = (m->wh - ty - gap) / (n - i);
            resize(c, m->wx + mw + gap, ty, 
                   m->ww - mw - 2 * gap,
                   h - gap, 0);
            ty += h;
        }
        i++;
    }
}

void monocle(Monitor *m)
{
    Client *c;
    int n = 0;
    int gap = enablegaps ? gappx : 0;

    for (c = m->clients; c; c = c->next)
        if (!c->isfloating && c->isvisible)
            n++;

    if (n > 0) {
        for (c = m->clients; c; c = c->next) {
            if (!c->isvisible || c->isfloating)
                continue;
            /* Monocle with gaps around edges */
            resize(c, m->wx + gap, m->wy + gap, 
                   m->ww - 2 * gap, m->wh - 2 * gap, 0);
        }
    }
}

void floating(Monitor *m)
{
    Client *c;
    for (c = m->clients; c; c = c->next) {
        if (!c->isvisible)
            continue;
        if (!c->isfloating)
            resize(c, c->x, c->y, c->w, c->h, 0);
    }
}
