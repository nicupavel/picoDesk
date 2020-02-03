/*
 * Copyright © 2008 Nicu Pavel
 * Copyright © 2003 Keith Packard
 */

#ifndef _PICODESK_H_
#define _PICODESK_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/poll.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>

#define WIDTH	640
#define HEIGHT	480
#define DEPTH   32
#define DEPTHNAME TrueColor
#define WINDOWNAME "picoDesk"

#define HIDE_DESKTOP 1

/* X Events */
#define IGNORE_NEW_WINDOWS 1
#define IGNORE_REPARENT 0

//#define USBOUTPUT
typedef struct { double x, y; } doublePoint;

typedef struct _Window {
    struct _Window    	*next;
    Window		id;
    Picture		p;
    XWindowAttributes	a;
    Damage		d;
    int			x, y;
    int			w, h;
    Bool		damaged;
    Bool		onscreen;
    double		area;
    doublePoint		pt;
} WindowRec, *WindowPtr;

#define WindowShow(w)	((w)->a.map_state == IsViewable && (w)->p && (w)->onscreen)

typedef struct _Screen {
    Display		*dpy;
    int			scr;
    Window		root;
    int			root_w, root_h;
    Visual		*visual;
    WindowPtr		windows;
    Window		id;
    Window		f;  /* frame we've been reparented to */
    Picture		p;
    Picture		b;
    Picture		t;
    Picture		d;
    int			w, h;
    int			x, y;
    int			sw;
} ScreenRec, *ScreenPtr;

#define SHADOW_WIDTH	0

/* layout.c */

void
LayoutWindows (ScreenPtr s);

/* window.c */

WindowPtr
FindWindow (ScreenPtr s, Window id);

void
PaintWindow (ScreenPtr s, WindowPtr w);

void
PaintAll (ScreenPtr s);

void
MapWindow (ScreenPtr s, Window id);

void
UnmapWindow (ScreenPtr s, Window id);

void
AddWindow (ScreenPtr s, Window id, Window prev);

void
RestackWindow (ScreenPtr s, WindowPtr w, Window new_above);

void
ConfigureWindow (ScreenPtr s, XConfigureEvent *ce);

void
CirculateWindow (ScreenPtr s, XCirculateEvent *ce);

void
DestroyWindow (ScreenPtr s, Window id, Bool gone);

void
DamageWindow (ScreenPtr s, XDamageNotifyEvent *de);

#endif /* _PICODESK_H_ */
