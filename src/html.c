/* html.c - display code for html

This file contains the code for displaying HTML documents, scrolling them
and dealing with hypertext jumps by recognising which buttons are clicked.

*/

/*

Janne Saarela
janne.saarela@hut.fi
28.7.1995

The SEQTEXT element in the PaintStream has the additional
attribute, emph, of width of two bytes.
The handling of different emph attributes is not finalized.
I tried using < and > symbols from the adobe symbol font
but they do not seem to work properly. OVERHAT, OVERDOT,
OVERDDOT and OVERTILDE are to be implemented.

*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "www.h"
#include "tools.h"
#include "forms.h"
#include "style.h"

extern Display *display;
extern int screen;
extern Window win;
extern GC disp_gc, gc_fill;
extern Cursor hourglass;
extern int UsePaper;
extern Context *context;
extern int ExposeCount; 

extern int debug;  /* controls display of errors */
/*extern int document; */ /* HTML_DOCUMENT or TEXT_DOCUMENT */
extern int busy;
extern int OpenURL;
extern int IsIndex;
extern int FindStr;
extern char *FindNextStr;
extern int SaveFile;
extern int sbar_width;
extern int statusHeight;
extern int ToolBarHeight;
extern unsigned long windowColor;
extern unsigned int win_width, win_height, tileWidth, tileHeight;

extern unsigned long textColor, labelColor, windowTopShadow,
                     strikeColor, windowBottomShadow, windowShadow, windowColor;

extern unsigned long stdcmap[128];  /* 2/3/2 color maps for gifs etc */
extern unsigned long greymap[16];  /* for mixing with unsaturated colors */
extern int depth;
extern Form *forms;

Byte color_text_ix, color_background_ix;

/*
    The current top line is displayed at the top of the window,the pixel
    offset is the number of pixels from the start of the document.
*/

extern char *buffer;            /* the start of the document buffer */
extern long PixelOffset;        /* the pixel offset to top of window */
extern int hdrlen;              /* MIME header length at start of buffer */
extern long buf_height;
extern long lineHeight;
extern long chDescent;
extern int buf_width;
extern int PixelIndent;
extern int chStrike;
extern int spWidth;             /* width of space char */
extern int chWidth;             /* width of average char */
extern Doc *CurrentDoc;
extern XFontStruct *pFontInfo;
extern XFontStruct *Fonts[FONTS];
extern int LineSpacing[FONTS], BaseLine[FONTS], StrikeLine[FONTS];
extern int LineThickness[FONTS];
extern int ListIndent1, ListIndent2;
extern char *LastBufPtr, *StartOfLine, *StartOfWord; /* in HTML document */

extern char *bufptr;  /* parse position in the HTML buffer */
extern Byte *TopObject;  /* first visible object in window */
extern Byte *paint; /* holds the sequence of paint commands */
extern int paintbufsize;     /* size of buffer, not its contents */
extern int paintlen;         /* where to add next entry */

extern Field *focus;
extern int font;  /* index into Fonts[] array */
int preformatted;
XRectangle displayRect; /* clipping limits for painting html */

long IdOffset;      /* offset for targetId */
char *targetptr;    /* for toggling view between HTML/TEXT views */
char *targetId;     /* for locating named Id during ParseHTML() */

/* globals associated with detecting which object is under the mouse */

char *anchor_start, *anchor_end;
Byte *clicked_element;
int img_dx, img_dy;

/* the background frame structure specifies:

    a) where to start painting in this frame

    b) where this frame ends in paint buffer

    c) tree of nested frames which all
       intersect the top of the window
*/
Frame background;

/* copy line from str to LineBuf, skipping initial spaces and SGML <tags> */
char *CopyLine(char *str, unsigned int len)
{
    int c, n, k;
    char *p;
    static char buf[1024];
    
    if (len == 0)
	return "";
    
    p = buf;
    
    while (len-- > 0)
    {
        c = *str;

        if (c == '\0')
            break;

        while (c == '<' && ((n = str[1]) == '!' || n == '?' || n == '/' || isalpha(n)))
	{                
	    
	    /* howcome 28/8/95: added code from Michael Van Biesbrouck
	       <mlvanbie@valeyard.csclub.uwaterloo.ca> to handle comments */
	    
	    if (strncmp(str+1, "!--", 3) == 0){
		str = FindEndComment( str + 2 );
		if (*str) str++;
	    } else {
		while (*str++ != '>');
	    }
            c = *str;
        }

        if (c == '&' && (isalnum(str[1]) || str[1] == '#')) /* howcome 25/11/94: isalpha -> isalnum */
        {
            n = entity(str + 1, &k);

            if (n)
            {
                /* TokenValue = n; */
                str += k;
                *p++  = n;
                continue;
            }
        }

        if (preformatted)
        {
            if (c == '\t')
            {
                n = 8 - (p - buf)%8;

                if (n > 0)
                    ++len;

                while (n-- > 0)
                {
                    *p++ = ' ';
                     --len;
                }
            }
            else
                *p++ = c;

            ++str;
            continue;
        }

        if (IsWhite(c))
        {
            do
                c = *++str;
            while (IsWhite(c));

            *p++ = c = ' ';
            continue;
        }

        *p++ = c;
        ++str;
    }

    *p = '\0';
    return buf;
}

void PrintFrameData(int depth, Frame *frame)
{
    int i;

    while (frame)
    {
        for (i = 0; i < depth; ++i)
            printf("  ");

        printf("x = %d, y = %ld; w = %d, h = %ld  size = %d, info at %d\n",
            frame->indent, frame->offset, frame->width, frame->height, frame->length, frame->info);

        PrintFrameData(depth+1, frame->child);
        frame = frame->next;
    }
}

void PrintFrame(Frame *frame)
{
    PrintFrameData(1, frame);
}

void PrintObject(unsigned char *p)
{
    int tag, emph, fnt, x1, yi, len, width;
    unsigned int c1, c2;

    long offset, str;
    char *s;
    BG_Style *bg_style;

    tag = *p++;

    if ((tag & 0xF) == STRING)
    {
	c1 = *p++; c2 = *p++; emph = c1 | c2<<8;
        fnt = *p++;

#ifdef STYLE_COLOR
	color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	bg_style=(BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
	if(bg_style)
	    color_background_ix = (bg_style->flag & S_BACKGROUND_COLOR) ? rgb2ix(0, bg_style->r, bg_style->g, bg_style->b,0) : 255; /* GRR */ 
	else
	    color_background_ix = 255;	 
#endif
        yi = *p++ - 128;
        c1 = *p++; c2 = *p++; x1 = (c1 | c2<<8);  /* here */
        c1 = *p++; c2 = *p++; len = c1 | c2<<8;
        c1 = *p++; c2 = *p++; width = c1 | c2<<8;
	str = (long) GetPointer(&p);
	p += POINTERSIZE;
        preformatted = (tag & PRE_TEXT);
        s = CopyLine((char *)str, len);
        printf("string: \"%s\"\n", s);
    }
    else
    {
        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        printf("tag = %d, offset = %ld\n", tag, offset);
    }
}


void OpenDoc(char *name)
{
  /* janet 21/07/95: not used:    char *p, *q; */
  /* janet 21/07/95: not used:    long wh, target; */

    OpenURL = 0;
    FindNextStr = 0;

/*    XDefineCursor(display, win, hourglass);*/
/*    XFlush(display);*/

    if (name)  /* attempt to get new document */
    {
        /* note current status string */

        SaveStatusString();

        /* quit Open, SaveAs or Find if active */

        OpenURL = SaveFile = FindStr = 0;

	libGetDocument(name, strlen(name), NULL, TRUE, FALSE, TRUE, FALSE, FALSE);
    }
}

void PostDoc(char *name, char *post)
{
    OpenURL = 0;
    FindNextStr = 0;

/*    XDefineCursor(display, win, hourglass);*/
/*    XFlush(display);*/

    if (name && post)  /* attempt to get new document */
    {
        /* note current status string */

        SaveStatusString();

        /* quit Open, SaveAs or Find if active */

        OpenURL = SaveFile = FindStr = 0;

	libPostDocument(name, strlen(name), post, strlen(post));
    }
}

void ReloadDoc(char *name)
{
  /* janet 21/07/95: not used:    char *p, *q; */
  /* janet 21/07/95; not used:    long wh, target; */

    OpenURL = 0;
    FindNextStr = 0;

/*    XDefineCursor(display, win, hourglass);*/
/*    XFlush(display);*/

    if (name)  /* attempt to get new document */
    {
        /* note current status string */

        SaveStatusString();

        /* quit Open, SaveAs or Find if active */

        OpenURL = SaveFile = FindStr = 0;

	/* free style sheets */

	if (CurrentDoc->style != context->style) {
	    FreeStyleSheet(CurrentDoc->style);
	    CurrentDoc->style = NULL;
	} else {
	    FreeStyleSheet(CurrentDoc->style);
	    CurrentDoc->style = NULL;
	    context->style = StyleGetInit();
	}
      
	Free(CurrentDoc->head_style);
	Free(CurrentDoc->link_style);

	libGetDocument(name, strlen(name), NULL, TRUE, FALSE, FALSE, TRUE, FALSE);
    }
}


/* find title text as defined by <TITLE>title text</TITLE> */

#define MAXTITLE 128

char *TitleText(char *buf)
{
    return "Dummy Title";
}

/*
    The global unsigned (char *)TopObject points to
    the paint stream and is adjusted to the first object
    that appears at the top of the window for an offset
    of h pixels from the start of the document.

    This involves a search thru the paint stream, and
    relies on the objects being ordered wrt increasing
    pixel offset from the start of the document.

    The paint stream is organised as a sequence of nested
    frames, intermingled with text lines.

    The procedure returns the pixel difference between
    the desired position and the current position.
*/

Frame *FrameForward(Frame *frame, long top);
Frame *FrameBackward(Frame *frame, long top);

long DeltaHTMLPosition(long h)
{
    long delta; 	/* janet 21/07/95; not used:  offset, TopOffset */
    /* janet 21/07/95; not used:    Byte *p, *q; */
    /* janet 21/07/95; not used:    int tag, c1, c2, k; */
    /* janet 21/09/95: not used:    Frame *frame; */

    if (h > PixelOffset)  /* search forwards */
        FrameForward(&background, h);
    else if (h == 0) /* shortcut to start */
    {
        FreeFrames(background.child);
        background.child = NULL;
        background.top = paint + FRAMESTLEN;
    }
    else  /* search backwards */
        FrameBackward(&background, h);

    delta = h - PixelOffset;
    PixelOffset = h;

    return delta;
}

/*
  Move forwards thru frame, adjusting frame->top to last text line
  before top. The routine iterates thru peer frames and recurses
  thru decendent frames. The frame (and its descendents) are removed
  from the peer list if it (and therefore they) finish before top.
  This is implemented by returning the frame if it is needed otherwise
  returning NULL

  Question: what happens when top is the bottom of the buffer?
            can this ever occur in practice, e.g. with a null file?
*/

Frame *FrameForward(Frame *frame, long top)
{
    long offset, height;
    unsigned int c1, c2;
    unsigned int width, length, len;
    int tag, indent, style, border;
    unsigned char *p, *p2, *obj;
    Frame *peer, *child, *last;
#ifdef STYLE_COLOR_BORDER
    Byte cb_ix;
#endif

    if (!frame)
        return NULL;

 /* move down each of peer frames */
    peer = FrameForward(frame->next, top);

 /* Does this frame and its descendants end before top ? */
    if (frame->offset + frame->height <= top)
    {
        if (frame == &background) /* should never occur in practice! */
        {
            FreeFrames(background.child);
            background.child = NULL;
            background.top = paint + paintlen;
            return &background; /* never has peers ! */
        }

     /* remove self and any descendents BUT not our peers! */
        frame->next = NULL;  /* unlink from list before freeing */
        FreeFrames(frame);
	frame = NULL;
        return peer;
    }

    frame->next = peer;
    frame->child = FrameForward(frame->child, top);

 /* find last child in list to avoid inserting
    new children in reverse order */

    for (last = frame->child; last; last = last->next)
    {
        if (last->next == NULL)
            break;
    }
    
 /* now move frame->top down until we reach top and insert
    any new children at end of frame->child list */

    p = frame->top;
    p2 = paint + frame->info + FRAMESTLEN + frame->length;

    while (p < p2)
    {
        obj = p;

        tag = *p++;

     /* if frame intersects top then create frame structure
        and find paint position within it (and its children)
        then insert it in front of frame->child list */

        if (tag == BEGIN_FRAME)
	{
            c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
            c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

            if (offset > top)
            {
                frame->top = obj;
                break;
            }

         /* otherwise pickup frame header params */

            c1 = *p++; c2 = *p++; indent = c1 | c2<<8;
            c1 = *p++; c2 = *p++; width = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height |= (c1 | c2<<8) << 16;
	    style = *p++; border = *p++;
#ifdef STYLE_COLOR_BORDER
	    cb_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
            c1 = *p++; c2 = *p++; length = c1 | c2<<8;
	    if (offset + height > top)
            {
                child = (Frame *)malloc(sizeof(Frame));
                child->next = NULL;
                child->child = NULL;
                child->offset = offset;
                child->indent = indent;
                child->width = width;
                child->height = height;
                child->info = obj - paint;
                child->top = p;  /* == obj + FRAMESTLEN */
                child->length = length;
                child->style = style;
                child->border = border;
		child->box_list = NULL;
#ifdef STYLE_COLOR_BORDER
                child->cb_ix = cb_ix;
#else
		child->cb_ix = 0;
#endif
                FrameForward(child, top);

             /* and insert new child in same order
                as it appears in paint buffer */

                if (last)
                    last->next = child;
                else
                    frame->child = child;

                last = child;
            }

            p += length+2; /* to skip over frame's contents */
            continue;
        }

     /* the END_FRAME is only used when scrolling backwards */

        if (tag == END_FRAME)
        {
            p += FRAMENDLEN - 1;
            continue;
        }

     /* safety net for garbled paint structure */

        if (tag != TEXTLINE)
        {
            fprintf(stderr, "Unexpected internal tag (1) %d\n", tag);
            Exit(1);
        }

      /* stop if textline overlaps top or starts after it */

        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        if (offset >= top)
        {
            frame->top = obj;
            break;
        }

        p += 4; /* skip over baseline, indent */
        c1 = *p++; c2 = *p++; height = c1 | c2<<8;

        if (offset + height > top)
        {
            frame->top = obj;
            break;
        }

        /* skip elements in text line to reach next object */

        while ((tag = *p++) != '\0')
        {
            switch (tag & 0xF)
            {
                case RULE:
                    p += RULEFLEN - 1;
                    break;

                case LINE:
                    p += LINEFLEN - 1;
                    break;

                case BULLET:
                    p += BULLETFLEN - 1;
                    break;

                case STRING:
                    p += STRINGFLEN - 1;
                    break;

                case SEQTEXT:
		    ++p; ++p;	/* skip over emph */
                    ++p;       /* skip over font */
#ifdef STYLE_COLOR
		    ++p; /* skip over color */
#endif
#ifdef STYLE_BACKGROUND
		    p+=POINTERSIZE; /* skip over background --Spif 18-Oct-95 */
#endif
                    ++p; ++p;  /* skip over x position */
                    ++p; ++p;  /* skip over y position */
                    len = *p++;
                    p += len;
                    break;

                case IMAGE:
                    p += IMAGEFLEN - 1;
                    break;

                case INPUT:
                    p += INPUTFLEN - 1;
                    break;

                default:
                    fprintf(stderr, "Unexpected internal tag (2) %d\n", tag);
/*                    Exit(1); */
            }
        }

        ++p; ++p;  /* skip over textline size param */
    }

    return frame;
}

/*
  Move backwards thru frame, adjusting frame->top to last text line
  before top. The routine iterates thru peer frames and recurses
  thru decendent frames. The frame (and its descendents) are removed
  from the peer list if it (and therefore they) start at or after top.
  This is implemented by returning the frame if it is needed otherwise
  returning NULL
*/

Frame *FrameBackward(Frame *frame, long top)
{
    long offset, height;
    unsigned int c1, c2;
    unsigned int width, length, size;	/* janet 21/07/95: not used: len */
    int tag, indent, style, border, k, known;
    unsigned char *p, *p1, *p2, *obj;
    Frame *peer, *child;
#ifdef STYLE_COLOR_BORDER
    Byte cb_ix;
#endif

    if (!frame)
        return NULL;

 /* if this frame starts after top then remove from peer list */

    if (frame->offset >= top)
    {
        if (frame == &background)
        {
            FreeFrames(background.child);
            background.child = NULL;
            background.top = paint + paintlen;
            return &background; /* never has peers ! */
        }

        peer = frame->next;
        frame->next = NULL; /* unlink from list before freeing */
        FreeFrames(frame);
	frame = NULL;
        return FrameBackward(peer, top);
    }

 /* move backwards through peer frames */
    frame->next = FrameBackward(frame->next, top);

 /* move backwards through current children */

    frame->child = FrameBackward(frame->child, top);

 /* now move frame->top back until we reach top and insert
    any new children in front of frame->child list */

    p = TopObject = frame->top;
    p2 = paint + frame->info + FRAMESTLEN;

    while (p > p2)
    {
        /* pop field size into k */
        c2 = *--p; c1 = *--p; k = c1 | c2<<8;

        p -= k;   /* p points to start of previous object */

        obj = p;
        tag = *p++;

        if (tag == BEGIN_FRAME)
        {
            --p;
            continue;
        }

     /* if frame intersects top then create frame structure
        unless it already is one of my children and find
        paint position within it (and its children)
        then insert it in front of frame->child list */

        if (tag == END_FRAME)
        {
            c1 = *p++; c2 = *p++; length = c1 | c2<<8;
            c1 = *p++; c2 = *p++; size = c1 | c2<<8;
            p = obj - (int)length;  /* kludge for bug in HP92453-01 A.09.19 HP C Compiler */

         /* p now points to BEGIN_FRAME tag */

            if (*p != BEGIN_FRAME)
            {
                fprintf(stderr, "FrameBackward: Unexpected internal tag %d when BEGIN_FRAME was expected\n", tag);
                Exit(1);
            }

            p1 = p;

         /* check if frame is one of my children, and hence already done */

            for (known = 0, child = frame->child; child; child = child->next)
            {
                if (child->info == p - paint)
                {
                    known = 1;
                    break;
                }
            }

            if (known)
            {
                p = obj;
                continue;
            }

            ++p; /* skip over frame tag */

            c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
            c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

            if (offset >= top)
            {
                p = obj;
                continue;
            }

         /* otherwise pickup frame header params */

            c1 = *p++; c2 = *p++; indent = c1 | c2<<8;
            c1 = *p++; c2 = *p++; width = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height |= (c1 | c2<<8) << 16;
            style = *p++; border = *p++;
#ifdef STYLE_COLOR_BORDER
	    cb_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
	    c1 = *p++; c2 = *p++; length = c1 | c2<<8;

            child = (Frame *)malloc(sizeof(Frame));
            child->next = NULL;
            child->child = NULL;
            child->offset = offset;
            child->indent = indent;
            child->width = width;
            child->height = height;
            child->info = p1 - paint;
            child->top = p + length;
            child->length = length;
            child->style = style;
            child->border = border;
	    child->box_list = NULL;
#ifdef STYLE_COLOR_BORDER
	    child->cb_ix = cb_ix;
#else
	    child->cb_ix = 0;
#endif
	    child = FrameBackward(child, top);

         /* and insert new child in front of current children */

            child->next = frame->child;
            frame->child = child;

            p = obj;
            continue;
        }

     /* safety net for garbled paint structure */

        if (tag != TEXTLINE)
        {
            fprintf(stderr, "FrameBackward: Unexpected internal tag: %d when TEXTLINE was expected\n", tag);
            Exit(1);
        }

      /* stop if textline overlaps top or starts before it */

        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        p = obj;

        if (offset < top)
            break;
    }

    frame->top = p;
    return frame;
}

void DrawHButtonUp(int x, int y, int w, int h)
{
    --x; ++y; ++w;  /* adjust drawing position */

    XSetForeground(display, disp_gc, windowTopShadow);
    XFillRectangle(display, win, disp_gc, x, y, w, 1);
    XFillRectangle(display, win, disp_gc, x, y, 1, h-1);

    XSetForeground(display, disp_gc, windowBottomShadow);
    XFillRectangle(display, win, disp_gc, x+1, y+h-1, w-1, 1);
    XFillRectangle(display, win, disp_gc, x+w-1, y+1, 1, h-2);
    XSetForeground(display, disp_gc, textColor);
}

void DrawHButtonDown(int x, int y, int w, int h)
{
    --x; ++y; ++w;  /* adjust drawing position */

    XSetForeground(display, disp_gc, windowBottomShadow);
    XFillRectangle(display, win, disp_gc, x, y, w, 1);
    XFillRectangle(display, win, disp_gc, x, y, 1, h-1);

    XSetForeground(display, disp_gc, windowTopShadow);
    XFillRectangle(display, win, disp_gc, x+1, y+h-1, w-1, 1);
    XFillRectangle(display, win, disp_gc, x+w-1, y+1, 1, h-2);
    XSetForeground(display, disp_gc, textColor);
}

/* Find which object if any is under the mouse at screen coords px, py
  event is one of BUTTONUP, BUTTONDOWN, MOVEUP, MOVEDOWN */

Byte *WhichFrameObject(int event, int indent, int x, int y, Byte *p1, Byte *p2,
               int *type, char **start, char **end, int *dx, int *dy)
{
    char *s;
    Byte *p, *q;
    unsigned int tag, len, emph;
    unsigned int c1, c2;
    int fnt, x1, y1, xi, yb, yi, y2, width, height; /* janet 21/07/95: not used: action, active */
    int style, border, length;
    long offset, str;
    BG_Style *bg_style;
#ifdef STYLE_COLOR_BORDER
    Byte cb_ix;
#endif
    int i; /* for debug only */

/*    x += PixelIndent; */ /* howcome: applied patch from dsr 18/11/94 */

    for (p = p1; p < p2;)
    {
        tag = *p++;

        if (tag == BEGIN_FRAME)
        {
            c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
            c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

         /* we are done if frame starts after bottom of window */

            y1 = WinTop + (offset - PixelOffset);

            if (y1 >= WinBottom)
                break;

         /* otherwise pickup frame header params */

            c1 = *p++; c2 = *p++; x1 = c1 | c2<<8;
            c1 = *p++; c2 = *p++; width = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height |= (c1 | c2<<8) << 16;
            style = *p++; border = *p++;
#ifdef STYLE_COLOR_BORDER
	    cb_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
            c1 = *p++; c2 = *p++; length = c1 | c2<<8;

         /* call self to find target object in this frame */

            q = WhichFrameObject(event, x1, x /* - x1 */, y, p, p + length, /* howcome 25/1/95 */
                                    type, start, end, dx, dy);

            if (q)
                return q;

            p += length+2; /* to skip over frame's contents */
            continue;
        }

        if (tag == END_FRAME)
        {
            p += FRAMENDLEN - 1;
            continue;
        }

        if (tag != TEXTLINE)
        {
            fprintf(stderr, "Unexpected internal tag (4) %d\n", tag);
            Exit(1);
        }

        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        /* y1 points to top of line */
        y1 = WinTop + (offset - PixelOffset);

        if (y1 > (int) WinBottom)
            break;

        c1 = *p++; c2 = *p++; yb = y1 + (c1 | c2<<8);
        c1 = *p++; c2 = *p++; xi = indent + (c1 | c2<<8);
        c1 = *p++; c2 = *p++; height = (c1 | c2<<8);

        while ((tag = *p++) != '\0')
        {
            switch (tag & 0xF)
            {
                case RULE:
                    p += RULEFLEN - 1;
                    break;

                case LINE:
                    p += LINEFLEN - 1;
                    break;

                case BULLET:
                    p += BULLETFLEN - 1;
                    break;

                case STRING:
		    c1 = *p++; c2 = *p++; emph = c1 | c2<<8;
                    fnt = *p++;
#ifdef STYLE_COLOR
		    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
		    bg_style=(BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
		    if (bg_style)
			color_background_ix = (bg_style->flag & S_BACKGROUND_COLOR) ? rgb2ix(0, bg_style->r, bg_style->g, bg_style->b,0) : 255; /* GRR */
		    else
			color_background_ix = 255;	
#endif
                    yi = *p++ - 128;
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; len = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;

		    p += POINTERSIZE;
		    str = (long)GetPointer(&p); /* only reference */
		    
                    height = LineSpacing[fnt];
                    y2 = yb - yi - ASCENT(fnt) - 1;

                    if ((emph & EMPH_ANCHOR) && x1 <= x && y2 <= y &&
                        x < x1 + width && y < y2 + height)
                    {
                        s = (char *)str;

                        while (s > buffer)
                        {
                            --s;

                            if (s[0] == '<' && TOLOWER(s[1]) == 'a' && s[2] <= ' ')
                                break;
                        }

                        *start = s;    /* used in next stage */
                        s = (char *)str;

                        while (strncasecmp(s, "</a>", 4) != 0)
                        {
                            if (*s == '\0')
                                break;

                            ++s;
                        }

                        *end = s;    /* used in next stage */
                        *dx = *dy = -1;
                        *type = TAG_ANCHOR;

                        /* return pointer to start of anchor object */
                        return p - STRINGFLEN;
                    }
                    break;

                case SEQTEXT:
		    p++; p++;	/* skip over emph */
                    p++;      /* skip over font */ /* janet: what about putting *p += 3 here? */
#ifdef STYLE_COLOR
		    ++p;  /* skip over color */ /* janet p += 2 */
#endif
#ifdef STYLE_BACKGROUND
		    p+=POINTERSIZE; /* skip over the background --Spif 18-Oct-95 */
#endif
                    ++p; ++p;  /* skip over x position */ 
                    ++p; ++p;  /* skip over y position */ /* janet: p += 4  */
                    len = *p++;
                    p += len;
                    break;

                case IMAGE:
                    c1 = *p++; c2 = *p++; y2 = yb - (c1 | c2<<8); /* delta */
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; height = c1 | c2<<8;
                    
                    if (x1 <= x && y2 <= y && x < x1 + width && y < y2 + height)
                    {
                        p += POINTERSIZE;  /* past pixmap */

			str = (long) GetPointer(&p);
#ifdef STYLE_BACKGROUND
			GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
			if (tag & EMPH_ANCHOR)
                        {
                            *dx = *dy = -1;

			    if ((tag & EMPH_INPUT) == EMPH_INPUT) /* --Spif image input */
			    {          /* can be removed, as INPUT is ANCHOR || ISMAP... */
			        *dx = x - x1;
				*dy = y - y2;
                            } 
			    else
			    {
                                if (tag & ISMAP)
				{
				    *dx = x - x1;
				    *dy = y - y2;
				};
			    };

                            s = (char *)str;
			    if ((tag & EMPH_INPUT) == EMPH_INPUT) /* --Spif 10-Oct-95 */
			    {
			        while (s > buffer) /* find input...*/
				{
				    --s;
				    if ( strncasecmp(s, "<input", 6) !=0 )
				        break;
				}
				*start = s;
				s = (char *)str;
				while ( *s != '>')
				{
				    if (*s == '\0')
				        break;
				    s++;
				}
				*end = s;
				*type = TAG_INPUT; /* ?? */
				
				return p - IMAGEFLEN;
			    }
			    else
			    {
                                while (s > buffer) /* find anchor... must also find an input...*/
				{
				    --s;

				    if (s[0] == '<' && TOLOWER(s[1]) == 'a' && s[2] <= ' ')
				        break;
				}

				*start = s;    /* used in next stage */
				s = (char *)str;

				while (strncasecmp(s, "</a>", 4) != 0)
				{
				    if (*s == '\0')
				        break;

				    ++s;
				}

				*end = s;    /* used in next stage */
				*type = TAG_ANCHOR;
				return p - IMAGEFLEN;
			    }
                        }
			else
			{
                            if (tag & ISMAP)
			    {
				*dx = x - x1;
				*dy = y - y2;
				*start = (char *)str;
				*end = (char *)str;
				*type = TAG_IMG;
				return p - IMAGEFLEN;
			    }
			}
                    }
                    else
		    {
			p += 2 * POINTERSIZE; /* past pixmap and buf pointer */
#ifdef STYLE_BACKGROUND
			p += POINTERSIZE;
#endif			
		    }
		    break;
			
                case INPUT: /* --Spif only text... */
		    ClipToWindow();
		    str = (long) GetPointer(&p);
#ifdef STYLE_BACKGROUND
		    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
		    if (ClickedInField(disp_gc, xi, yb, (Field *)str, x, y, event))
                    {
			s = ((Field *)str)->name; /* updating *start --Spif 14-Oct-95 */ 
			while (s > buffer)
			{
			    if ( strncasecmp(s, "<input", 6) ==0 )
			        break;
			    if ( strncasecmp(s, "<select", 7) ==0 )
			        break;
			    s--;
			}
 			*start = s;
 			bufptr = s;
			while(*s!='>') s++;
			*end = s;
			*type = TAG_INPUT;
			return (p - INPUTFLEN); 
		    };
		    break;
		    
                default:
                    fprintf(stderr, "Unexpected internal tag (5) %d\n", tag);
                    Exit(1);
            }
        }

        ++p; ++p;  /* skip final frame length field */ /* janet: p += 2? , and: is this ever used? */
    }

    return NULL;
}

/* find which hypertext button contains given point */
Byte *WhichObj(Frame *frame, int event, int x, int y,
               int *type, char **start, char **end, int *dx, int *dy)
{
    Byte *p, *q;
    Frame *peer;

    if (frame->child)
    {
        p = WhichObj(frame->child, event, x, y, type, start, end, dx, dy);

        if (p)
            return p;
    }

    for (peer = frame->next; peer; peer = peer->next)
    {
        p = WhichObj(peer, event, x, y, type, start, end, dx, dy);

        if (p)
            return p;
    }

    p = frame->top;
    q = paint + frame->info + FRAMESTLEN + frame->length;
    x += PixelIndent; /* howcome: applied patch from dsr 18/11/94 */
    return WhichFrameObject(event, frame->indent, x, y, p, q, type, start, end, dx, dy);
}

/* find which hypertext button contains given point */
Byte *WhichObject(int event, int x, int y,
               int *type, char **start, char **end, int *dx, int *dy)
{
    if (focus && focus->type == OPTIONLIST &&
                focus->flags & CHECKED &&
                ClickedInDropDown(disp_gc, focus, -1, event, x, y))
    {
        *type = TAG_SELECT;
        *start = *end = focus->name;
        return paint + focus->object;
    }

    return WhichObj(&background, event, x, y, type, start, end, dx, dy);
}

/* drawn anchors in designated state */
void DrawFrameAnchor(int up, int indent, Byte *start, Byte *end)
{
    char *s;
    Byte *p;
    unsigned int tag, len, emph;
    unsigned int c1, c2;
    int fnt, x1, y1, xi, y2, yi, yb, width, height;	/* janet 21/07/95: not used: action*/
    int style, border, length;
    long offset, str;
    BG_Style *bg_style;
#ifdef STYLE_COLOR_BORDER
    Byte cb_ix;
#endif

    displayRect.x = WinLeft;
    displayRect.y = WinTop;
    displayRect.width = WinWidth;
    displayRect.height = WinHeight;
    XSetClipRectangles(display, disp_gc, 0, 0, &displayRect, 1, Unsorted);

    for (p = start; p < end;)
    {
        tag = *p++;

        if (tag == BEGIN_FRAME)
        {
           c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
            c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

         /* we are done if frame starts after bottom of window */

            y1 = WinTop + (offset - PixelOffset);

            if (y1 >= WinBottom)
                break;

         /* otherwise pickup frame header params */

            c1 = *p++; c2 = *p++; x1 = c1 | c2<<8;
            c1 = *p++; c2 = *p++; width = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height |= (c1 | c2<<8) << 16;
	    style = *p++; border = *p++;
#ifdef STYLE_COLOR_BORDER
	    cb_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	    bg_style = (BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif 
            c1 = *p++; c2 = *p++; length = c1 | c2<<8;

         /* call self to draw anchors in this frame */

            DrawFrameAnchor(up, x1, p, p + length);
            p += length+2; /* to skip over frame's contents */
            continue;
        }

        if (tag == END_FRAME)
        {
            p += FRAMENDLEN - 1;
            continue;
        }

        if (tag != TEXTLINE)
        {
            fprintf(stderr, "Unexpected internal tag (6) %d\n", tag);
            Exit(1);
        }

        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        /* y1 points to top of line */
        y1 = WinTop + (offset - PixelOffset);

        if (y1 > (int) WinBottom)
            break;

        c1 = *p++; c2 = *p++; yb = y1 + (c1 | c2<<8);
        c1 = *p++; c2 = *p++; xi = (c1 | c2<<8) - PixelIndent;
        c1 = *p++; c2 = *p++; height = (c1 | c2<<8);
        xi += indent;

        while ((tag = *p++) != '\0')
        {
            switch (tag & 0xF)
            {
                case RULE:
                    p += RULEFLEN - 1;
                    break;

                case LINE:
                    p += LINEFLEN - 1;
                    break;

                case BULLET:
                    p += BULLETFLEN - 1;
                    break;

                case STRING:
                    c1 = *p++; c2 = *p++; emph = c1 | c2<<8;
                    fnt = *p++;
#ifdef STYLE_COLOR
		    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
		    bg_style=(BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
		    if(bg_style)
			color_background_ix = (bg_style->flag & S_BACKGROUND_COLOR) ? rgb2ix(0, bg_style->r, bg_style->g, bg_style->b,0) : 255; /* GRR */
		    else
			color_background_ix = 255;	 
#endif
                    yi = *p++ - 128;
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; len = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;
		    str = (long)GetPointer(&p);
		    p += POINTERSIZE; /* skip the reference pointer */
#ifdef STYLE_COLOR
		    font = fnt;

		    SetColor(disp_gc, color_text_ix, color_background_ix);
		    SetFont(disp_gc, font);
#else
                    if (font != fnt)
                    {
                        font = fnt;
                        SetFont(disp_gc, font);
                    }
#endif

                    height = LineSpacing[font];
                    y2 = yb - yi - ASCENT(font) - 1;

                    if (emph & EMPH_ANCHOR) 
                    {
                        s = (char *)str;

                        if (anchor_start <= s && s <= anchor_end)
                        {
                            if (up)
                                DrawHButtonUp(x1, y2, width, height);
                            else
                                DrawHButtonDown(x1, y2, width, height);
                        }
                    }
                    break;

                case SEQTEXT:
		    p++; p++;	/* skip over emph */
                    p++;      /* skip over font */ /* janet: what about putting *p += 3 here? */
#ifdef STYLE_COLOR
		    ++p;  /* skip over color */ /* janet p += 2 */
#endif
#ifdef STYLE_BACKGROUND
		    p+=POINTERSIZE; /* skip over background */
#endif
                    ++p; ++p;  /* skip over x position */ 
                    ++p; ++p;  /* skip over y position */ /* janet: p += 4  */
                    len = *p++;
                    p += len;
                    break;

                case IMAGE:
                    c1 = *p++; c2 = *p++; y2 = yb - (c1 | c2<<8);
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; height = c1 | c2<<8;

                    p += POINTERSIZE;  /* past pixmap id */
		    s = (char *)GetPointer(&p);
#ifdef STYLE_BACKGROUND
		    p += POINTERSIZE; /* skip background */
#endif
		 		    
                    if (tag & ISMAP && anchor_start == s)
                    {
                        x1 -= PixelIndent;

                        if (up)
                        {
                            DrawOutSet(win, disp_gc, x1, y2, width, height);
                            XSetForeground(display, disp_gc, textColor);
                            DrawHButtonDown(x1+4, y2+2, width-7, height-6);
                        }
                        else
                        {
                            DrawInSet(win, disp_gc, x1, y2, width, height);
                            XSetForeground(display, disp_gc, textColor);
                            DrawHButtonUp(x1+4, y2+2, width-7, height-6);
                        }

                        width -= 8;
                        height -= 8;
                        x1 += 4;
                        y2 += 4;
                    }
                    break;

                case INPUT:
		  /* --Spif 13-Oct-95 take care of this...it may be the cause of crashes */
                    p += INPUTFLEN - 1;
                    break;

                default:
                    fprintf(stderr, "Unexpected internal tag (7) %d\n", tag);
                    Exit(1);
            }
        }

        ++p; ++p;  /* skip final frame length field */
    }

    XFlush(display);
}

void DrawAnchor(Frame *frame, int state)
{
    Byte *start, *end;
    Frame *peer;

    if (frame->child)
        DrawAnchor(frame->child, state);

    for (peer = frame->next; peer; peer = peer->next)
        DrawAnchor(peer, state);

    start = frame->top;
    end = paint + frame->info + FRAMESTLEN + frame->length;
    DrawFrameAnchor(state, frame->indent, start, end);
}

/* find topmost object's pointer to the html buffer */
char *TopStrSelf(Byte *start, Byte *end)
{
    Byte *p;
    char *s;
    unsigned int tag, len, emph, fnt;
    unsigned int c1, c2;
    int style, border, length, x1, y1, xi, yb, yi, width, height; /* janet 21/07/95: not used: action, y2 */
    long offset, str;
#ifdef STYLE_COLOR_BORDER
    Byte cb_ix;
#endif

    for (p = start; p < end;)
    {
        tag = *p++;

        if (tag == BEGIN_FRAME)
        {
            c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
            c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;
            c1 = *p++; c2 = *p++; x1 = c1 | c2<<8;
            c1 = *p++; c2 = *p++; width = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height |= (c1 | c2<<8) << 16;
            style = *p++; border = *p++;
#ifdef STYLE_COLOR_BORDER
	    cb_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
            c1 = *p++; c2 = *p++; length = c1 | c2<<8;

            if ((s = TopStrSelf(p, p + length)))
                return s;

            p += length+2; /* to skip over frame's contents */
            continue;
        }

        if (tag == END_FRAME)
        {
            p += FRAMENDLEN - 1;
            continue;
        }

        if (tag != TEXTLINE)
        {
            fprintf(stderr, "Unexpected internal tag (8) %d\n", tag);
            Exit(1);
        }

        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        /* y1 points to top of line */
        y1 = WinTop + (offset - PixelOffset);

        if (y1 > (int) WinBottom)
            break;
        c1 = *p++; c2 = *p++; yb = y1 + (c1 | c2<<8);
        c1 = *p++; c2 = *p++; xi = (c1 | c2<<8);
        c1 = *p++; c2 = *p++; height = (c1 | c2<<8);

        while ((tag = *p++) != '\0')
        {
            switch (tag & 0xF)
            {
                case RULE:
                    p += RULEFLEN - 1;
                    break;

                case LINE:
                    p += LINEFLEN - 1;
                    break;

                case BULLET:
                    p += BULLETFLEN - 1;
                    break;

                case STRING:
                    c1 = *p++; c2 = *p++; emph = c1 | c2<<8;
                    fnt = *p++;
#ifdef STYLE_COLOR
		    p++; /* skip over color */
#endif
#ifdef STYLE_BACKGROUND
		    p+=POINTERSIZE; /* --Spif 20-Oct-95 skip over style background */
#endif
                    yi = *p++ - 128;
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; len = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;
		    p += POINTERSIZE; 
		    str = (long)GetPointer(&p);
                    return (char *)str; /* return (char *)GetPointer(&p); ??? */

                case SEQTEXT:
		    ++p; ++p;  /* skip over emph */
                    ++p;       /* skip over font */
#ifdef STYLE_COLOR
		    ++p; /* skip over color */
#endif
#ifdef STYLE_BACKGROUND
		    p+=POINTERSIZE; /* skip over background -_Spif 20-Oct-95 */
#endif
                    ++p; ++p;  /* skip over x position */
                    ++p; ++p;  /* skip over y position */
                    len = *p++;
                    p += len;
                    break;

                case IMAGE:
                    p += IMAGEFLEN - 1;
                    break;

                case INPUT:
                    p += INPUTFLEN - 1;
                    break;

                default:
                    fprintf(stderr, "Unexpected internal tag (9) %d\n", tag);
                    Exit(1);
            }
        }

        ++p; ++p;  /* skip final frame length field */ /* janet: p += 2? , and: is this ever used? */
    }

    return NULL;
}

char *TopStr(Frame *frame)
{
    Frame *peer;
    char *s;
    Byte *start, *end;

    if (frame->child && (s = TopStr(frame->child)))
        return s;

    for (peer = frame->next; peer; peer = peer->next)
        if ((s = TopStr(peer)))
            return s;

    start = frame->top;
    end = paint + frame->info + FRAMESTLEN + frame->length;
    return TopStrSelf(start, end);
}

void ClipToWindow(void)
{
    displayRect.x = WinLeft;
    displayRect.y = WinTop;
    displayRect.width = WinWidth;
    displayRect.height = WinHeight;
    XSetClipRectangles(display, disp_gc, 0, 0, &displayRect, 1, Unsorted);
/*    XSetClipRectangles(display, gc_fill, 0, 0, &displayRect, 1, Unsorted); -- tracking */
}

void DrawBorder(int border, int x, int y, unsigned int w, unsigned int h, Byte cb_ix)
{

#ifdef STYLE_COLOR_BORDER
    if (cb_ix < 128) {
	XSetForeground(display, disp_gc, stdcmap[cb_ix]);
    } else if (cb_ix < 144) {
	XSetForeground(display, disp_gc, greymap[cb_ix & 0xf]);
    }
#endif

    XFillRectangle(display, win, disp_gc, x, y, w, 1);
    XFillRectangle(display, win, disp_gc, x+w, y, 1, h);
    XFillRectangle(display, win, disp_gc, x, y+h, w, 1);
    XFillRectangle(display, win, disp_gc, x, y, 1, h);
}

void FreeBox(box_link *box_list)
{
    box_link *current_link;
    box_link *next_link;

    if(!box_list)
	return;

    current_link = box_list;
    next_link = box_list->next;
    
    while(next_link)
    {
	Free(current_link->box);
	Free(current_link);
	current_link = next_link;
	next_link = next_link->next;
    }
    Free(current_link->box);
    Free(current_link);
}

/* free frame structures except for background frame */
void FreeFrames(Frame *frame)
{
  /* janet 21/07/95    Frame *peer; */

    if (frame)
    {
/*
   patch from Kai.Arstila@Helsinki.FI replaces

        FreeFrames(frame->child);
        for (peer = frame->next; peer; peer = peer->next)
             FreeFrames(peer);

        Free(frame);

   with:

*/

        if(frame->child) 
	{
	    FreeFrames(frame->child);
	    frame->child = NULL;
	}
        if(frame->next)
	{
	    FreeFrames(frame->next);
	    frame->next = NULL;
	}
        FreeBox(frame->box_list);
	Free(frame);
	
/* end replacement */
	
    }
}

void PaintSelf(Frame *frame, int y, unsigned int h);
void PaintPeers(Frame *frame, int y, unsigned int h);
void PaintFrame(Window w,unsigned char *p, unsigned char *p_end, int x, int y, unsigned int h);

void DisplayHTML(int x, int y, unsigned int w, unsigned int h)
{
    long count = 0;
    BG_Style *bg_style;
    unsigned char *p;
    GC gc_bg;
    XGCValues values;
    unsigned int valuemask;
    XRectangle frameRect;

    if (UsePaper)
       XSetTSOrigin(display, gc_fill,
            -PixelIndent % tileHeight, -(int)(PixelOffset % tileHeight));

 /* make absolutely certain we don't overwrite the scrollbar */

    if (w > WinWidth)
        w = WinWidth;

    if (y < WinTop)
    {
        h -= (WinTop - y);
        y = WinTop;

        if (h <= 0)
            return;
    }

 /* make absolutely certain we don't overwrite the status bar */

    if (y + h > WinBottom)
    {
        h = WinBottom - y;

        if (h <= 0)
            return;
    }

 /* the text must be clipped to avoid running over into adjacent
    regions, i.e. the scrollbar at the rhs */

    displayRect.x = x;
    displayRect.y = y;
    displayRect.width = w;
    displayRect.height = h;
    XSetClipRectangles(display, disp_gc, 0, 0, &displayRect, 1, Unsorted);
    XSetClipRectangles(display, gc_fill, 0, 0, &displayRect, 1, Unsorted);

    /* howcome 25/4/95 */

    FormatElementStart(TAG_HTML, NULL, 0);

#ifdef STYLE_BACKGROUND
    p = paint + 15; /* --Spif 25-Oct-95 offset to style_bckng pointer */
    bg_style=(BG_Style *)GetPointer(&p);
    if(bg_style)
    {
	if(bg_style->flag & S_BACKGROUND_COLOR) /* background color */
	{
	    /* if(!((bg_style->image) && (bg_style->flag & S_BACKGROUND_X_REPEAT) && (bg_style->flag & S_BACKGROUND_Y_REPEAT)))
	     */{
		XSetForeground(display, disp_gc, rgb2color(0, bg_style->r, bg_style->g, bg_style->b,0)); 
		XFillRectangle(display, win, disp_gc, x,y,w,h);
	    } 
	}
	else
	    XFillRectangle(display, win, gc_fill, x,y,w,h);
	if(bg_style->flag & S_BACKGROUND_IMAGE) /* background image */
	{
	    if(bg_style->image)
	    {
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
#else
    count = StyleGet(S_BACKGROUND);
    if (count == 255) { /* i.e. is the background transparent? */
	XFillRectangle(display, win, gc_fill, x, y, w, h);
    } else {
	XSetForeground(display, disp_gc, ix2color(count));
	XSetFillStyle(display, disp_gc, FillSolid);
	XFillRectangle(display, win, disp_gc, x, y, w, h);
    } 
#endif

    FormatElementEnd();

 /* and paint all frames intersecting top of window */

    PaintSelf(&background, y, h);

    if (focus && focus->type == OPTIONLIST && focus->flags & CHECKED)
        PaintDropDown(disp_gc, focus, -1);
}

/* paint children then self - first called for background frame */
void PaintSelf(Frame *frame, int y, unsigned int h)
{
    long y1;
    unsigned char *p1, *p2;

    /* Test that this frame is at least partly visible */

    y1 = PixelOffset + y - WinTop;   /* pixel offset for screen coord y */

    if (frame->offset < y1 + h && frame->offset + frame->height > y1)
    {
        if (frame->child)
            PaintPeers(frame->child, y, h);

        if (frame->border)
        {
            DrawBorder(frame->border, frame->indent-PixelIndent,
                               WinTop + (frame->offset - PixelOffset),
                               frame->width, frame->height, frame->cb_ix);
        }

	if (TAG_TRACE)
#if defined PRINTF_HAS_PFORMAT
	    fprintf(stderr,"html.c: PaintSelf p1 = frame->top = %p, info = %d, lenght = %d\n",
#else
	    fprintf(stderr,"html.c: PaintSelf p1 = frame->top = %lx, info = %d, lenght = %d\n",
#endif /* PRINTF_HAS_PFORMAT */
		    frame->top, frame->info, frame->length);

        p1 = frame->top;
        p2 = paint + frame->info + FRAMESTLEN + frame->length;
        PaintFrame(win,p1, p2, frame->indent, y, h);
    }
}

/* paint list of peer frames */
void PaintPeers(Frame *frame, int y, unsigned int h)
{
    while (frame)
    {
        PaintSelf(frame, y, h);
        frame = frame->next;
    }
}

/*
    p, p_end point to paint buffer while x,y,w,h define region to paint

    This routine recursively calls itself to paint nested frames when
    it comes across the BEGIN_FRAME tag. Note that the border for the
    current frame is already drawn - and saves having to pass the
    relevant params to this routine.
*/
void PaintFrame(Window w,unsigned char *p, unsigned char *p_end, int x, int y, unsigned int h)
{
    char *s;
    unsigned int tag, len, width;
    unsigned int c1, c2;

    int fnt, fnt_size, x1, y1, x2, y2, xi, yb, yi, depth, tmp; /* janet 21/07/95: not used: action, active */
    unsigned int emph;
    int style, border, length;
    long offset, height, str;
    BG_Style *bg_style;
#ifdef STYLE_COLOR_BORDER
    Byte cb_ix;
#endif
    GC gc_bg = NULL;
    XGCValues values;
    unsigned int valuemask;
    Pixmap bg_pixmap = (Pixmap)NULL;
    XRectangle frameRect,bgRect;
    int dirtyscroll;

    dirtyscroll=0;
    displayRect.x = WinLeft;
    displayRect.y = WinTop;
    displayRect.width = WinWidth;
    displayRect.height = WinHeight;

    if (TAG_TRACE)
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr,"html.c: PaintFrame %p\n",p);
#else
	fprintf(stderr,"html.c: PaintFrame %lx\n",p);
#endif /* PRINTF_HAS_PFORMAT */

    while (p < p_end)
    {
        if (p >= paint + paintlen)
        {
            fprintf(stderr, "Panic: ran off end of paint buffer!\n");
	    Exit(1);
        }
	
	if (TAG_TRACE && VERBOSE_TRACE)
#if defined PRINTF_HAS_PFORMAT
	    fprintf(stderr,"PaintFrame: tag %d read from %p, p_end = %p\n",*p, p, p_end);
#else
	    fprintf(stderr,"PaintFrame: tag %d read from %lx, p_end = %lx\n",*p, p, p_end);
#endif /* PRINTF_HAS_PFORMAT */

        tag = *p++;

        if (tag == BEGIN_FRAME)
        {
            c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
            c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

         /* we are done if frame starts after bottom of window */

            y1 = WinTop + (offset - PixelOffset);
	    /*   
	    if (y1 >= y + (int)h)
                break; 
	    */
         /* otherwise pickup frame header params */

            c1 = *p++; c2 = *p++; x1 = c1 | c2<<8;
            c1 = *p++; c2 = *p++; width = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height = c1 | c2<<8;
            c1 = *p++; c2 = *p++; height |= (c1 | c2<<8) << 16;
	    style = *p++; border = *p++;
#ifdef STYLE_COLOR_BORDER
	    cb_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
	    bg_style=(BG_Style *)GetPointer(&p);
	    if(bg_style)
	    {
		if(bg_style->flag & S_BACKGROUND_COLOR) /* background color */
		{
		    XSetForeground(display, disp_gc, rgb2color(0, bg_style->r, bg_style->g, bg_style->b,0)); 
		    if(bg_style->flag & S_BACKGROUND_IMAGE)
		    {
			if(!((bg_style->image) && (bg_style->flag & S_BACKGROUND_X_REPEAT) && (bg_style->flag & S_BACKGROUND_Y_REPEAT)))
			    XFillRectangle(display, w, disp_gc,x1-PixelIndent, y1 , width, height);
		    }
		    else
			XFillRectangle(display, w, disp_gc,x1-PixelIndent, y1 , width, height);
		}   
		if(bg_style->flag & S_BACKGROUND_IMAGE) /* bacground image */
		{
		    if(bg_style->image)
		    {
			dirtyscroll = 1;
			
			frameRect.x = x1;
			frameRect.y = y1;
			frameRect.width = (frameRect.x+width < WinLeft+WinWidth) ? width : width+x-frameRect.x;
			frameRect.height = (frameRect.y+height < WinTop+WinHeight) ? height : height+y-frameRect.y;
			bg_pixmap = XCreatePixmap(display, RootWindow(display, screen),WinWidth+WinLeft,WinHeight+WinTop,DefaultDepth(display,screen));
			gc_bg = XCreateGC(display, w,0,NULL);
			XCopyGC(display,gc_fill,0xFFFF,gc_bg);	
			valuemask = GCTile|GCFillStyle|GCGraphicsExposures|GCTileStipXOrigin|GCTileStipYOrigin;
			values.tile = bg_style->image->pixmap;
			values.fill_style = FillTiled;
			values.graphics_exposures = FALSE;
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
			XSetClipRectangles(display, disp_gc, 0, 0, &frameRect, 1, Unsorted);
			XSetClipRectangles(display, gc_fill, 0, 0, &frameRect, 1, Unsorted);
			if(bg_style->flag & S_BACKGROUND_COLOR)
			    XFillRectangle(display, bg_pixmap, disp_gc,x1-PixelIndent, y1 , width, height);
			else
			    XFillRectangle(display, bg_pixmap, gc_fill,x1-PixelIndent, y1 , width, height);
			if(bg_style->flag & S_BACKGROUND_Y_REPEAT)
			{
			    bgRect.y      = WinTop;
			    bgRect.height = WinHeight;
			}
			else
			{
			    bgRect.y      = values.ts_y_origin;
			    bgRect.height = bg_style->image->height; 
			}
			if(bg_style->flag & S_BACKGROUND_X_REPEAT)
			{
			    bgRect.x = WinLeft;
			    bgRect.width = WinWidth;
			}
			else
			{
			    bgRect.x = values.ts_x_origin;
			    bgRect.width = bg_style->image->width;
			}
			XSetClipRectangles(display, gc_bg, 0, 0,&bgRect, 1, Unsorted); 
			XFillRectangle(display, bg_pixmap, gc_bg, x1-PixelIndent, y1 , width, height);
			XSetClipRectangles(display, gc_bg, 0, 0, &frameRect, 1, Unsorted);
		    }
		    else
			if(!(bg_style->flag & (S_BACKGROUND_COLOR))) /* use paper if no pixmap and color */
			   XFillRectangle(display, w, gc_fill, x1-PixelIndent, y1 , width, height);
		}
		if(!(bg_style->flag & (S_BACKGROUND_COLOR |S_BACKGROUND_IMAGE)))
		    XFillRectangle(display, w, gc_fill,x1-PixelIndent, y1 , width, height);
		
	    };
#endif
            c1 = *p++; c2 = *p++; length = c1 | c2<<8;

	    /* printf("BEGIN_FRAME %xh (x1 %d, y1 %d, width %d, height %d, length %d\n",(p-12),x1,y1,width,height,length);  
	     */
	    
         /* call self to paint this frame */
	    
	    if(dirtyscroll && bg_style)
	    {
		PaintFrame(bg_pixmap, p, p + length, x1, y, h);
	    }
		else
		    PaintFrame(w, p, p + length, x1, y, h);
	    p += length+2; /* to skip over frame's contents */
	    if(dirtyscroll && bg_style)
	    {
		XCopyGC(display,gc_fill,0xFFFF,gc_bg);	
		valuemask = GCGraphicsExposures;
		valuemask |= GCTile|GCFillStyle;
		values.tile = bg_style->image->pixmap; 
		values.fill_style = FillTiled;
		values.graphics_exposures = 0;
		XChangeGC(display, gc_bg, valuemask, &values);
		XSetClipRectangles(display, disp_gc, 0, 0, &displayRect, 1, Unsorted);
		XSetClipRectangles(display, gc_fill, 0, 0, &displayRect, 1, Unsorted); 
		XSetClipRectangles(display, gc_bg, 0, 0, &displayRect, 1, Unsorted);
		XCopyArea(display,bg_pixmap,w,gc_bg,frameRect.x,frameRect.y,frameRect.width,frameRect.height,frameRect.x,frameRect.y); 
		XFreePixmap(display,bg_pixmap); 
		XFreeGC(display, gc_bg); 
	    };
	    if (border)
		DrawBorder(border, x+x1-PixelIndent, y1, width, height, 0 /* cb_ix */);
	    continue;
	}

        /* skip end of frame marker */
  
       if (tag == END_FRAME)
        {
	    p += FRAMENDLEN - 1;  /* skip start/size params */
            continue;
        }

        if (tag != TEXTLINE)
        {
	    printf("Unexpected Internal Tag %d at %X\n",tag,(p-1));
	    tag = *p++;
	}

        c1 = *p++; c2 = *p++; offset = c1 | c2<<8;
        c1 = *p++; c2 = *p++; offset |= (c1 | c2<<8) << 16;

        /* we are done if TextLine starts after bottom of window */

        y1 = WinTop + (offset - PixelOffset);
/*
	if (y1 >= y + (int)h)
            break; 
	    */
        c1 = *p++; c2 = *p++; yb = y1 + (c1 | c2<<8);
        c1 = *p++; c2 = *p++; xi = x + (c1 | c2<<8);
        c1 = *p++; c2 = *p++; height = (c1 | c2<<8);

	while ((tag = *p++) != '\0')
        {
            switch (tag & 0xF)
            {
	    case RULE:
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);
                    c1 = *p++; c2 = *p++; x2 = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; yi = c1 | c2<<8;

                    if (tag & HLINE)
                        x2 += x1;

#ifdef STYLE_COLOR
		    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
		    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
/*                    if (tag & GROOVE)*/

		    if (color_text_ix == 255)
                    {
                        XSetForeground(display, disp_gc, windowBottomShadow);
                        XFillRectangle(display, w, disp_gc, x1-PixelIndent, yb-yi, x2-x1, 1);
                        XSetForeground(display, disp_gc, windowTopShadow);
                        XFillRectangle(display, w, disp_gc, x1-PixelIndent, yb+1-yi, x2-x1, 1);
                        XSetForeground(display, disp_gc, textColor);
                    }
                    else {
			XSetForeground(display, disp_gc, ix2color(color_text_ix)); /* howcome 22/5/95 */
                        XFillRectangle(display, w, disp_gc, x1-PixelIndent, yb-yi, x2-x1, 1);
		    }
                        
                    break;

                case LINE:
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);
                    c1 = *p++; c2 = *p++; x2 = xi + (c1 | c2<<8);
                    c1 = *p++; c2 = *p++; y1 = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; y2 = c1 | c2<<8;

#ifdef STYLE_COLOR
                    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
                    GetPointer(&p);
#endif

                    XSetForeground(display, disp_gc, ix2color(color_text_ix));
                    XDrawLine(display, win, disp_gc, x1-PixelIndent, yb-y1, x2, yb-y2);
                    break;

                case BULLET:
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);
                    c1 = *p++; c2 = *p++; depth = (c1 | c2<<8);

                    fnt = *p++;	/* howcome 25/4/95 */
		    fnt_size = *p++;
		    
#ifdef STYLE_COLOR
		    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
		    bg_style=(BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
		    if(bg_style)
			color_background_ix = (bg_style->flag & S_BACKGROUND_COLOR ) ? rgb2ix(0, bg_style->r, bg_style->g, bg_style->b,0) : 255; 
		    else
			color_background_ix = 255;	
#endif
                    SetColor(disp_gc, color_text_ix, color_background_ix);
		    if (depth > 0)
                        XFillRectangle(display, w, disp_gc, x1 - PixelIndent, yb - LineSpacing[fnt]*3/8, LineSpacing[fnt]/4, 2);
                    else
		    {
			XFillRectangle(display, w, disp_gc, x1 - PixelIndent, yb - LineSpacing[fnt]*3/8, LineSpacing[fnt]/5, LineSpacing[fnt]/5);
		    }
                    break;

                case STRING:
                    c1 = *p++; c2 = *p++; emph = c1 | c2<<8;
                    fnt = *p++;
#ifdef STYLE_COLOR
		    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
		    bg_style=(BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
		    if (bg_style)
		    {
			if(!bg_style->image)
			    color_background_ix = (bg_style->flag & S_BACKGROUND_COLOR) ? rgb2ix(0, bg_style->r, bg_style->g, bg_style->b,0) : 255; 
			else
			    color_background_ix = 255; 
		    }	
		    else
			color_background_ix = 255;
		   
#endif
                    yi = *p++ - 128;
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; len = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;
		    str = (long) GetPointer(&p);
		    p += POINTERSIZE; /* skip ref */
		    /*	    printf("howcome reading %x\n", str); */
                    preformatted = (tag & PRE_TEXT);
                    s = CopyLine((char *)str, len);
		    

#ifdef STYLE_COLOR
		    font = fnt;
		    SetColor(disp_gc, color_text_ix, color_background_ix);
		    SetFont(disp_gc, font);
#else
                    if (font != fnt)
                    {
                        font = fnt;
                        SetFont(disp_gc, font);
                    }
		    
#endif
                    if (emph & EMPH_HIGHLIGHT)
                    {
                        XSetForeground(display,disp_gc, labelColor);
                        XDrawString(display, w, disp_gc, x1-PixelIndent, yb-yi, s, len);
                        XSetForeground(display,disp_gc, textColor);
                    }
                    else {
#ifdef STYLE_COLOR
			if (color_background_ix == 255)
#endif
			    XDrawString(display, w, disp_gc, x1-PixelIndent, yb-yi, s, len);
#ifdef STYLE_COLOR
			else
			    XDrawImageString(display, w, disp_gc, x1-PixelIndent, yb-yi, s, len);
#endif
		    }
		    if (emph & EMPH_ANCHOR)
                    {
                        y2 = yb - yi - ASCENT(font) - 1;
                        DrawHButtonUp(x1-PixelIndent, y2, width, LineSpacing[fnt]);
                    }

                    if (emph & EMPH_UNDERLINE)
                    {
                        y2 = yb - yi + DESCENT(font);
                        XFillRectangle(display, w, disp_gc, x1+1-PixelIndent, y2 - LineThickness[font], width-2, LineThickness[font]);
                    }

                    if (emph & EMPH_STRIKE)
                    {
                        y2 = yb - yi - ASCENT(font)/3;
                        XSetForeground(display,disp_gc, strikeColor);
                        XFillRectangle(display, w, disp_gc, x1-PixelIndent, y2 - LineThickness[font], width, LineThickness[font]);
                        XSetForeground(display,disp_gc, textColor);
                    }
                    break;

                case SEQTEXT:
		    c1 = *p++; c2 = *p++; emph = (c1 | c2<<8);
                    fnt = *p++;
#ifdef STYLE_COLOR
		    color_text_ix = *p++;
#endif
#ifdef STYLE_BACKGROUND
		    bg_style=(BG_Style *)GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
		    if (bg_style)
			color_background_ix = (bg_style->flag & S_BACKGROUND_COLOR) ? rgb2ix(0, bg_style->r, bg_style->g, bg_style->b,0) : 255;
		    else
			color_background_ix = 255;	
#endif
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; yi = (c1 | c2<<8);       /* y offset */
                    len = *p++;
		    width = XTextWidth(Fonts[fnt], (char *)p, len);
#ifdef STYLE_COLOR
		    font = fnt;
		    SetColor(disp_gc, color_text_ix, color_background_ix);
		    SetFont(disp_gc, font);

		    if (color_background_ix == 255)
			XDrawString(display, w, disp_gc,
				    x1-PixelIndent, yb-yi, (char *)p, len);
		    else
			XDrawImageString(display, w, disp_gc,
					 x1-PixelIndent, yb-yi,
					 (char *)p, len);

#else
                    if (font != fnt)
                    {
                        font = fnt;
                        SetFont(disp_gc, font);
                    }

                    XDrawString(display, w, disp_gc, x1-PixelIndent,
				yb-yi, (char *)p, len);
#endif
		    /* different emphasis used in maths */

		    /* under effects */
                    if (emph & EMPH_UNDERLINE)
                    {
                        y2 = yb - yi + DESCENT(font);
                        XFillRectangle(display, w, disp_gc,
				       x1+1-PixelIndent,
				       y2 - LineThickness[font], width-2,
				       LineThickness[font]);
                    }
                    if (emph & EMPH_UNDERLARR)
                    {
                        y2 = yb - yi + DESCENT(font);
                        XFillRectangle(display, w, disp_gc,
				       x1+1-PixelIndent,
				       y2 - LineThickness[font], width,
				       LineThickness[font]);
                        SetFont(disp_gc, IDX_SUBSYMFONT);
                        tmp = 60;
                        XDrawString(display, w, disp_gc,
				    x1-PixelIndent, y2+3,
				    (char *)&tmp, 1);
                        SetFont(disp_gc, font);
                    }
                    if (emph & EMPH_UNDERRARR)
                    {
                        y2 = yb - yi + DESCENT(font);
                        XFillRectangle(display, w, disp_gc,
				       x1+1-PixelIndent,
				       y2 - LineThickness[font], width,
				       LineThickness[font]);
                        SetFont(disp_gc, IDX_SUBSYMFONT);
                        tmp = 62;
                        XDrawString(display, w, disp_gc,
				    x1-PixelIndent+width-4, y2+3,
				    (char *)&tmp, 1);
                        SetFont(disp_gc, font);
                    }
                    if (emph & EMPH_UNDERHAT)
                    {
                    }
                    if (emph & EMPH_UNDERTILDE)
                    {
                    }

		    /* OVER effects */
                    if (emph & EMPH_OVERLINE)
                    {
                        y2 = yb - yi + DESCENT(font)/2 - ASCENT(font);
                        XFillRectangle(display, w, disp_gc,
				       x1+1-PixelIndent,
				       y2 - LineThickness[font], width-2,
				       LineThickness[font]);
                    }
                    if (emph & EMPH_OVERLARR)
                    {
                        y2 = yb - yi + DESCENT(font)/2 - ASCENT(font);
                        XFillRectangle(display, w, disp_gc,
				       x1+1-PixelIndent,
				       y2 - LineThickness[font], width-2,
				       LineThickness[font]);
                        SetFont(disp_gc, IDX_SUBSYMFONT);
                        tmp = 60;
                        XDrawString(display, w, disp_gc,
				    x1-PixelIndent, y2+3,
				    (char *)&tmp, 1);
                        SetFont(disp_gc, font);
                    }
                    if (emph & EMPH_OVERRARR)
                    {
                        y2 = yb - yi + DESCENT(font)/2 - ASCENT(font);
                        XFillRectangle(display, w, disp_gc,
				       x1-PixelIndent, y2, width, 1);
                        SetFont(disp_gc, IDX_SUBSYMFONT);
                        tmp = 62;
                        XDrawString(display, w, disp_gc,
				    x1-PixelIndent+width-4, y2+3,
				    (char *)&tmp, 1);
                        SetFont(disp_gc, font);
                    }
                    if (emph & EMPH_OVERHAT)
                    {
                    }
                    if (emph & EMPH_OVERDOT)
                    {
                    }
                    if (emph & EMPH_OVERDDOT)
                    {
                    }
                    if (emph & EMPH_OVERTILDE)
                    {
                    }

                    p += len;
                    break;

                case IMAGE:
                    c1 = *p++; c2 = *p++; y2 = yb - (c1 | c2<<8);
                    c1 = *p++; c2 = *p++; x1 = xi + (c1 | c2<<8);  /* here */
                    c1 = *p++; c2 = *p++; width = c1 | c2<<8;
                    c1 = *p++; c2 = *p++; height = c1 | c2<<8;
		    str = (long)GetPointer(&p);
		   
                    p += POINTERSIZE;  /* skip past buffer pointer */
#ifdef STYLE_BACKGROUND
		    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
		    if (y2 > y + (int)h)
                        break;

                    if (y2 + height < y)
                        continue;
			
                    x1 -= PixelIndent;

                    if (tag & (ISMAP | EMPH_ANCHOR))
                    {
                        DrawOutSet(w, disp_gc, x1, y2, width, height);
                        XSetForeground(display, disp_gc, textColor);
                        DrawHButtonDown(x1+4, y2+2, width-7, height-6);
                        width -= 8;
                        height -= 8;
                        x1 += 4;
                        y2 += 4;
                    }

		    if (str) {
			XCopyArea(display, (Pixmap)str, w, disp_gc, 0, 0, width, height, x1, y2);
		    } else if (!(tag & (ISMAP | EMPH_ANCHOR))){
			XPoint points[3];

			points[0].x = x1;
			points[0].y = y2 + IMG_INDICATOR;
			points[1].x = x1;
			points[1].y = y2;
			points[2].x = x1 + IMG_INDICATOR;
			points[2].y = y2;
			XDrawLines(display, w, disp_gc, points, 3, CoordModeOrigin);

			points[0].x = x1 + width - IMG_INDICATOR;
			points[0].y = y2;
			points[1].x = x1 + width;
			points[1].y = y2;
			points[2].x = x1 + width;
			points[2].y = y2 + IMG_INDICATOR;
			XDrawLines(display, w, disp_gc, points, 3, CoordModeOrigin);

			points[0].x = x1 + width;
			points[0].y = y2 + height - IMG_INDICATOR;
			points[1].x = x1 + width;
			points[1].y = y2 + height;
			points[2].x = x1 + width - IMG_INDICATOR;
			points[2].y = y2 + height;
			XDrawLines(display, w, disp_gc, points, 3, CoordModeOrigin);

			points[0].x = x1 + IMG_INDICATOR ;
			points[0].y = y2 + height;
			points[1].x = x1;
			points[1].y = y2 + height;
			points[2].x = x1;
			points[2].y = y2 + height - IMG_INDICATOR;
			XDrawLines(display, w, disp_gc, points, 3, CoordModeOrigin);
		    }
                    break;

                case INPUT:
		    str = (long)GetPointer(&p);
#ifdef STYLE_BACKGROUND
		    GetPointer(&p); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
                    PaintField(disp_gc, xi, yb, (Field *)str);
                    break;
            }
        }

        ++p; ++p;  /* skip size param to start of next object */
    }

    if (TAG_TRACE && VERBOSE_TRACE)
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr,"PaintFrame: after while p = %p, p_end = %p\n", p, p_end);
#else
	fprintf(stderr,"PaintFrame: after while p = %lx, p_end = %lx\n", p, p_end);
#endif /* PRINTF_HAS_PFORMAT */
}

/* search for anchor point in document */
/* search index for given keywords */

void SearchIndex(char *keywords)
{
#if 0
    char *p, *q;
    /* janet 21/07/95: not used:    int where; */


    p = SearchRef(keywords);

    if (*p)  /* attempt to get new document */
    {
/*        XDefineCursor(display, win, hourglass);*/
/*        XFlush(display);*/

        q = libGetDocument(p, NULL, REMOTE); /* howcome */

        if (q && *q)
        {
            CurrentDoc.offset = PixelOffset;
            PushDoc(CurrentDoc.offset);
            SetBanner(BANNER);

            SetCurrent();
            NewBuffer(q);

            if (IsIndex)
                ClearStatus();

            SetStatusString(NULL);  /* to refresh screen */
        }

        DisplayScrollBar();
        DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
        XUndefineCursor(display, win);
        XFlush(display);
    }
#endif
}

int WindowButtonDown(int x, int y)
{
    int tag, dx, dy;

    /* the window should only be cliackable for certain content_types and modes */

    if (!DocHTML(CurrentDoc))
	return VOID;

    if (CurrentDoc->show_raw)
	return VOID;

    anchor_start = anchor_end = 0;
    tag = 0;

    clicked_element = WhichObject(BUTTONDOWN, x, y, &tag, &anchor_start, &anchor_end, &dx, &dy);

   
    if (tag == TAG_ANCHOR || tag == TAG_IMG) /* redraw anchor if type=image or src= */
    {
        DrawAnchor(&background, 0);
	return WINDOW;
    }

    if (tag == TAG_INPUT || TAG_SELECT)
        /* toggle checked here ??? --SPif 13-oct-95 */
      return WINDOW;
	
    Beep();
    return VOID;
}

/* --Spif must do this right (11-Oct-95) */

void www_encode(char *raw)
{
    int i,rlen;
    char *encoded,*j;

    rlen=strlen(raw);
    encoded=(char *)malloc(3*rlen+1); /* max size = size*3 */
    j=encoded;
    for(i=0;i<rlen;i++)
    {
      if( *(raw+i) ==' ')
	*j++ = '+';       
      else if ((*(raw+i) == '-')||(*(raw+i) == '_'))
	*j++=*(raw+i);
      else if ((*(raw+i) < '0' ) || ((*(raw+i)<'A')&&(*(raw+i)>'9'))){
	*j++ = '%';
	sprintf(j,"%X",*(raw+i));
	j+=2;
      } else if ((*(raw+i) > 'z') || ((*(raw+i)<'a')&&(*(raw+i)>'Z'))){
	*j++ = '%';
	sprintf(j,"%X",*(raw+i));
	j+=2;
      } else 
	*j++=*(raw+i);
    };
    *j=0;
    strcpy(raw,encoded); /* short term hack... must return the pointer */
}


void WindowButtonUp(int shifted, int px, int py)
{
    Byte *object;
    char *start, *end, *href, *name, *link, *value, buf[16], *action;
    int tag, hreflen, namelen, align, ismap, dx, dy, width, height;
    int type, vlen, flags, size, method, alen, nlen;
    Image *image = NULL;
    Form *form = NULL;

    /* the window should only be cliackable for certain content_types and modes */

    if (!DocHTML(CurrentDoc))
	return;

    if (CurrentDoc->show_raw)
	return;

    if (anchor_start && anchor_end)
        DrawAnchor(&background, 1);

    tag=UNKNOWN; /* -- avoid any pb with this --Spif 16-Oct-95 */

    object = WhichObject(BUTTONUP, px, py, &tag, &start, &end, &dx, &dy);
    
    if ((tag == TAG_ANCHOR || tag == TAG_IMG) &&
	start == anchor_start && end == anchor_end)
    {
        if (tag == TAG_IMG)
        {
            bufptr = anchor_start+5;
            ParseImageAttrs(&href, &hreflen, &align, &ismap, &width, &height);
            sprintf(buf, "?%d,%d", dx, dy);
            link = (char *)malloc(hreflen+strlen(buf)+1);
            memcpy(link, href, hreflen);
            link[hreflen] = '\0';
            strcat(link, buf);
        }
        else /* tag == TAG_ANCHOR */
        {
	    int class_len; 
	    char *class = NULL;

            bufptr = anchor_start+3;
            ParseAnchorAttrs(&href, &hreflen, &name, &namelen, &class, &class_len);

            if (dx >= 0 && dy >= 0)
            {
                sprintf(buf, "?%d,%d", dx, dy);
                link = (char *)malloc(hreflen+strlen(buf)+1);
                memcpy(link, href, hreflen);
                link[hreflen] = '\0';
                strcat(link, buf);
            }
            else
            {
                link = (char *)malloc(hreflen+1);
                memcpy(link, href, hreflen);
                link[hreflen] = '\0';
            }
        }
/*
        if (shifted)
        {
            if (CloneSelf())
                OpenDoc(link);
        }
        else */
	OpenDoc(link);

        Free(link);
    }
    else
    { 
        if (tag == TAG_INPUT) /* --Spif FORM 10-Oct-95 */
	{
	    bufptr = start + 7 ;
	    ParseInputAttrs(&type, &name, &nlen, &value, &vlen, &size, &flags, &image);
	    /* backtrack to find the previous <FORM..> */
	    for(;(strncasecmp(bufptr, "<form", 5) != 0)&&(*bufptr);bufptr--);
	    bufptr+=5; 
	    ParseFormAttrs(&method,&action,&alen);  /* must test that a form was found */
	    form = FindForm(forms,action);
	    if(form!=NULL)
	    {
		if (type == RESETBUTTON)
		{/* printf("reset form\n"); --Spif 12-Oct-95 */}; /* reset the whole form */
		
		if (type == SUBMITBUTTON)
		{
		    SubmitForm(form,method,action,alen,type,name,nlen,value,vlen,image,dx,dy,bufptr);
		}
		else
		{
		    /* not a submitbutton  but who cares ? ;)  -- Spif 18-Oct-95*/
		};
	    };
	};
    };
    XFlush(display);
}

