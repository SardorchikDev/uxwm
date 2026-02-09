#ifndef CONFIG_H
#define CONFIG_H

#include <X11/XF86keysym.h>

/* Appearance - WITH GAPS, NO BORDERS */
static const unsigned int borderpx = 0;
static const unsigned int gappx = 15;  /* Gap size in pixels */
static const unsigned int snap = 32;

/* Layout settings */
static const float mfact = 0.55;
static const int nmaster = 1;
static const int resizehints = 1;

/* Modifier key */
#define MOD Mod4Mask

#endif
