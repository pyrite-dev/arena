/* display the scrollbar and handle button events */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <stdio.h>

#include "HTUtils.h"	/* WWW general purpose macros */
#include "tcp.h"
#include "HTList.h"
#include "HTAccess.h"

#include "www.h"

void DisplayHSlider();
void DisplayVSlider();

extern Display *display;
extern int screen;
extern unsigned int win_width, win_height;
extern int statusHeight;
extern int ToolBarHeight;
extern long PixelOffset;
extern long buf_height;
extern int buf_width, PixelIndent;
extern RepeatButtonDown;
extern unsigned long windowTopShadow, windowBottomShadow, windowShadow, windowColor;
extern int debug;

int sbar_width = 15;
int sy = 20;
int sh = 60;
int sx = 20;
int sw = 60;
int px;
int py;
int ox, oy;
int scrolling = 0;
int sliding = 0;
int uparrow = 0, downarrow = 0;
int leftarrow = 0, rightarrow = 0;
int vertical;

extern Window win;
GC sb_gc;

void SetScrollBarWin(Window aWin)
{
    win = aWin;
}

void SetScrollBarGC(GC aGC)
{
    sb_gc = aGC;
}

void SetScrollBarVPosition(long offset, long buffer_height)
{
    int h;

    if (buffer_height > WinHeight)
    {
        /* h is pixel height scrollbar can move */
        h = WinHeight - 2*(sbar_width - 1) - sh;

        sy = WinTop + sbar_width - 1 + (h * offset)/(buffer_height - WinHeight);
    }
    else
        sy = WinTop + sbar_width - 1;
}

void SetScrollBarHPosition(int indent, int buffer_width)
{
    int w;

    if (buffer_width > WinWidth)
    {
        /* w is pixel width scrollbar can move */
        w = WinWidth - 2*(sbar_width - 1) - sw;

        sx = WinLeft + sbar_width - 1 + (w * indent)/(buffer_width - WinWidth);
    }
    else
        sx = WinLeft + sbar_width - 1;
}

void SetScrollBarHeight(long buffer_height)
{
    int h;
    double x;

    h = WinHeight - 2*(sbar_width - 1);

    if (buffer_height <= WinHeight)
        sh = h;
    else
    {
        x = ((double)buffer_height)/WinHeight;
        sh = (int)(0.5 + h/x);

        /* ensure a minimum size for decent visibility */

        if (sh < 6)
            sh = 6;
    }
}

void SetScrollBarWidth(int buffer_width)
{
    int w;
    double x;

    w = WinWidth - 2*(sbar_width - 1);

    if (buffer_width <= WinWidth)
        sw = w;
    else
    {
        x = ((double)buffer_width)/WinWidth;
        sw = (int)(0.5 + w/x);

        /* ensure a minimum size for decent visibility */

        if (sw < 6)
            sw = 6;
    }
}

int ScrollButtonDown(int x, int y)
{
    px = x;
    py = y;
    sliding = 0;
    RepeatButtonDown = 0;

    if (x > WinRight)
    {
        scrolling = 1;

        if (y < WinTop + sbar_width - 1)
        {
            uparrow = 1;
            RepeatButtonDown = 1;
            MoveUpLine();
            return SCROLLBAR;
        }

        if (y < sy)
        {
            MoveUpPage();
        }

        if (y >= sy && y < sy + sh)
        {
            sliding = 1;
            vertical = 1;
            oy = y - sy;
            return SCROLLBAR;
        }

        if (y > WinBottom - sbar_width + 1)
        {
            downarrow = 1;
            RepeatButtonDown = 1;
            MoveDownLine();
            return SCROLLBAR;
        }

        if (y >= sy + sh)
        {
            MoveDownPage();
        }
    }
    else if (y > WinBottom)
    {
        scrolling = 1;

        if (x < WinLeft + sbar_width - 1)
        {
            leftarrow = 1;
            RepeatButtonDown = 1;
            MoveLeftLine();
            return SCROLLBAR;
        }

        if (x < sx)
        {
            MoveLeftPage();
        }

        if (x >= sx && x < sx + sw)
        {
            sliding = 1;
            vertical = 0;
            ox = x - sx;
            return SCROLLBAR;
        }

        if (x > WinRight - sbar_width + 1)
        {
            rightarrow = 1;
            RepeatButtonDown = 1;
            MoveRightLine();
            return SCROLLBAR;
        }

        if (x >= sx + sw)
        {
            MoveRightPage();
        }
    }
    return VOID;
}

void ScrollButtonUp(int x, int y)
{
    px = 0;
    py = 0;

    if (sliding)
        ScrollButtonDrag(x, y);

    scrolling = 0;
    sliding = 0;
    uparrow = downarrow = 0;
    leftarrow = rightarrow = 0;
}

/* update/move slider to reflect new position */
void MoveHSlider(int indent, int buffer_width)
{
    int sx1;

    sx1 = sx;
    SetScrollBarHPosition(indent, buffer_width);

    if (sx != sx1)
    {
        XSetForeground(display, sb_gc, windowShadow);
        XFillRectangle(display, win, sb_gc,
            sx1, WinBottom + 2,
            sw, sbar_width - 4);

        DisplayHSlider();
    }
}

/* update/move slider to reflect new position */
void MoveVSlider(long offset, long buffer_height)
{
    int sy1;

    sy1 = sy;
    SetScrollBarVPosition(offset, buffer_height);

    if (sy != sy1)
    {
        XSetForeground(display, sb_gc, windowShadow);
        XFillRectangle(display, win, sb_gc,
            WinWidth + 2, sy1,
            sbar_width - 4, sh);

        DisplayVSlider();
    }
}

void ScrollButtonHDrag(int x, int y)
{
    int x1, w1;

    px = x;
    py = y;

    x1 = WinLeft + sbar_width - 1;

    if (x - ox < x1)
    {
        if (x > x1)
            ox = x - x1;
        else
        {
            ox = 0;
            x = x1;
        }
    }

    x1 = WinRight - (sbar_width - 1);

    if (x - ox + sw > x1)
    {
        if (x < x1)
            ox = x - x1 + sw;
        else
        {
            ox = sw;
            x = x1;
        }
    }

    if (x - ox > sx)
    {
        x1 = sx;
        w1 = x - ox - x1;
    }
    else
    {
        x1 = x - ox + sw;
        w1 = sx + sw - x1;
    }

    if (w1 > 0)
    {
        XSetForeground(display, sb_gc, windowShadow);
        XFillRectangle(display, win, sb_gc,
                       x1, WinBottom + 2,
                       w1, sbar_width - 4);
    }

    if (sx != x - ox)
    {
        sx = x - ox;
        DisplayHSlider();
        SlideHDisplay(sx - WinLeft - sbar_width + 1, WinWidth - 2*(sbar_width - 1));
    }

}

void ScrollButtonVDrag(int x, int y)
{
    int y1, h1;

    px = x;
    py = y;

    y1 = WinTop + sbar_width - 1;

    if (y - oy < y1)
    {
        if (y > y1)
            oy = y - y1;
        else
        {
            oy = 0;
            y = y1;
        }
    }

    y1 = WinBottom - (sbar_width - 1);

    if (y - oy + sh > y1)
    {
        if (y < y1)
            oy = y - y1 + sh;
        else
        {
            oy = sh;
            y = y1;
        }
    }

    if (y - oy > sy)
    {
        y1 = sy;
        h1 = y - oy - y1;
    }
    else
    {
        y1 = y - oy + sh;
        h1 = sy + sh - y1;
    }

    if (h1 > 0)
    {
        XSetForeground(display, sb_gc, windowShadow);
        XFillRectangle(display, win, sb_gc,
                       WinRight + 2, y1,
                       sbar_width - 4, h1);
    }

    if (sy != y - oy)
    {
        sy = y - oy;
        DisplayVSlider();
        SlideVDisplay(sy - WinTop - sbar_width + 1, WinHeight - 2*(sbar_width - 1));
    }
}

void ScrollButtonDrag(int x, int y)
{
    if (sliding)
    {
	if (SCROLL_TRACE)
	    fprintf(stderr,"drag x %d y %d\n",x,y);

        if (vertical)
            ScrollButtonVDrag(x, y);
        else
            ScrollButtonHDrag(x, y);
    }
}

int AtStart(long offset)
{
    long h;

    h = buf_height - WinHeight;

    return (h <= 0 || offset == 0);
}

int AtEnd(long offset)
{
    long h;

    h = buf_height - WinHeight;

    return (h <= 0 || offset >= h);
}

int AtLeft(int indent)
{
    return (indent > 0 ? 0 : 1);
}

int AtRight(int indent)
{
    return  (indent >= buf_width - WinWidth);

}

void DisplayLeftArrow()
{
    int x, y;
    XPoint points[3];

    x = WinLeft;
    y = WinBottom;

    points[0].x = x + sbar_width - 2;
    points[0].y = y + 2;
    points[1].x = x + sbar_width - 2;
    points[1].y = y + sbar_width - 3;
    points[2].x = x + 2;
    points[2].y = y + sbar_width/2 + 1;

    XSetForeground(display, sb_gc, windowColor);
    XFillPolygon(display, win, sb_gc, points, 3, Convex, CoordModeOrigin);

    XSetForeground(display, sb_gc, windowBottomShadow);
    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);

    points[0].x -= 1;
    points[0].y += 1;
    points[1].x -= 1;
    points[1].y -= 1;
    points[2].x += 1;

    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);


    points[0].x = x + sbar_width - 2;
    points[0].y = y + 2;
    points[1].x = x + 2;
    points[1].y = y + sbar_width/2;

    XSetForeground(display, sb_gc, windowTopShadow);
    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);

    points[0].x -= 1;
    points[0].y += 1;
    points[1].x += 3;
    points[1].y -= 1;
    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);
}

void DisplayUpArrow()
{
    int x, y;
    XPoint points[3];

    x = WinRight;
    y = WinTop;

    points[0].x = x + 2;
    points[0].y = y + sbar_width - 2;
    points[1].x = x + sbar_width - 3;
    points[1].y = y + sbar_width - 2;
    points[2].x = x + sbar_width/2 + 1;
    points[2].y = y + 2;

    XSetForeground(display, sb_gc, windowColor);
    XFillPolygon(display, win, sb_gc, points, 3, Convex, CoordModeOrigin);

    XSetForeground(display, sb_gc, windowBottomShadow);
    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);

    points[0].x += 1;
    points[0].y -= 1;
    points[1].x -= 1;
    points[1].y -= 1;
    points[2].y += 1;

    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);


    points[0].x = x + 2;
    points[0].y = y + sbar_width - 2;
    points[1].x = x + sbar_width/2;
    points[1].y = y + 2;

    XSetForeground(display, sb_gc, windowTopShadow);
    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);

    points[0].x += 1;
    points[0].y -= 1;
    points[1].x -= 1;
    points[1].y += 3;
    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);
}

void DisplayRightArrow()
{
    int x, y;
    unsigned int w, h;
    XPoint points[3];

    x = WinLeft;
    y = WinBottom;
    w = WinWidth;
    h = sbar_width;

    points[0].x = x + w - 3;
    points[0].y = y + sbar_width/2;
    points[1].x = x + w - (sbar_width - 1);
    points[1].y = y + 2;
    points[2].x = x + w - (sbar_width - 1);
    points[2].y = y + sbar_width - 3;

    XSetForeground(display, sb_gc, windowColor);
    XFillPolygon(display, win, sb_gc, points, 3, Convex, CoordModeOrigin);

    XSetForeground(display, sb_gc, windowTopShadow);
    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);

    points[0].x -= 1;
    points[0].y -= 1;
    points[1].x += 1;
    points[1].y += 1;
    points[2].x += 1;
    points[2].y -= 1;

    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);

    points[0].x = x + w - 4;
    points[0].y = y + sbar_width/2 + 1;
    points[1].x = x + w - (sbar_width - 2);
    points[1].y = y + sbar_width - 4;

    XSetForeground(display, sb_gc, windowBottomShadow);
    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);

    points[0].x -= 1;
    points[0].y += 1;
    points[1].x += 1;
    points[1].y -= 1;

    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);
}

void DisplayDownArrow()
{
    int x, y;
    unsigned int w, h;
    XPoint points[3];

    x = WinRight;
    y = WinTop;
    w = sbar_width;
    h = WinHeight;

    points[0].x = x + sbar_width/2;
    points[0].y = y + h - 3;
    points[1].x = x + 2;
    points[1].y = y + h - (sbar_width - 1);
    points[2].x = x + sbar_width - 3;
    points[2].y = y + h - (sbar_width - 1);

    XSetForeground(display, sb_gc, windowColor);
    XFillPolygon(display, win, sb_gc, points, 3, Convex, CoordModeOrigin);

    XSetForeground(display, sb_gc, windowTopShadow);
    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);

    points[0].x -= 1;
    points[0].y -= 1;
    points[1].x += 1;
    points[1].y += 1;
    points[2].x -= 1;
    points[2].y += 1;

    XDrawLines(display, win, sb_gc, points, 3, CoordModeOrigin);

    points[0].x = x + sbar_width/2 + 1;
    points[0].y = y + h - 4;
    points[1].x = x + sbar_width - 4;
    points[1].y = y + h - (sbar_width - 2);

    XSetForeground(display, sb_gc, windowBottomShadow);
    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);

    points[0].x += 1;
    points[0].y -= 1;
    points[1].x -= 1;
    points[1].y += 1;

    XDrawLines(display, win, sb_gc, points, 2, CoordModeOrigin);
}

void DisplayHSlider()
{
    int x, y;
    unsigned int w, h;

    x = WinLeft;
    y = WinBottom;
    w = WinHeight;
    h = sbar_width;

    y += 2;
    x = sx;
    w = sw;
    h -= 4;
    XSetForeground(display, sb_gc, windowColor);
    XFillRectangle(display, win, sb_gc, x+2, y+2, w-4, h-4);

    XSetForeground(display, sb_gc, windowTopShadow);
    XFillRectangle(display, win, sb_gc, x, y, w, 1);
    XFillRectangle(display, win, sb_gc, x, y+1, w-1, 1);
    XFillRectangle(display, win, sb_gc, x, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+1, y+1, 1, h-1);
    
    XSetForeground(display, sb_gc, windowBottomShadow);
    XFillRectangle(display, win, sb_gc, x, y+h-1, w, 1);
    XFillRectangle(display, win, sb_gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, win, sb_gc, x+w-1, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+w-2, y+1, 1, h-1);
}

void DisplayVSlider()
{
    int x, y;
    unsigned int w, h;

    x = WinRight;
    y = WinTop;
    w = sbar_width;
    h = WinHeight;

    x += 2;
    y = sy;
    h = sh;
    w -= 4;
    XSetForeground(display, sb_gc, windowColor);
    XFillRectangle(display, win, sb_gc, x+2, y+2, w-4, h-4);

    XSetForeground(display, sb_gc, windowTopShadow);
    XFillRectangle(display, win, sb_gc, x, y, w, 1);
    XFillRectangle(display, win, sb_gc, x, y+1, w-1, 1);
    XFillRectangle(display, win, sb_gc, x, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+1, y+1, 1, h-1);
    
    XSetForeground(display, sb_gc, windowBottomShadow);
    XFillRectangle(display, win, sb_gc, x, y+h-1, w, 1);
    XFillRectangle(display, win, sb_gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, win, sb_gc, x+w-1, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+w-2, y+1, 1, h-1);
}

void DisplayScrollBar()
{
    int x, y;
    unsigned int w, h;

    /* fill-in bottom right corner square */
    XSetForeground(display, sb_gc, windowColor);
    XFillRectangle(display, win, sb_gc, WinRight, WinBottom, sbar_width, sbar_width);

    x = WinRight;
    y = WinTop;
    w = sbar_width;
    h = WinHeight;

    /* draw vertical scrollbar background */

    XSetForeground(display, sb_gc, windowShadow);
    XFillRectangle(display, win, sb_gc, x, y, w, h);

    XSetForeground(display, sb_gc, windowBottomShadow);
    XFillRectangle(display, win, sb_gc, x, y, w, 1);
    XFillRectangle(display, win, sb_gc, x, y+1, w-1, 1);
    XFillRectangle(display, win, sb_gc, x, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+1, y+1, 1, h-1);
    
    XSetForeground(display, sb_gc, windowTopShadow);
    XFillRectangle(display, win, sb_gc, x, y+h-1, w, 1);
    XFillRectangle(display, win, sb_gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, win, sb_gc, x+w-1, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+w-2, y+1, 1, h-1);

    x = WinLeft;
    y = WinBottom;
    w = WinWidth;
    h = sbar_width;

    /* draw horizontal scrollbar background */

    XSetForeground(display, sb_gc, windowShadow);
    XFillRectangle(display, win, sb_gc, x, y, w, h);

    XSetForeground(display, sb_gc, windowBottomShadow);
    XFillRectangle(display, win, sb_gc, x, y, w, 1);
    XFillRectangle(display, win, sb_gc, x, y+1, w-1, 1);
    XFillRectangle(display, win, sb_gc, x, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+1, y+1, 1, h-1);
    
    XSetForeground(display, sb_gc, windowTopShadow);
    XFillRectangle(display, win, sb_gc, x, y+h-1, w, 1);
    XFillRectangle(display, win, sb_gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, win, sb_gc, x+w-1, y, 1, h);
    XFillRectangle(display, win, sb_gc, x+w-2, y+1, 1, h-1);

    /* draw the sliders */

    DisplayHSlider();
    DisplayVSlider();

    /* draw the arrow buttons */

    DisplayUpArrow();
    DisplayDownArrow();
    DisplayLeftArrow();
    DisplayRightArrow();
}
