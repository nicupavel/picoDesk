#ifndef _WINDOW_H
#define _WINDOW_H_


#define MAX_PROPERTY_VALUE_LEN 4096
#define WMLIST 1
#define XLIST  2
#define WINDOW_LIST_METHOD  WMLIST

void GetWindowsList(ScreenPtr s, Window *root_return, Window *parent_return, Window **children, unsigned int *nchildren); 
long *GetWindowDesktop (ScreenPtr s, Window id);
char *GetWindowTitle (ScreenPtr s, Window id);
#endif