#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BAR_HEIGHT 18
#define FONT_NAME "monospace:size=11"
#define PADDING 10
#define RADIUS 0  // rounded corner radius in px

int read_battery(void) {
    FILE *f = fopen("/sys/class/power_supply/BAT0/capacity", "r");
    int cap = -1;
    if (f) {
        fscanf(f, "%d", &cap);
        fclose(f);
    }
    return cap;
}

void apply_rounded_mask(Display *dpy, Window win, int width, int height, int radius) {
    Pixmap mask = XCreatePixmap(dpy, win, width, height, 1);
    GC gc = XCreateGC(dpy, mask, 0, NULL);
    XSetForeground(dpy, gc, 0);
    XFillRectangle(dpy, mask, gc, 0, 0, width, height);
    XSetForeground(dpy, gc, 1);

    // draw central rectangle
    XFillRectangle(dpy, mask, gc, radius, 0, width - 2*radius, height);
    XFillRectangle(dpy, mask, gc, 0, radius, width, height - 2*radius);

    // draw corners (simple fill with arcs)
    XFillArc(dpy, mask, gc, 0, 0, 2*radius, 2*radius, 0, 23040);
    XFillArc(dpy, mask, gc, width - 2*radius, 0, 2*radius, 2*radius, 0, 23040);
    XFillArc(dpy, mask, gc, 0, height - 2*radius, 2*radius, 2*radius, 0, 23040);
    XFillArc(dpy, mask, gc, width - 2*radius, height - 2*radius, 2*radius, 2*radius, 0, 23040);

    XShapeCombineMask(dpy, win, ShapeBounding, 0, 0, mask, ShapeSet);

    XFreePixmap(dpy, mask);
    XFreeGC(dpy, gc);
}

int main(void) {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;

    int screen = DefaultScreen(dpy);
    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);

    XftFont *font = XftFontOpenName(dpy, screen, FONT_NAME);
    if (!font) return 1;

    XftColor fg;
    XRenderColor white = {0xffff, 0xffff, 0xffff, 0xffff};
    XftColorAllocValue(dpy, DefaultVisual(dpy, screen),
                       DefaultColormap(dpy, screen), &white, &fg);

    Window win = XCreateSimpleWindow(
        dpy, RootWindow(dpy, screen),
        0, 0, 1, BAR_HEIGHT,
        0, 0, 0
    );

    XSetWindowAttributes wa = { .override_redirect = True };
    XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &wa);
    XMapRaised(dpy, win);

    XftDraw *draw = XftDrawCreate(dpy, win,
                                  DefaultVisual(dpy, screen),
                                  DefaultColormap(dpy, screen));

    char text[64];

    while (1) {
        // build text
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        int bat = read_battery();
        if (bat >= 0)
            snprintf(text, sizeof(text), "%02d:%02d:%02d  |  BAT %d%%",
                     tm->tm_hour, tm->tm_min, tm->tm_sec, bat);
        else
            snprintf(text, sizeof(text), "%02d:%02d:%02d",
                     tm->tm_hour, tm->tm_min, tm->tm_sec);

        // measure text
        XGlyphInfo ext;
        XftTextExtentsUtf8(dpy, font, (XftChar8*)text, strlen(text), &ext);

        int win_w = ext.width + PADDING * 2;
        int win_x = (sw - win_w) / 2;
        int win_y = sh - BAR_HEIGHT;

        // resize and position window
        XMoveResizeWindow(dpy, win, win_x, win_y, win_w, BAR_HEIGHT);
        XClearWindow(dpy, win);

        // apply rounded corners
        apply_rounded_mask(dpy, win, win_w, BAR_HEIGHT, RADIUS);

        // draw text
        XftDrawStringUtf8(draw, &fg, font, PADDING, 14,
                          (XftChar8*)text, strlen(text));

        XFlush(dpy);
        sleep(1);
    }
}
