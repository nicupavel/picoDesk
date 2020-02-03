/*
 * Copyright © 2008 Nicu Pavel <npavel@linuxconsulting.ro>
 * Based on Keith Packard examples
 *
 */

#include "picodesk.h"
#include "window.h"

WindowPtr windowList;

static char *
GetWindowProperty(Display *dpy, Window id, Atom xa_prop_type, char *prop_name, unsigned long *size)
{
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems;
    unsigned long ret_bytes_after;
    unsigned long tmp_size;
    unsigned char *ret_prop;
    char *ret;

    if (!dpy)
        return NULL;

    xa_prop_name = XInternAtom(dpy, prop_name, False);

    /* MAX_PROPERTY_VALUE_LEN / 4 explanation (XGetWindowProperty manpage):
    *
    * long_length = Specifies the length in 32-bit multiples of the
    *               data to be retrieved.
    */
    if (XGetWindowProperty(dpy, id, xa_prop_name, 0, MAX_PROPERTY_VALUE_LEN / 4, False,
                           xa_prop_type, &xa_ret_type, &ret_format,
                           &ret_nitems, &ret_bytes_after, &ret_prop) != Success)
    {
        fprintf(stderr, "Cannot get %s property.\n", prop_name);
        return NULL;
    }

    if (xa_ret_type != xa_prop_type)
    {
        fprintf(stderr, "Invalid type of %s property.\n", prop_name);
        XFree(ret_prop);
        return NULL;
    }

    /* null terminate the result to make string handling easier */
    tmp_size = (ret_format / 8) * ret_nitems;
    ret = malloc(tmp_size + 1);
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size)
    {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

char *
GetWindowTitle(ScreenPtr s, Window id)
{
    char *wm_name;

#ifdef DEBUG
    fprintf(stderr, "Getting title for window id: %ld\n", id);
#endif
    wm_name = GetWindowProperty(s->dpy, id, XInternAtom(s->dpy, "UTF8_STRING", False), "_NET_WM_NAME", NULL);
    
    /* try alternate methods (WINWM) compliant WM */
    if (!wm_name) 
        wm_name = GetWindowProperty(s->dpy, id, XA_STRING, "_NET_WM_NAME", NULL);

    if (!wm_name)
        wm_name = GetWindowProperty(s->dpy, id, XA_STRING, "WM_NAME", NULL);
       
    if (!wm_name)
        wm_name = "No Title";

    return wm_name;
}

long *
GetWindowDesktop(ScreenPtr s, Window id)
{
    long *desktop;

    desktop = (long *)GetWindowProperty(s->dpy, id, XA_CARDINAL, "_NET_WM_DESKTOP", NULL);

    /* try alternate method WINWM compliant WM */
    if (!desktop)
        desktop = (long *)GetWindowProperty(s->dpy, id, XA_CARDINAL, "_WIN_WORKSPACE", NULL);

    if (!desktop)
        *desktop = -1;

    return desktop;
}

void GetWindowsList(ScreenPtr s, Window *root_return, Window *parent_return, Window **children, unsigned int *nchildren)
{

    if (WINDOW_LIST_METHOD == XLIST)
    {
        fprintf(stderr, "Using XWindows (XQueryTree) method\n");
        XQueryTree(s->dpy, s->root, root_return, parent_return, children, nchildren);
    }

    if (WINDOW_LIST_METHOD == WMLIST)
    {
        /* dummy size */
        unsigned long size;

        fprintf(stderr, "Using NETWM compliant window manager list method\n");
        if ((*children = (Window *)GetWindowProperty(s->dpy, DefaultRootWindow(s->dpy), XA_WINDOW, "_NET_CLIENT_LIST", &size)) == NULL)
        {
            fprintf(stderr, "Failed using NETWM methond falling back to WINWM compliant\n");
            if ((*children = (Window *)GetWindowProperty(s->dpy, DefaultRootWindow(s->dpy), XA_CARDINAL, "_WIN_CLIENT_LIST", &size)) == NULL)
            {
                fprintf(stderr, "Cannot get children\n");
            }
        }
        if (children) {
            *nchildren = size / sizeof(Window);
            fprintf(stderr, "Found %d windows\n", *nchildren);
        }

    }
}

WindowPtr
FindWindow(ScreenPtr s, Window id)
{
    WindowPtr w;

    for (w = s->windows; w; w = w->next)
        if (w->id == id)
            return w;
    return 0;
}

void PaintShadow(ScreenPtr s, WindowPtr w)
{
    if (WindowShow(w))
    {
        static XRenderColor shade = {0, 0, 0, 0x8000};

        XRenderFillRectangle(s->dpy, PictOpSrc, s->d,
                             &shade, w->x - s->sw, w->y - s->sw,
                             w->w + s->sw * 2, w->h + s->sw * 2);
    }
}

void PaintWindow(ScreenPtr s, WindowPtr w)
{

#ifdef DEBUGPAINT
    printf("Paint window 0x%x at %d %d (%d x %d)\n",
           w->id, w->x, w->y, w->w, w->h);
#endif

    XDamageSubtract(s->dpy, w->d, None, None);
    if (WindowShow(w))
    {
        XRenderComposite(s->dpy, PictOpOver,
                         w->p, None, s->d,
                         0, 0, 0, 0, w->x, w->y, w->w, w->h);
    }
    else
        w->damaged = 0;
}

void PaintAll(ScreenPtr s)
{
    WindowPtr w;
    Pixmap doublePixmap;

    if (!s->d)
    {
        doublePixmap = XCreatePixmap(s->dpy, s->root, s->w, s->h, 32);
        s->d = XRenderCreatePicture(s->dpy, doublePixmap,
                                    XRenderFindVisualFormat(s->dpy, s->visual),
                                    0, 0);
        XFreePixmap(s->dpy, doublePixmap);
    }

    XRenderComposite(s->dpy, PictOpSrc,
                     s->b, None, s->d,
                     0, 0, 0, 0, 0, 0, s->w, s->h);

    /*
    for (w = s->windows; w; w = w->next)
	    if (w->damaged)
	        PaintShadow (s, w);
*/
    for (w = s->windows; w; w = w->next)
        if (w->damaged)
            PaintWindow(s, w);

    XRenderComposite(s->dpy, PictOpSrc,
                     s->d, None, s->p,
                     0, 0, 0, 0, 0, 0, s->w, s->h);
                     
}

void MapWindow(ScreenPtr s, Window id)
{
    WindowPtr w = FindWindow(s, id);

    if (!w)
        return;
    w->a.map_state = IsViewable;
    w->damaged = 1;
}

void UnmapWindow(ScreenPtr s, Window id)
{
    WindowPtr w = FindWindow(s, id);

    if (!w)
        return;
    w->a.map_state = IsUnmapped;
    w->damaged = 1;
}

Bool OnscreenWindow(ScreenPtr s, WindowPtr w)
{
    if ((0 < w->a.x + (int)w->a.width && w->a.x < s->root_w) &&
        (0 < w->a.y + (int)w->a.height && w->a.y < s->root_h))
        return True;
    return False;
}

void AddWindow(ScreenPtr s, Window id, Window prev)
{
    WindowPtr new;
    WindowPtr *p;
    XRenderPictureAttributes pa;
    XRenderPictFormat *format;

    new = malloc(sizeof(WindowRec));
    if (!new)
        return;
    p = &s->windows;
    if (prev)
    {
        for (; *p; p = &(*p)->next)
            if ((*p)->id == prev)
                break;
    }
    new->id = id;
    if (!XGetWindowAttributes(s->dpy, id, &new->a))
    {
        free(new);
        return;
    }
    new->onscreen = OnscreenWindow(s, new);
    new->damaged = 0;
    if (new->a.class == InputOnly || new->id == s->id || new->id == s->f)
    {
        new->p = 0;
        new->d = 0;
    }
    else
    {
        new->d = XDamageCreate(s->dpy, id, XDamageReportNonEmpty);
        format = XRenderFindVisualFormat(s->dpy, new->a.visual);
        pa.subwindow_mode = IncludeInferiors;
        new->p = XRenderCreatePicture(s->dpy, id,
                                      format,
                                      CPSubwindowMode,
                                      &pa);
        XRenderSetPictureFilter(s->dpy, new->p, FilterBilinear, 0, 0);
    }

    new->next = *p;
    *p = new;
    if (new->a.map_state == IsViewable)
        MapWindow(s, id);

#ifdef DEBUG
    fprintf(stderr, "Added window id: %ld name: %s\n", id, GetWindowTitle(s, id));
#endif
}

void RestackWindow(ScreenPtr s, WindowPtr w, Window new_above)
{
    Window old_above;

    if (w->next)
        old_above = w->next->id;
    else
        old_above = None;
    if (old_above != new_above)
    {
        WindowPtr *prev;

        /* unhook */
        for (prev = &s->windows; *prev; prev = &(*prev)->next)
            if ((*prev) == w)
                break;
        *prev = w->next;

        /* rehook */
        for (prev = &s->windows; *prev; prev = &(*prev)->next)
        {
            if ((*prev)->id == new_above)
                break;
        }
        w->next = *prev;
        *prev = w;
    }
}

void ConfigureWindow(ScreenPtr s, XConfigureEvent *ce)
{
    WindowPtr w = FindWindow(s, ce->window);
    Window above;

    if (!w)
        return;
    w->a.x = ce->x;
    w->a.y = ce->y;
    w->a.width = ce->width;
    w->a.height = ce->height;
    w->a.border_width = ce->border_width;
    w->a.override_redirect = ce->override_redirect;
    w->onscreen = OnscreenWindow(s, w);
    RestackWindow(s, w, ce->above);
    w->damaged = 1;
}

void CirculateWindow(ScreenPtr s, XCirculateEvent *ce)
{
    WindowPtr w = FindWindow(s, ce->window);
    Window new_above;

    if (ce->place == PlaceOnTop)
        new_above = s->windows->id;
    else
        new_above = None;
    RestackWindow(s, w, new_above);
}

void DestroyWindow(ScreenPtr s, Window id, Bool gone)
{
    WindowPtr *prev, w;

    for (prev = &s->windows; (w = *prev); prev = &w->next)
        if (w->id == id)
        {
            if (!gone)
                UnmapWindow(s, id);
            *prev = w->next;
            if (w->p)
                XRenderFreePicture(s->dpy, w->p);
            free(w);
            break;
        }
}

void DamageWindow(ScreenPtr s, XDamageNotifyEvent *de)
{
    WindowPtr w = FindWindow(s, de->drawable);

    if (!w)
        return;
    w->damaged = 1;
}
