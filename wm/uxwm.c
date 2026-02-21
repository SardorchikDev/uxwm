/* uxwm - ultra-minimal X11 window manager
 * Based on dwm by suckless.org — split into modules.
 * Terminal: st   |   Launcher: dmenu_run
 */
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "wm.h"

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("uxwm-1.0");
	else if (argc != 1)
		die("usage: uxwm [-v]");

	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("uxwm: warning: no locale support\n", stderr);

	if (!(dpy = XOpenDisplay(NULL)))
		die("uxwm: cannot open display");

	checkotherwm();
	setup();
	scan();
	run();
	cleanup();
	XCloseDisplay(dpy);
	return EXIT_SUCCESS;
}
