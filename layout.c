/*
 * Copyright © 2008 Nicu Pavel <npavel@linuxconsulting.ro>
 * Based on Keith Packard examples
 */

#include "picodesk.h"

WindowPtr *
getActive(ScreenPtr s, int *np)
{
    WindowPtr w, p;
    WindowPtr *ws;
    int n;

    n = 0;
    for (w = s->windows; w; w = w->next)
    {
        if (WindowShow(w))
            n++;
        p = w;
    }

    ws = (WindowPtr *)malloc(n * sizeof(WindowPtr));
    *np = n;
    n = 0;
    for (w = s->windows; w; w = w->next)
        if (WindowShow(w))
            ws[n++] = w;
    return ws;
}

double
area(WindowPtr w)
{
    return (double)w->a.width * (double)w->a.height;
}

void findWeightedArea(WindowPtr w)
{
    doublePoint p;
    int x = w->a.x + w->a.width / 2;
    int y = w->a.y + w->a.height / 2;

    w->area = area(w);
    w->pt.x = x * w->area;
    w->pt.y = y * w->area;
#ifdef DEBUG
    printf("Window 0x%lux area %g weight %g, %g\n",
           w->id, w->area, w->pt.x, w->pt.y);
#endif
}

static doublePoint centroid;

double
windowDist(const WindowPtr w)
{
    int x, y;
    double dx, dy;

    x = w->a.x + w->a.width / 2;
    y = w->a.y + w->a.height / 2;
    dx = (x - centroid.x);
    dy = (y - centroid.y);
    return (dx * dx) + (dy * dy);
}

int compareWindows(const void *a, const void *b)
{
    const WindowPtr aw = *((WindowPtr *)a);
    const WindowPtr bw = *((WindowPtr *)b);
    double ad = windowDist(aw);
    double bd = windowDist(bw);

    if (ad < bd)
        return -1;
    if (ad > bd)
        return 1;
    return 0;
}

WindowPtr *
sortWindows(ScreenPtr s, int *np)
{
    WindowPtr *ws;
    int n;
    WindowPtr w;
    double total;
    doublePoint totalWeight;
    double min;
    double dist2;
    WindowPtr minWindow;
    int i;

    ws = getActive(s, &n);
    total = 0;
    totalWeight.x = 0;
    totalWeight.y = 0;
    for (i = 0; i < n; i++)
    {
        w = ws[i];
        findWeightedArea(w);
        total += w->area;
        totalWeight.x += w->pt.x;
        totalWeight.y += w->pt.y;
    }
    centroid.x = totalWeight.x / total;
    centroid.y = totalWeight.y / total;

    qsort(ws, n, sizeof(WindowPtr), compareWindows);
    *np = n;
    return ws;
}

static int iabs(int i)
{
    if (i < 0)
        i = -i;
    return i;
}

static int
windowsOverlap(WindowPtr a, WindowPtr b)
{
    if (((a->x < b->x + b->w) &&
         (b->x < a->x + a->w)) &&
        ((a->y < b->y + b->h) &&
         (b->y < a->y + a->h)))
        return 1;
    return 0;
}

void LayoutWindows(ScreenPtr s)
{
    WindowPtr *ws;
    int n;
    int i;
    WindowPtr w;
    int x = 0;
    int wid, hei;
    XTransform t;
    int p;
    int dx;
    int dy;
    int cx0, cy0;
    int cx, cy;
    int maxx, maxy;
    int minx, miny;
    double scale_x, scale_y;
    double scale;

    ws = sortWindows(s, &n);

    cx0 = ws[0]->a.x + ws[0]->a.width / 2;
    cy0 = ws[0]->a.y + ws[0]->a.height / 2;
    /*
     * place windows, one at a time
     */
    for (i = 0; i < n; i++)
    {
        w = ws[i];
        cx = w->a.x + w->a.width / 2;
        cy = w->a.y + w->a.height / 2;
        dx = cx - cx0;
        dy = cy - cy0;
#ifdef DEBUG
        printf("0x%08lx: pos %4d, %4d, center %4d, %4d direction %4d, %4d\n", w->id,
               w->a.x, w->a.y, cx, cy, dx, dy);
#endif
        w->x = (cx - cx0) - w->a.width / 2;
        w->y = (cy - cy0) - w->a.height / 2;
        w->w = w->a.width;
        w->h = w->a.height;

        for (p = 0; p < i;)
        {
            if (windowsOverlap(w, ws[p]))
            {
                int newx = w->x, newy = w->y;
                int distx, disty;

#ifdef DEBUG
                printf("0x%08lx overlaps 0x%08lx\n", w->id, ws[p]->id);
#endif
                if (dx < 0)
                    newx = ws[p]->x - w->w;
                else
                    newx = ws[p]->x + ws[p]->w;
                if (dy < 0)
                    newy = ws[p]->y - w->h;
                else
                    newy = ws[p]->y + ws[p]->h;
                distx = iabs(newx - w->x);
                disty = iabs(newy - w->y);
                if (distx <= disty)
                    w->x = newx;
                else
                    w->y = newy;
#ifdef DEBUG
                printf("0x%08lx moved to %d, %d\n", w->id, w->x, w->y);
#endif
                p = 0;
            }
            else
                p++;
        }
#ifdef DEBUG
        printf("0x%08lx placed at %d, %d\n", w->id, w->x, w->y);
#endif
    }

    /* compute size of layout */

    maxx = -32767;
    maxy = -32767;
    minx = 32767;
    miny = 32767;

    /*
    maxx = 320;
    maxy = 240;
    minx = 0;
    miny = 0;
    */
    for (i = 0; i < n; i++)
    {
        w = ws[i];
        if (w->x + w->w > maxx)
            maxx = w->x + w->w;
        if (w->y + w->h > maxy)
            maxy = w->y + w->h;
        if (w->x < minx)
            minx = w->x;
        if (w->y < miny)
            miny = w->y;
    }
#ifdef DEBUG
    printf("Layout bounds %d, %d -> %d, %d\n",
           minx, miny, maxx, maxy);
#endif

    wid = maxx - minx;
    hei = maxy - miny;
    if (wid < 1)
        wid = 1;
    if (hei < 1)
        hei = 1;
    /* compute scale */
    scale_x = (double)(s->w - SHADOW_WIDTH * 2) / wid;
    scale_y = (double)(s->h - SHADOW_WIDTH * 2) / hei;
    scale = scale_x;
    if (scale_y < scale_x)
        scale = scale_y;

    /* fix up transforms for new scale */
    /* Could use different filters per Andrei suggestions */
    /* XRenderSetPictureFilter( dpy, picture, FilterBilinear, 0, 0 ); */

    t.matrix[0][0] = XDoubleToFixed(1.0 / scale);
    t.matrix[0][1] = 0.0;
    t.matrix[0][2] = 0.0;

    t.matrix[1][0] = 0.0;
    t.matrix[1][1] = XDoubleToFixed(1.0 / scale);
    t.matrix[1][2] = 0.0;

    t.matrix[2][0] = 0.0;
    t.matrix[2][1] = 0.0;
    t.matrix[2][2] = XDoubleToFixed(1.0);

    /* could also rotate the windows if LCD is landscape/portrait */
    /*
    double angle = M_PI / 180 * 90; // 90 degrees
    double sina = std::sin( angle );
    double cosa = std::cos( angle );

   
    XTransform xform = {{
            { XDoubleToFixed(  cosa ), XDoubleToFixed( sina ), XDoubleToFixed( 0 ) },
            { XDoubleToFixed( -sina ), XDoubleToFixed( cosa ), XDoubleToFixed( 0 ) },
            { XDoubleToFixed(     0 ), XDoubleToFixed(    0 ), XDoubleToFixed( 1 ) }
    }};
    
    */

    s->x = (s->w - (wid * scale)) / 2;
    s->y = (s->h - (hei * scale)) / 2;

    for (i = 0; i < n; i++)
    {
        w = ws[i];
        XRenderSetPictureTransform(s->dpy, w->p, &t);
        w->x = s->x + (w->x - minx) * scale;
        w->y = s->y + (w->y - miny) * scale;
        w->w = w->w * scale;
        w->h = w->h * scale;
        w->damaged = 1;
    }
    s->sw = SHADOW_WIDTH * scale;
    free(ws);
}
