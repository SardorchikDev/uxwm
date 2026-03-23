/* uxwm config.h - edit this file, then recompile */
#ifndef CONFIG_H
#define CONFIG_H

/* appearance */
unsigned int borderpx  = 2;
unsigned int gappx     = 10;
unsigned int snap      = 32;
int showbar            = 1;
int topbar             = 1;
const char *fonts[]    = { "JetBrainsMono Nerd Font Mono:size=12", "monospace:size=10" };
const char dmenufont[] = "JetBrainsMono Nerd Font Mono:size=12";

const char col_gray1[] = "#1a1b26";   /* bg */
const char col_gray2[] = "#414868";   /* inactive border */
const char col_gray3[] = "#a9b1d6";   /* fg */
const char col_gray4[] = "#c0caf5";   /* selected fg */
const char col_cyan[]  = "#7aa2f7";   /* active border / selected bg */

const char *colors[][3] = {
    /*               fg          bg          border    */
    [SchemeNorm] = { col_gray3,  col_gray1,  col_gray2 },
    [SchemeSel]  = { col_gray4,  col_cyan,   col_cyan  },
};

/* tagging - 9 workspaces */
const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

const Rule rules[] = {
    /* class         instance  title           tags mask  isfloating  monitor */
    { "Gimp",        NULL,     NULL,           0,         1,          -1 },
    { "Firefox",     NULL,     NULL,           1 << 8,    0,          -1 },
    { "mpv",         NULL,     NULL,           0,         1,          -1 },
    { "Pavucontrol", NULL,     NULL,           0,         1,          -1 },
    { NULL,          NULL,     "Event Tester", 0,         0,          -1 },
};

/* layout settings */
const float mfact          = 0.55;
const int nmaster          = 1;
const int resizehints      = 0;
const int lockfullscreen   = 1;
const int refreshrate      = 120;

const Layout layouts[] = {
    { "[]=", tile    },   /* default: tiling */
    { "><>", NULL    },   /* floating */
    { "[M]", monocle },
};

/* modifier */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY, view,       {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY, toggleview, {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY, tag,        {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY, toggletag,  {.ui = 1 << TAG} },

#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
char dmenumon[2] = "0";
const char *dmenucmd[] = {
    "dmenu_run", "-m", dmenumon,
    "-fn", dmenufont,
    "-nb", col_gray1, "-nf", col_gray3,
    "-sb", col_cyan,  "-sf", col_gray4,
    NULL
};
const char *termcmd[]    = { "st", NULL };
const char *browsercmd[] = { "firefox", NULL };

/* audio */
const char *volupcmd[]  = { "pamixer", "-i", "5",  NULL };
const char *voldncmd[]  = { "pamixer", "-d", "5",  NULL };
const char *voltogcmd[] = { "pamixer", "-t",        NULL };

/* brightness */
const char *brupcmd[]   = { "brightnessctl", "set", "5%+", NULL };
const char *brdncmd[]   = { "brightnessctl", "set", "5%-", NULL };

/* screenshots */
const char *scrotcmd[]  = { "scrot", "%Y-%m-%d_%H-%M-%S.png", "-e", "mv $f ~/Pictures/", NULL };
const char *sscmd[]     = { "scrot", "-s", "%Y-%m-%d_%H-%M-%S.png", "-e", "mv $f ~/Pictures/", NULL };

const Key keys[] = {
    /* modifier              key                       function        argument */

    /* apps */
    { MODKEY,                XK_p,                     spawn,          {.v = dmenucmd } },
    { MODKEY,                XK_Return,                spawn,          {.v = termcmd } },
    { MODKEY,                XK_w,                     spawn,          {.v = browsercmd } },

    /* screenshots */
    { 0,                     XK_Print,                 spawn,          {.v = scrotcmd } },
    { ShiftMask,             XK_Print,                 spawn,          {.v = sscmd } },

    /* audio (XF86 media keys) */
    { 0, XF86XK_AudioRaiseVolume, spawn,               {.v = volupcmd } },
    { 0, XF86XK_AudioLowerVolume, spawn,               {.v = voldncmd } },
    { 0, XF86XK_AudioMute,        spawn,               {.v = voltogcmd } },

    /* brightness */
    { 0, XF86XK_MonBrightnessUp,  spawn,               {.v = brupcmd } },
    { 0, XF86XK_MonBrightnessDown,spawn,               {.v = brdncmd } },

    /* window management */
    { MODKEY,                XK_b,                     togglebar,      {0} },
    { MODKEY,                XK_j,                     focusstack,     {.i = +1 } },
    { MODKEY,                XK_k,                     focusstack,     {.i = -1 } },
    { MODKEY,                XK_i,                     incnmaster,     {.i = +1 } },
    { MODKEY,                XK_d,                     incnmaster,     {.i = -1 } },
    { MODKEY,                XK_h,                     setmfact,       {.f = -0.05} },
    { MODKEY,                XK_l,                     setmfact,       {.f = +0.05} },
    { MODKEY,                XK_g,                     togglegaps,     {0} },
    { MODKEY|ShiftMask,      XK_Return,                zoom,           {0} },
    { MODKEY,                XK_Tab,                   view,           {0} },
    { MODKEY|ShiftMask,      XK_c,                     killclient,     {0} },
    { MODKEY|ShiftMask,      XK_f,                     togglefullscreen,{0} },
    { MODKEY|ShiftMask,      XK_space,                 togglefloating, {0} },

    /* layouts */
    { MODKEY,                XK_t,                     setlayout,      {.v = &layouts[0]} },
    { MODKEY,                XK_f,                     setlayout,      {.v = &layouts[1]} },
    { MODKEY,                XK_m,                     setlayout,      {.v = &layouts[2]} },
    { MODKEY,                XK_space,                 setlayout,      {0} },

    /* tags: view all / tag all */
    { MODKEY,                XK_0,                     view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,      XK_0,                     tag,            {.ui = ~0 } },

    /* monitors */
    { MODKEY,                XK_comma,                 focusmon,       {.i = -1 } },
    { MODKEY,                XK_period,                focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,      XK_comma,                 tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,      XK_period,                tagmon,         {.i = +1 } },

    /* tag number row */
    TAGKEYS(XK_1, 0)
    TAGKEYS(XK_2, 1)
    TAGKEYS(XK_3, 2)
    TAGKEYS(XK_4, 3)
    TAGKEYS(XK_5, 4)
    TAGKEYS(XK_6, 5)
    TAGKEYS(XK_7, 6)
    TAGKEYS(XK_8, 7)
    TAGKEYS(XK_9, 8)

    { MODKEY|ShiftMask,      XK_q,                     quit,           {0} },
};

const Button buttons[] = {
    /* click           mask    button    function        argument */
    { ClkLtSymbol,     0,      Button1,  setlayout,      {0} },
    { ClkLtSymbol,     0,      Button3,  setlayout,      {.v = &layouts[2]} },
    { ClkWinTitle,     0,      Button2,  zoom,           {0} },
    { ClkStatusText,   0,      Button2,  spawn,          {.v = termcmd } },
    { ClkClientWin,    MODKEY, Button1,  movemouse,      {0} },
    { ClkClientWin,    MODKEY, Button2,  togglefloating, {0} },
    { ClkClientWin,    MODKEY, Button3,  resizemouse,    {0} },
    { ClkTagBar,       0,      Button1,  view,           {0} },
    { ClkTagBar,       0,      Button3,  toggleview,     {0} },
    { ClkTagBar,       MODKEY, Button1,  tag,            {0} },
    { ClkTagBar,       MODKEY, Button3,  toggletag,      {0} },
};

#endif /* CONFIG_H */
