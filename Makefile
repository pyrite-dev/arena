CC = cc
AR = ar
CFLAGS = -g -fcommon -fPIC -Isrc -Iinclude -Ilibwww/include -Ilibpng -I/usr/X11R7/include -I/usr/pkg/include -Wno-error=implicit-function-declaration -Wno-error=incompatible-pointer-types -Wno-error=int-conversion -Wno-error=implicit-int -DNO_TIMEZONE
LDFLAGS = -L/usr/X11R7/lib -L/usr/pkg/lib
LIBS = -lz -lm -lX11 -ljpeg

WWW = libwww.a
PNG = libpng.a

.PHONY: all clean
.SUFFIXES: .c .o

include objects.mk

all: libarena.a libarena.so arena

ALLOBJS = $(OBJS) $(PNGOBJS) $(WWWOBJS)

libarena.a: $(ALLOBJS)
	$(AR) rcs $@ $(ALLOBJS)

$(WWW): $(WWWOBJS)
	$(AR) rcs $@ $(WWWOBJS)

$(PNG): $(PNGOBJS)
	$(AR) rcs $@ $(PNGOBJS)

libarena.so: $(OBJS) $(PNG) $(WWW)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS) $(LIBS) $(PNG) $(WWW)

arena: $(OBJS) $(PNG) $(WWW)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(PNG) $(WWW)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f arena src/*.o libwww/src/*.o libpng/*.o *.a *.so
