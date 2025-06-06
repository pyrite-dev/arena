/* display the file in the window */

#include <stdio.h>
#include "www.h"
#include "style.h"

#define IsWhiteSpace(c)  (c == ' ' || c == '\n' || c == '\t' || c == '\r')


extern HTAtom *text_atom;
extern HTAtom *html_atom;
extern HTAtom *html3_atom;
extern HTAtom *html_level3_atom;
extern HTAtom *gif_atom;
extern HTAtom *jpeg_atom;
extern HTAtom *xpm_atom;
extern HTAtom *xbm_atom;



extern Context *context;
extern Display*display;
extern int screen;
extern Window win;
extern int ExposeCount;  /* used to monitor GraphicsExpose events during scrolling */
extern int error;        /* set by HTML parser */
extern int statusHeight;
extern int ToolBarHeight;
extern unsigned long textColor, windowColor;
extern int sbar_width;
extern unsigned int win_width, win_height;
/*extern int document; */ /* HTML_DOCUMENT or TEXT_DOCUMENT */
extern int IsIndex;   /* HTML searchable flag */
extern int UsePaper;
extern int FindStr;   /* search this document for string */
extern XFontStruct *fixed_font;
extern char *FindStrVal;
extern Doc *CurrentDoc;
extern GC gc_fill;
extern unsigned int tileHeight;
extern char *targetptr;    /* for toggling view between HTML and TEXT */
extern char *targetId;     /* named ID (or anchor NAME) */
extern long ViewOffset;    /* for toggling between HTML/TEXT views */
extern long IdOffset;      /* offset of named ID */
extern Frame background;
extern XFontStruct *Fonts[FONTS];
extern int debug;
extern int depth;
extern Byte *paint;
extern unsigned long stdcmap[128];  /* 2/3/2 color maps for gifs etc */
extern unsigned long greymap[16];  /* for mixing with unsaturated colors */

extern char *Editor; /* howcome 12/10/94: external editor string */
extern double lens_size_factor;



/* 
    The current top line is displayed at the top of the window,the pixel
    offset is the number of pixels from the start of the document.
*/

GC disp_gc;

char *buffer;            /* the start of the document buffer */
extern char *StartOfLine;       /* the start of the current top line */
int hdrlen;              /* offset to start of data from top of buffer*/
long PixelOffset;        /* the pixel offset to this line */
long buf_height;
extern long lineHeight;
long chDescent;
int buf_width;
int PixelIndent;
int chStrike;
int chWidth;            /* width of average char */
int spWidth;            /* width of space char */
int font = -1;               /* index into Fonts[] */
/*Byte color_text_ix, color_background_ix;*/

XFontStruct *pFontInfo;

void SetDisplayWin(Window aWin)
{
    win = aWin;
}

void SetDisplayGC(GC aGC)
{
    disp_gc = aGC;
}

void SetDisplayFont(XFontStruct *pf)
{
    pFontInfo = pf;
    XSetFont(display, disp_gc, pFontInfo->fid);
    XSetForeground(display, disp_gc, textColor);
    XSetBackground(display, disp_gc, windowColor);
}

void SetColor(GC gc, int color_text_ix, int color_background_ix)
{
    XSetForeground(display, gc, ix2color(color_text_ix));
    /* GRR:  this works now that html.c is fixed */
    XSetBackground(display, gc, ix2color(color_background_ix));

#if 0

    XGCValues values;
    unsigned int valuemask = 0;

    if (color_text_ix < 128) {
	if (depth == 24)
	    values.foreground = magic2color(color_text_ix);
	else
	    values.foreground = stdcmap[color_text_ix];
	valuemask |= GCForeground; 
    } else if (color_text_ix < 144) {
	if (depth == 24)
	    values.foreground = magic2color(color_text_ix & 0xf);
	else
	    values.foreground = greymap[color_text_ix & 0xf];
	valuemask |= GCForeground; 
    }

    /* background colormap entry has to be found based on the index
       stored on the paint stream */

    if (color_background_ix < 128) {
	if (depth == 24)
	    values.background = magic2color(color_background_ix);
	else
	    values.background = stdcmap[color_background_ix];
	valuemask |= GCBackground;
    } else if (color_background_ix < 144) {
	if (depth == 24)
	    values.background = magic2color(color_background_ix & 0xf);
	else
	    values.background = greymap[color_background_ix & 0xf];
	valuemask |= GCBackground;
    }

    XChangeGC(display, gc, valuemask, &values);
#endif

}

void SetFont(GC gc, int fontIndex)
{
    font = fontIndex;
    pFontInfo = Fonts[fontIndex];
    XSetFont(display, gc, pFontInfo->fid);
/*
    XSetForeground(display, gc, textColor);
    XSetBackground(display, gc, windowColor);
*/
    lineHeight = 2 + pFontInfo->max_bounds.ascent + pFontInfo->max_bounds.descent;
    chDescent = pFontInfo->max_bounds.descent;
    chStrike = lineHeight - 2 - (pFontInfo->max_bounds.ascent + chDescent)/2;
    spWidth = XTextWidth(pFontInfo, " ", 1);
    chWidth = XTextWidth(pFontInfo, "ABCabc", 6)/6;
}

#if 0
void SetFont(GC gc, int fontIndex)
{
    XGCValues values;
    unsigned int valuemask;

    font = fontIndex;
    pFontInfo = Fonts[fontIndex];
    
    
    values.font = pFontInfo->fid;

    valuemask = GCFont;


    if (color_text_ix < 128) {
	if (depth == 24)
	    values.foreground = magic2color(color_text_ix);
	else
	    values.foreground = stdcmap[color_text_ix];
    } else if (color_text_ix < 144) {
	if (depth == 24)
	    values.foreground = magic2color(color_text_ix & 0xf);
	else
	    values.foreground = greymap[color_text_ix & 0xf];
    }
    valuemask |= GCForeground; 

    /* background colormap entry has to be found based on the index
       stored on the paint stream */

    if (color_background_ix < 128) {
	if (depth == 24)
	    values.background = magic2color(color_background_ix);
	else
	    values.background = stdcmap[color_background_ix];
    } else if (color_background_ix < 144) {
	if (depth == 24)
	    values.background = magic2color(color_background_ix & 0xf);
	else
	    values.background = greymap[color_background_ix & 0xf];
    }
    valuemask |= GCBackground;

    XChangeGC(display, gc, valuemask, &values);

    lineHeight = 2 + pFontInfo->max_bounds.ascent + pFontInfo->max_bounds.descent;
    chDescent = pFontInfo->max_bounds.descent;
    chStrike = lineHeight - 2 - (pFontInfo->max_bounds.ascent + chDescent)/2;
    spWidth = XTextWidth(pFontInfo, " ", 1);
    chWidth = XTextWidth(pFontInfo, "ABCabc", 6)/6;
}

#endif


void SetEmphFont(GC gc, XFontStruct *pFont, XFontStruct *pNormal)
{
    pFontInfo = pFont;
    XSetFont(display, gc, pFont->fid);
    XSetForeground(display, gc, textColor);
    XSetBackground(display, gc, windowColor);
    lineHeight = 2 + pNormal->max_bounds.ascent + pNormal->max_bounds.descent;
    chDescent = pNormal->max_bounds.descent;
    chStrike = lineHeight - 2 - (pFontInfo->max_bounds.ascent + chDescent)/2;
    spWidth = XTextWidth(pFontInfo, " ", 1);
    chWidth = XTextWidth(pFontInfo, "ABCabc", 6)/6;
}

/*
    When a new file buffer is created, the first thing to do is to measure
    the length of the buffer in pixels and set up the scrollbar appropriately.
    The reference point should be set to the beginning of the buffer.

    Assumes that pFontInfo has been set up in advance,
                 and that buffer and hdrlen are ok.
*/

void NewBuffer(Doc *doc)
{
    long target;
    char *buf = doc->content_buffer;
    char *tag = doc->tag;

/* should freeing be taken care of by the  library? */
/*
    if (buffer && buffer != buf)
        Free(buffer);
*/

    StyleClearDoc(); /* howcome 26/2/95: clear previous document's style sheet */

    buffer = buf;
/*    hdrlen = CurrentDoc.hdrlen;*/
/*    document = CurrentDoc.content_type;*/
    StartOfLine = buffer+hdrlen;
    PixelOffset = 0;
    PixelIndent = 0;

    if (DocHTML(doc))
    {
/*        SetFont(disp_gc, IDX_NORMALFONT);*/
        targetptr = 0;
        targetId = tag;
    }
/*    else
        SetDisplayFont(fixed_font);
*/

    background.child = NULL;
    IsIndex = 0;  /* clear the HTML searchable flag */
/*
    lineHeight = 2 + pFontInfo->max_bounds.ascent + pFontInfo->max_bounds.descent;
    chDescent = pFontInfo->max_bounds.descent;
    chWidth = XTextWidth(pFontInfo, " ", 1);
*/
    IdOffset = 0; /* howcome 10/1/95 */

/*    buf_height = DocHeight(buffer+hdrlen, &buf_width);*/
    DocDimension(doc, &buf_width, &buf_height);

    if (doc->state != DOC_PROCESSED)
        return;

    if (DocHTML(doc) && (IdOffset > 0)) {
	context->current_history->y = target = IdOffset;
	
	if (target > buf_height - WinHeight) {
	    target = buf_height - WinHeight;
	    
	    if (target < 0)
		target = 0; /* was target == 0; wm 18.Jan.95 */
	}	
	DeltaHTMLPosition(target);
    }

    SetScrollBarWidth(buf_width);
    SetScrollBarHeight(buf_height);
    SetScrollBarHPosition(PixelIndent, buf_width);
    SetScrollBarVPosition(PixelOffset, buf_height);
}

void DisplaySizeChanged(int all)
{
    int max_indent;
    long h, target;

    if (DocHTML(CurrentDoc))
    {
        ResizeForm();

/*        targetptr = TopStr(&background);*/ /* howcome 13/2/95: people report this as a bug */

/*	StyleWindowChange();*/ /* howcome 30/3/95 */

        PixelOffset = 0;
        buf_height = ParseHTML(&buf_width, FALSE);

        if (ViewOffset > 0)
        {
            target = ViewOffset;
            h = buf_height - WinHeight;

            if (h <= 0)
                target = 0;
            else if (target > h)
                target = h;

            DeltaHTMLPosition(target);
        }
    }
    else if (all || buf_height == 0)
    {
        PixelOffset = CurrentHeight(buffer);

	if (IdOffset > 0) {
	    target = IdOffset;

	    if (target > buf_height - WinHeight) {
		target = buf_height - WinHeight;

		if (target < 0)
		    target = 0; /* was target == 0; wm 18.Jan.95 */
	    }
	    DeltaHTMLPosition(target);
	}
        DocDimension(CurrentDoc, &buf_width, &buf_height);
/*        buf_height = DocHeight(buffer, &buf_width);*/
    }

    max_indent = (buf_width > WinWidth ? buf_width - WinWidth : 0);

    if (max_indent < PixelIndent)
        PixelIndent = max_indent;

    SetScrollBarWidth(buf_width);
    SetScrollBarHeight(buf_height);
    SetScrollBarHPosition(PixelIndent, buf_width);
    SetScrollBarVPosition(PixelOffset, buf_height);
}

int LineLength(char *buf)
{
    char *s;
    int len, c;

    s = buf;
    len = 0;

    while ((c = *s++) && c != '\n')
        ++len;

    if (buf[len-1] == '\r')
        --len;

    return len;
}

/* DEBUG: return pointer to null terminated line of text from string s */
char *TextLine(char *s)
{
    static char buf[128];
    int i, c;

    for (i = 0; i < 127; ++i)
    {
        c = *s;

        if (c == '\0' || c == '\r' || c == '\n')
            break;

        buf[i] = c;
        ++s;
    }

    buf[i] = '\0';
    return buf;
}


/* work out how far window has moved relative to document */

int DeltaTextPosition(long h)
{
    long d1;
    char *p1;
    int delta;
    int nClipped;            /* the number of pixels hidden for this line */

    nClipped = PixelOffset % lineHeight;

    /* find the text line which intersects/starts from top of window */
    /* d1 is pixel offset to new top line, p1 points to its text */
    /* PixelOffset is the pixel offset to the previous top line */

    if (h > PixelOffset)
    {     /* search forward */
        d1 = PixelOffset-nClipped;
        p1 = StartOfLine;

        while (d1 + lineHeight <= h)
        {
            while (*p1)
            {
                if (*p1++ != '\n')
                     continue;

                d1 += lineHeight;
                break;
            }

            if (*p1 == '\0')
                break;         /* this should be unnecessary */
        }
    }
    else  /* search backward */
    {
        d1 = PixelOffset-nClipped;
        p1 = StartOfLine;

        while (d1 > h)
        {
            if (p1 == buffer+hdrlen)
                 break;         /* this should be unnecessary */

            /* now move back to start of previous line*/

            --p1;  /* to first point to \n at end of previous line */

            for (;;)   /* and find start of that line */
            {
                if (p1 == buffer+hdrlen)
                        break;
                else if (*--p1 == '\n')
                {
                        ++p1;
                        break;
                }
            }

            /* finally adjust pixel offset to start of that line */
            d1 -= lineHeight;
        }
    }
    
    /* delta is required movement of window in pixels */

    delta = h - PixelOffset;

    StartOfLine = p1;
    PixelOffset = h;
    return delta;
}

/* move display to place the left of the window at indent pixels from left hand edge */

void MoveHDisplay(int indent)
{
    XRectangle rect;
    int delta;

    /* see if change in pixel offset from start of document is
       small enough to justify a scroll of window contents     */

    delta = indent - PixelIndent;
    PixelIndent += delta;


    if (delta > 0 && delta < (2 * WinWidth)/3)
    {
        /* document moves left by delta pixels thru window */

        rect.x = WinLeft;
        rect.y = WinTop;
        rect.width = WinWidth;
        rect.height = WinHeight;
        XSetClipRectangles(display, disp_gc, 0, 0, &rect, 1, Unsorted);

        XCopyArea(display, win, win, disp_gc,
                    WinLeft + delta, WinTop,
                    WinWidth - delta, WinHeight,
                    WinLeft, WinTop);

        /* we must note that a copy request has been issued, and avoid further
           such requests until all resulting GraphicsExpose events are handled
           as these will repair any holes caused by windows above this one */

        ExposeCount = 1;

        DisplayDoc(WinRight - delta, WinTop, delta, WinHeight);
    }
    else if (delta < 0 && -delta < (2 * WinWidth)/3)
    {
        /* document moves right by -delta pixels thru window */

        rect.x = WinLeft;
        rect.y = WinTop;
        rect.width = WinWidth;
        rect.height = WinHeight;
        XSetClipRectangles(display, disp_gc, 0, 0, &rect, 1, Unsorted);

        XCopyArea(display, win, win, disp_gc,
                    WinLeft, WinTop,
                    WinWidth + delta, WinHeight,
                    WinLeft - delta, WinTop);

        DisplayDoc(WinLeft, WinTop, - delta, WinHeight);        
    }
    else if (delta != 0)
        DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
}

/* move display to make h the top of the window in pixels from start of document */

void MoveVDisplay(long h)
{
    XRectangle rect;
    int delta;
    BG_Style *bg_style;
    unsigned char *p;
    int fixed_bg;

    /* remember where we are */

    context->current_history->y = h;

    /* see if change in pixel offset from start of document is
       small enough to justify a scroll of window contents     */

    /* howcome 13/2/94: how should we handle image dosuments ?? */

    if (DocHTML(CurrentDoc) && !CurrentDoc->show_raw)
        delta = DeltaHTMLPosition(h);
    else
        delta = DeltaTextPosition(h);

    CurrentDoc->already_displayed = FALSE;

    p=paint+15;
    
    fixed_bg = (bg_style=(BG_Style *)GetPointer(&p)) ? (bg_style->flag & S_BACKGROUND_FIXED) : 0; 
    
    if(fixed_bg)
    {  /* to avoid flickering we display things in a pixmap before --Spif 8-Nov-95 */
	Window windump;

	windump = win;
	win = XCreatePixmap(display, RootWindow(display, screen),WinWidth+WinLeft,WinHeight+WinTop,depth); /* tor@cs.brown.edu 22-Jan-96 */
        rect.x = WinLeft;
        rect.y = WinTop;
        rect.width = WinWidth;
        rect.height = WinHeight;
        XSetClipRectangles(display, disp_gc, 0, 0, &rect, 1, Unsorted);
	DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
	XCopyArea(display,win,windump,disp_gc,WinLeft,WinTop,WinWidth,WinHeight,WinLeft,WinTop); 
	XFreePixmap(display,win);
	win=windump;
	ExposeCount = 0;
    }
    else
    {
	if (delta > 0 && delta < (2 * WinHeight)/3)
	{
	    /* document moves up by delta pixels thru window */
	    
	    rect.x = WinLeft;
	    rect.y = WinTop;
	    rect.width = WinWidth;
	    rect.height = WinHeight;
	    XSetClipRectangles(display, disp_gc, 0, 0, &rect, 1, Unsorted);
	    
	    XCopyArea(display, win, win, disp_gc,
		      WinLeft, WinTop + delta,
		      WinWidth, WinHeight - delta,
		      WinLeft, WinTop);
	    
	    /* we must note that a copy request has been issued, and avoid further
	       such requests until all resulting GraphicsExpose events are handled
	       as these will repair any holes caused by windows above this one */
	    
	    ExposeCount = 1;
	    
	    DisplayDoc(WinLeft, WinBottom - delta, WinWidth, delta);
	}
	else if (delta < 0 && -delta < (2 * WinHeight)/3)
	{
	    /* document moves down by delta pixels thru window */
	    
	    rect.x = WinLeft;
	    rect.y = WinTop;
	    rect.width = WinWidth;
	    rect.height = WinHeight;
	    XSetClipRectangles(display, disp_gc, 0, 0, &rect, 1, Unsorted);
	    
	    XCopyArea(display, win, win, disp_gc,
		      WinLeft, WinTop,
		      WinWidth, WinHeight + delta,
		      WinLeft, WinTop - delta);
	    
	    DisplayDoc(WinLeft, WinTop, WinWidth, - delta);        
	}
	else if (delta != 0)
	    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
    }
}

/* Should these adjust window offset to ensure that nClipped == 0 ? */
/*   (i.e. so that top line is never clipped after MoveUpLine)     */

void MoveLeftLine()
{
    int offset;

    offset = PixelIndent - lineHeight;

    if (offset < 0)
        offset = 0;

    if (!AtLeft(offset))
    {
        MoveHDisplay(offset);
        MoveHSlider(offset, buf_width);
    }
    else
        MoveToLeft();
}

void MoveUpLine()
{
    long offset;

    offset = PixelOffset - lineHeight;

    if (offset < 0)
        offset = 0;

    if (!AtStart(offset))
    {
        MoveVDisplay(offset);
        MoveVSlider(offset, buf_height);
    }
    else
        MoveToStart();
}

void MoveLeftPage()
{
    int offset;

    offset = WinWidth - lineHeight * 2;

    offset = PixelIndent - offset;

    if (offset < 0)
        offset = 0;

    if (!AtStart(offset))
    {
        MoveHDisplay(offset);
        MoveHSlider(offset, buf_width);
    }
    else
        MoveToLeft();
}

void MoveUpPage()
{
    long offset;

    offset = WinHeight - lineHeight * 2;

    offset = PixelOffset - offset;

    if (offset < 0)
        offset = 0;

    if (!AtStart(offset))
    {
        MoveVDisplay(offset);
        MoveVSlider(offset, buf_height);
    }
    else
        MoveToStart();
}

void MoveRightLine()
{
    int offset;

    offset = PixelIndent + lineHeight;

    if (!AtRight(offset))
    {
        MoveHDisplay(offset);
        MoveHSlider(offset, buf_width);
    }
    else
        MoveToRight();
}

void MoveDownLine()
{
    long offset;

    offset = PixelOffset + lineHeight;

    if (!AtEnd(offset))
    {
        MoveVDisplay(offset);
        MoveVSlider(offset, buf_height);
    }
    else
        MoveToEnd();
}

void MoveRightPage()
{
    int offset;

    offset = WinWidth - lineHeight * 2;
    offset = PixelIndent + offset;

    if (!AtRight(offset))
    {
        MoveHDisplay(offset);
        MoveHSlider(offset, buf_width);
    }
    else
        MoveToRight();
}

void MoveDownPage()
{
    long offset;

    offset = WinHeight - lineHeight * 2;
    offset = PixelOffset + offset;

    if (!AtEnd(offset))
    {
        MoveVDisplay(offset);
        MoveVSlider(offset, buf_height);
    }
    else
        MoveToEnd();
}

void MoveToLeft()
{
    int offset;

    if (PixelIndent > 0)
    {
        offset = 0;
        MoveHDisplay(offset);
        MoveHSlider(offset, buf_width);
    }

}

void MoveToStart()
{
    long offset;

    if (PixelOffset > 0)
    {
        offset = 0;
        MoveVDisplay(offset);
        MoveVSlider(offset, buf_height);
    }

}

void MoveToRight()
{
    int offset;

    offset = buf_width - WinWidth;

    if (PixelIndent != offset)
    {
        MoveHDisplay(offset);
        MoveHSlider(offset, buf_width);
    }

}

void MoveToEnd()
{
    long offset;

    offset = buf_height - WinHeight;

    if (offset > 0 && PixelOffset != offset)
    {
        MoveVDisplay(offset);
        MoveVSlider(offset, buf_height);
    }
}

void SlideHDisplay(int slider, int scrollExtent)
{
    double dh;

    /* compute the new pixel offset to top of window */

    dh = ((double)slider * buf_width) / scrollExtent;
    MoveHDisplay((long)(dh+0.5));
}


void SlideVDisplay(int slider, int scrollExtent)
{
    double dh;

    /* compute the new pixel offset to top of window */

    dh = ((double)slider * buf_height) / scrollExtent;
    MoveVDisplay((long)(dh+0.5));
}


/*

  Display the text in the buffer appearing in the view defined
  by the rectangle with upper left origin (x, y) and extent (w, h)

  The first line of text is pointed to by (char *)StartOfLine
  and is PixelOffset-nClipped pixels from the start of the document.
  There are nClipped pixels of this line hidden above the top
  of the window, so that the window starts at PixelOffset pixels
  from the start of the document itself.

*/

void DisplayDoc(int x, int y, unsigned int w, unsigned int h)
{
    int line_number, c, len, x1, y1;
    char *p, *r, lbuf[512];
    XRectangle rect;
    int nClipped;            /* the number of pixels hidden for this line */
    Byte font_color_ix;
    BG_Style *bg_style;
    GC gc_bg;
    XGCValues values;
    unsigned int valuemask;
    XRectangle frameRect;
    int do_buffering;
    Window windump;

    /* janet 21/07/95: not used (needed?):    extern int sliding; */
   
    do_buffering = CurrentDoc->already_displayed;
    CurrentDoc->already_displayed = TRUE;
    if(do_buffering)
    {
	windump = win;
	win = XCreatePixmap(display, RootWindow(display, screen),WinWidth+WinLeft,WinHeight+WinTop, depth);
    };
 	
    if ((DocHTML(CurrentDoc) || DocImage(CurrentDoc)) && (!CurrentDoc->show_raw))
    {
	if(do_buffering)
	    DisplayHTML(WinLeft, WinTop, WinWidth, WinHeight);
	else
	    DisplayHTML(x, y, w, h);
	if(do_buffering)
	{
	    frameRect.x = WinLeft;
	    frameRect.y = WinTop;
	    frameRect.width = WinWidth;
	    frameRect.height = WinHeight;
	    XSetClipRectangles(display, disp_gc, 0, 0, &frameRect, 1, Unsorted);
	    XCopyArea(display,win,windump,disp_gc,WinLeft,WinTop,WinWidth,WinHeight,WinLeft,WinTop); 
	    XFreePixmap(display,win);
	    win = windump;
	};
	PaintVersion(error);
	return;
    };
    error = 0;
    PaintVersion(error);


    nClipped = PixelOffset % lineHeight;
/*    SetFont(disp_gc, IDX_FIXEDFONT);*/

    /* make absolutely certain we don't overwrite the scrollbar */

    if (w > WinWidth)
        w = WinWidth;

    /* make absolutely certain we don't overwrite the status bar */

    if (y < WinTop) 
    {
/*         h += WinTop;  */        /*  janet: not sure we need this,  it works without it */
        y = WinTop;          
    } 
        /* commented the following */
/*     if (h > WinHeight)  */
/*         h = WinHeight; */

    /* the text must be clipped to avoid running over into adjacent
       regions, i.e. the scrollbar at the rhs */

    
    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;
    
    XSetClipRectangles(display, disp_gc, 0, 0, &rect, 1, Unsorted);
    XSetClipRectangles(display, gc_fill, 0, 0, &rect, 1, Unsorted);

    x1 = WinLeft;
    y1 = WinTop - nClipped;
    line_number = 0;
    p = StartOfLine;

    if (UsePaper)
        XSetTSOrigin(display, gc_fill,
           -PixelIndent % tileHeight, -(int)(PixelOffset % tileHeight));

/*    XFillRectangle(display, win, gc_fill, x, y, w, h);*/

    /* the style was fetched in ToggleView */

/*    if ( DocHTML(CurrentDoc) && (CurrentDoc->show_raw))  */


      {
	FormatElementStart(TAG_HTML, NULL, 0);
	FormatElementStart(TAG_HTML_SOURCE, NULL, 0);

	font_color_ix = (Byte)StyleGet(SI_COLOR);
	bg_style = (BG_Style *)StyleGet(SI_BACKGROUND); 
	
	if(bg_style)
	{
	    if(bg_style->flag & S_BACKGROUND_COLOR) /* background color */
	    {
		XSetForeground(display, disp_gc, rgb2color(0, bg_style->r, bg_style->g, bg_style->b,0)); 
		XFillRectangle(display, win, disp_gc, x,y,w,h);	 
	    }
	    if(bg_style->flag & S_BACKGROUND_IMAGE) /* background image */
	    {
		if(bg_style->image)
		{
		    if(!((bg_style->flag & S_BACKGROUND_COLOR) && (bg_style->flag & S_BACKGROUND_X_REPEAT) && (bg_style->flag & S_BACKGROUND_Y_REPEAT)))
			XFillRectangle(display, win, disp_gc, x,y,w,h);
		    gc_bg = XCreateGC(display, win,0,NULL);
		    XCopyGC(display,gc_fill,0xFFFF,gc_bg);
		    valuemask = GCTile|GCFillStyle|GCTileStipXOrigin|GCTileStipYOrigin;
		    values.fill_style = FillTiled;
		    values.tile = bg_style->image->pixmap;
		    if(bg_style->flag & S_BACKGROUND_ORIGIN)
		    {
			if(bg_style->flag & S_BACKGROUND_FIXED)
			{
			    values.ts_x_origin = bg_style->x_pos*(WinWidth-bg_style->image->width)/100;
			    values.ts_y_origin = bg_style->y_pos*(WinHeight-bg_style->image->height)/100 + WinTop;
			}
			else
			{
			    values.ts_x_origin = bg_style->x_pos*(WinWidth-bg_style->image->width)/100 -PixelIndent;
			    values.ts_y_origin = bg_style->y_pos*(WinHeight-bg_style->image->height)/100 + WinTop - PixelOffset;
			};
		    }
		    else
		    {
			if(bg_style->flag & S_BACKGROUND_FIXED)
			{
			    values.ts_x_origin = 0;
			    values.ts_y_origin = WinTop;
			}
			else
			{
			    values.ts_x_origin = -PixelIndent;
			    values.ts_y_origin = WinTop - PixelOffset;
			}
		    }
		    XChangeGC(display, gc_bg, valuemask, &values);
		
		    if(bg_style->flag & S_BACKGROUND_Y_REPEAT)
		    {
			frameRect.y      = WinTop;
			frameRect.height = WinHeight;
		    }
		    else
		    {
			frameRect.y      = values.ts_y_origin;
			frameRect.height = bg_style->image->height; 
		    }
		    if(bg_style->flag & S_BACKGROUND_X_REPEAT)
		    {
			frameRect.x = WinLeft;
			frameRect.width = WinWidth;
		    }
		    else
		    {
			frameRect.x = values.ts_x_origin;
			frameRect.width = bg_style->image->width;
		    }
		    XSetClipRectangles(display, gc_bg, 0, 0,&frameRect, 1, Unsorted); 
		    XFillRectangle(display, win, gc_bg, x, y, w, h);
		    XFreeGC(display, gc_bg);
		}
		else
		    if(!(bg_style->flag & (S_BACKGROUND_COLOR))) /* use paper if no pixmap and color */
			XFillRectangle(display, win, gc_fill, x, y, w, h);
	    }
	    if(!(bg_style->flag & (S_BACKGROUND_COLOR |S_BACKGROUND_IMAGE)))
		XFillRectangle(display, win, gc_fill, x, y, w, h);
	} else 
	    XFillRectangle(display, win, gc_fill, x, y, w, h);
	
	SetFont(disp_gc, (int)StyleGet(SI_FONT));
	SetColor(disp_gc, font_color_ix, 255);
	
	FormatElementEnd();
	FormatElementEnd();
    }
    

    while ( (y1 < y+(int)h) && *p)  /* howcome 2/12/94 dump */
    {
        y1 += lineHeight;

        r = lbuf;
        len = 0;

        while ((c = *p) && c != '\n')
        {
            ++p;

            if (len > 512 - TABSIZE)
                continue;

            if (c == '\t')
            {
                do
                    *r++ = ' ';
                while (++len % TABSIZE);

                continue;
            }

            if (c == '\r')
                continue;

            if (c == '\b')
            {
                if (len > 0)
                {
                    --len;
                    --r;
                }

                continue;
            }

            ++len;
            *r++ = c;
        }

	/* howcome 1/7/95 */

        if (y1 > y)
	    XDrawString(display, win, disp_gc, x1+4-PixelIndent, y1 - chDescent, lbuf, len);
       
        if (*p == '\n')
            ++p;
    };
    if(do_buffering)
    {
	XCopyArea(display,win,windump,disp_gc,WinLeft,WinTop,WinWidth,WinHeight,WinLeft,WinTop); 
	XFreePixmap(display,win);
	win = windump;
    };
}

void DisplayAll()
{
    DisplayToolBar();
    DisplayStatusBar();
    DisplayScrollBar();
    DisplayIcon();
    DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
    XFlush(display);
}
	   
/* what is the offset from the start of the file to the current line? */

long CurrentHeight(char *buf)
{
    long height;

    if (!buf)
        return 0;

    if (DocHTML(CurrentDoc) && !CurrentDoc->show_raw)
    {
        height = 0;
        /* ParseSGML(HEIGHT, &height, &buf, 0, 0, StartOfLine); */
    }
    else
        height = (*buf ? PixelOffset : 0);

    return height;
}


/* how big (in pixels) is the file ? */

void DocDimension(Doc *doc, int *width, long *height)
{
    char *p, *buf;
    int w;
    HTAtom *a = NULL;

    if (doc && doc->anchor)
	a = HTAnchor_format(doc->anchor->parent);

    *width = WinWidth;
    buf = doc->content_buffer;

    if (DocHTML(doc) || (DocRawText(doc)))
	{
	    /* document is HTML, but we're in source mode */

	    if (doc->show_raw || (a == text_atom)) {
		*height = (*buf ? lineHeight : 0);
		p = buf;

		while (*buf)
		    {
			if (*buf++ == '\n')
			    {
				*height += lineHeight;
				w = chWidth * (buf - p);
				p = buf;

				if (w > *width)
				    *width = w;
			    }
		    }

		w = chWidth * (buf - p - 1);

		if (w > *width)
		    *width = w;

		/* should we call StyleParse manually here?? */

	    }
	    else {

		/* we're about to parse an HTML document (and thereby
                   generate a paint stream */

		*height = ParseHTML(width, TRUE);
	    }
	    doc->state = DOC_PROCESSED;
    } else if (DocImage(doc)) {

	/* the document is of a image type we can display internally */

	if (!doc->image)
	    ProcessLoadedImage(doc);

	if (doc->state == DOC_PROCESSED)
	    *height = ParseImage(doc, width);
    } else {

	/* unknown format */

	if (a && a->name)
	    Announce("Please register application to handle %s in ~/.mailcap", (char *)a->name);
    }
}


/* setup skip table for searching str forwards thru document */
void ForwardSkipTable(unsigned char *skip, int len, char *str)
{
    int i;
    unsigned char c;

    for ((i = 0); i < 256; ++i)
        skip[i] = len;

    for (i = 1; (c = *(unsigned char *)str++); ++i)
        skip[c] = len - i;
}

/* setup skip table for searching str backwards thru document */
void BackwardSkipTable(unsigned char *skip, int len, char *str)
{
    int i;
    /* janet 21/07/95: not used:    unsigned char c; */

    for ((i = 0); i < 256; ++i)
        skip[i] = len;

    str += len;

    for (i = len - 1; i >= 0; --i)
        skip[(unsigned char) *--str] = i;
}


void FindString(char *str, char **next)
{
#if 0
    char *p;
    int i, j, c1, c2, patlen, patlen1;
    long len, h, offset;
    unsigned char skip[256];
    static char *np;

    FindStr = 0;
    DisplayStatusBar();

    patlen = strlen(str);
    patlen1 = patlen - 1;
    ForwardSkipTable(skip, patlen, str);
    len = CurrentDoc->content_length;

    p = *next;

    if (!p ||  p <  buffer || p > buffer + len - 2)
        p = StartOfLine;

    i = p - buffer + patlen1;
    j = patlen1;

    while (j >= 0 && i < len && i >= 0)
    {
        c1 = buffer[i];
        c1 = TOLOWER(c1);

        c2 = str[j];
        c2 = TOLOWER(c2);

/*	fprintf(stderr, "howcome %c %c\n", c1, c2);*/

        if (IsWhiteSpace(c1))
            c1 = ' ';

        if (c1 == c2)
        {
            --i;
            --j;
            continue;
        }

    retry1:

        i += patlen - j;    /* to next New char */
        j = patlen1;

        if (i >= len)
            break;

        c1 = buffer[i];
        c1 = TOLOWER(c1);

        if (IsWhiteSpace(c1))
            c1 = ' ';

        i += skip[c1];
    }

    if (++j > 0)
    {
        *next = 0;
        Warn("Can't find \"%s\"", str);
    }
    else
    {
        /* move to start of current line */

        *next = buffer+i+patlen1;

        while (i > 0)
        {
            if (buffer[i] != '\n')
            {
                --i;
                continue;
            }

            ++i;
            break;
        }

        /* and display accordingly */

        len = 0;

/*        if (document == HTML_DOCUMENT)
        {
            offset = 0;
            p = StartOfLine;
            ParseSGML(HEIGHT, &offset, &p, 0, 0, buffer+i);
            DeltaHTMLPosition(PixelOffset+offset);

            if (AtLastPage())
            {
                offset = buf_height - WinHeight + lineHeight;

                if (offset > buf_height)
                    offset = buf_height - lineHeight;;

                if (offset < 0)
                    offset = 0;

                DeltaHTMLPosition(offset);
            }
        }
        else   */
        {
            p = buffer;
            StartOfLine = buffer+i;
            PixelOffset = (*p ? lineHeight : 0);

            while (*p && p < StartOfLine)
            {
                if (*p++ == '\n')
                    PixelOffset += lineHeight;
            }

            if (PixelOffset + WinHeight > buf_height)
            {
                len = buf_height - WinHeight;

                if (len < 0)
                    len = 0;

                MoveVDisplay(len);
            }
        }



        Announce("Found \"%s\", press F3 for next match", FindStrVal);
        SetScrollBarHPosition(PixelIndent, buf_width);
        SetScrollBarVPosition(PixelOffset, buf_height);
        DisplayScrollBar();
        DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
    }
#endif
}

/* toggle view between HTML and PLAIN */
void ToggleView(void)
{
    char *p, *start;	/* janet 21/07/95: not used: q */
    long offset, maxOffset, target;

    if (DocHTML(CurrentDoc))
    {
        if (!CurrentDoc->show_raw)
        {
	    /* howcome 12/10/94: added support for external editing */

	    CurrentDoc->show_raw = TRUE;

#ifdef STYLE_COLOR
	    {
#if 0
		Byte color_text_ix = rgb2ix(0, 0, 0, 0, False);  /* set color to black */


		color_text_ix = rgb2color(0, 0, 0);
		Byte color_text_ix = rgb2magic(0, 0, 0);  /* set color to black */
		if (color_text_ix < 128)
		    textColor = stdcmap[color_text_ix];
		else if (color_text_ix < 144)
		    textColor = stdcmap[color_text_ix & 0xF]; /* set color to black */
#endif
	    }
#endif
/*	    SetFont(disp_gc, IDX_FIXEDFONT);*/
	    
/*	    SetFont(disp_gc, GetFont(0, str2list("lucidasanstypewriter", "courier"), F_ENCODE(F_WEIGHT_BOLD, F_SLANT_ROMAN, F_SERIF_IGNORE), 14));*/

	    /* at this point, we need to set the font for html source. It will be used to calculate the display height */

	    FormatElementStart(TAG_HTML, NULL, 0);
	    FormatElementStart(TAG_HTML_SOURCE, NULL, 0);
	    SetFont(disp_gc, (int)StyleGet(SI_FONT));

	    p = buffer+hdrlen;
/*	    p = buffer;*/
	    start = TopStr(&background);
	    offset = 0;   /* (*p ? lineHeight : 0); */
	    
	    while (*p && p < start)
		{
		    if (*p++ == '\n')
			offset += lineHeight;
		}

/*	    buf_height = DocHeight(buffer+hdrlen, &buf_width);*/
	    DocDimension(CurrentDoc, &buf_width, &buf_height);
	    maxOffset = buf_height - WinHeight;
	    
	    if (offset > maxOffset)
		offset = maxOffset;

	    PixelOffset = 0;
	    StartOfLine = buffer+hdrlen;
/*	    DeltaTextPosition(offset); */
	    FormatElementEnd();
	    FormatElementEnd();
	}
        else
        {
/*            document = HTML_DOCUMENT;*/
	    CurrentDoc->show_raw = FALSE;

/*            SetFont(disp_gc, IDX_NORMALFONT); */
/*            SetFont(disp_gc, GetFont(0, (char *)"courier", F_ENCODE(F_WEIGHT_BOLD, F_SLANT_ROMAN, F_SERIF_DEFAULT), pt2px(18.0 * lens_size_factor))); */

            targetptr = StartOfLine;
            PixelOffset = 0;
            buf_height = ParseHTML(&buf_width, False);

            if (ViewOffset > 0)
            {
                target = ViewOffset;

                if (target > buf_height - WinHeight)
                {
                    target = buf_height - WinHeight;

                    if (target < 0)
                        target = 0; /* was target == 0; wm 18.Jan.95 */
                }

                maxOffset = buf_height - WinHeight;

                if (target > maxOffset && maxOffset >= 0)
                    target = maxOffset;

                DeltaHTMLPosition(target);
            }
        }

        SetScrollBarWidth(buf_width);
        SetScrollBarHeight(buf_height);
        SetScrollBarHPosition(PixelIndent, buf_width);
        SetScrollBarVPosition(PixelOffset, buf_height);
        DisplayScrollBar();
        DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
    }
    else
        Warn("You can only see the source of HTML documents");
}
