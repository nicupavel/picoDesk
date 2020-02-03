#include "picodesk.h"

int error (Display *dpy, XErrorEvent *ev)
{
    printf ("error %d request %d minor %d\n",
            ev->error_code, ev->request_code, ev->minor_code);

    return 0;
}
