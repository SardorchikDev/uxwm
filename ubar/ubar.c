#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define BAR_HEIGHT 18
#define BAR_GAP 15             
#define FONT_NAME "JetBrainsMono Nerd Font:size=11"

int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    int screen = DefaultScreen(dpy);
    int sw = DisplayWidth(dpy, screen);
    int sh = DisplayHeight(dpy, screen);

    // --------------------------
    // IMPORTANT: UXWM should reserve:
    // usable_height = sh - (BAR_HEIGHT + BAR_GAP)
    // --------------------------

    // Create bottom bar window
    Window win = XCreateSimpleWindow(
        dpy,
        RootWindow(dpy, screen),
        0, sh - BAR_HEIGHT, // bar at bottom
        sw, BAR_HEIGHT,
        0,
        BlackPixel(dpy, screen),
        BlackPixel(dpy, screen)
    );

    // Override redirect to prevent WM decorations
    XSetWindowAttributes wa;
    wa.override_redirect = True;
    XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &wa);

    XSelectInput(dpy, win, ExposureMask | ButtonPressMask);
    XMapRaised(dpy, win);

    // Create Xft draw context
    XftDraw *draw = XftDrawCreate(dpy, win, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen));
    XftFont *font = XftFontOpenName(dpy, screen, FONT_NAME);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", FONT_NAME);
        return 1;
    }

    XftColor color;
    XRenderColor renderColor = { .red=0xffff, .green=0xffff, .blue=0xffff, .alpha=0xffff };
    XftColorAllocValue(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), &renderColor, &color);

    char time_str[64];
    XEvent ev;

    while (1) {
        // Handle expose events
        while (XPending(dpy)) {
            XNextEvent(dpy, &ev);
            if (ev.type == Expose) {
                // Redraw handled below
            }
        }

        // Get current time
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

        // Clear bar
        XClearWindow(dpy, win);

        // Measure text width for center alignment
        XGlyphInfo extents;
        XftTextExtentsUtf8(dpy, font, (XftChar8 *)time_str, strlen(time_str), &extents);
        int x = (sw - extents.width) / 2;
        int y = (BAR_HEIGHT + font->ascent - font->descent) / 2;

        // Draw time centered
        XftDrawStringUtf8(draw, &color, font, x, y, (XftChar8 *)time_str, strlen(time_str));

        usleep(1000000); // update every second
    }

    // Cleanup
    XftFontClose(dpy, font);
    XftDrawDestroy(draw);
    XftColorFree(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), &color);
    XCloseDisplay(dpy);

    return 0;
}
