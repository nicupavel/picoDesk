# picoDesk
Send desktop or selected applications window to a USB LCD. The USB LCD must support bulk write and accept a buffer 
with the image as input. For adding more LCD types see lcdusb.c. 

It should work with any NETWM compatible window manager and implements [EWMH Specifications](https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html).
With XDamage extensions it tracks changes to the window content and updates the output. The final image of multiple windows content is composed with xcomposite/xrender
and then encoded as a buffer for usb lcd using the code in mbl_encoder/.

![picoDesk](https://github.com/nicupavel/picoDesk/blob/master/screenshot.png?raw=true)


# Configuring
For USB LCD output compile with -DUSBOUTPUT.

## LCD/Window size and colors:
Edit picodesk.h and set:

        #define WIDTH	640
        #define HEIGHT	480
        #define DEPTH   32
        #define DEPTHNAME TrueColor

## Rotate output (landscape/portrait) 
See layout.c LayoutWindows()


## Window filtering
See main.c main(), addWindow()


# Building

        sudo apt install libxcomposite-dev libxrender-dev libxdamage-dev libxrender-dev libxfixes-dev
        make

