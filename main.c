/*
 * Copyright © 2008 Nicu Pavel <npavel@linuxconsulting.ro>
 */
#define _GNU_SOURCE

#include <string.h>

#include "picodesk.h"

#include "error.h"
#include "visual.h"
#include "window.h"
#include "timing.h"

#ifdef USBOUTPUT
#include "./mbl_encoder/mbl_encoder.h"
#include "lcdusb.h"
#endif

int main(int argc, char **argv)
{
    int event_base, error_base;
    int damage_event, damage_error;
    int xfixes_event, xfixes_error;
    int damaged = 0;

    int last_update;
    int now;
    int timeout;
    int n;
    int p;
    int i;

    struct pollfd ufd;

    Window root_return, parent_return;
    Window *children;
    unsigned int nchildren;

    ScreenRec s;
    XSetWindowAttributes a;
    XRenderColor c;
    XRenderPictureAttributes pa;

    Colormap colormap;
    unsigned long amask = 0;
    Pixmap backPixmap;
    Pixmap transPixmap;

    XWMHints *wmhints;
    XSizeHints *normalhints;
    XClassHint *classhint;

    XEvent ev;

    long *wmdesktop;
    char *wmtitle;

#ifdef USBOUTPUT

    if (!USBConnect())
    {
        fprintf(stderr, "Can't open usb\n");
        exit(1);
    }

    MBL_ENCODER *m_pEncoder = mbl_createEncoder(MBL_ENCODE_RAW_16BIT, WIDTH, HEIGHT);

#endif

    s.dpy = XOpenDisplay(NULL);

    if (!s.dpy)
    {
        fprintf(stderr, "Can't open display\n");
        exit(1);
    }

    XSetErrorHandler(error);

    if (!XCompositeQueryExtension(s.dpy, &event_base, &error_base))
    {
        fprintf(stderr, "No composite extension\n");
        exit(1);
    }
    if (!XDamageQueryExtension(s.dpy, &damage_event, &damage_error))
    {
        fprintf(stderr, "No damage extension\n");
        exit(1);
    }
    if (!XFixesQueryExtension(s.dpy, &xfixes_event, &xfixes_error))
    {
        fprintf(stderr, "No XFixes extension\n");
        exit(1);
    }

    s.scr = DefaultScreen(s.dpy);
    s.root = RootWindow(s.dpy, s.scr);
    s.visual = FindARGBVisual(s.dpy, s.scr);

    s.windows = 0;

    s.root_w = DisplayWidth(s.dpy, s.scr);
    s.root_h = DisplayHeight(s.dpy, s.scr);

    XCompositeRedirectSubwindows(s.dpy, s.root, CompositeRedirectAutomatic);

    XSelectInput(s.dpy, s.root,
                 SubstructureNotifyMask |
                     StructureNotifyMask |
                     PropertyChangeMask);

    /* MAIN WINDOW HANDLING */

    colormap = XCreateColormap(s.dpy, s.root, s.visual, AllocNone);

    a.event_mask = (ExposureMask |
                    StructureNotifyMask);
    amask = CWEventMask;

    a.background_pixel = 0;
    a.border_pixel = 0;
    a.colormap = colormap;
    amask |= CWBackPixel | CWBorderPixel | CWColormap;

    s.w = WIDTH;
    s.h = HEIGHT;
    s.id = XCreateWindow(s.dpy, s.root, 0, 0, s.w, s.h, 0,
                         DEPTH,
                         InputOutput, s.visual,
                         amask, &a);
    s.f = None;

    s.p = XRenderCreatePicture(s.dpy, s.id,
                               XRenderFindVisualFormat(s.dpy, s.visual),
                               0, 0);
    c.red = c.green = c.blue = 0x0000;
    c.alpha = 0xffff;
    backPixmap = XCreatePixmap(s.dpy, s.root, 1, 1, DEPTH);
    pa.repeat = True;
    s.b = XRenderCreatePicture(s.dpy, backPixmap,
                               XRenderFindVisualFormat(s.dpy, s.visual),
                               CPRepeat, &pa);
    XRenderFillRectangle(s.dpy, PictOpSrc, s.b, &c, 0, 0, 1, 1);

    c.red = c.green = c.blue = 0x0000;
    c.alpha = 0xc000;
    transPixmap = XCreatePixmap(s.dpy, s.root, 1, 1, 8);
    pa.repeat = True;
    s.t = XRenderCreatePicture(s.dpy, transPixmap,
                               XRenderFindStandardFormat(s.dpy, PictStandardA8),
                               CPRepeat, &pa);
    XRenderFillRectangle(s.dpy, PictOpSrc, s.t, &c, 0, 0, 1, 1);

    s.d = None;

    normalhints = XAllocSizeHints();
    normalhints->flags = 0;
    normalhints->x = 0;
    normalhints->y = 0;
    normalhints->width = s.w;
    normalhints->height = s.h;

    classhint = XAllocClassHint();
    classhint->res_name = "picodesk";
    classhint->res_class = "picoDesk";

    wmhints = XAllocWMHints();
    wmhints->flags = InputHint;
    wmhints->input = True;

    Xutf8SetWMProperties(s.dpy, s.id, WINDOWNAME, WINDOWNAME, 0, 0,
                         normalhints, wmhints, classhint);
    XFree(wmhints);
    XFree(classhint);
    XFree(normalhints);

    /* END MAIN WINDOW HANDLING */

    /* Show Window */
    XMapWindow(s.dpy, s.id);
    XSync(s.dpy, 0);

    /* Query XWindows for client list   */
    GetWindowsList(&s, &root_return, &parent_return, &children, &nchildren);

    if (!children)
    {
        fprintf(stderr, "Failed to get window list exiting \n");
        exit(1);
    }
    /* list windows */
    #ifdef DEBUG
    for (i = 0; i < nchildren; i++)
    {
        wmdesktop = (unsigned long *)GetWindowDesktop(&s, children[i]);
        fprintf(stderr, "Window: %ld Name: %s On Desktop: %ld\n", children[i], GetWindowTitle(&s, children[i]), *wmdesktop);
    }
    #endif

    for (i = 0; i < nchildren; i++)
    {
        wmdesktop = (long *)GetWindowDesktop(&s, children[i]);
        wmtitle = (char *)GetWindowTitle(&s, children[i]);
        if (*wmdesktop == -1)
            continue;
        if (HIDE_DESKTOP && strcasestr(wmtitle, "desktop") != NULL)
            continue;
        if (children[i] != s.id)
        {
            fprintf(stderr, "Adding window %s id: %lu own id: %lu\n", wmtitle, children[i], s.id);
            AddWindow(&s, children[i], i ? children[i - 1] : None);
        }
    }

    XFree(children);
    LayoutWindows(&s);
    PaintAll(&s);
    last_update = time_in_millis();

    /* cycle forever */
    for (;;)
    {

        int busy_start = 0;
        
        do
        {
            XNextEvent(s.dpy, &ev);
            if (!busy_start)
                busy_start = time_in_millis();
#if EVENTDEBUG
            printf("event %10.10s serial 0x%08x window 0x%08x\n",
                   ev_name(&ev), ev_serial(&ev), ev_window(&ev));
#endif

            switch (ev.type)
            {
            case CreateNotify:                
                if (IGNORE_NEW_WINDOWS)
                    break;
                fprintf(stderr, "EVENT: CreateNotify\n");
                if (ev.xcreatewindow.window == s.id)
                    break;
                AddWindow(&s, ev.xcreatewindow.window, 0);
                break;             
            case ConfigureNotify:
                fprintf(stderr, "EVENT: ConfigureNotify\n");
                ConfigureWindow(&s, &ev.xconfigure);
                if (ev.xconfigure.window == s.id)
                {
                    s.w = ev.xconfigure.width;
                    s.h = ev.xconfigure.height;
                    if (s.d)
                    {
                        XRenderFreePicture(s.dpy, s.d);
                        s.d = None;
                    }
                }
                else if (ev.xconfigure.window == s.root)
                {
                    s.root_w = ev.xconfigure.width;
                    s.root_h = ev.xconfigure.height;
                }
                LayoutWindows(&s);
                damaged = 1;
                break;
            case DestroyNotify:
                fprintf(stderr, "EVENT: DestroyNotify\n");
                DestroyWindow(&s, ev.xdestroywindow.window, True);
                break;
            case MapNotify:
                fprintf(stderr, "EVENT: MapNotify\n");
                MapWindow(&s, ev.xmap.window);
                LayoutWindows(&s);
                damaged = 1;
                break;
            case UnmapNotify:
                fprintf(stderr, "EVENT: UnmapNotify\n");
                UnmapWindow(&s, ev.xunmap.window);
                LayoutWindows(&s);
                damaged = 1;
                break;
            case ReparentNotify:
                if (IGNORE_REPARENT)
                    break;
                fprintf(stderr, "EVENT: ReparentNotify\n");    
                if (ev.xreparent.window == s.id)
                {
                    WindowPtr w;
                    s.f = ev.xreparent.parent;
                    printf("self reparented to 0x%lx\n", ev.xreparent.parent);
                    w = FindWindow(&s, ev.xreparent.parent);
                    if (w)
                    {
                        if (w->p)
                            XRenderFreePicture(s.dpy, w->p);
                        w->p = None;
                        if (w->d)
                            XDamageDestroy(s.dpy, w->d);
                        w->d = None;
                    }
                }
                if (ev.xreparent.parent == s.root)
                    AddWindow(&s, ev.xreparent.window, 0);
                else
                    DestroyWindow(&s, ev.xreparent.window, False);
                LayoutWindows(&s);
                damaged = 1;
                break;
            case CirculateNotify:
                fprintf(stderr, "EVENT: CirculateNotify\n");
                CirculateWindow(&s, &ev.xcirculate);
                break;
            case Expose:
                fprintf(stderr, "EVENT: Expose\n");
                damaged = 1;
                break;
            default:
                if (ev.type == damage_event + XDamageNotify)
                {
                    DamageWindow(&s, (XDamageNotifyEvent *)&ev);
                    damaged = 1;
                }
                break;
            }
            
        } while (QLength(s.dpy));

        now = time_in_millis();
        timeout = INTERVAL - (now - last_update);

        if (timeout > 0)
        {
            ufd.fd = ConnectionNumber(s.dpy);
            ufd.events = POLLIN;
            n = poll(&ufd, 1, timeout);
            if (n > 0 && (ufd.revents & POLLIN) && XEventsQueued(s.dpy, QueuedAfterReading))
                continue;
        }

        if (damaged)
        {
            int old_update = last_update;
            last_update = time_in_millis();
            PaintAll(&s);
#ifdef USBOUTPUT
            XImage *image = XGetImage(s.dpy, s.id, 0, 0, s.w, s.h, AllPlanes, ZPixmap);
            mbl_encode_from_32bit(m_pEncoder, image->data);
            XDestroyImage(image);

            USBWriteReport(m_pEncoder->dBuffer4Encoding.pBuffer, m_pEncoder->dBuffer4Encoding.nBufferSizeUsed);
#endif
            damaged = 0;
        }
        //PaintAll(&s);
    }
}
