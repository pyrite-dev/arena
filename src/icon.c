/* howcome 11/10/94 */

#include <stdio.h>
#include <stdarg.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#include "HTUtils.h"	/* WWW general purpose macros */
#include "tcp.h"
#include "HTList.h"
#include "HTAccess.h"
#include "www.h"

extern int debug;
/*extern Doc *CurrentDoc;*/ 
extern char * CacheDir;
extern char * Editor;
extern char * Icon;

/* #define ICON_URL "http://info.cern.ch/hypertext/WWW/Icons/Experimental/w3o1.gif"*/
#define ICON_URL "file://w3o1.gif"

extern Display *display;
extern int screen;
extern int sbar_width;
extern unsigned int win_width, win_height;
extern int statusHeight;
extern int ToolBarHeight;
extern int depth;
extern Pixmap icon_pixmap;

Image *image = NULL;
extern Window win;
GC icon_gc;
extern GC gc_fill, toolbar_gc;
extern unsigned long windowColor;

#define HARD_ICON_LENGTH 524
#define HARD_ICON_64 "R0lGODlhMgAjAIQAAP///92amvYiF+AGEHkDCIf3opuRimZmr3pxdlli0jUqrhi/cwljBoiRwzUwkTQzacKleICR5Dkl04WFhQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAEAAAAALAAAAAAyACMAAAX+ICCOZGmeaKqubOu+ZiDPcK0GQp4PBGH/I5ygR9ANgT+cTyQ8ImHCJaApfbp6JKoVKNNVt6xZ0wlujXPfMotYRKt/bfL7pX0V7viCKa8v8bUGBwcICQqGJwULiooMe4uNQQCJjEIDBggNBpqFDoYKjo9+i4xMQ5MLDDoICJqaDZ6woqOQIqekUZMMQqutmgewhp0ktqjDo8VNuTkPDggQzwYNEQkSwIe1x7TEpEWnqQTMzs8NEgnmChLpnsah2MfFAATePswKB60N0gmZrq+f7u22wfMm4kE9QwgEJYjArxcCByOINRJIiiAAgw46eUpXrpemh+wqMno3kVRBZhqKDXHc5xHBP4DFckmaNRPeRZSwViZoCTHiu1AUbWLMaM2QIAMQDCjoCZNmTZIkhqa0du/ZUlnZfEIdYRDlVGDODmTE+kjbT1oncRZVEJapVqdNbXKVuhbTWFDt3pos0TXj16W/3Ibc2xRtVKl+/YJEYcvwU7kmuvadvIKBZct7LmNeMUGywQlz3oQAADs=     "


/* Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore) */

#if defined __QNX__ || (defined(sco) && !defined(sco_os5))
#define index strchr
#else
extern char *index();
#endif

static char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/* janet: not used
static char index_64[128] = {
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
    52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
    -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
    -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};
*/
/* #define char64(char c)  (((c) < 0 || (c) > 127) ? -1 : index_64[(c)]) */

int char64(char c)
{
    char *s = (char *) index(basis_64, c);
    if (s) return(s - basis_64);
    return(-1); 
}



void SetIconWin(Window aWin)
{
    win = aWin;
}

void SetIconGC(GC aGC)
{
    icon_gc = aGC;
}

void LoadIcon ()
{

    int i,ii;
    Block block;
    unsigned int width, height;
    GC drawGC;
    Pixmap pixmap;
    XImage *ximage;
    char *data;
    HTAnchor *a = NULL;
    Doc *d = NULL;

    if (Icon) {
	block.next = 0;
/*	fprintf(stderr,"LoadIcon sorry, cusomized icons temprarily disabled..\n");*/
	a = libGetDocument(Icon, strlen(Icon), NULL, FALSE, TRUE, FALSE, FALSE, TRUE);
    }

    if (a && a->parent && a->parent->document) {
	d = (Doc *)a->parent->document;
    }

    if (d && d->content_buffer) {
	block.buffer = d->content_buffer;
	block.size = d->loaded_length;
	block.next = 0;
    }
    else {
	int c1, c2, c3, c4;
	char *b;
	char *buf;
	
	b = HARD_ICON_64;
	buf = (char *)malloc(HARD_ICON_LENGTH + 10);
	
	for (i=0, ii=0; i<176; i++) {	/* janet 28/07/95: something's wrong here, */
	    c1 = char64(*b++);		/*        probably 177 should be 176(?), because then it loops 177 times */
	    c2 = char64(*b++);		/*        either that or *b is too short */
	    c3 = char64(*b++);
	    c4 = char64(*b++);

	    buf[ii++] = ((c1<<2) | ((c2&0x30)>>4));
	    buf[ii++] = (((c2&0XF) << 4) | ((c3&0x3C) >> 2));
	    buf[ii++] = (((c3&0x03) <<6) | c4);
	}
	
	block.buffer = buf;
	block.next = 0;
	block.size = 524; /* size of gif file */
    }
    
    if (!block.buffer){
	image = NULL;
	return;
    }
    
    image = (Image *)malloc(sizeof(Image));
    image->url = NULL;
    image->npixels = 0;
    
    data = (char *)LoadGifImage(image, &block, depth);
    Free(block.buffer);
    
    width = image->width;
    height = image->height;
    
    
    if ((ximage = XCreateImage(display, DefaultVisual(display, screen),
			       depth, ZPixmap, 0, data,
			       width, height, (depth == 24 ? 32 : (depth == 16) ? 16 : 8), 0)) == 0)
	{
	    fprintf(stderr,"LoadIcon: Failed to create image\n");
	    Free(data);	
	    Free(image);
	    image = NULL;
	}

    /* howcome 22/2/95: do we need to set these?? */

    ximage->byte_order = MSBFirst;
    ximage->bitmap_bit_order = BitmapBitOrder(display);

    if ((pixmap = XCreatePixmap(display, win, width, height, depth)) == 0)
	{
	    fprintf(stderr,"LoadIcon: Failed to create pixmap\n");
	    Free(data);
	    Free(image);
	    image = NULL;
	    XDestroyImage(ximage);
	}
    
    drawGC = XCreateGC(display, pixmap, 0, 0);
    XSetFunction(display, drawGC, GXcopy);
    XPutImage(display, pixmap, drawGC, ximage, 0, 0, 0, 0, width, height);
    XFreeGC(display, drawGC);
    XDestroyImage(ximage);
    
    image->pixmap = pixmap;
    image->width = width;
    image->height = height;
    image->next = NULL;
    
    icon_pixmap = pixmap;
}


void DisplayIcon ()
{
    XRectangle rect;
    int x, y;
    unsigned int w, h;
    
    if (!image)
        LoadIcon();

    rect.x = x = win_width - ICON_WIDTH;
    rect.y = y = 0; 

    rect.width = w = ICON_WIDTH;
    rect.height = h = ICON_HEIGHT;

/*    XSetClipRectangles(display, icon_gc, 0, 0, &rect, 1, Unsorted);*/
/*    XSetClipRectangles(display, gc_fill, 0, 0, &rect, 1, Unsorted);*/

    XSetForeground(display, toolbar_gc, windowColor);
    XSetClipRectangles(display, toolbar_gc, 0, 0, &rect, 1, Unsorted);

    XFillRectangle(display, win, toolbar_gc,
                   x + 2, y + 2, ICON_WIDTH - 4, ICON_HEIGHT - 4);

    if (image)
    {
        XCopyArea(display, image->pixmap, win, icon_gc,
                  0, 0, image->width, image->height,
                  x + (ICON_WIDTH - image->width) / 2,
                  y + (ICON_HEIGHT - image->height) / 2); 

    }

    DrawOutSet(win, icon_gc, x, y, w, h);
}
