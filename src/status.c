/* a status bar at bottom on window */

#include <stdarg.h>
#include <stdio.h>  /* STUPID ALPHA/OSF - GCC !!!!! */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#include "HTUtils.h"	/* WWW general purpose macros */
#include "tcp.h"
#include "HTList.h"
#include "HTAccess.h"

#include "www.h"

#define LEFTMARGIN    50
#define RIGHTMARGIN   7
#define status_width    (win_width - ICON_WIDTH)

extern int debug;
extern int initialised;   /* if false it is unsafe to do X output */
extern int busy;
extern Display *display;
extern int screen;
extern int sbar_width;
extern XFontStruct *pFontInfo;
extern unsigned long textColor, labelColor, windowColor, strikeColor,
                     windowTopShadow, windowBottomShadow, statusColor;

#ifdef SELECTION
#include <X11/Xatom.h>
extern unsigned long statusSelectColor; /* howcome 1/9/94 */
#endif

extern unsigned int win_width, win_height;
extern char *user;
char *gatewayUser; /* howcome 20/9/94: removed extern since tcp.c no longer is loaded */
extern int ToolBarHeight;
extern Doc *CurrentDoc; /*, NewDoc; */

#define STATSIZ 256
#define ActiveTextColor labelColor

extern Window win;
GC status_gc;
int statusHeight;
char status[STATSIZ];
char authreq[STATSIZ];
int AbortFlag;
int AbortButton = 1;
int AbortButtonChanged = 0;
int ABup = 1;
int OpenURL;
int FindStr;
int IsIndex;
int SaveFile;
int Authorize;
int statusOffset;
int cursor; /* position of cursor */

#ifdef SELECTION
int startSelect, stopSelect, beginChr, endChr, textWidth=1;
Bool selecting = False;
#endif

/* save strings for specific funcions */

char *OpenString;
char *SaveAsString;
char *FindStrVal;
char *SearchStrVal;
char *FindNextStr;

static int charheight;
static XFontStruct *pStatusFontInfo;
static Button abButton;

void SetStatusWin(Window aWin)
{
    win = aWin;
}

void SetStatusGC(GC aGC)
{
    status_gc = aGC;
}

void SetStatusFont(XFontStruct *pf)
{
    pStatusFontInfo = pf;
    charheight = pf->max_bounds.ascent + pf->max_bounds.descent;
    statusHeight = (Authorize ? 16 + (charheight<<1) : 14 + charheight);
    abButton.label = "Abort";
}

char *LabelString(int *len)
{
    if (OpenURL)
    {
        *len = 5;
        return "Open:";
    }

    if (SaveFile)
    {
        *len = 7;
        return "SaveAs:";
    }

    if (FindStr)
    {
        *len = 5;
        return "Find:";
    }

    if (IsIndex)
    {
        *len = 6;
        return "Match:";
    }

    return NULL;
}

int StatusActive(void)
{
    int c;

    return (Authorize || LabelString(&c));
}


void RestoreStatusString(void)
{
    if (OpenURL)
    {
        if (OpenString)
            strcpy(status, OpenString);
        else
            *status = '\0';
    }
    else if (SaveFile)
    {
        if (SaveAsString)
            strcpy(status, SaveAsString);
        else
            *status = '\0';
    }
    else if (FindStr)
    {
        if (FindStrVal)
            strcpy(status, FindStrVal);
        else
            *status = '\0';
    }
    else if (IsIndex)
    {
        if (SearchStrVal)
            strcpy(status, SearchStrVal);
        else
            *status = '\0';
    }

#ifdef SELCTION
    beginChr = 0;
    endChr = 0;
#endif

    cursor = strlen(status);
}

void SaveStatusString(void)
{
    if (OpenURL)
    {
        if (OpenString)
            Free(OpenString);

        OpenString = strdup(status);
    }
    else if (SaveFile)
    {
        if (SaveAsString)
            Free(SaveAsString);

        SaveAsString = strdup(status);
    }
    else if (FindStr)
    {
        if (FindStrVal)
            Free(FindStrVal);

        FindStrVal = strdup(status);
    }
    else if (IsIndex)
    {
        if (SearchStrVal)
            Free(SearchStrVal);

        SearchStrVal = strdup(status);
    }
}
/* functions DrawOutSet amd DrawInSet changed by Janet for more general use (added Window argument)*/
void DrawOutSet(Window aWin,GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, windowTopShadow);

    XFillRectangle(display, aWin, gc, x, y, w, 1);
    XFillRectangle(display, aWin, gc, x, y+1, w-1, 1);
    XFillRectangle(display, aWin, gc, x, y, 1, h);
    XFillRectangle(display, aWin, gc, x+1, y+1, 1, h-1);

    XSetForeground(display, gc, windowBottomShadow);

    XFillRectangle(display, aWin, gc, x, y+h-1, w, 1);
    XFillRectangle(display, aWin, gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, aWin, gc, x+w-1, y, 1, h);
    XFillRectangle(display, aWin, gc, x+w-2, y+1, 1, h-1);
}


void DrawInSet(Window aWin, GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, windowBottomShadow);

    XFillRectangle(display, aWin, gc, x, y, w, 1);
    XFillRectangle(display, aWin, gc, x, y+1, w-1, 1);
    XFillRectangle(display, aWin, gc, x, y, 1, h);
    XFillRectangle(display, aWin, gc, x+1, y+1, 1, h-1);

    XSetForeground(display, gc, windowTopShadow);

    XFillRectangle(display, aWin, gc, x, y+h-1, w, 1);
    XFillRectangle(display, aWin, gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, aWin, gc, x+w-1, y, 1, h);
    XFillRectangle(display, aWin, gc, x+w-2, y+1, 1, h-1);
}

void DrawOutSetCircle(GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, windowTopShadow);
    XDrawArc(display, win, gc, x, y, w, h, 45<<6, 180<<6);
    XDrawArc(display, win, gc, x+1, y+1, w-2, h-2, 45<<6, 180<<6);

    XSetForeground(display, gc, windowBottomShadow);
    XDrawArc(display, win, gc, x, y, w, h, 225<<6, 180<<6);
    XDrawArc(display, win, gc, x+1, y+1, w-2, h-2, 225<<6, 180<<6);
}

void DrawInSetCircle(GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, windowBottomShadow);
    XDrawArc(display, win, gc, x, y, w, h, 45<<6, 180<<6);
    XDrawArc(display, win, gc, x+1, y+1, w-2, h-2, 45<<6, 180<<6);

    XSetForeground(display, gc, windowTopShadow);
    XDrawArc(display, win, gc, x, y, w, h, 225<<6, 180<<6);
    XDrawArc(display, win, gc, x+1, y+1, w-2, h-2, 225<<6, 180<<6);
}

void DisplayStatusBar()
{
    int x, y, r, n, active;
    unsigned int w, h;
    char *p, *s=NULL;
    XRectangle rect;

    active = ((Authorize || (s = LabelString(&n))) ? 1 : 0);
    statusHeight = (Authorize ? 16 + (charheight<<1) : 14 + charheight);

    rect.x = x = 0;
    rect.y = y = StatusTop; 

    rect.width = w = status_width;
    rect.height = h = statusHeight;

    XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);

    XSetForeground(display, status_gc, windowColor);
    XFillRectangle(display, win, status_gc, x, y, w, h);

    DrawOutSet(win, status_gc, x, y, w, h);

    if (Authorize)
    {
        XSetForeground(display, status_gc, labelColor);

        rect.width -= 2;
        XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);
        XDrawString(display, win, status_gc,
                18, y + 3 + pStatusFontInfo->max_bounds.ascent,
                authreq, strlen(authreq));
    }

    /* howcome 24/2/95: by using a fixed LEFTMARGIN, we're unable to
       scale the abort button horisontally */


    x += LEFTMARGIN;
    y = StatusTop + StatusFontOffset - 2;
    h = 6 + charheight;
    w -= LEFTMARGIN + RIGHTMARGIN;

    XSetForeground(display, status_gc, (active ? statusColor : windowColor));
    XFillRectangle(display, win, status_gc, x, y, w, h);

    DrawInSet(win, status_gc, x, y, w, h);

    /* needs a call to set clipping rectangle */
    /* would currently interact with scrollbar gc */

    rect.x = x + 2;
    rect.y = y;
    rect.width = w - 4;
    rect.height = h;

    r = XTextWidth(pStatusFontInfo, "Abort", 5) + 8;

    abButton.x = 6;
    abButton.y = y + 2;
    abButton.w = r;
    abButton.h = h-3;

    if (AbortButton)
    {
        if (ABup)
            DrawOutSet(win, status_gc, 6, y+2, r, h-3);
        else
            DrawInSet(win, status_gc, 6, y+2, r, h-3);

        XSetForeground(display, status_gc, labelColor);

        XDrawString(display, win, status_gc,
                10, y + 3 + pStatusFontInfo->max_bounds.ascent,
                "Abort", 5);
    }
    else if (s)
    {
        XSetForeground(display, status_gc, labelColor);

        XDrawString(display, win, status_gc,
            8, y + 2 + pStatusFontInfo->max_bounds.ascent, s, n);
    }
    else
        XSetForeground(display, status_gc, labelColor);

    if (Authorize && (p = strchr(status, ':')))
        n = p - status + 1;
    else
        n = strlen(status);

    XSetForeground(display, status_gc, (active ? ActiveTextColor : labelColor));
    XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);
    XDrawString(display, win, status_gc,
        x + 4 - statusOffset, y + 2 + pStatusFontInfo->max_bounds.ascent,
        status, n);

    r = XTextWidth(pStatusFontInfo, status, cursor);

    if (s || Authorize)   /* draw cursor */
    {
        XSetForeground(display, status_gc, strikeColor);
        XFillRectangle(display, win, status_gc, x+3+r - statusOffset, y+2, 1, h-4);
    }

#ifdef SELECTION

    /* howcome 2/8/94: added code to draw highlighted strings. 
       b (=beginChr) and e (=endChr) contains the indexes to start 
       and stop of highlighted chars in status string 
       */


    if (beginChr > 0) {
      XSetForeground(display, status_gc, labelColor);
      XDrawString(display, win, status_gc,
		  (x + 4 - statusOffset), y + 2 + pStatusFontInfo->max_bounds.ascent,
		  status,
		  min(n,beginChr));
    }

    if (beginChr < n) {
      XSetForeground(display, status_gc, strikeColor);
      XDrawString(display, win, status_gc,
		  (x + 4 - statusOffset) + (beginChr * textWidth), y + 2 + pStatusFontInfo->max_bounds.ascent,
		  (status + min(n,beginChr)),
		  (min(n,endChr) - min(n,beginChr)));
    }

    if (endChr < n) {
      XSetForeground(display, status_gc, labelColor);
      XDrawString(display, win, status_gc,
		  (x + 4 - statusOffset) + (endChr * textWidth), y + 2 + pStatusFontInfo->max_bounds.ascent,
		  (status + min(n,endChr)),
		  (n - min(n,endChr)));
    }

    r = XTextWidth(pStatusFontInfo, status, n);

    if (s || Authorize)   /* draw cursor */
    {
        XSetForeground(display, status_gc, strikeColor);
        XFillRectangle(display, win, status_gc, x+3+r - statusOffset, y+2, 1, h-4);
    }

#endif /* SELECTION */

}


#ifdef SELECTION
/* howcome added SelectStatus 1/8/94 */

void SelectStatus(int x_in, int y_in)
{
    int x, y; /* r, n, ch, active;*/ /* janet 21/07/95: not used */
    /*    unsigned int w, h; */	/* janet 21/07/95: not used */
    /*    char *p, *s; */	/* janet 21/07/95: not used */
    /*    XRectangle rect; */ 	/* janet 21/07/95: not used */ 

    if (selecting) {
      x = 50;
      y = win_height - 10 - charheight;
      stopSelect = (x_in - x) / textWidth;
      stopSelect = max(0,stopSelect);
      
      beginChr = min(startSelect,stopSelect);
      endChr = max(startSelect,stopSelect);
      
      DisplayStatusBar();
    }
}

#endif /* SELECTION */

void SetStatusString(char *s)
{
    char *p;
    int x, y, r, n, ch;
    unsigned int w, h;
    XRectangle rect;

    if (initialised)
    {
        if (s)
        {
            strncpy(status, s, STATSIZ-1);
            status[STATSIZ-1] = '\0';

            /* trim trailing \r\n */

            n = strlen(status) - 1;

            if (n > 0 && status[n] == '\n')
            {
                status[n--] = '\0';

                if (status[n] == '\r')
                     status[n] = '\0';
            }
        }

        ch = pStatusFontInfo->max_bounds.ascent + pStatusFontInfo->max_bounds.descent;
        statusHeight = (Authorize ? 16 + (charheight<<1) : 14 + ch);

        rect.x = x = 0;
        rect.y = StatusTop; 

        rect.width = w = status_width;
        rect.height = statusHeight;

        XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);
        XSetForeground(display, status_gc, windowColor);

        h = 14 + charheight;
        y = rect.y + rect.height - h;

        s = LabelString(&r);

        if (AbortButton)
        {
            if (AbortButtonChanged)
            {
                XFillRectangle(display, win, status_gc, 6, y+2, (LEFTMARGIN - 8), h-4);

                if (ABup)
                    DrawOutSet(win, status_gc, abButton.x, abButton.y, abButton.w, abButton.h);
                else
                    DrawInSet(win, status_gc, abButton.x, abButton.y, abButton.w, abButton.h);

                XSetForeground(display, status_gc, labelColor);

                XDrawString(display, win, status_gc,
                        abButton.x + 4, abButton.y + 1 + pStatusFontInfo->max_bounds.ascent,
                        "Abort", 5);

                AbortButtonChanged = 0;  /* avoid flickering buffer */
            }
        }
        else if (s)
        {
            XFillRectangle(display, win, status_gc, 6, y+2, (LEFTMARGIN - 8), h-4);
            XSetForeground(display, status_gc, labelColor);

            XDrawString(display, win, status_gc,
                8, y + 6 + pStatusFontInfo->max_bounds.ascent, s, r);
        }
        else
            XFillRectangle(display, win, status_gc, 6, y+2, (LEFTMARGIN - 8), h-4);

        rect.x = x = LEFTMARGIN + 2;
        rect.y = y = StatusTop + StatusFontOffset; 

        rect.width = w = status_width - LEFTMARGIN - RIGHTMARGIN - 4;
        rect.height = h = 2 + charheight;

        XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);

        XSetForeground(display, status_gc, ((s||Authorize) ? statusColor : windowColor) );
        XFillRectangle(display, win, status_gc, x, y, w, h);

        cursor = n = strlen(status);

        if (Authorize && (p = strchr(status, ':')))
            n = p - status + 1;

        XSetForeground(display, status_gc, ((s || Authorize) ? ActiveTextColor : labelColor));
        XDrawString(display, win, status_gc,
            x + 2 - statusOffset, y + pStatusFontInfo->max_bounds.ascent,
            status, n);

        r = XTextWidth(pStatusFontInfo, status, cursor);

        if (s||Authorize)   /* draw cursor */
        {
            XSetForeground(display, status_gc, strikeColor);
            XFillRectangle(display, win, status_gc, x+1+r - statusOffset, y, 1, h);
        }
    }
    else if (s)
    {
        strncpy(status, s, STATSIZ-1);
        status[STATSIZ-1] = '\0';

        /* trim trailing \r\n */

        n = strlen(status) - 1;

        if (n > 0 && status[n] == '\n')
        {
            status[n--] = '\0';

            if (status[n] == '\r')
                status[n] = '\0';
        }

        cursor = strlen(status);

#ifdef SELECTION
	ClearStatusSelection();
#endif

    }
    XFlush(display); /* howcome 5/10/94: this seems to be required */
}


void RepairStatus(int x1, int moved)
{
    char *p;
    int x, y, r, n, active;
    unsigned int w, h;
    XRectangle rect;

    rect.x = x = LEFTMARGIN + 2;
    rect.y = y = StatusTop + StatusFontOffset; 

    rect.width = w = status_width - LEFTMARGIN - RIGHTMARGIN - 4;
    rect.height = h = 2 + charheight;

    if (!moved)
    {
        if (rect.x < x1)
        {
            rect.width -= x1 - x;
            rect.x = x1;
        }

/*
        r = statusHeight;

        if (r < rect.width)
            rect.width = r;
*/    }

    active = ((Authorize || LabelString(&n)) ? 1 : 0);
    XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);

    XSetForeground(display, status_gc, (active ? statusColor : windowColor));
    XFillRectangle(display, win, status_gc, x, y, w, h);

    if (Authorize && (p = strchr(status, ':')))
        n = p - status + 1;
    else
        n = strlen(status);

    XSetForeground(display, status_gc, (active ? ActiveTextColor : labelColor));
    XDrawString(display, win, status_gc,
        x + 2 - statusOffset, y + pStatusFontInfo->max_bounds.ascent,
        status, n);

    r = XTextWidth(pStatusFontInfo, status, cursor);

    if (Authorize || LabelString(&n))   /* draw cursor */
    {
        XSetForeground(display, status_gc, strikeColor);
        XFillRectangle(display, win, status_gc, x+1+r - statusOffset, y, 1, h);
    }
}

#ifdef SELECTION
void ClearStatusSelection() /* howcome 2/8/94 */
{
    beginChr = 0;
    endChr = 0;
}
#endif

void ClearStatus()
{
    cursor = 0;
    status[0] = '\0';
    statusOffset = 0;
#ifdef SELECTION
    beginChr = 0;
    endChr = 0;
#endif
}

int IsEditChar(char c)
{
    if (c == '\b' || c == '\n' || c == '\r' || c == 21) /* 21 == ^U */
        return 1;

    if (c >= ' ')
        return 1;

    return 0;
}

void EditChar(char c)
{
    char *who;
    int i, n, x1, x2, moved;

    /* x1 is position of cursor, x2 is right edge of clipped text */

    n = strlen(status);

    if (busy && !Authorize)
        Beep();
    else if (c == '\b')
    {
        if (cursor > 0)
        {
            --cursor;
            strcpy(status+cursor, status+cursor+1);
            x1 = LEFTMARGIN + 4 + XTextWidth(pStatusFontInfo, status, cursor);
            x2 = status_width - RIGHTMARGIN - 4;
            n = (x1 > x2 ? x1 - x2 : 0);
            moved = ( (n == statusOffset) ? 0 : 1);
            statusOffset = n;
            RepairStatus(x1 - statusOffset - 1, moved);
        }
        else
            XBell(display, 0);
    }
    else if (c == 21) /* Ctrl-U */
    {
	ClearStatus();
        RepairStatus(statusOffset, 1);
    }
    else if (c == 127)  /* DEL */
    {
        if (cursor < strlen(status))
        {
            strcpy(status+cursor, status+cursor+1);
            x1 = LEFTMARGIN + 4 + XTextWidth(pStatusFontInfo, status, cursor);
            x2 = status_width - RIGHTMARGIN - 4;
            n = (x1 > x2 ? x1 - x2 : 0);
            moved = ( (n == statusOffset) ? 0 : 1);
            statusOffset = n;
            RepairStatus(x1 - statusOffset - 1, moved);
        }
        else
            XBell(display, 0);
    }
    else if ((Authorize || LabelString(&i)) && (c == '\n' || c == '\r') )
    {
        SaveStatusString();

        if (Authorize)
        {
            if (Authorize == GATEWAY)
            {
                if (gatewayUser)
                    Free(gatewayUser);

                gatewayUser = strdup(status);
                Authorize = 0;
                busy = 0;
                status[0] = '\0';
#ifdef SELECTION
		beginChr = endChr = 0;
#endif
                DisplayStatusBar();
                DisplaySizeChanged(0);
                DisplayScrollBar();
                i = 2 + charheight;
                DisplayDoc(WinLeft, WinBottom-i, WinWidth, i);
                XFlush(display);
/*                OpenDoc(NewDoc.url, 0, (strchr(NewDoc.url, ':') ? REMOTE : LOCAL));*/
            }
            else
            {
                who = strdup(status);
                Authorize = 0;
                busy = 0;
                status[0] = '\0';
#ifdef SELECTION
		beginChr = endChr = 0; /* howcome */
#endif
                DisplayStatusBar();
                DisplaySizeChanged(0);
                DisplayScrollBar();
                i = 2 + charheight;
                DisplayDoc(WinLeft, WinBottom-i, WinWidth, i);
                XFlush(display);
/*                OpenDoc(NewDoc.url, who, (strchr(NewDoc.url, ':') ? REMOTE : LOCAL));*/
                Free(who);
            }
        }
        else if (OpenURL) {
	    if (STATUS_TRACE)
		fprintf(stderr,"EditChar: calling OpenDoc(%s, NULL, %s)\n",status, (strchr(status, ':') ? "REMOTE" : "LOCAL"));
#ifdef __QNX__
	    /* assume http:// if no protocol specified */
	    if (strchr(status, ':') == 0) {
		memmove(status + sizeof "http://" - 1, status, strlen(status) + 1);
		memcpy(status, "http://", sizeof "http://" - 1);
	    }
#endif
            OpenDoc(status);
	}
        else if (SaveFile)
            SaveDoc(status);
        else if (FindStr)
        {
            FindNextStr = 0;
            FindString(status, &FindNextStr);
        }
        else if (IsIndex)
            SearchIndex(status);
    }
    else if (c >= ' ')
    {
        if (n < STATSIZ-1)
        {
            for (i = n; i >= cursor; --i)
                status[i+1] = status[i];

            status[cursor++] = c;
            
            x1 = LEFTMARGIN + 3 + XTextWidth(pStatusFontInfo, status, cursor);
            x2 = status_width - RIGHTMARGIN - 4;
            n = (x1 > x2 ? x1 - x2 : 0);
            moved = ( (n == statusOffset) ? 0 : 1);
            statusOffset = n;
            RepairStatus(x1 - statusOffset - XTextWidth(pStatusFontInfo, &c, 1), moved);
        }
        else
            XBell(display, 0);
    }
}

void MoveStatusCursor(int key)
{
    int was, x, y, x1, x2, moved = 0, r, n;
    unsigned int w, h;
    XRectangle rect;

    was = cursor;

    if (key == XK_Left)
    {
        if (cursor > 0)
        {
            --cursor;
            x1 = LEFTMARGIN + 4 + XTextWidth(pStatusFontInfo, status, cursor);
            x2 = status_width - RIGHTMARGIN - 4;
            n = (x1 > x2 ? x1 - x2 : 0);
            moved = ( (n == statusOffset) ? 0 : 1);
            statusOffset = n;

            if (moved)
                RepairStatus(x1 - statusOffset - 1, moved);
        }
        else
            XBell(display, 0);
    }
    else if (key == XK_Right)
    {
        if (cursor < strlen(status))
        {
            ++cursor;
            x1 = LEFTMARGIN + 3 + XTextWidth(pStatusFontInfo, status, cursor);
            x2 = status_width - RIGHTMARGIN - 4;
            n = (x1 > x2 ? x1 - x2 : 0);
            moved = ( (n == statusOffset) ? 0 : 1);
            statusOffset = n;

            if (moved)
                RepairStatus(x1 - statusOffset - XTextWidth(pStatusFontInfo, status+cursor-1, 1), moved);
        }
        else
            XBell(display, 0);
    }

    if (!moved && was != cursor)
    {
        rect.x = x = LEFTMARGIN + 2;
        rect.y = y = StatusTop + StatusFontOffset; 

        rect.width = w = status_width - (LEFTMARGIN + RIGHTMARGIN + 4);
        rect.height = h = 2 + charheight;

        XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);

        XSetForeground(display, status_gc, statusColor);
        r = XTextWidth(pStatusFontInfo, status, was);
        XFillRectangle(display, win, status_gc, x+1+r - statusOffset, y, 1, h);

        XSetForeground(display, status_gc, strikeColor);
        r = XTextWidth(pStatusFontInfo, status, cursor);
        XFillRectangle(display, win, status_gc, x+1+r - statusOffset, y, 1, h);
    }
}

void Announce(char *args, ...)
{
    va_list ap;
    char buf[512];

    if (initialised)
    {
        va_start(ap, args);
        vsprintf(buf, args, ap);
        va_end(ap);

        SetStatusString(buf);
        XFlush(display);
    }
    else
    {
        va_start(ap, args);
        vsprintf(buf, args, ap);
        va_end(ap);

        SetStatusString(buf);
    }
    /* howcome 16/10/94: moved the following statments outside block */
    if (STATUS_TRACE)
	fprintf(stderr, "%s\n", buf);
}

void Warn(char *args, ...)
{
    va_list ap;
    char buf[512];

    if (initialised)
    {
        va_start(ap, args);
        vsprintf(buf, args, ap);
        va_end(ap);

        if (STATUS_TRACE)
            fprintf(stderr, "%s\n", buf);

        SetStatusString(buf);
	/* XBell(display, 0);*/
        XFlush(display);
    }
    else
    {
        va_start(ap, args);
        vsprintf(buf, args, ap);
        va_end(ap);

        SetStatusString(buf);
        fprintf(stderr, "%s\n", buf);
    }
}

void Beep()
{
    XBell(display, 0);
    XFlush(display);
}

int StatusButtonDown(int button, int px, int py)
{
    char *s;
    int x, y, n;
    unsigned int w, h;
    XRectangle rect;

    x = abButton.x;
    y = abButton.y;
    w = abButton.w;
    h = abButton.h;

    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;


#ifdef SELECTION
    /* howcome 1/8/94 */
    if (button == Button1 && !OpenURL /* && !AbortButton */)
    {
      textWidth = XTextWidth(pStatusFontInfo, " ", 1);
      if (textWidth > 0) {
	startSelect = (px - 50 - 4) / textWidth;	/* 50 should be a define */
/*	startSelect = min(startSelect,strlen(status));*/
	if (startSelect >= 0) { 
	  beginChr = endChr = startSelect;
	  selecting = True;
	}
      }
/*      fprintf(stderr,"%d down x %d y %d px %d py %d w %d h %d\n",startSelect,x,y,px,py,w,h);*/
    }

#endif


    if (button == Button1 && AbortButton &&
        x <= px &&
        px < x + w &&
        y <= py &&
        py < y + h)
    {
        ABup = 0;
        XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);
        DrawInSet(win, status_gc, x, y, w, h);
        return STATUS;
    }

    /* now check for middle or right button down over status window */

    x = LEFTMARGIN + 2;
    y = StatusTop + StatusFontOffset; 

    w = status_width - (LEFTMARGIN + RIGHTMARGIN + 4);
    h = statusHeight - 12;

    if ((button == Button2 || button == Button3) && LabelString(&n) &&
        x <= px &&
        px < x + w &&
        y <= py &&
        py < y + h)
    {
        s = XFetchBytes(display, &n);
	s = strtok(s,"\n");		/* howcome 24/5/95: remove newlines */
	n = strlen(s);

        if (s)
        {
            x = strlen(status);
            if (x + n > STATSIZ-1)
                n = STATSIZ-1-x;

            strncpy(status+x, s, n);
            status[x+n] = '\0';
            cursor = x+n;
 
            XFree(s);
            Redraw(x, y, w, h);
        }
    }
#ifdef SELECTION
    return STATUS; /* howcome 1/8/94: indicate interest in buttonup since user may select (part of) url */
#else
    return VOID;
#endif
}

void HideAuthorizeWidget(void)
{
    AbortButton = 0;
    Authorize = 0;
    busy = 0;
    ClearStatus();
    DisplayStatusBar();
    DisplaySizeChanged(0);
    DisplayScrollBar();
    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
    XFlush(display);
}

void StatusButtonUp(int px, int py)
{
    int x, y;
    unsigned int w, h;
    XRectangle rect;

    x = abButton.x;
    y = abButton.y;
    w = abButton.w;
    h = abButton.h;

    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;

    if (AbortButton)
    {
        /* redraw button in up state */

        if (ABup == 0)
        {
            ABup = 1;
            XSetClipRectangles(display, status_gc, 0, 0, &rect, 1, Unsorted);
            DrawOutSet(win, status_gc, x, y, w, h);
        }

        /* check if up event occurs in button */

        if (x <= px &&
            px < x + w &&
            y <= py &&
            py < y + h)
        {
            AbortFlag = 1;

            if (Authorize)  /* hide password widget */
                HideAuthorizeWidget();
        }
    }

#ifdef SELECTION
    if (selecting) {	/* howcome 1/8/94 */
      char *s;
      const int n = strlen(status);
      
      if (beginChr > n && endChr > n) {		/* off bounds, select everything */
	beginChr = 0;
	endChr = strlen(status);
      }

      if (beginChr == endChr && beginChr < n) {
	char *pb, *pe;

	pb = pe = status + beginChr;			/* search for sensible strings */
	while(pb >= status && *pb != ':' && *pb !='/')
	  pb--;
	pb++;
	while(*pe != '\0' && *pe != ':' && *pe !='/')
	  pe++;
	  
	beginChr = pb - status;
	endChr = pe - status;
      }

      if (beginChr > endChr) {		/* this happens to be true when a ':' of '/' has been pressed, select everything */
	beginChr = 0;
	endChr = n;
      }

      endChr = min(n, endChr); /* howcome 8/3/95 */

      s = strdup(status);
      strncpy(s, status + beginChr, endChr - beginChr);
      s[endChr-beginChr] = '\0';

      if (STATUS_TRACE) 
	fprintf(stderr,"Set selection: \"%s\"\n",s);
      DisplayStatusBar();

      SetSelection(s);
      Free(s);
      selecting = False;
    }

#endif /* SELECTION */

}

void ShowAbortButton(int n)
{
    AbortFlag = 0;
    AbortButton = n;
    AbortButtonChanged = 1;
}

/* reconfigure status bar to ask for authorisation
   mode is REMOTE for remote hosts, and GATEWAY for the gateway */

void GetAuthorization(int mode, char *host)
{
    Authorize = mode;
    busy = 1;
    ClearStatus();

    ShowAbortButton(1);

    if (mode == GATEWAY)
    {
        strcpy(authreq, "Enter name:password for gateway ");
        strncpy(authreq+32, host, STATSIZ-33);
    }
    else
    {
        strcpy(authreq, "Enter name:password for ");
        strncpy(authreq+24, host, STATSIZ-25);
    }


    authreq[STATSIZ-1] = '\0';

    if (user)
        sprintf(status, "%s:", user);
    else {
        status[0] = '\0';
#ifdef SELCTION
	beginChr = endChr = 0; /* howcome */
#endif
    }
    cursor = strlen(status);

    if (initialised)
    {
        DisplayStatusBar();
        DisplaySizeChanged(0);
        DisplayScrollBar();
    }
    else
        DisplaySizeChanged(0);
}

/* extract name from "name:password" */

char *UserName(char *who)
{
    char *p;
    static char name[32];

    strncpy(name, who, 30);
    name[31] = '\0';
    p = strchr(name, ':');

    if (p)
        *p = '\0';
    return name;
}

/* extract password from "name:password" */

char *PassStr(char *who)
{
    char *p;

    p = strchr(who, ':');

    return (p ? p+1 : "");
}
