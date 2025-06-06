#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include "www.h"

extern int statusHeight;
extern int sbar_width;
extern int ToolBarHeight;
extern long PixelOffset;
extern long buf_height;
extern int PixelIndent;
extern int buf_width;
extern char *buffer;
extern int hdrlen;
extern int Authorize;
extern int OpenURL;
extern int IsIndex;
extern int SaveFile;
extern int FindStr;
extern char *FindStrVal, *FindNextStr;
extern int font;
extern Field *focus;
extern Frame background;
extern Byte *paint;
extern Bool TextFieldFocus;
extern double lens_factor;
extern GC disp_gc, gc_fill;
extern Doc *CurrentDoc;
extern char *bufptr;
extern Display *display;

void ShowLink(x, y)
    int x,y;
{
    int element,tag, end;
    char *c, *anchor_start, *anchor_end;
    int dx,dy;
    char *href, *name, *class,*ref;
    int hreflen, namelen;
    int class_len;
    XEvent e;
   
    ParseAnchorAttrs(&href, &hreflen, &name, &namelen, &class, &class_len);
    anchor_start = anchor_end = 0;
    tag = 0;
    
    if (!DocHTML(CurrentDoc))
	return;

    if (CurrentDoc->show_raw)
	return;

    element = WhichObject(BUTTONDOWN, x, y, &tag, &anchor_start, &anchor_end, &dx, &dy);
    
    if (tag == TAG_ANCHOR)
    {
	bufptr = anchor_start;
	ParseAnchorAttrs(&href, &hreflen, &name, &namelen, &class, &class_len);
        ref = HTAnchor_address(libExpandHref(href, hreflen));
	if(ref)
	{
	    c = (char *)calloc(strlen(ref)+7, sizeof(char));
	    strcpy(c,"Link: ");
	    strcpy(c+6, ref);
	    Announce(c);
	    Free(c);
	    end = 0;
	    while (!end)
	    {
		XNextEvent(display, &e);
		switch(e.type)
		{
		case Expose:
		    while (XCheckTypedEvent(display, Expose, &e));
		    break;
		case ButtonRelease:
		    end=TRUE;
		    Announce(CurrentDoc->url);
		    break;
		default:
		    break;
		};
	    };
	    Free(ref);
	}
    }
}
