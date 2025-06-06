CC = cc
AR = ar
CFLAGS = -fcommon -fPIC -Isrc -Iinclude -Ilibwww/include -Ilibpng -I/usr/X11R7/include -I/usr/pkg/include
LDFLAGS = -L/usr/X11R7/lib -L/usr/pkg/lib
LIBS = -lz -lm -lX11 -ljpeg

.PHONY: all clean
.SUFFIXES: .c .o

include objects.mk

all: libarena.a libarena.so arena

ALLOBJS = $(OBJS) $(PNGOBJS) $(WWWOBJS)

libarena.a: $(ALLOBJS)
	$(AR) rcs $@ $(ALLOBJS)

libarena.so: $(ALLOBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(ALLOBJS) $(LIBS)

arena: $(ALLOBJS)
	$(CC) $(LDFLAGS) -o $@ $(ALLOBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f libarena.a libarena.so arena src/*.o libwww/src/*.o libpng/*.o
