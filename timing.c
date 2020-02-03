#include "timing.h"

int time_in_millis (void)
{
    struct timeval  tp;

    gettimeofday (&tp, 0);
    return(tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}
