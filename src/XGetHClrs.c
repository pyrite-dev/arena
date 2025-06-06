/*#include <X11/copyright.h>*/

/* $XConsortium: XGetHClrs.c,v 11.10 88/09/06 16:07:50 martin Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

#define NEED_REPLIES
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#include <X11/Xlibint.h>

/*
     #define __USE_SLOW_WAY__ if XAllocColors hangs in the _XReply call.
     This might happen if you are using shared memory transport.
*/ 

#ifndef __USE_SLOW_WAY__

static int counter;
static Status *st;    
static Status status;

static int dummy_handle_x_errors(Display *dpy, XErrorEvent *event)
{
    st[counter++] = 0;
    status = False;
    return status;	/* janet 1/8/95: added. correct? Xlib ref. says you don't return
			   anything, but that would be ugly C, not? */
}

Status XAllocColors(register Display *dpy,Colormap cmap, XColor *defs, int ndefs, Status *statuses)
{
    xAllocColorReply rep;
    register xAllocColorReq *req;
    int (*function)();
 

    XSync(dpy,False);
    
    function = XSetErrorHandler( dummy_handle_x_errors );
    st = statuses;

    LockDisplay(dpy);

    status = True;
    for (counter = 0; counter < ndefs; counter++)
    {
	GetReq(AllocColor, req);
	
	req->cmap = cmap;
	req->red = defs[counter].red;
	req->green = defs[counter].green;
	req->blue = defs[counter].blue;
	
	statuses[counter] = _XReply(dpy, (xReply *) &rep, 0, xTrue);
	if (statuses[counter]) {
	    defs[counter].pixel = rep.pixel;
	    defs[counter].red = rep.red;
	    defs[counter].green = rep.green;
	    defs[counter].blue = rep.blue;
	}
	else 
	    status = False;
    }

    UnlockDisplay(dpy);
    SyncHandle();

    XSetErrorHandler( function );
    
    return(status);
}

#else

Status XAllocColors(register Display *dpy,Colormap cmap, XColor *defs, int ndefs, Status *statuses)
{
    int			i;
    unsigned long	planeMask;
    unsigned long	pixelValue;
    Status		status = True;
    
    for (i = 0; i < ndefs; i++) {
 	if (XAllocColor(dpy, cmap, defs+i))
 	    statuses[i] = 1;
 	else {
 	    status = False;
 	    break;
 	}
    }
    if (status != False) {
 	XQueryColors(dpy, cmap, defs, ndefs);
    }
    return status;
}

#endif	/* !__USE_SLOW_WAY__ */
