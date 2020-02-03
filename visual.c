#include "picodesk.h"

Visual *FindARGBVisual(Display *dpy, int scr)
{
    XVisualInfo *xvi;
    XVisualInfo template;
    int nvi;
    int i;
    XRenderPictFormat *format;
    Visual *visual;

    template.screen = scr;
    template.depth = DEPTH;
    template.class = DEPTHNAME;
    xvi = XGetVisualInfo(dpy,
                         VisualScreenMask |
                             VisualDepthMask |
                             VisualClassMask,
                         &template,
                         &nvi);
    if (!xvi)
        return 0;
    visual = 0;
    for (i = 0; i < nvi; i++)
    {
        format = XRenderFindVisualFormat(dpy, xvi[i].visual);

#if DEBUG
        fprintf(stderr, "Query Visual Format:  Type %d AlphaMask %d\n", format->type,
                format->direct.alphaMask);
#endif
        if (format->type == PictTypeDirect && format->direct.alphaMask)
        {
            visual = xvi[i].visual;
            break;
        }
    }

    XFree(xvi);
    return visual;
}
