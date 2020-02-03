PACKAGES=xcomposite xfixes xdamage xrender libusb
LIBS=`pkg-config --libs ${PACKAGES}` -lm
INCS=`pkg-config --cflags ${PACKAGES}` -I./mbl_encoder/
#CFLAGS=-DDEBUG -DUSBOUTPUT -DDEBUGPAINT
.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $*.c  
	$(CC) -std=c99 $(CFLAGS) $(INCS) -c ./mbl_encoder/mbl_encoder.c -c ./mbl_encoder/mbl_encoder_32_16.c -c ./mbl_encoder/mbl_screen_32.c -c ./mbl_encoder/mbl_utils.c
	
OBJS=main.o error.o timing.o visual.o window.o layout.o lcdusb.o 
#OBJS_ENCODER=mbl_encoder.o mbl_screen_32.o mbl_utils.o mbl_encoder_32_16.o
OBJS_ENCODER=mbl_utils.o mbl_encoder_32_16.o mbl_screen_32.o mbl_encoder.o 
picodesk: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(OBJS_ENCODER) $(LIBS)

$(OBJS) $(OBJS_ENCODER): picodesk.h

clean:
	rm -f $(OBJS) $(OBJS_ENCODER) picodesk