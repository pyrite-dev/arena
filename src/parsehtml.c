/* parsehtml.c - display code for html

ParseHTML() parses the HTML elements and generates a stream of commands for
displaying the document in the Paint buffer. The paint commands specify the
appearence of the document as a sequence of text, lines, and images. Each
command includes the position as a pixel offset from the start of the
document. This makes it easy to scroll efficiently in either direction.
The paint buffer must be freed and recreated if the window is resized.

The model needs to switch to relative offsets to enable the transition
to an wysiwyg editor for html+. Relative values for pixel offsets and
pointers to the html+ source would make it much easier to edit documents
as it would limit revisions to the paint stream to the region changed.

*/

/*

Janne Saarela
janne.saarela@hut.fi
28.7.1995

PrintSeqText() and PutText() now use PushValue() to place
the emph attribute to the PaintStream. The function prototypes
now have emph added as unsigned int.

*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "www.h"
#include "tools.h"
#include "style.h"
#include "forms.h"

#define LBUFSIZE 1024

extern Display *display;
extern int screen;
extern Window win;
extern GC disp_gc, gc_fill;
extern Cursor hourglass;
extern int UsePaper;
extern int debug;  /* controls display of errors */
extern int document;  /* HTMLDOCUMENT or TEXTDOCUMENT */
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
extern BOOL NoStyle;
extern Context *context;
extern unsigned long textColor, labelColor, windowTopShadow,
                     strikeColor, windowBottomShadow, windowShadow, windowColor;

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
/*extern Doc NewDoc, CurrentDoc;*/
extern XFontStruct *pFontInfo;
extern XFontStruct *Fonts[FONTS];
extern int LineSpacing[FONTS], BaseLine[FONTS], StrikeLine[FONTS];
extern int ListIndent1, ListIndent2;
extern Frame background;
extern Image *images, *note_image, *caution_image, *warning_image;
extern Form *forms;
extern Doc *CurrentDoc;

char *bufptr;  /* parse position in the HTML buffer */
char *lastbufptr;  /* keep track of last position to store delta's */

Byte *TopObject;  /* first visible object in window */
Byte *paint; /* holds the sequence of paint commands */
int paintbufsize;     /* size of buffer, not its contents */
int paintlen;         /* where to add next entry */

int paintStartLine;   /* where line starts in the paint stream */
int above;            /* above baseline */
int below;            /* below baseline */
int voffset;          /* if positive then above baseline */
int IdAttributeFlag;  /* signals attribute is ID value */

int error;            /* set by parser */
int prepass;          /* true during table prepass */
int html_width;       /* tracks maximum width */
int min_width, max_width; /* table cell width */
int list_indent;
int damn_table=0; /* to debug table formatter */
extern int preformatted;
extern int font;  /* index into Fonts[] array */

static int EndTag, TagLen;
static int TokenClass, TokenValue, Token;
/* janet: not used: static char *EntityValue; */

unsigned int ui_n;
unsigned long ul_n;
int baseline;       /* from top of line */
long TermTop, TermBottom;

long PixOffset;     /* current offset from start of document */
long PrevOffset;    /* keep track for saving delta's */
long LastLIoffset;  /* kludge for <LI><LI> line spacing */
long ViewOffset;    /* for toggling between HTML/TEXT views */

extern long IdOffset;      /* offset for targetId */
extern char *targetptr;    /* for toggling view between HTML/TEXT views */
extern char *targetId;     /* for locating named Id during ParseHTML() */

int Here, TextLineWidth;
int HTMLInit = 0;
Image *start_figure, *figure;
long figEnd;
Form *form;

int LeftFlowMargin, RightFlowMargin;
int LeftMarginIndent = 0, RightMarginIndent = 0;
long FigureEnd = 0;

int class_len = 0;
char *class = NULL;
/* BOOL initial_cap = FALSE;*/

char *LastBufPtr, *StartOfLine, *StartOfWord; /* in HTML document */
static int LineLen, LineWidth, WordStart, WordWidth;
static char LineBuf[LBUFSIZE]; /* line buffer */

static char *Ones[] = {"i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
static char *Tens[] = {"x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
static char *Hundreds[] = {"c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};

char small_caps[256] = {   0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 
			  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
			  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
			  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
			  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
			  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
			  96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
			  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
			 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
			 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
			 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
			 176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
			 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
			 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
			 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
			 208,209,210,211,212,213,214,247,216,217,218,219,220,221,222,255 }; 

/* push 16 bit value onto paint buffer */

#define PushValue(p, value) ui_n = (unsigned int)value; *p++ = ui_n & 0xFF; *p++ = (ui_n >> 8) & 0xFF
#define Push32(p, value) ul_n = (unsigned long)value; *p++ = ul_n & 0xFF; *p++ = (ul_n >> 8) & 0xFF; *p++ = (ul_n >> 16) & 0xFF; *p++ = (ul_n >> 24) & 0xFF

/* expand paint stream to fit len bytes */

Byte *MakeRoom(int len)
{
    Byte *p;

    if (paintlen > paintbufsize - len)
    {
        paintbufsize = paintbufsize << 1;
        paint = (Byte *)realloc(paint, paintbufsize);
    }

    p = paint + paintlen;
    paintlen += len;
    return p;
}


void PrintBeginFrame(Frame *frame)
{
    Byte *p;
    /* janet: not used:    unsigned int len; */
    long offset;

    if (!prepass)
    {
        p = MakeRoom(FRAMESTLEN);
        frame->info = (p - paint);
        offset = frame->offset;
        *p++ = BEGIN_FRAME;
        PushValue(p, offset & 0xFFFF);
        PushValue(p, (offset >> 16) & 0xFFFF);
        PushValue(p, frame->indent);
        PushValue(p, frame->width);
        PushValue(p, 0);        /* subsequently filled in with height(1) */
	PushValue(p, 0);       /* subsequently filled in with height(2) */
        *p++ = frame->style;    /* frame's background style */
        *p++ = frame->border;   /* frame's border style */
#ifdef STYLE_COLOR_BORDER
        *p++ = frame->cb_ix;    /* frame's foreground color index */
#endif

#ifdef STYLE_BACKGROUND
	PutPointer(&p,(void *)StyleGet(SI_BACKGROUND));
#endif

        PushValue(p, 0);        /* subsequently filled in with length */
    }
}

/* the size field after end of frame contents */
void PrintFrameLength(Frame *frame)
{
    Byte *p;
    unsigned int len;

    if (!prepass)
    {
     /* write the length field in frame's header */
        p = paint + frame->info + FRAMESTLEN - 2 ; /* --Spif 19-Oct-95 grmph !!! position of lenght in the paint stream */
        PushValue(p, frame->length);

     /* write the size field after frame's contents */
	p = MakeRoom(2); 
        len = p - paint - frame->info; /* --Spif 19-Oct-95 must see this */
	PushValue(p, len); 
    }
}

/* marker for pixel offset to end of frame */
void PrintEndFrame(Frame *parent, Frame *frame)
{
    Byte *p;
    unsigned int len;

    if (!prepass)
    {
        p = MakeRoom(FRAMENDLEN);
        len = p - (paint + frame->info);
        *p++ = END_FRAME;
        PushValue(p, len);
        PushValue(p, FRAMENDLEN-2);

     /* increase width of parent frame if necessary */

        if (parent)
        {
            len = frame->indent + frame->width + 2;

            if (parent->width < len)
                parent->width = len;

        /* and restore parent frame margins if this frame
           was involved in a text flow indent and is not
           obstructed by a subsequent text flow frame */

            /* howcome 7/10/94: prolonged the "if parent" block */

            if (frame->flow == ALIGN_LEFT)
                {
                    if (frame->pushcount == parent->leftcount)
                        {
                            parent->leftcount -= 1;
                            parent->leftmargin = frame->oldmargin;
                        }
                }
            else if (frame->flow == ALIGN_RIGHT)
                {
                    if (frame->pushcount == parent->rightcount)
                        {
                            parent->rightcount -= 1;
                            parent->rightmargin = frame->oldmargin;
                        }
                }
            else if (frame->flow == ALIGN_NOTE)
                {
                    if (frame->pushcount == parent->leftcount)
                        parent->leftcount -= 1;
                }
        }

    }
}

/* Write end markers for all peer frames and children.
   Note that the lists are flushed in the same order
   that items were pushed onto the list */
void FlushAllFrames(Frame *parent, Frame *frame)
{
    if (frame)
    {
        FlushAllFrames(parent, frame->next);
	frame->next = NULL;
        FlushAllFrames(frame, frame->child);
	frame->child = NULL;
        PrintEndFrame(parent, frame);
        FreeFrames(frame);
	frame = NULL;
    }
}

/*
 Write end markers for any frames in peer list which have a
 finishing offset <= PixOffset. For any such frames, all
 descendant frames are flushed first. The process frees
 frames and removes them from the list of peers.

 If frame is the current frame then this procedure
 should be invoked as:

    frame->child = FlushFrames(frame, frame->child);

 This procedure assumes that BeginFrame() pushes
 new frames onto the front of the list of children,
 and guarantees that frames with the same offset are
 flushed to the paint stream in the order they were
 created. This property is needed for display purposes.
*/

Frame *FlushFrames(Frame *parent, Frame *frame)
{
    Frame *next;

    if (frame)
    {
        next = FlushFrames(parent, frame->next);

        if (frame->offset <= PixOffset)
        {
         /* first flush frame's children */
            FlushAllFrames(frame, frame->child);
	    frame->child = NULL;

         /* and now the frame itself */
            PrintEndFrame(parent, frame);
            Free(frame);
	    frame = NULL;
            return next;
        }

        frame->next = next;
    }

    return frame;
}

void FlushPending(Frame *frame)
{
    if (frame && frame->child)
        frame->child = FlushFrames(frame, frame->child);
}

/* 
 The frame is created here, the new frame is returned so that the
 parser can later call EndFrame() at the end of the frame. Any frames
 which end before PixOffset are first flushed.
*/

Frame *BeginFrame(Frame *parent, int style, int border, int left, int right, BG_Style *bg)
{
    Frame *frame;

    FlushPending(parent);

 /* create frame and write begin frame marker */
    frame = (Frame *)calloc(1, sizeof(Frame));
    memset(frame, 0, sizeof(Frame));
    frame->offset = PixOffset;
    frame->indent = (parent ?  parent->leftmargin + left : left);
    frame->width = right - left;
    frame->style = style;
    frame->border = border;
#ifdef STYLE_COLOR_BORDER
    frame->cb_ix = (int)bg;
#else
    frame->cb_ix = 0;
#endif
    frame->flow = ALIGN_CENTER; /* implies this is a figure frame */
    frame->next = frame->child = NULL;
    frame->box_list = NULL;
    PrintBeginFrame(frame);
    return frame;
}

/* This writes the frame's height in the frame's header.
   Here we need to push the frame onto the front of the
   list of the parent frame's children so that FlushFrames()
   can write the end frame marker to the paint queue */
void EndFrame(Frame *parent, Frame *frame)
{
    Byte *p;
 /* janet: not used: Frame *next,  *prev; */

 /* update background.height if needed */

    if (PixOffset > background.height)
        background.height = PixOffset;

 /* write height into paint struct for frame */

    frame->height = PixOffset - frame->offset;
    p = paint + frame->info + 9; /* --Spif 19-Oct-95 offset in frame to height(1) and (2) */
    PushValue(p, frame->height & 0xFFFF);
    PushValue(p, (frame->height >> 16) & 0xFFFF);

 /* change frame->offset to end of frame */
    frame->offset = PixOffset;

 /* and now push onto list of children */
    frame->next = parent->child;
    parent->child = frame;
}

int ListCells(Frame *cells)
{
    int n;

    for (n = 0; cells != NULL; ++n)
    {
#if defined PRINTF_HAS_PFORMAT
        printf("address = %p, indent = %d, width = %d, height = %ld\n",
            cells, cells->indent, cells->width, cells->height);
#else
        printf("address = %lx, indent = %d, width = %d, height = %ld\n",
            cells, cells->indent, cells->width, cells->height);
#endif /* POINTER_IS_64BIT */
        cells = cells ->next;
    }

    return n;
}

/*
 Insert cell at end of list of cells
*/
void /* wm 19.Jan.95 */
InsertCell(Frame **cells, Frame *cell)
{
    Frame *frame, *next;

    frame = *cells;
    cell->next = NULL;

    if (frame == NULL)
        *cells = cell;
    else
    {
        for (frame = *cells;;)
        {
            next = frame->next;

            if (next == NULL)
            {
                frame->next = cell;
                break;
            }

            frame = next;
        }
    }
}

/*
 This routine adjusts height of all table cells which end
 on this row and then calls EndFrame() to move them to
 the list of frames awaiting PrintEndFrame()
*/

void FlushCells(Frame *parent, int row, Frame **cells)
{
    Frame *prev, *frame, *next;

    prev = NULL;
    frame = *cells;

    while (frame)
    {
        if (frame->lastrow <= row)
        {
            next = frame->next;

            if (prev)
                prev->next = next;
            else
                *cells = next;

            frame->height = PixOffset - frame->offset;
            frame->next = NULL;
            EndFrame(parent, frame);
            frame = next;
            continue;
        }

        prev = frame;
        frame = frame->next;
    }
}

/* insert TEXTLINE container */

void TextLineFrame(Frame *frame)
{
    Byte *p;

    if (prepass)
    {
        paintStartLine = 0;
        TextLineWidth = 0;
    }
    else
    {
        FlushPending(frame);
        TextLineWidth = 0;
        p = MakeRoom(TXTLINLEN);
        paintStartLine = p - paint;
        *p++ = TEXTLINE;
    
        PushValue(p, PixOffset & 0xFFFF);
        PushValue(p, (PixOffset >> 16) & 0xFFFF);

 /* baseline & indent set at end of line by EndOfLine() */

        *p++ = 0; *p++ = 0;
        *p++ = 0; *p++ = 0;
	*p++ = 0; *p++ = 0;
    }
}

/*
    This procedure writes the end of text line element
    and the baseline, line height and indent as appropriate
    for the frame's current margin settings. This handles
    horizontal alignment in the face of changing margins.
*/
void EndOfLine(Frame *frame, int align)
{
    unsigned int n, height, w;
    int indent, delta = 0, len;
    Byte *p;

    if (!prepass)
	StyleSetFlag(S_LEADING_FLAG,FALSE);

    if (paintStartLine >= 0)
    {
        /* fill in baseline for current line */

        if (!prepass)
        {
	    if (frame == &background)
            {
/*                w = background.width - background.rightmargin;*/
                w = background.width - background.rightmargin - (int)StyleGet(SI_MARGIN_RIGHT);  /* howcome 26/2/95 */

                if (w > WinWidth - 4)
                    w = WinWidth - 4;

/*                w -= background.leftmargin;*/
                w -= background.leftmargin;

                if (align == ALIGN_LEFT)
                    delta = 0;
                else if (align == ALIGN_CENTER)
                    delta = (w - TextLineWidth) / 2;
                else if (align == ALIGN_RIGHT)
                    delta = w - TextLineWidth;
		else if (align == ALIGN_JUSTIFY)
		    delta = 0;
            }
            else
            {
                if (align == ALIGN_LEFT)
                    delta = 0;
                else if (align == ALIGN_CENTER)
                    delta = (frame->width /* - frame->leftmargin
                               - frame->rightmargin*/ - TextLineWidth) / 2;
                else if (align == ALIGN_RIGHT)
                    delta = frame->width/* - frame->leftmargin
                               - frame->rightmargin*/ - TextLineWidth;
		else if (align == ALIGN_JUSTIFY)
		    delta = 0;
	    }

            indent = (delta > 0 ? frame->leftmargin + delta : frame->leftmargin);
            height = above + below;

            paint[paintStartLine + 5] = (above & 0xFF);
            paint[paintStartLine + 6] = (above >> 8) & 0xFF;
            paint[paintStartLine + 7] = (indent & 0xFF);
            paint[paintStartLine + 8] = (indent >> 8) & 0xFF;
            paint[paintStartLine + 9] = (height & 0xFF);
            paint[paintStartLine + 10] = (height >> 8) & 0xFF;

	    p = MakeRoom(3);
            *p++ = '\0';  /* push end of elements marker */

            /* and write text line frame length */

            n = p - paint - paintStartLine;
            PushValue(p, n);
        }
	
	if(frame->flow == ALIGN_JUSTIFY) 
 	    len = LineWidth + frame->leftmargin;
 	else
 	    len = TextLineWidth + frame->leftmargin; 
  
        if (len > frame->width)
                frame->width = len;

        PixOffset += above + below;
        paintStartLine = -1;
        above = 0;
        below = 0;
        TextLineWidth = 0;
#if 0
        EndFigure();
#endif
    }

#if 0
    if (start_figure)
        PrintFigure(BEGIN_FIG);
#endif
}

/* push horizontal rule onto paint stream */

void PrintRule(Frame *frame, int type, int left, int right, int dy)
{
    Byte *p;
    /* not used:     int x; */

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (type == HLINE)
    {
        if (!prepass)
        {
            p = MakeRoom(RULEFLEN);
            *p++ = (RULE | type);
            PushValue(p, left);
            PushValue(p, right);
            PushValue(p, dy);
#ifdef STYLE_COLOR
	    *p++ = (Byte) StyleGet(SI_COLOR);
#endif
#ifdef STYLE_BACKGROUND
	    PutPointer(&p,(void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
            above = max(above, 2);
            below = max(below, 2);
        }
    }
    else
    {
        if (frame == &background)
        {
            if (background.width - right > WinWidth - right)
                right += background.width - WinWidth;
        }

        right += frame->rightmargin;

        if (frame->width - right > TextLineWidth)
            TextLineWidth = frame->width - right;

/*
        above = max(above, 3);
        below = max(below, 5);
*/
        above = max(above, (int)StyleGet(SI_MARGIN_TOP));
        below = max(below, (int)StyleGet(SI_MARGIN_BOTTOM));

        if (!prepass)
        {
	    p = MakeRoom(RULEFLEN);
            *p++ = (RULE | type);
            PushValue(p, left);
            PushValue(p, (frame->width - right));
            PushValue(p, dy);
#ifdef STYLE_COLOR
	    *p++ = (Byte) StyleGet(SI_COLOR);
#endif
#ifdef STYLE_BACKGROUND
	    PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
        }
    }
}


void PrintLine(Frame *frame, int left, int right, int top, int bottom)  /* staalesc 13/12/95 */
{
    Byte *p;

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (!prepass)
    {
        p = MakeRoom(LINEFLEN);
        *p++ = (LINE);
        PushValue(p, left);
        PushValue(p, right);
        PushValue(p, top);
        PushValue(p, bottom);
#ifdef STYLE_COLOR
      *p++ = (Byte) StyleGet(SI_COLOR);
#endif
#ifdef STYLE_BACKGROUND
      PutPointer(&p, (void *)StyleGet(SI_BACKGROUND));
#endif

        above = max(above, 2); /* staalesc: Are these correct??? */
        below = max(below, 2);
    }
}

/* push bullet onto paint stream */

void PrintBullet(Frame *frame, int depth, int font, int size)
{
    Byte *p;

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (Here + B_SIZE> TextLineWidth)
        TextLineWidth = Here + B_SIZE;

    above = max(above, ASCENT(font));
    below = max(below, DESCENT(font));

    if (!prepass)
    {
        p = MakeRoom(BULLETFLEN);
        *p++ = BULLET;
        PushValue(p, Here);
        PushValue(p, depth);

	*p++ = font; /*StyleGet(SI_FONT);*/ /* howcome 25/4/95: adding vertical position of bullet item */
	*p++ = size; /*StyleGet(SI_FONT_SIZE);*/ /* spif 8/1/96: size... */
	*p++ = (Byte) StyleGet(SI_COLOR);
#ifdef STYLE_BACKGROUND
 	PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
    }
}

void WrapImageIfNeeded(Frame *frame, int align,
                       int left, int right, int width, int up, int down)
{
    if (!preformatted)
    {
        if (Here > left && Here + width + 2 > frame->width - frame->rightmargin - right)
        {
            EndOfLine(frame, align);

            Here = left;
            above = up;
            below = down;
        }
        else
        {
            if (up > above)
                above = up;

            if (down > below)
                below = down;
        }
    }
    else
    {
        if (up > above)
            above = up;

        if (down > below)
            below = down;
    }
}

void WrapFieldIfNeeded(Field *field, Frame *frame, int align, int left, int right)
{
    if (!preformatted)
    {
        if (Here > left && field->x + field->width > frame->width - frame->rightmargin - right)
        {
            EndOfLine(frame, align);
            Here = left;
            field->x = Here;
            above = field->above;
            below = field->height - above;
        }
        else
        {
            if (above < field->above)
                above = field->above;

            if (below < field->height - field->above)
                below = field->height - above;
        }
    }
    else
    {
        if (above < field->above)
            above = field->above;

        if (below < field->height - field->above)
            below = field->height - field->above;
    }
}

/* push text input field */

void PrintInputField(Frame *frame, Field *field)
{
    Byte *p;

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (field->x + field->width > TextLineWidth)
        TextLineWidth = field->x + field->width;

    if (!prepass)
    {
        p = MakeRoom(INPUTFLEN);
        field->object = p - paint;
        *p++ = INPUT;
	PutPointer(&p,(void *)field);
#ifdef STYLE_BACKGROUND
	PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
    }
}

/* push normal or preformatted string */
/* emph contains font and emphasis */

void RealPrintString(Frame *frame, unsigned int emph, int font,
                    int delta, char *ref, char *buf, int len, int width, BOOL tight_font, Byte text_color, BG_Style *text_background)
{
    Byte *p;

/*    int indent; */

/*    indent = Here + LeftMarginIndent;*/ /* figures on LHS */
/*    indent = Here + style->margin_left;*/ /* howcome 21/2/95 */

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (!prepass)
        p = MakeRoom(STRINGFLEN);

    if (Here + width > TextLineWidth)
        TextLineWidth = Here + width;


    /* In the case of a big initial, the descent should be calculated
       specifically for the character in question */

    if (tight_font) {
	above = max(above, Fonts[font]->per_char[*buf].ascent);
/*
	if (above == 0)
	    above = max(above, ASCENT(font));
*/
	below = max(below, Fonts[font]->per_char[*buf].descent);
/*
	if (below == 0)
	    below = max(below, DESCENT(font));
*/
    } else {
	if (!StyleGetFlag(S_LEADING_FLAG))
	    above = max(above, ASCENT(font) + (int)StyleGet(SI_LINE_HEIGHT));
/*	    above = (int)StyleGet(SI_LINE_HEIGHT); */
	else
	    above = max(above, ASCENT(font));
	below = max(below, DESCENT(font)); /* howcome 28/8/95: is this the right place to add leading? */
    }                                      /* --Spif 13-Nov-95 this was not the right place ;) */

    if (!prepass)
    {
        *p++ = (preformatted ? (STRING | PRE_TEXT) : STRING);
	PushValue (p, emph);
        *p++ = font;

	if ( StyleGetFlag(S_INDENT_FLAG))
	{
	    /*   Here += StyleGet(S_INDENT); */
	    Here += StyleGetFlag(S_INDENT_FLAG); /* flag carrying a value... hum... --Spif 14-Nov-95 */
	    StyleSetFlag(S_INDENT_FLAG,FALSE);
	};
#if 0
	if (StyleGetFlag(S_MARGIN_TOP_FLAG)) /* this is the right place for this, but it doesn't work... yet ;) --Spif 16-Jan-96 */
	{
	    PixOffset += StyleGetFlag(S_MARGIN_TOP_FLAG);
	    StyleSetFlag(S_MARGIN_TOP_FLAG, FALSE);
	};
#endif
#ifdef STYLE_COLOR
	*p++ = text_color;
#endif
#ifdef STYLE_BACKGROUND
	PutPointer(&p,(void *)text_background); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
        *p++ = delta + 128;  /* baseline offset 128 is zero, 255 is +127, 0 is -128 */

/*
	if (frame->leftcount) {
	    PushValue(p, Here - style->margin_left);
	} else {
*/
	    PushValue(p, Here);
/*	}*/

        PushValue(p, len);
        PushValue(p, width);
	PutPointer(&p, (void *)buf);
	PutPointer(&p, (void *)ref);
    }
}


void PrintString(Frame *frame, unsigned int emph, int font,
                    int delta, char *buf, int len, int width, int wrapped)
{
    int i, j, k, fix, index, icw = 0;
    int sc_width,dumpint; /* --Spif 5/10/95 small caps fix */
    int OldTextLineWidth, OldHere; 
    BOOL emph_set = FALSE;
    BOOL ok;
    char dump_char[2];

    fix = (int)StyleGet(SI_FONT);

#if 0
    if ( ((int)StyleGet(S_TEXT_EFFECT) == TEXT_EFFECT_INITIAL_CAP) && (StyleGetFlag(S_INITIAL_FLAG)) ) {
	int alt_fix = (int)StyleGet(S_ALT_FONT);
	
	RealPrintString(frame, (emph_set ? 0 : emph), alt_fix, delta, buf, buf, 1, width, 
			TRUE, (Byte) StyleGet(S_ALT_COLOR), (BG_Style *)StyleGet(S_ALT_BACKGROUND) );
	emph_set = TRUE;
	icw = XTextWidth(Fonts[alt_fix], buf, 1);
	Here += icw;
	StyleSetFlag(S_INITIAL_FLAG, False);
	buf++; len--;
    }
    StyleSetFlag(S_FONT_LEADING,FALSE);
#endif

    if ((int)StyleGet(SI_FONT_STYLE_SMALL_CAPS)) {
	int sc_fix = /*(int)StyleGet(S_SMALL_CAPS_FONT);*/
	              GetFont((char *)StyleGet(SI_FONT_FAMILY), (int)StyleGet(SI_FONT_SIZE) *4 / 5,
			      (int)StyleGet(SI_FONT_WEIGHT),
			      (int)StyleGet(SI_FONT_STYLE_SLANT), True);

	for(i=k=0; k<len; i++, k++) {
	    sc_width=CompoundTextWidth(buf + i, 0, 1,&dumpint) + (int)StyleGet(SI_LETTER_SPACING);
	    OldTextLineWidth = TextLineWidth ;    /* --Spif 6-Oct-95 keep this to update TextLineWidth with the right width */     
	    OldHere = Here;
	    /* we must give the entire width if it is emphasized, to tell the paint stream to display a beeautiful box around
	       the text, but the real TextLineWidth must be calculated with the width of only ONE character at a time */

	    if (buf[i]=='&') {
		for(ok= FALSE,j=i; !ok && j<len; j++)
	       		    ok = ((buf[j]==';') || (buf[j]==' '));  /* ' ' is here for dummy html writers ;) */		
		index = *dump_char = (char)entity(buf+i+1, &j);
		if (index < 0)
		    index+=256; 
		dump_char[1]=0;
 		RealPrintString(frame, (emph_set ? 0 : emph), sc_fix, delta, buf, &small_caps[index], 1, width,  /* how can we capitalize entities? */
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
		emph_set = TRUE;
		if(((index + small_caps[index]) % 256))
		    Here += XTextWidth(Fonts[sc_fix], dump_char,1) + (int)StyleGet(SI_LETTER_SPACING);
		else
		    Here += XTextWidth(Fonts[fix], dump_char,1) + (int)StyleGet(SI_LETTER_SPACING);
		i += j -1;
	    } else if (islower(buf[i])) {
		RealPrintString(frame, (emph_set ? 0 : emph), sc_fix, delta, buf, &small_caps[(unsigned char)buf[i]], 1, width,
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
		emph_set = TRUE;
 		Here += XTextWidth(Fonts[sc_fix], &small_caps[(unsigned char)buf[i]], 1) + (int)StyleGet(SI_LETTER_SPACING); 
	    } else if (isdigit(buf[i])) {
		RealPrintString(frame, (emph_set ? 0 : emph), sc_fix, delta, buf, &small_caps[(unsigned char)buf[i]], 1, width, 
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
		emph_set = TRUE;
		Here += XTextWidth(Fonts[sc_fix], &small_caps[(unsigned char)buf[i]], 1) + (int)StyleGet(SI_LETTER_SPACING); 
	    } else {
		RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, &small_caps[(unsigned char)buf[i]], 1, width, 
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
		emph_set = TRUE;
		Here += XTextWidth(Fonts[fix], &small_caps[(unsigned char)buf[i]], 1) + (int)StyleGet(SI_LETTER_SPACING); 
	    }
	    if( OldHere + sc_width > OldTextLineWidth)  /* -- Spif: 6-Oct-95 small caps fix */ 
	        TextLineWidth = OldHere + sc_width;
	 };
	TextLineWidth -= (int)StyleGet(SI_LETTER_SPACING);
	Here -= (int)StyleGet(SI_LETTER_SPACING);
    } else if(StyleGet(SI_LETTER_SPACING))
    {
	for(i=k=0; k<len; i++, k++) {
	    sc_width=CompoundTextWidth(buf + i, 0, 1,&dumpint) + (int)StyleGet(SI_LETTER_SPACING); /* a hack... rename this stuff */
	    OldTextLineWidth = TextLineWidth ;  
	    OldHere = Here;
	    /* we must give the entire width if it is emphasized, to tell the paint stream to display a beautiful box around
	       the text, but the real TextLineWidth must be calculated with the width of only ONE character at a time */
	    
	    if (buf[i]=='&') {
		for(ok= FALSE,j=i; !ok && j<len; j++)
		    ok = ((buf[j]==';') || (buf[j]==' '));  /* ' ' is here for dummy html writers ;) */		
		RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf + i, 1, width,  /* how can we capitalize entities? */
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
		emph_set = TRUE;
		*dump_char = entity(buf+i+1, &j);
		dump_char[1]=0;
 		Here += XTextWidth(Fonts[fix], dump_char,1) + (int)StyleGet(SI_LETTER_SPACING);
		i += j -1;
	    } 
	    else {
		RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf + i, 1, width, 
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
		emph_set = TRUE;
		Here += XTextWidth(Fonts[fix], buf + i, 1) + (int)StyleGet(SI_LETTER_SPACING); 
	    }
	    if( OldHere + sc_width > OldTextLineWidth)
	        TextLineWidth = OldHere + sc_width;
	};
	TextLineWidth -= (int)StyleGet(SI_LETTER_SPACING);
	Here -= + (int)StyleGet(SI_LETTER_SPACING);
    } else {
	if(frame->flow != ALIGN_JUSTIFY)
	    RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf, len, width, 
			    FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
	else
	{
	    int nb_space = 0;
	    int remaining_space;
	    
	    
	    for(k=0; k<len; k++)
		nb_space += (buf[k]==' ')||(buf[k]=='\n');
	    if(!nb_space || !wrapped){
		RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf, len, width, 
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
	    }
	    else
	    {
		remaining_space = frame->width - width - Here - StyleGetFlag(S_INDENT_FLAG);
/*#if 1*/
/*		printf("len %d :nbspace %d : remaining_space %d\n", len, nb_space, remaining_space);
 *		printf("width %d, frame->width %d, frame->indent %d, Here %d\n", width, frame->width, frame->indent, Here);
 */ 
		for(i=k=0; k<len; i++, k++) {
		    if(buf[i]=='\n')
			sc_width=XTextWidth(Fonts[fix], " ", 1) + (remaining_space/(nb_space ? nb_space :1));
		    else
			sc_width=CompoundTextWidth(buf + i, 0, 1,&dumpint) + (*(buf+i)==' ')*(remaining_space/(nb_space ? nb_space :1));
		    OldTextLineWidth = TextLineWidth ;  
		    OldHere = Here;
		    /* we must give the entire width if it is emphasized, to tell the paint stream to display a beeautiful box around
		       the text, but the real TextLineWidth must be calculated with the width of only ONE character at a time */
		    
		    if (buf[i]=='&') {
			for(ok= FALSE,j=i; !ok && j<len; j++)
			    ok = ((buf[j]==';') || (buf[j]==' '));  /* ' ' is here for dummy html writers ;) */		
			RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf + i, 1, width,  /* how can we capitalize entities? */
					FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
			emph_set = TRUE;
			*dump_char = entity(buf+i+1, &j);
			dump_char[1]=0;
			Here += XTextWidth(Fonts[fix], dump_char,1);
			i += j -1;
		    } 
		    else {
			if(buf[i]=='\n')
			{
			    emph_set = TRUE;
			    Here += XTextWidth(Fonts[fix], " ", 1) + (remaining_space/(nb_space ? nb_space :1)); 
			    remaining_space -= (remaining_space/(nb_space ? nb_space :1));
			    nb_space--;
			}
			else
			{
			    RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf + i, 1, width, 
					    FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
			    emph_set = TRUE;
			    Here += XTextWidth(Fonts[fix], buf + i, 1) + (*(buf+i)==' ')*(remaining_space/(nb_space ? nb_space :1)); 
			    if(*(buf+i)==' ')
			    {
				remaining_space -= (remaining_space/(nb_space ? nb_space :1));
				nb_space--;
			    }
			}
		    }
		    if( OldHere + sc_width > OldTextLineWidth)
			TextLineWidth = OldHere + sc_width;
		}
#if 0
		RealPrintString(frame, (emph_set ? 0 : emph), fix, delta, buf, buf, len, width, 
				FALSE, (Byte) StyleGet(SI_COLOR), (BG_Style *)StyleGet(SI_BACKGROUND));
#endif
	    }
	}
	emph_set = TRUE;
        Here += (width - icw);
    }
}
 



int CompoundTextWidth(char *s, int start, int len, int *space_p)
{
    int width = 0, i, k, j, ok;
    int fix = (int)StyleGet(SI_FONT);
    char dump_char[2];


#if 0
    if (((int)StyleGet(S_TEXT_EFFECT) == TEXT_EFFECT_INITIAL_CAP) && (StyleGetFlag(S_INITIAL_FLAG))) {
	int alt_fix = (int)StyleGet(S_ALT_FONT);

	width += XTextWidth(Fonts[alt_fix], s, 1);
	start++; len--;
    }
#endif

    if ((int)StyleGet(SI_FONT_STYLE_SMALL_CAPS)) {
	int sc_fix = /*(int)StyleGet(S_SMALL_CAPS_FONT);*/
	    GetFont((char *)StyleGet(SI_FONT_FAMILY), (int)StyleGet(SI_FONT_SIZE) *4 / 5,
		    (int)StyleGet(SI_FONT_WEIGHT),
		    (int)StyleGet(SI_FONT_STYLE_SLANT), True);

	for(i=k=start; k < (len + start); i++,k++) {
	    width += (int)StyleGet(SI_LETTER_SPACING);
	    if (islower(s[i]))
		width += XTextWidth(Fonts[sc_fix], &small_caps[s[i]], 1);
	    else if (s[i]=='&') {
		for(ok= FALSE,j=i; !ok && j<(len+start); j++)
		    ok = ((s[j]==';') || (s[j]==' ')); 
		*dump_char = entity(s+i+1, &j);
		dump_char[1]=0;
		i+=j-1;
		width += XTextWidth(Fonts[sc_fix], dump_char, 1);
	    } else if (isdigit(s[i])) {
		width += XTextWidth(Fonts[sc_fix], s + i, 1);
	    } else
		width += XTextWidth(Fonts[fix], s + i, 1);
	}
	*space_p = XTextWidth(Fonts[sc_fix], " ", 1) + (int)StyleGet(SI_LETTER_SPACING);
	return width;
    } else if(StyleGet(SI_LETTER_SPACING)) {
	for(i=k=start; k < (len + start); i++,k++) {
	    width += (int)StyleGet(SI_LETTER_SPACING);
	    if (s[i]=='&') {
		for(ok= FALSE,j=i; !ok && j<(len+start); j++)
		    ok = ((s[j]==';') || (s[j]==' ')); 
		*dump_char = entity(s+i+1, &j);
		dump_char[1]=0;
		i+=j-1;
		width += XTextWidth(Fonts[fix], dump_char, 1);
	    } else
		width += XTextWidth(Fonts[fix], s + i, 1);
	}
	*space_p = XTextWidth(Fonts[fix], " ", 1) + (int)StyleGet(SI_WORD_SPACING); /* WORD_SPACING instead */
	return width;
    } else {
	*space_p = XTextWidth(Fonts[fix], " ", 1);
	return (width + XTextWidth(Fonts[fix], s + start, len));
    }
}


/* Push explicit text onto paint stream */

void PrintSeqText(Frame *frame, unsigned int emph, int font, char *s, int width)
{
    Byte *p;
    int len;

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (!prepass)
    {
        len = strlen(s);
        p = MakeRoom(SEQTEXTFLEN(len));

        if (Here + width > TextLineWidth)
            TextLineWidth = Here + width;

        above = max(above, ASCENT(font));
        below = max(below, DESCENT(font));

        *p++ = SEQTEXT;
	PushValue (p, emph);
        *p++ = font;
#ifdef STYLE_COLOR
	*p++ = (Byte) StyleGet(SI_COLOR);
#endif
#ifdef STYLE_BACKGROUND
  	PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
        PushValue(p, Here);
        PushValue(p, 0);
        *p++ = len;
        memcpy(p, s, len);
    }
}

/* for use by html-math parser */
int TextWidth(int font, char *str, int len, int *up, int *down)
{
    *up = Fonts[font]->max_bounds.ascent;
    *down = Fonts[font]->max_bounds.descent;
    return XTextWidth(Fonts[font], str, len);
}

/* for use by html-math parser */

void FontSize(int font, int *ascent, int *descent)
{
    *ascent = Fonts[font]->max_bounds.ascent;
    *descent = Fonts[font]->max_bounds.descent;
}

box *CreateBox(int x, int y, int width, int height)
{
    box *new_box;
    
    new_box = (box *)malloc(sizeof(box));
    
    if(!new_box)
    {
	fprintf(stderr, "Ran Out of memory in CreateBox, Exiting\n");
	exit(1);
    }

    new_box->x      = x;
    new_box->y      = y;
    new_box->width  = width;
    new_box->height = height;
    
    return new_box;
}

void AddBox(Frame *frame, int x, int y, int width, int height)
{
    box_link *new_link;
    box      *new_box;

    if(!frame)
	return;
#if 0
    printf("Adding Box %d,%d -> %d,%d\n", x, y, width, height); 
#endif
    new_link = (box_link *)malloc(sizeof(box_link));
    if(!new_link)
    {
    	fprintf(stderr, "Ran Out of memory in AddBox, Exiting\n");
	exit(1);
    }
    new_link->box  = CreateBox(x, y, width, height);
    new_link->next = frame->box_list; /* we add the box in first position */
    frame->box_list = new_link;
}

box *IsInBox(Frame *frame, int x, int y)
{
    box_link *curr_box;
    box_link *is_in;

    if(!frame)
	return 0;

    for(is_in=NULL, curr_box = frame->box_list;!is_in && curr_box; curr_box = curr_box->next)
	if((x >= curr_box->box->x) && (x <= curr_box->box->x + curr_box->box->width)
	   && (y >= curr_box->box->y) && (y <= curr_box->box->y + curr_box->box->height))
	    is_in = curr_box;

    return (is_in) ? is_in->box : (box *)NULL;
}

void CopyBoxList(Frame *dest_frame, Frame *frame)
{
    box_link *curr_box;

    if(!(dest_frame && frame))
	return;
    
    for(curr_box = frame->box_list;curr_box; curr_box = curr_box->next)
	AddBox(dest_frame, curr_box->box->x, curr_box->box->y, curr_box->box->width, curr_box->box->height);
}

int RightMargin(Frame *frame, int right, int voffset)
{
    int rightMargin;
    box *thebox;

    if (frame == &background)
    {
        rightMargin = background.width - background.rightmargin - right;

        if (rightMargin > WinWidth - 4)
            rightMargin = WinWidth - 4;

        rightMargin -= frame->leftmargin;
    }
    else
        rightMargin = frame->width - right /*- frame->leftmargin - frame->rightmargin*/;

    if(frame)
    {
	thebox = IsInBox(frame, rightMargin, voffset);
	while(thebox)
	{
	    rightMargin = thebox->x - 1;
	    thebox = IsInBox(frame, rightMargin, voffset);
	}
    }	
    return rightMargin;
}

int LeftMargin(Frame *frame, int left, int voffset)
{
    int current_left;
    box *thebox;

    current_left = left;
    if(frame)
    {
	thebox = IsInBox(frame, current_left, voffset);
	while(thebox)
	{
	    current_left = thebox->x + thebox->width + 1;
	    thebox = IsInBox(frame, current_left, voffset);
	}
    }	
    return current_left;
}

int Width(Frame *frame, int voffset)
{
    return frame->width;
}

void PutText(Frame *frame, unsigned int emph, int font, char *s, int len, int x, int y)
{
    Byte *p;

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (!prepass)
    {
        p = MakeRoom(SEQTEXTFLEN(len));
        *p++ = SEQTEXT;
	PushValue (p, emph);
        *p++ = font;
#ifdef STYLE_COLOR
	*p++ = (Byte) StyleGet(SI_COLOR);
#endif
#ifdef STYLE_BACKGROUND
 	PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
	PushValue(p, x);
        PushValue(p, y);
        *p++ = len;
        memcpy(p, s, len);
    }
}

/* buf points to start of element in html source iff ismap is present */

void PrintImage(Frame *frame, int delta, unsigned int emph,
                char *buf, Image *image, unsigned int width, unsigned int height)
{
    Byte *p;
    Pixmap pixmap = 0;

    if (image)
	pixmap = image->pixmap;

    if (paintStartLine < 0)
        TextLineFrame(frame);

    if (Here + width > TextLineWidth)
        TextLineWidth = Here + width;

    if (prepass)  /* just update min/max widths */
    {
        if (width > min_width)
            min_width = width;
    }
    else
    {
        p = MakeRoom(IMAGEFLEN);

        *p++ = (IMAGE & 0xF) | (emph & (ISMAP | EMPH_ANCHOR | EMPH_INPUT ));
        PushValue(p, delta);
        PushValue(p, Here);
        PushValue(p, width);
        PushValue(p, height);
	PutPointer(&p,(void *)pixmap);
	PutPointer(&p, (void *)buf);
#ifdef STYLE_BACKGROUND
	PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
    }
}

#define NOBREAK 0
#define BREAK   1

/* check if current word forces word wrap and flush line as needed */
void WrapIfNeeded(Frame *frame, int align, unsigned int emph, int font, int left, int right)
{
    int WordLen, space, rightMargin;
    long line;

    if (paintStartLine < 0)
        TextLineFrame(frame);  /* alters the flow margins, if a figure just ended */

   
#ifdef OLD
    if (frame == &background)
    {
        rightMargin = background.width - background.rightmargin - right;

        if (rightMargin > WinWidth - 4)
            rightMargin = WinWidth - 4;

        rightMargin -= frame->leftmargin;
    }
    else
        rightMargin = frame->width - right /*- frame->leftmargin - frame->rightmargin*/;
#else
    rightMargin = RightMargin(frame, right, PixOffset);
#endif

    if(StyleGetFlag(S_INDENT_FLAG))
	rightMargin -= StyleGetFlag(S_INDENT_FLAG);

    LineBuf[LineLen] = '\0';  /* debug*/
    WordLen = LineLen - WordStart;
/*    WordWidth = XTextWidth(Fonts[font], LineBuf+WordStart, WordLen);*/
    WordWidth = CompoundTextWidth(LineBuf, WordStart, WordLen, &space);

/*    space = XTextWidth(Fonts[StyleGet(S_FONT)], " ", 1);*/    /* width of a space char */
/*    space = XTextWidth(Fonts[font], " ", 1);   */ /* width of a space char */
    line = LineSpacing[font];                   /* height of a line */

    if (WordWidth > min_width)      /* for tables */
        min_width = WordWidth;

    if (prepass)
    {
        TextLineWidth += WordWidth;
        LineWidth = LineLen = WordStart = 0;
        StartOfLine = bufptr;
    }
    else if (WordStart == 0 && Here + WordWidth > rightMargin)
    {
        /* word wider than window */
        if (left + WordWidth > rightMargin)
        {
            if (emph & EMPH_ANCHOR)
                WordWidth += 2;

            PrintString(frame, emph, font, voffset, StartOfLine, WordLen, WordWidth, FALSE);
            EndOfLine(frame, align);
	    LineWidth = LineLen = WordStart = 0;
            StartOfLine = bufptr;
        }
        else /* wrap to next line */
        {
            EndOfLine(frame, align);
	    LineWidth = WordWidth;
            LineLen = WordLen;
            WordStart = LineLen;
            StartOfLine = StartOfWord;
        }

       	Here = LeftMargin(frame, left, PixOffset);
    }
    else if (WordStart > 0 && Here + LineWidth + space + WordWidth > rightMargin)
    {
        if (emph & EMPH_ANCHOR)
            LineWidth += 2;

        PrintString(frame, emph, font, voffset, StartOfLine, WordStart-1, LineWidth, TRUE);

        EndOfLine(frame, align);
        Here = left;

     /* was memmove(LineBuf, LineBuf+WordStart, WordLen); 
        but memmove not available for SUNs and
        memcpy screws up for overlapping copies */

        {
            int n;
            char *p, *q;

            n = WordLen;
            p = LineBuf;
            q = LineBuf+WordStart;
            while (n-- > 0)
                *p++ = *q++;
        }

        LineWidth = WordWidth;
        LineLen = WordLen;
        WordStart = LineLen;
        StartOfLine = StartOfWord;
    }
    else /* word will fit on end of current line */
    {
        if (WordStart > 0)
            LineWidth += space;

        if (WordWidth > 0)
            LineWidth += WordWidth;

        WordStart = LineLen;
    }
}

void FlushLine(int linebreak, Frame *frame, int align,
                unsigned int emph, int font, int left, int right)
{
    int WordLen, space; /* janet: not used: delta, rightMargin */

/*    *StartOfLine = toupper(*StartOfLine);*//* howcome playing 5/5/95 */

    if (preformatted)
    {
        WordLen = LineLen - WordStart;
/*        LineWidth = XTextWidth(Fonts[font], LineBuf+WordStart, WordLen);*/
        LineWidth = CompoundTextWidth(LineBuf, WordStart, WordLen, &space);
    }
    else if (LineLen > 1 || (LineLen == 1 && LineBuf[0] != ' '))
        WrapIfNeeded(frame, align, emph, font, left, right);

    if (LineLen > 0)
    {
        if (emph & EMPH_ANCHOR)
            LineWidth += 2;

        /* watch out for single space as leading spaces
           are stripped by CopyLine */

        if (LineLen > 1 || LineBuf[0] != ' ')
            PrintString(frame, emph, font, voffset, StartOfLine, LineLen, LineWidth, FALSE);

        if (linebreak)
        {
            EndOfLine(frame, align);
            Here = left;
        }
/*
        else
            Here += LineWidth;
*/

        LineWidth = LineLen = WordStart = 0;
    }
    else if (linebreak)
    {
	/* watch out for empty preformatted lines */
        if (preformatted && TextLineWidth == 0)
            PixOffset += ASCENT(font) + DESCENT(font);

        if (paintStartLine >= 0)  /* was if (Here > left) */
        {
            EndOfLine(frame, align);
            Here = left;
        }
    }

    StartOfLine = StartOfWord = bufptr;
}




/* needs to cope with > in quoted text for ' and " */
void SwallowAttributes(void)
{
    int c;

    while ((c = *bufptr) && c != '>')
    {
        ++bufptr;
    }

    if (c == '>')
        ++bufptr;
}


/*
 Return a pointer to the ">" in an end comment (or to "\0" if there
 isn't one).  Example:
    "hiya <!-- <foo>there</foo> -- --whatever-- > there"
 pass this -^                      return this -^
 */

char *FindEndComment( char *ptr )
{
    char c, *s;

	for(;;)
	{
		ptr += 2; /* first character after opening "--" */

		if(!((s = strstr(ptr, "--")))) break;

		ptr = s + 1; /* second "-" -- ++ will put us on the next character */

		/* Skip past the optional white space (<= ' ' from GetToken) */

		while((c=*(++ptr)) && c <= ' ');

		if(c == '>')
		{
			return ptr; /* comment terminated correctly */
		}

		if(!strncmp(ptr, "--", 2))
		{
			/* comment is something like "<!--fine-- wrong>" */

			if(s=strchr(ptr,'>')) return s;

			break;
		}
	}

	/* Document ends in unfinished comment -- seek for '0' */

	if(*ptr) while(*(++ptr));

	return ptr;
}


/*
 char *tag points to start of tag string which is terminated
 by whitespace (including EOF) or a '>' character.

 return tag code or 0 if unknown.
*/

int RecogniseTag(void)
{
    int c, len;
    char *s;

     s = bufptr + 1;
  
    /* handle comments correctly -- code by mlvanbie@valeyard.uwaterloo.ca */

    if (*s == '!' && (*(s+1) == '-') && (*(s+2) == '-'))
    {
        bufptr += 4; /* eat "<!--" */
        EndTag = 0;
    	TagLen = 0;  /* we will be manipulating bufptr directly */
        TokenClass = EN_UNKNOWN;

	bufptr = FindEndComment( bufptr + 2 );    /* +2: eat "<!" */

#if 0
        while ((s = strstr(bufptr, "--")))
        {
            bufptr = s + 1; /* stay on second "-" */

	        /* Skip past the optional white space (<= ' ' from GetToken) */

	        while((c=*(++bufptr)) && c <= ' ');

            if(c == '>')
            {
                return UNKNOWN; /* pretend we have an unknown tag */
            }

            bufptr = s + 1; /* handles "<!-- ... --->" (nonoptimal) */
        }

        /* Document ends in unfinished comment -- seek for '0' */

        if(*bufptr) while(*(++bufptr));
#endif

        return UNKNOWN; /* unknown tag */
    }

    if (*s == '/')
    {
        EndTag = 1;
        ++s;
    }
    else
        EndTag = 0;

    if ((c = *s) == '!' || c == '?')
        ++s;
    else if (c != '>' && !isalpha(c))
        return PCDATA;

  /* find end of tag to allow use of strncasecmp */

    while ((c = *s, isalpha(c)) || isdigit(c))
        ++s;

    TagLen = s - bufptr;        /* how far to next char after tag name */
    len = TagLen - EndTag - 1;  /* number of chars in tag name itself */
    s -= len;
    c = TOLOWER(*s);

    if(len == 6 && strncasecmp(s, "script", len) == 0){
	    TagLen = 0;
	    TokenClass = EN_UNKNOWN;
	    while(strncasecmp(bufptr, "/script", 7) != 0) bufptr++;
	    return UNKNOWN;
    }

    if (isalpha(c))
    {
        if (c == 'a')
        {
            if (len == 1 && strncasecmp(s, "a", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_ANCHOR;
            }

            if (len == 3 && strncasecmp(s, "alt", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_ALT;
            }

            if (len == 5 && strncasecmp(s, "added", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_ADDED;
            }

            if (len == 7 && strncasecmp(s, "address", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_ADDRESS;
            }

            if (len == 8 && strncasecmp(s, "abstract", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_ABSTRACT;
            }
        }
        else if (c == 'b')
        {
            if (len == 1)
            {
                TokenClass = EN_TEXT;
                return TAG_BOLD;
            }

            if (len == 2 && strncasecmp(s, "br", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_BR;
            }

            if (len == 4 && strncasecmp(s, "body", len) == 0)
            {
                TokenClass = EN_MAIN;
                return TAG_BODY;
            }

            if (len == 10 && strncasecmp(s, "blockquote", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_QUOTE;
            }

            if (len == 4 && strncasecmp(s, "base", len) == 0)
            {
                TokenClass = EN_SETUP;
                return TAG_BASE;
            }
        }
        else if (c == 'c')
        {
            if (len == 4)
            {
                if (strncasecmp(s, "code", len) == 0)
                {
                    TokenClass = EN_TEXT;
                    return TAG_CODE;
                }

                if (strncasecmp(s, "cite", len) == 0)
                {
                    TokenClass = EN_TEXT;
                    return TAG_CITE;
                }
            }
            else if (len == 7 && (strncasecmp(s, "caption", len) == 0))/* howcome 3/2/95: = -> == after hint from P.M.Hounslow@reading.ac.uk */
            {
                TokenClass = EN_BLOCK;
                return TAG_CAPTION;
            }
        }
        else if (c == 'd')
        {
            if (len == 3 && strncasecmp(s, "dfn", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_DFN;
            }

	    /* howcome 11/8/95: added upport for DIR */

            if (len == 3 && strncasecmp(s, "dir", len) == 0)
            {
                TokenClass = EN_LIST;
                return TAG_UL;
            }


            if (len != 2)
                return 0;

            if (strncasecmp(s, "dl", len) == 0)
            {
                TokenClass = EN_LIST;
                return TAG_DL;
            }

            if (strncasecmp(s, "dt", len) == 0)
            {
                TokenClass = EL_DEFLIST;
                return TAG_DT;
            }

            if (strncasecmp(s, "dd", len) == 0)
            {
                TokenClass = EL_DEFLIST;
                return TAG_DD;
            }
        }
        else if (c == 'e')
        {
            if (len == 2 && strncasecmp(s, "em", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_EM;
            }
        }
        else if (c == 'f')
        {
            if (len == 3 && strncasecmp(s, "fig", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_FIG;
            }
	    
	    /* --Spif 10/10/95 Form tag must be checked ! */
	    if (len == 4 && strncasecmp(s, "form", len) == 0)
	    {
	        TokenClass = EN_TEXT /*EN_BLOCK*/;
		return TAG_FORM;
	    };
        }
        else if (c == 'h')
        {
            if (len == 4 && strncasecmp(s, "head", len) == 0)
            {
                TokenClass = EN_SETUP;
                return TAG_HEAD;
            }

            if (len != 2)
                return 0;

            TokenClass = EN_HEADER;
            c = TOLOWER(s[1]);

            switch (c)
            {
                case '1':
                    return TAG_H1;
                case '2':
                    return TAG_H2;
                case '3':
                    return TAG_H3;
                case '4':
                    return TAG_H4;
                case '5':
                    return TAG_H5;
                case '6':
                    return TAG_H6;
                case 'r':
                    TokenClass = EN_BLOCK;
                    return TAG_HR;
            }
        }
        else if (c == 'i')
        {
            if (len == 1)
            {
                TokenClass = EN_TEXT;
                return TAG_ITALIC;
            }

            if (len == 3 && strncasecmp(s, "img", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_IMG;
            }

            if (len == 5 && strncasecmp(s, "input", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_INPUT;
            }

            if (len == 7 && strncasecmp(s, "isindex", len) == 0)
            {
                TokenClass = EN_SETUP;
                return TAG_ISINDEX;
            }
        }
        else if (c == 'k')
        {
            if (len == 3 && strncasecmp(s, "kbd", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_KBD;
            }
        }
        else if (c == 'l')
        {
            if (len == 2 && strncasecmp(s, "li", len) == 0)
            {
                TokenClass = EN_LIST;
                return TAG_LI;
            }

            if (len == 4 && strncasecmp(s, "link", len) == 0)
            {
                TokenClass = EN_SETUP;
                return TAG_LINK;
            }

        }
        else if (c == 'm')
        {
            if (len == 4 && strncasecmp(s, "math", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_MATH;
            }

            if (len == 6 && strncasecmp(s, "margin", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_MARGIN;
            }

	    /* howcome 11/8/95: added MENU to be compatible with HTML2 */
            if (len == 4 && strncasecmp(s, "menu", len) == 0)
            {
                TokenClass = EN_LIST;
                return TAG_UL;
            }


        }
        else if (c == 'n')
        {
            if (len == 4 && strncasecmp(s, "note", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_NOTE;
            }
        }
        else if (c == 'o')
        {
            if (len == 2 && strncasecmp(s, "ol", len) == 0)
            {
                TokenClass = EN_LIST;
                return TAG_OL;
            }

            if (len == 6 && strncasecmp(s, "option", len) == 0)
            {
                TokenClass = EN_TEXT;  /* kludge for error recovery */
                return TAG_OPTION;
            }
        }
        else if (c == 'p')
        {
            if (len == 1)
            {
                TokenClass = EN_BLOCK;
                return TAG_P;
            }

            if (len == 3 && strncasecmp(s, "pre", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_PRE;
            }
        }
        else if (c == 'q')
        {
            if (len == 1)
            {
                TokenClass = EN_TEXT;
                return TAG_Q;
            }

            if (len == 5 && strncasecmp(s, "quote", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_QUOTE;
            }
        }
        else if (c == 'r')
        {
            if (len == 7 && strncasecmp(s, "removed", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_REMOVED;
            }
        }
        else if (c == 's')
        {
            if (len == 1)
            {
                TokenClass = EN_TEXT;
                return TAG_STRIKE;
            }

            if (len == 3 && strncasecmp(s, "sup", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_SUP;
            }

            if (len == 3 && strncasecmp(s, "sub", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_SUB;
            }

            if (len == 4 && strncasecmp(s, "samp", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_SAMP;
            }

            if (len == 5 && strncasecmp(s, "small", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_SMALL;
            }

            if (len == 6 && strncasecmp(s, "strong", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_STRONG;
            }

            if (len == 6 && strncasecmp(s, "select", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_SELECT;
            }

            if (len == 6 && strncasecmp(s, "strike", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_STRIKE;
            }

	    /* howcome 26/2/95 */

            if (len == 5 && strncasecmp(s, "style", len) == 0)
            {
                TokenClass = EN_SETUP;
                return TAG_STYLE;
            }

        }
        else if (c == 't')
        {
            if (len == 5 && strncasecmp(s, "title", len) == 0)
            {
                TokenClass = EN_SETUP;
                return TAG_TITLE;
            }

            if (len == 2 && strncasecmp(s, "tt", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_TT;
            }

            if (len == 2 && strncasecmp(s, "tr", len) == 0)
            {
                TokenClass = EN_TABLE;
                return TAG_TR;
            }

            if (len == 2 && strncasecmp(s, "th", len) == 0)
            {
                TokenClass = EN_TABLE;
                return TAG_TH;
            }

            if (len == 2 && strncasecmp(s, "td", len) == 0)
            {
                TokenClass = EN_TABLE;
                return TAG_TD;
            }

            if (len == 5 && strncasecmp(s, "table", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_TABLE;
            }

            if (len == 8 && strncasecmp(s, "textarea", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_TEXTAREA;
            }
        }
        else if (c == 'u')
        {
            if (len == 1)
            {
                TokenClass = EN_TEXT;
                return TAG_UNDERLINE;
            }

            if (len == 2 && strncasecmp(s, "ul", len) == 0)
            {
                TokenClass = EN_LIST;
                return TAG_UL;
            }
        }
        else if (c == 'v')
        {
            if (len == 3 && strncasecmp(s, "var", len) == 0)
            {
                TokenClass = EN_TEXT;
                return TAG_VAR;
            }
        }
        else if (c == 'x')
        {
            if (len == 3 && strncasecmp(s, "xmp", len) == 0)
            {
                TokenClass = EN_BLOCK;
                return TAG_PRE;
            }
        }
    }

    TokenClass = EN_UNKNOWN;
    return UNKNOWN; /* unknown tag */
}


void UnGetToken(void)
{
    bufptr = LastBufPtr;
}

/*
   The token type is returned in the global token.
   Characters are returned in TokenValue while TokenClass
   is used to return a class value e.g. EN_SETUP or EN_BLOCK.
   Entity definitions are pointed to by EntityValue.

   The bufptr is moved past the token, except at the end
   of the buffer - as a safety precaution.
*/

int GetToken(void)
{
    int c, k, n;

    LastBufPtr = bufptr;
    c = *(unsigned char *)bufptr;
    TokenValue = c;

    if(targetptr)
	if (bufptr <= targetptr)
	    ViewOffset = PixOffset;

    if (c == '<' && (Token = RecogniseTag()) != PCDATA)
    {
        bufptr += TagLen;   /* to first char after tag name */
        return Token;
    }

    TokenClass = EN_TEXT;
    EndTag = 0;

    if (c == '&' && (isalpha(bufptr[1]) || bufptr[1] == '#'))
    {
        n = entity(bufptr + 1, &k);

        if (n)
        {
            bufptr += k;
            TokenValue = n;
            Token = PCDATA;
            return Token;
        }
    }

    if (c <= ' ')
    {
        if (c == '\0')
        {
            Token = ENDDATA;
            TokenClass = EN_UNKNOWN;
            return Token;
        }
        
        ++bufptr;
        Token = WHITESPACE;
        return Token;
    }


    ++bufptr;
    Token = PCDATA;
    return Token;
}

/* void ParseAnchorAttrs(char **href, int *hreflen, char **name, int *namelen, char **class_p, int *class_len_p);*/

/* get token, skipping white space and unknown tokens */

int SkipSpace(void)
{
    for (;;)
    {
        while (GetToken() == WHITESPACE);

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        break;
    }

    return Token;
}



/* assumes bufptr points to start of attribute */
char *ParseAttribute(int *len)
{
    int c;
    char *attr;

    *len = 0;
    attr = bufptr;
    IdAttributeFlag = 0;

    for (;;)
    {
        c = *bufptr;

        if (c == '>' || c == '\0')
            return attr;

        if (c == '=' || IsWhite(c))
            break;

        ++(*len);
        ++bufptr;
    }

    if (*len == 2 && strncasecmp(attr, "id", 2) ==0)
        IdAttributeFlag = 1;

    return attr;
}

/* values start with "=" or " = " etc. */
char *ParseValue(int *len)
{
    int c, delim;
    char *value;

    *len = 0;

    while (c = *bufptr, IsWhite(c))
        ++bufptr;

    if (c != '=')
        return 0;

    ++bufptr;   /* past the = sign */

    while (c = *bufptr, IsWhite(c))
        ++bufptr;

    if (c == '"' || c == '\'')
    {
        delim = c;
        ++bufptr;
    }
    else
        delim = 0;

    value = bufptr;

    for (;;)
    {
        c = *bufptr;

        if (c == '\0')
	    if(delim)
		return 0;  /* a big and dirty hack --Spif 27-Oct-95 */
	    else
		break;

        if (delim)
        {
            if (c == delim)
            {
                ++bufptr;
                break;
            }
        }
        else if (c == '>' || IsWhite(c))
            break;

        ++(*len);
        ++bufptr;
    }

    if (IdAttributeFlag && value && targetId)
    {
        if (strlen(targetId) == *len &&
            strncasecmp(value, targetId, *len) == 0)
        {
            IdOffset = PixOffset;
        }
    }

    IdAttributeFlag = 0;
    return value;
}


/*
   HREF attribute defines original document URL and is added by
   the browser when making a local copy of a document so that
   relative references can be deferenced to their original links
*/

void ParseBase(Frame *frame)
{
    char *href, *name;
    int hreflen, namelen;

    ParseAnchorAttrs(&href, &hreflen, &name, &namelen, &class, &class_len);
    if(href)
	CurrentDoc->base = www_strndup(href, hreflen);
}

void ParseLink(Frame *frame)
{
    char *href=NULL, *rel=NULL;
    int hreflen = 0, rellen = 0;
    int c, n, m;
    char *attr, *value;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        value = ParseValue(&m);

        if (n == 4 && strncasecmp(attr, "href", n) == 0)
        {
            href = value;
            hreflen = m;
            continue;
        }

        if (n == 3 && strncasecmp(attr, "rel", n) == 0)
        {
            rel = value;
            rellen = m;
            continue;
        }
    }

    if (href && rel) {
	if (!NoStyle && !CurrentDoc->link_style && (strncasecmp(rel, "style", 5) == 0))
	    CurrentDoc->link_style = (char *)StyleLoad(href, hreflen, CurrentDoc->pending_reload); 
	/* (char *) because StyleLoad not declared -> int implied , must add include files */
    }
}


void ParseTitle(int implied, Frame *frame)
{
    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
        SwallowAttributes();

    /* skip leading white space - subsequently contigous
       white space is compressed to a single space */
    while (GetToken() == WHITESPACE);
    UnGetToken();

    LineLen = 0;

    for (;;)
    {
        GetToken();

        if (Token == TAG_TITLE && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == WHITESPACE)
        {
            while (GetToken() == WHITESPACE);
            UnGetToken();

            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';
            continue;
        }

        if (Token != PCDATA)
        {
            UnGetToken();
            break;
        }

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    SetBanner(LineBuf);
}

/* howcome 26/2/95 */

void ParseStyle(Frame *frame)
{
    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    SwallowAttributes();

    /* skip leading white space - subsequently contigous
       white space is compressed to a single space */
    while (GetToken() == WHITESPACE);
    UnGetToken();

    LineLen = 0;

    for (;;)
    {
        GetToken();

        if (Token == TAG_STYLE && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    if (!NoStyle && !CurrentDoc->head_style)
	CurrentDoc->head_style = strdup (LineBuf);
}



void ParseSetUp(int implied, Frame *frame)
{
    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
        SwallowAttributes();

    for (;;)
    {
        while (GetToken() == WHITESPACE);
/*        UnGetToken(); */ /* howcome 10/7/95 part of comment handling */

        if (Token == TAG_HEAD && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == TAG_TITLE)
        {
            UnGetToken();
	    ParseTitle(0, frame);
            continue;
        }

        if (Token == TAG_ISINDEX)
        {
            SwallowAttributes();
            IsIndex = 1;
            ClearStatus();
            continue;
        }

        if (Token == TAG_BASE)
        {
	    UnGetToken();
	    ParseBase(frame);
            continue;
        }

	/* howcome 26/2/95 */

	if (Token == TAG_STYLE)
	{
	    UnGetToken();
	    ParseStyle(frame);
	    continue;
	}

	/* howcome 25/4/95 */

	if (Token == TAG_LINK)
	{
	    UnGetToken();
	    ParseLink(frame);
	    continue;
	}

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == PCDATA || Token == ENTITY)
        {
            UnGetToken();
            break;
        }

        if (Token == ENDDATA || TokenClass != EN_SETUP)
        {
            UnGetToken();
            break;
        }
    }
}

void ParseAnchorAttrs(char **href, int *hreflen, char **name, int *namelen, char **class_p, int *class_len_p)
{
    int c, n, m;
    char *attr, *value;

    *href = 0;
    *name = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        value = ParseValue(&m);

        if (n == 4 && strncasecmp(attr, "href", n) == 0)
        {
            *href = value;
            *hreflen = m;
            continue;
        }

        if (n == 4 && strncasecmp(attr, "name", n) == 0)
        {
            *name = value;
            *namelen = m;
            continue;
        }

        if (n == 5 && strncasecmp(attr, "class", n) == 0)
        {
	    if (STYLE_TRACE) {
		char *s = www_strndup(attr, n);
		fprintf(stderr,"style_class %s\n",s);
		Free(s);
	    }

	    *class_p = value;
	    *class_len_p = m;
        }

    }

    if (*href == NULL)
	if (TAG_TRACE)
	    fprintf(stderr,"ParseAnchorAttrs: *href == NULL, hreflen = %d\n", *hreflen);
}

/* howcome30/5/95: addded support for widht and height */

void ParseImageAttrs(char **href, int *hreflen, int *align, int *ismap, int *width, int *height)
{
    int c, n, m;
    char *attr, *value;

    *href = NULL;               /* howcome: NULL !! */
    *align = ALIGN_BOTTOM;
    *ismap = 0;
    *width = 0; *height = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        value = ParseValue(&m);

        if (n == 3 && strncasecmp(attr, "src", n) == 0)
        {
            *href = value;
            *hreflen = m;
            continue;
        }


        if (n == 5 && strncasecmp(attr, "width", n) == 0)
        {
	    *width = atoi(value);
            continue;
        }

        if (n == 6 && strncasecmp(attr, "height", n) == 0)
        {
            *height = atoi(value);
            continue;
        }

        if (n == 5 && strncasecmp(attr, "align", n) == 0)
        {
            if (m == 3 && strncasecmp(value, "top", m) == 0)
                *align = ALIGN_TOP;
            else if (m == 6 && strncasecmp(value, "middle", m) == 0)
                *align = ALIGN_MIDDLE;
            else if (m == 6 && strncasecmp(value, "bottom", m) == 0)
                *align = ALIGN_BOTTOM;

            continue;
        }

        if (n == 5 && strncasecmp(attr, "ismap", n) == 0)
            *ismap = 1;
    }
}

void ParseFigureAttrs(char **href, int *hreflen, int *align, int *width, int *height)
{
    int c, n, m;
    char *attr, *value;

    *href = 0;
    *align = ALIGN_CENTER;
    *width = 0; *height = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        value = ParseValue(&m);

        if (n == 3 && strncasecmp(attr, "src", n) == 0)
        {
            *href = value;
            *hreflen = m;
            continue;
        }

        if (n == 5 && strncasecmp(attr, "width", n) == 0)
        {
	    *width = atoi(value);
            continue;
        }

        if (n == 6 && strncasecmp(attr, "height", n) == 0)
        {
            *height = atoi(value);
            continue;
        }

        if (n == 5 && strncasecmp(attr, "align", n) == 0)
        {
            if (m == 4 && strncasecmp(value, "left", m) == 0)
                *align = ALIGN_LEFT;
            else if (m == 6 && strncasecmp(value, "center", m) == 0)
                *align = ALIGN_CENTER;
            else if (m == 5 && strncasecmp(value, "right", m) == 0)
                *align = ALIGN_RIGHT;
            else if (m == 9 && strncasecmp(value, "bleedleft", m) == 0)
                *align = ALIGN_BLEEDLEFT;
            else if (m == 10 && strncasecmp(value, "bleedright", m) == 0)
                *align = ALIGN_BLEEDRIGHT;

            continue;
        }
    }
}
void /* wm 19.Jan.95 */
ParseTextAreaAttrs(int *type, char **name, int *nlen,
                char **value, int *vlen, int *rows, int *cols, int *flags)
{
    int c, n, m; /* janet: not used: checked */
    char *attr, *attrval;

    *type = TEXTFIELD;
    *rows = 4;
    *cols = 20;
    *flags = 0;
    *name = *value = "";
    *nlen = *vlen = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 4 && strncasecmp(attr, "type", n) == 0)
        {
            if (m == 4 && strncasecmp(attrval, "text", m) == 0)
                *type = TEXTFIELD;
            else if (m == 8 && strncasecmp(attrval, "checkbox", m) == 0)
                *type = CHECKBOX;
            else if (m == 5 && strncasecmp(attrval, "radio", m) == 0)
                *type = RADIOBUTTON;

            continue;
        }

        if (n == 4 && strncasecmp(attr, "name", n) == 0)
        {
            *name = attrval;
            *nlen = m;
            continue;
        }

        if (n == 5 && strncasecmp(attr, "value", n) == 0)
        {
            *value = attrval;
            *vlen = m;
            continue;
        }

        if (n == 4 && strncasecmp(attr, "rows", n) == 0)
        {
            sscanf(attrval, "%d", rows);
            continue;
        }

        if (n == 4 && strncasecmp(attr, "cols", n) == 0)
        {
            sscanf(attrval, "%d", cols);
            continue;
        }

        if (n == 5 && strncasecmp(attr, "error", n) == 0)
        {
            *flags |= IN_ERROR;
            continue;
        }

        if (n == 8 && strncasecmp(attr, "disabled", n) == 0)
        {
            *flags |= DISABLED;
            continue;
        }

        if (n == 7 && strncasecmp(attr, "checked", n) == 0)
            *flags |= CHECKED;
    }
}

void /* --Spif 11-Oct-95 */
ParseFormAttrs(int *method, char **action, int *alen)
{
    int c, n, m;
    char *attr, *attrval;
    
    *action = NULL;
    *method = GET; /* implied must add encoded-type.. */
    for (;;)
    {
        c= *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

	--bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);
	if (n == 6 && strncasecmp(attr, "method", n) == 0)
        {
	    if ( m == 3 && strncasecmp(attrval, "get", m) ==0)
	        *method = GET;
	    else if ( m == 4 && strncasecmp(attrval, "post", m) ==0)
	        *method = POST;
	}
	if ( n == 6 && strncasecmp(attr, "action", n) == 0)
	{
	  *action = attrval;
	  *alen = m;
	  continue;
	};
    };
}

void /* wm 19.Jan.95 */
ParseInputAttrs(int *type, char **name, int *nlen,
                char **value, int *vlen, int *size, int *flags, Image **image)
{
    int c, n, m; /* janet: not used: checked */
    char *attr, *attrval, *href = NULL;
    int hreflen = 0; /* --SPif */

    *type = TEXTFIELD;
    *size = 20;
    *flags = 0;
    *name = *value = "";
    *nlen = *vlen = 0;
    hreflen = 0;
    *image=NULL;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 4 && strncasecmp(attr, "type", n) == 0)
        {
            if (m == 4 && strncasecmp(attrval, "text", m) == 0)
                *type = TEXTFIELD;
            else if (m == 8 && strncasecmp(attrval, "checkbox", m) == 0)
                *type = CHECKBOX;
            else if (m == 5 && strncasecmp(attrval, "radio", m) == 0)
                *type = RADIOBUTTON;
            else if (m == 6 && strncasecmp(attrval, "submit", m) == 0)
                *type = SUBMITBUTTON;
            else if (m == 5 && strncasecmp(attrval, "reset", m) == 0)
                *type = RESETBUTTON;
	    else if (m == 5 && strncasecmp(attrval, "image", m) ==0)
	        *type = SUBMITBUTTON; /* --Spif image in html2 = Submit */
	    else if (m == 6 && strncasecmp(attrval, "hidden", m) ==0)
	        *type = HIDDEN;
	    else if (m == 6 && strncasecmp(attrval, "passwd", m) ==0)
	        *type = PASSWD;
            continue;
        }

        if (n == 4 && strncasecmp(attr, "name", n) == 0)
        {
            *name = attrval;
            *nlen = m;
            continue;
        }

        if (n == 5 && strncasecmp(attr, "value", n) == 0)
        {
            *value = attrval;
            *vlen = m;
            continue;
        }

        if (n == 4 && strncasecmp(attr, "size", n) == 0)
        {
            sscanf(attrval, "%d", size);
            continue;
        }

        if (n == 5 && strncasecmp(attr, "error", n) == 0)
        {
            *flags |= IN_ERROR;
            continue;
        }

        if (n == 8 && strncasecmp(attr, "disabled", n) == 0)
        {
            *flags |= DISABLED;
            continue;
        }

        if (n == 7 && strncasecmp(attr, "checked", n) == 0)
            *flags |= CHECKED;

        if (n == 8 && strncasecmp(attr, "multiple", n) == 0)
            *flags |= MULTIPLE;

	/* --Spif 9-Oct-1995 */ 
	if (n == 3 && strncasecmp(attr, "src", n) == 0)
        {
	    href = attrval;
	    hreflen = m;
	};

    }
    /* --Spif 9-Oct-1995 */
    if (hreflen > 0)
        *image = GetImage(href, hreflen, CurrentDoc->pending_reload);

}

void /* wm 19.Jan.95 */
ParseCellAttrs(int *rowspan, int *colspan, int *align, int *nowrap)
{
    int c, n, m;
    char *attr, *attrval;

    *rowspan = *colspan = 1;
    *align = ALIGN_CENTER;
    *nowrap = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 7 && strncasecmp(attr, "rowspan", n) == 0)
        {
            sscanf(attrval, "%d", rowspan);
            continue;
        }

        if (n == 7 && strncasecmp(attr, "colspan", n) == 0)
        {
            sscanf(attrval, "%d", colspan);
            continue;
        }

        if (n == 5 && strncasecmp(attr, "align", n) == 0)
        {
            if (m == 6 && strncasecmp(attrval, "center", m) == 0)
                *align = ALIGN_CENTER;
            else if (m == 5 && strncasecmp(attrval, "right", m) == 0)
                *align = ALIGN_RIGHT;
            else if (m == 4 && strncasecmp(attrval, "left", m) == 0)
                *align = ALIGN_LEFT;

            continue;
        }

        if (n == 6 && strncasecmp(attr, "nowrap", n) == 0)
            *nowrap = 1;
    }
}

void /* wm 19.Jan.95 */
ParseTableAttrs(int *border)
{
    int c, n, m;
    char *attr, *attrval;

    *border = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 6 && strncasecmp(attr, "border", n) == 0)
        {
	    int bd=1;   /* DJB 17-Jan-96 */
	    
	    if (attrval)
		sscanf(attrval, "%d", &bd);
	    
	    *border = bd ? 1: 0; /* Restrict to 1 and 0 ? */
            continue;
        }
    }
}

void 
ParseClassAttrs(char **class_p, int *class_len_p)
{
    int c, n, m;
    char *attr, *attrval;

    *class_p = NULL;
    *class_len_p = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 5 && strncasecmp(attr, "class", n) == 0)
        {
	    if (STYLE_TRACE) {
		char *s = www_strndup(attrval, m);
		fprintf(stderr,"style_class %s\n",s);
		Free(s);
	    }

	    *class_p = attrval;
	    *class_len_p = m;
        }
    }
}



void /* wm 19.Jan.95 */
ParseParaAttrs(int *align, char **class_p, int *class_len_p)
{
    int c, n, m;
    char *attr, *attrval;

    *class_p = NULL;
    *class_len_p = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 5 && strncasecmp(attr, "align", n) == 0)
        {
            if (m == 6 && strncasecmp(attrval, "center", m) == 0)
                *align = ALIGN_CENTER;
            else if (m == 5 && strncasecmp(attrval, "right", m) == 0)
                *align = ALIGN_RIGHT;
            else if (m == 4 && strncasecmp(attrval, "left", m) == 0)
                *align = ALIGN_LEFT;
	    else if (m == 7 && strncasecmp(attrval, "justify", m) == 0)
		*align = ALIGN_JUSTIFY;
        }

        if (n == 5 && strncasecmp(attr, "class", n) == 0)
        {
	    if (STYLE_TRACE) {
		char *s = www_strndup(attrval, m);
		fprintf(stderr,"style_class %s\n",s);
		Free(s);
	    }

	    *class_p = attrval;
	    *class_len_p = m;
        }
    }
}

#ifndef NO_BACKGROUND_IN_HTML
ParseBodyAttrs(char **bg_img, int *bg_len)
{
    int c, n, m;
    char *attr, *attrval;
    char *d;

    d = bufptr;
    *bg_img = NULL;
    *bg_len = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if (n == 10 && strncasecmp(attr, "background", n) == 0)
        {          
	    *bg_img = attrval;
	    *bg_len = m;
	}
    }
    bufptr = d;
}
#endif /* NO_BACKGROUND_IN_HTML */

Image *ParseNoteAttrs(void)
{
    int c, n, m, hreflen;
    char *attr, *attrval, *href = NULL;
    Image *image;

    image = NULL;
    hreflen = 0;

    for (;;)
    {
        c = *bufptr++;

        if (c == '\0')
            break;

        if (c == '>')
            break;

        if (IsWhite(c))
            continue;

        --bufptr;
        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);

        if ((n == 4 && strncasecmp(attr, "role", n) == 0) || (n == 5 && strncasecmp(attr, "class", n) == 0))
        {
            if (m == 4 && strncasecmp(attrval, "note", m) == 0)
                image = note_image;
            else if (m == 3 && strncasecmp(attrval, "tip", m) == 0)
                image = note_image;
            else if (m == 7 && strncasecmp(attrval, "caution", m) == 0)
                image = caution_image;
            else if (m == 7 && strncasecmp(attrval, "warning", m) == 0)
                image = warning_image;
        }

        if (n == 3 && strncasecmp(attr, "src", n) == 0)
        {
            href = attrval;
            hreflen = m;
        }
    }

    if (hreflen > 0)
        image = GetImage(href, hreflen, CurrentDoc->pending_reload);

    return image;
}

void ParseOption(int implied, Frame *frame, Field *field, int font)
{
  /* janet: not used:     int width; */

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
        SwallowAttributes();

    LineLen = 0;

    for (;;)
    {
        GetToken();

        if (Token == TAG_OPTION && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        /* condense whitespace */

        if (Token == WHITESPACE)
        {
            while (GetToken() == WHITESPACE);
            UnGetToken();

            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';

            continue;
        }

        if (Token == PCDATA)
        {
            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = TokenValue;

            continue;
        }

     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }

    LineBuf[LineLen] = '\0';

    if (LineLen > 0)
        AddOption(field, font, LineBuf, LineLen);
}

void ParseSelect(int implied, Frame *frame, int font, int align, int left, int right)
{
    int type, nlen, vlen, size, flags;
    char *name, *value;
    Field *field;
    Image *image;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
    {
        /* bodge parsing select's attributes as short term hack */
        ParseInputAttrs(&type, &name, &nlen, &value, &vlen, &size, &flags, &image);
    }
    else
    {
        name = value = bufptr; /* --Spif 16-Otc-95 click bug */
        nlen = vlen = flags = 0;

    }

    if (form == NULL)
        form = DefaultForm();
    if(nlen)
      field = GetField(form, OPTIONLIST, Here, name, nlen, value, vlen, 1, 6, flags|font);
    else
      field = GetField(form, OPTIONLIST, Here, bufptr, nlen, value, vlen, 1, 6, flags|font);
    WrapFieldIfNeeded(field, frame, align, left, right);
    PrintInputField(frame, field);

    for (;;)
    {
        while (GetToken() == WHITESPACE);

        if (Token == TAG_SELECT && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_OPTION)
        {
            ParseOption(0, frame, field, font);
            continue;
        }

        if (Token == PCDATA)
        {
            ParseOption(1, frame, field, font);
            continue;
        }

     /* unexpected tag so terminate element */

        error |= ERR_SELECT;
        RecordError(bufptr, "ERR_SELECT"); /* howcome 11/10/94 */
        UnGetToken();
        break;
    }    

    Here += field->width;
}

void ParseEmph(Frame *frame, int align, unsigned int emph, int font, int left, int right)
{
    int ThisToken, WordLen, hreflen, namelen, nlen, vlen, size, valign, up, down,
        suboffset, type, rows, cols, delta, ismap, width, height, flags, indent = 0; /* janet: not used: h */
    char *href, *name, *value, *p;
    Image *image;
    Field *field;
    Bool element_started = FALSE;
    
/*    StylePush(style);*/
    
    if (EndTag)
    {
        SwallowAttributes();
        return;
    }
    
    suboffset = 0;
    ThisToken = Token;
    
    /* we only want to mark anchors that are hrefs, so we'll have to
       wait until we parse the anchor attributes to dterermine this */
/*
  if (ThisToken != TAG_ANCHOR)
  style = FormatElementStart(ThisToken, class, class_len)
  */
    switch (ThisToken)
    {
    case TAG_ANCHOR:
	ParseAnchorAttrs(&href, &hreflen, &name, &namelen, &class, &class_len);
	
	if (href) {
	    emph |= EMPH_ANCHOR;
	    FormatElementStart(Token, class, class_len);
	    element_started = TRUE;
	}
	
	if (name && targetId)
	{
	    if (strlen(targetId) == namelen &&
		strncasecmp(name, targetId, namelen) == 0)
	    {
		IdOffset = PixOffset;
	    }
	}
	
	if (preformatted)
	    break;
	
	/* otherwise skip leading white space - subsequently contigous
	   white space is compressed to a single space */
	
	while (GetToken() == WHITESPACE);
	UnGetToken();
	break;
	
    case TAG_MARGIN:
	SwallowAttributes();
	font = IDX_H2FONT;
	indent = XTextWidth(Fonts[IDX_NORMALFONT], "mmm", 3);
	left += indent;
	right -= indent;
	EndOfLine(frame, align);
	Here = left;
	PixOffset += LineSpacing[font];
	break;
	
    case TAG_TT:
    case TAG_CODE:
    case TAG_SAMP:
    case TAG_KBD:
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = StyleGet(SI_FONT);
/*
  if (font == IDX_BNORMALFONT)
  font = IDX_BIFIXEDFONT;
  else
  font = IDX_FIXEDFONT;
  */
	break;
	
    case TAG_SMALL:
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = (int)StyleGet(SI_FONT);
	break;
	
    case TAG_SUP:
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = (int)StyleGet(SI_FONT);
	
	suboffset = ASCENT(font)/2;
	voffset += suboffset;
	above = max(above, voffset + ASCENT(font));
	break;
	
    case TAG_SUB:
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = (int)StyleGet(SI_FONT);
	
	suboffset = -ASCENT(font)/2;
	voffset += suboffset;
	below = max(below, DESCENT(font) - voffset);
	break;
	
    case TAG_MATH:
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = (int)StyleGet(SI_FONT);
/*	    left += style->indent;*/
	
	FlushLine(NOBREAK, frame, align, emph, font, left, right);
/*            font = IDX_IFIXEDFONT;*/
	
/*	    StylePush(style);*/
	ParseMath(frame, &up, &down);
/*	    style = StylePop();*/
	
	above = max(above, up);
	below = max(below, down);
	
	if (Here > TextLineWidth)
	    TextLineWidth = Here;
	
	if (element_started)
	    FormatElementEnd();
	return;
	
    case TAG_ITALIC:
    case TAG_EM:
    case TAG_DFN:
    case TAG_CITE:
    case TAG_VAR:
    case TAG_Q:
    case TAG_BOLD:
    case TAG_STRONG:
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = (int)StyleGet(SI_FONT);
	break;
	
    case TAG_UNDERLINE:
	SwallowAttributes();
	emph |= EMPH_UNDERLINE;
	break;
	
    case TAG_STRIKE:
    case TAG_REMOVED:  /* <removed> doesn't work across <P> etc. */
	SwallowAttributes();
	emph |= EMPH_STRIKE;
	break;
	
    case TAG_ADDED:  /* doesn't work across <P> etc! */
	SwallowAttributes();
	emph |= EMPH_HIGHLIGHT;
	
	ParseClassAttrs(&class, &class_len);
	FormatElementStart(ThisToken, class, class_len);
	element_started = TRUE;
	font = (int)StyleGet(SI_FONT);
/*
  if (preformatted)
  {
  if (font == IDX_BFIXEDFONT)
  font = IDX_BIFIXEDFONT;
  else
  font = IDX_IFIXEDFONT;
  }
  else
  {
  if (font == IDX_BNORMALFONT)
  font = IDX_BINORMALFONT;
  else
  font = IDX_INORMALFONT;
  }
  */
	break;
	
    case TAG_FORM: /* --Spif */
        name=NULL;
	ParseFormAttrs(&type, &name, &nlen);
	if ((form = (Form *)FindForm(forms, name)) == NULL)
	{
	    form = GetForm(type, name, nlen);
	};
	break;
	
    case TAG_TEXTAREA:  /* a short term bodge */
	ParseTextAreaAttrs(&type, &name, &nlen, &value, &vlen, &rows, &cols, &flags);
	FormatElementStart(ThisToken, NULL, 0);
	element_started = TRUE;
	
	if (form == NULL)
	    form = DefaultForm(); /* what a hack ! input outside a form is permitted ???? */
	
	field = GetField(form, type, Here+1, name, nlen, value, vlen, rows, cols, flags|font);
	WrapFieldIfNeeded(field, frame, align, left, right);
	PrintInputField(frame, field);
	Here += field->width + 2;
	StartOfLine = StartOfWord = bufptr;
	LineLen = LineWidth = WordStart = 0;
	
	if (element_started)
	    FormatElementEnd();
	return;
	
    case TAG_INPUT:   /* --Spif src=img ? */
	ParseInputAttrs(&type, &name, &nlen, &value, &vlen, &size, &flags, &image);
	
	FormatElementStart(ThisToken, NULL, 0);
	element_started = TRUE;
	
	if (form == NULL)
	    form = DefaultForm();
	
	if (type != HIDDEN) /* --Spif don't show hidden fields */ 
	{		
	    if (type == SUBMITBUTTON || type == RESETBUTTON)
		/* font = IDX_H3FONT; */
		font = (int)StyleGet(SI_FONT);
	    
	    /* --Spif image only on submit and reset buttons... */    
	    if ((type == SUBMITBUTTON || type == RESETBUTTON) && (image !=NULL)) 
	    {
		int above1,below1;
		p=bufptr;
		while (*p != '<') p--;
		width = image->width + 8; /* +8 because of the emph_whatever... */
		height = image->height+ 8;
		if (! (width && height)){
		    width = IMG_WIDTH;
			height = IMG_HEIGHT;
		};
		/* --Spif hack */ emph |= EMPH_INPUT ; 
		delta = height;
		above1 = delta + 2;
		below1 = 0;
		field = GetField(form, type, Here+1, name, nlen, value, vlen, 1, size, flags|font);
		WrapImageIfNeeded(frame, align, left, right, width, above1, below1);
		PrintImage(frame, delta, emph, p, image, width, height);
		Here += width + 2; /* check that with emph... */ 
	    }
	    else
	    {
		if (type == SUBMITBUTTON || type == RESETBUTTON) /* test if no name and/or value tags --SPif 12-Oct-95 */
		{
		    if(nlen)
			field = GetField(form, type, Here+1, name, nlen, value, vlen, 1, size, flags|font);
		    else
			if(vlen)
			    field = GetField(form, type, Here+1, value, vlen, value, vlen, 1, size, flags|font); /* value as id */
			else  /* do this 'cause name is not necessary in submit & reset buttons */
			    if (type == SUBMITBUTTON)
				field = GetField(form, type, Here+1, bufptr , 14 , " Submit Query ", 14, 1, size, flags|font);
			    else                                     /* --Spif 11-Oct-95 it's ugly but it works */
				field = GetField(form, type, Here+1, bufptr , 7, " Reset ", 7, 1, size, flags|font); 
		}
		else
		    field = GetField(form, type, Here+1, name, nlen, value, vlen, 1, size, flags|font);
		
		WrapFieldIfNeeded(field, frame, align, left, right);
		PrintInputField(frame, field);
		Here += field->width+2;
	    };
	    
	    StartOfLine = StartOfWord = bufptr;
	    LineLen = LineWidth = WordStart = 0;
	    if (element_started)
		FormatElementEnd();
	}
	else
	{ /* add hidden input field */
	    field = GetField(form, type, Here+1, name, nlen, value, vlen, 1, size, flags|font);
	};
	return;
	
    case TAG_SELECT:
	ParseSelect(0, frame, font, align, left, right);
	StartOfLine = StartOfWord = bufptr;
	LineLen = LineWidth = WordStart = 0;
	if (element_started)
	    FormatElementEnd();
	return;
	
    case TAG_OPTION:
	ParseSelect(1, frame, font, align, left, right);
	StartOfLine = StartOfWord = bufptr;
	LineLen = LineWidth = WordStart = 0;
	if (element_started)
	    FormatElementEnd();
	return;
	
    case TAG_BR:
	ParseParaAttrs(&align, &class, &class_len);
	if (paintStartLine >= 0)
	    EndOfLine(frame, align);
	else
	    PixOffset += ASCENT((int)StyleGet(SI_FONT)) + DESCENT((int)StyleGet(SI_FONT));
	
	Here = left;
	StartOfLine = StartOfWord = bufptr;
	LineLen = LineWidth = WordStart = 0;
	if (element_started)
	    FormatElementEnd();
	return;
	
    case TAG_IMG:
	p = bufptr - 4;
	ParseImageAttrs(&href, &hreflen, &valign, &ismap, &width, &height);
	
	if ( (image = GetImage(href, hreflen, CurrentDoc->pending_reload)) ) {
	    width = image->width;
	    height = image->height;
	} 
	
	if (! (width && height)){
	    width = IMG_WIDTH;
	    height = IMG_HEIGHT;
	}
	
	if (ismap | (emph & EMPH_ANCHOR))
	{
	    if (ismap)
		emph |= ISMAP;
	    
	    width += 8;
	    height += 8;
	}
	else
	    p = NULL;
	
	{
	    int above1, below1;
	    
	    if (valign == ALIGN_BOTTOM)
	    {
		delta = height;
		above1 = delta + 2;
		below1 = 0;
		WrapImageIfNeeded(frame, align, left, right, width, above1, below1);
		PrintImage(frame, delta, emph, p, image, width, height);
	    }
	    else if (valign == ALIGN_MIDDLE)
	    {
		delta = height/2;
		above1 = delta + 2;
		below1 = (above1 < height ? height - above1 : 0);
		WrapImageIfNeeded(frame, align, left, right, width, above1, below1);
		PrintImage(frame, delta, emph, p, image, width, height);
	    }
	    else  /* ALIGN_TOP */
	    {
		delta = ASCENT(font) - 2;
		below1 = 2 + height - delta;
		
		if (below1 < 0)
		    below1 = 0;
		
		above1 = 2 + (height > below1 ? height - below1 : 0);
		
		WrapImageIfNeeded(frame, align, left, right, width, above1, below1);
		PrintImage(frame, delta, emph, p, image, width, height);
	    }
	    
	    Here += width + 2;
	}
	
	StartOfLine = StartOfWord = bufptr;
	LineLen = LineWidth = WordStart = 0;
	
	if (element_started)
	    FormatElementEnd();
	return;
	
    default:
	SwallowAttributes();
	break;
    }
    
    StartOfLine = StartOfWord = bufptr;
    LineLen = LineWidth = WordStart = 0;
    
    if (ThisToken == TAG_Q)
    {
	/* open quote char  */
/*        width = XTextWidth(Fonts[IDX_NORMALFONT], "``", 2);*/
/*        PrintSeqText(frame, EMPH_NORMAL, IDX_NORMALFONT, "``", width);*/
        width = XTextWidth(Fonts[(int)StyleGet(SI_FONT)], "``", 2);
        PrintSeqText(frame, EMPH_NORMAL, (int)StyleGet(SI_FONT), "``", width);
        Here += width;
    }

    for (;;)
    {
        GetToken();

        if (Token == ThisToken && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }


        if (TokenClass != EN_TEXT)
        {
	    if (Token != TAG_HR) {
	        error |= ERR_EMPH;
		RecordError(bufptr, "ERR_EMPH"); /* howcome 11/10/94 */
	    };
            UnGetToken();
            break;
        }

        if (Token == WHITESPACE)
        {
            if (preformatted)
            {
                if (TokenValue == '\n')
                    FlushLine(BREAK, frame, align, emph, font, left, right);
                else
                {
                    if (LineLen < LBUFSIZE - 1)
                        LineBuf[LineLen++] = ' ';
                }
                continue;
            }

            while (GetToken() == WHITESPACE);
            UnGetToken();

            if (Here == left && LineLen == 0)
            {
                StartOfLine = StartOfWord = bufptr;
                continue;
            }

            /* check that we have a word */

            if ((WordLen = LineLen - WordStart) > 0)
                WrapIfNeeded(frame, align, emph, font, left, right);

            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';

            WordStart = LineLen;
            StartOfWord = bufptr;
            continue;
        }

        if (IsTag(Token))
        {
            FlushLine(NOBREAK, frame, align, emph, font, left, right);
	    ParseEmph(frame, align, emph, font, left, right);
/*	    if (Token != TAG_BR && Token !=TAG_HR)
		FormatElementStart(s, class, class_len); */ /* howcome 30/2/95: revert style after emph section */
            continue;
        }

        /* must be PCDATA */

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    FlushLine(NOBREAK, frame, align, emph, font, left, right);
    voffset -= suboffset;

    if (ThisToken == TAG_Q)
    {
      /* close quote char  */
/*        width = XTextWidth(Fonts[IDX_NORMALFONT], "''", 2);*/
/*        PrintSeqText(frame, IDX_NORMALFONT, "''", width);*/
        width = XTextWidth(Fonts[(int)StyleGet(SI_FONT)], "''", 2);
        PrintSeqText(frame, EMPH_NORMAL, (int)StyleGet(SI_FONT), "''", width);
        Here += width;
    }

    if (ThisToken == TAG_MARGIN)
    {
        EndOfLine(frame, align);
        Here = left -  indent;
        PixOffset += LineSpacing[font];
        StartOfLine = StartOfWord = bufptr;
        LineLen = LineWidth = WordStart = 0;
    }

/*    style = StylePop();*/

/*    style = FormatElementStart(revert_tag, NULL, 0); *//* reset to P style after emph */

    if (element_started)
	FormatElementEnd();
}

void ParseHeader(Frame *frame, int align, int left, int right, int outdent)
{
    int HeaderTag, WordLen, emph; /* janet: not used: , font; */

    align = -1;
    ParseParaAttrs(&align, &class, &class_len);

    FormatElementStart(Token, class, class_len);
    font = (int)StyleGet(SI_FONT); 
    if (align < 0)
	align = StyleGet(SI_TEXT_ALIGN);

/*
    left += StyleGet(SI_MARGIN_LEFT);
    right += StyleGet(SI_MARGIN_RIGHT);
*/

    left = (int)StyleGet(SI_MARGIN_LEFT);
    right = (int)StyleGet(SI_MARGIN_RIGHT);

    if (EndTag) {
        return;
    }

    if (frame->leftmargin <= MININDENT) /* howcome: applied patch from dsr 18/11/94 */
        left -= outdent;

    HeaderTag  = Token;
    Here = left;

/*
    switch (HeaderTag)
    {
        case TAG_H1:
            font = IDX_H1FONT;
            break;

        case TAG_H2:
            font = IDX_H2FONT;
            break;

        case TAG_H3:
        case TAG_H4:
            font = IDX_H3FONT;
            break;

        default:
            font = IDX_H4FONT;
            break;
    }
*/

    PixOffset += (int)StyleGet(SI_MARGIN_TOP);

    emph = EMPH_NORMAL;

    StartOfLine = StartOfWord = bufptr;
    LineLen = LineWidth = WordStart = 0;

    for (;;)
    {
        GetToken();

        if (Token == HeaderTag && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (TokenClass != EN_TEXT)
        {
            error |= ERR_HEADER;
            RecordError(bufptr, "ERR_HEADER"); /* howcome 11/10/94 */
            UnGetToken();
            break;
        }

        if (Token == WHITESPACE)
        {
            while (GetToken() == WHITESPACE);
            UnGetToken();

            if (Here == left && LineLen == 0)
            {
                StartOfLine = StartOfWord = bufptr;
                continue;
            }

            /* check that we have a word */

            if ((WordLen = LineLen - WordStart) > 0)
                WrapIfNeeded(frame, align, emph, (int)StyleGet(SI_FONT), left, right);

            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';

            WordStart = LineLen;
            StartOfWord = bufptr;
            continue;
        }

        if (IsTag(Token))
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            FlushLine(NOBREAK, frame, align, emph, (int)StyleGet(SI_FONT), left, right);
            ParseEmph(frame, align, emph, font /* StyleGet(SI_FONT) */, left, right);
/*	    if (Token != TAG_BR && Token !=TAG_HR)
		FormatElementStart(HeaderTag, class, class_len); */ /* howcome 30/2/95: revert style after emph section */
            continue;
        }

        /* must be PCDATA */

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    FlushLine(BREAK, frame, align, emph, (int)StyleGet(SI_FONT), left, right);
/*    PixOffset += LineSpacing[StyleGet(SF_ONT)]/2;*/
    PixOffset += (int)StyleGet(SI_MARGIN_BOTTOM);

#if 0
    FormatElementStart(TAG_P, NULL, 0); /* reset to P style after headline */
    align = StyleGet(SI_TEXT_ALIGN); /* dummy call to force the initialization of last_value array */
#endif

    FormatElementEnd();

}

void ParsePara(int implied, Bool initial, Frame *frame, int palign, int tag, int left, int right)
{
    int WordLen, emph;
    int font;
    int align = -1;
    Frame *new_frame;
    int hack = 0;


    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    hack = paintStartLine>0 || damn_table;

    if (!implied) {
        ParseParaAttrs(&align, &class, &class_len);
    };/* else printf("implied P\n");*/

    FormatElementStart(tag, class, class_len);
    if (align < 0)
	align = (int)StyleGet(SI_TEXT_ALIGN);

    if (initial) {
	left = (int)StyleGet(SI_MARGIN_LEFT);
    }

    font = (int)StyleGet(SI_FONT);
    right = (int)StyleGet(SI_MARGIN_RIGHT);

    if (Here < left)
        Here = left;

    if(!implied&&!hack)
    {
/*
 *	printf("----------------------------\nold values:\nindent %d, marginleft %d, right %d, width %d, left %d, right %d\n",
 *	       frame->indent, frame->leftmargin, frame->rightmargin, frame->width,left, right);
 *	printf("data margins left %d, right %d, padding %d\n", StyleGet(SI_MARGIN_LEFT),StyleGet(SI_MARGIN_RIGHT)
 *	       ,StyleGet(S_PADDING));
 */
	new_frame = (Frame *)calloc(1, sizeof(Frame));
	PixOffset += (int)StyleGet(SI_PADDING_TOP)+ (int)StyleGet(SI_MARGIN_TOP); /* must be S_PADDING_TOP, righT! */
	new_frame->offset = PixOffset;
	new_frame->leftmargin = StyleGet(SI_PADDING_LEFT);
	new_frame->rightmargin = StyleGet(SI_PADDING_RIGHT);
#if 0
	    right + frame->rightmargin/* + StyleGet(SI_MARGIN_RIGHT)*/;
#endif
	new_frame->indent = frame->indent + left; /* S_PADDING_LEFT */
	new_frame->width = frame->width - left - right/* -left -right */;
	new_frame->style = 0;
	new_frame->border = 0;
#ifdef STYLE_COLOR_BORDER
	new_frame->cb_ix = 0;
#else
	new_frame->cb_ix = 0;
#endif
	new_frame->flow = align;
	new_frame->next = new_frame->child = NULL;
	new_frame->box_list = NULL;
	CopyBoxList(new_frame, frame);
	new_frame->leftcount = frame->leftcount;
	new_frame->pushcount = frame->pushcount;
	new_frame->oldmargin = frame->oldmargin;
	PrintBeginFrame(new_frame); 
	left = (int)StyleGet(SI_PADDING_LEFT); /* S_PADDING_LEFT */
	right = (int)StyleGet(SI_PADDING_RIGHT); /* S_PADDING_RIGHT */
	Here = left;
	Push(new_frame);
/*
 *	printf("new values:\nindent %d, marginleft %d, right %d, width %d, left %d, right %d\n",
 *	       new_frame->indent, new_frame->leftmargin, new_frame->rightmargin, new_frame->width, left, right);
 */ 
    }
    else
	new_frame = frame;
    emph = EMPH_NORMAL;

    /* skip leading white space - subsequently contigous
       white space is compressed to a single space */

    while (GetToken() == WHITESPACE);
    UnGetToken();

/*    above = below = 0;       */ /* commented out by howcome 26/7/94: in some cases (eg when there is a tall image in a DT), these still carry important information, e.g. in http://info.cern.ch/ */

    StartOfLine = StartOfWord = bufptr;
    LineLen = LineWidth = WordStart = 0;

    for (;;)
    {
        GetToken();

        if (Token == TAG_P && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (TokenClass != EN_TEXT)
        {
            UnGetToken();
            break;
        }

        if (Token == WHITESPACE)
        {
            while (GetToken() == WHITESPACE);
            UnGetToken();

            if (Here == left && LineLen == 0)
            {
                StartOfLine = StartOfWord = bufptr;
                continue;
            }

            /* check that we have a word */

            if ((WordLen = LineLen - WordStart) > 0)
                WrapIfNeeded(new_frame, align, emph, font, left, right);

            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';

            WordStart = LineLen;
            StartOfWord = bufptr;
            continue;
        }

        if (IsTag(Token))
        {
            if (EndTag)
            {
                SwallowAttributes();
                error |= ERR_EMPH;
		RecordError(bufptr, "ERR_EMPH"); /* howcome 24/11/94 */
                continue;
            }

            FlushLine(NOBREAK, frame, align, emph, font, left, right);
            ParseEmph(new_frame, align, emph, font, left, right);
	    StartOfLine = StartOfWord = bufptr; /* howcome 28/10/95: added to reset buffer after math -- does it break anything? */
/*	    if (Token != TAG_BR && Token !=TAG_HR)
		FormatElementStart(revert_tag, class, class_len);  *//* howcome 30/2/95: revert style after emph section */
            continue;
        }

        /* must be PCDATA */

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    FlushLine(BREAK, new_frame, align, emph, font, left, right);
/*    PixOffset += LineSpacing[font]/3;*/

    if(!implied&&!hack)
    {
	char *p;

	Pop();
	new_frame->length = paintlen - new_frame->info - FRAMESTLEN;
	new_frame->height = PixOffset - new_frame->offset + StyleGet(SI_PADDING_TOP) + StyleGet(SI_PADDING_BOTTOM);
	new_frame->offset -= (int)StyleGet(SI_PADDING_TOP); /* ?? */
	PixOffset += (int)StyleGet(SI_PADDING_TOP);  /* ?? */
	new_frame->indent -= (int)StyleGet(SI_PADDING_LEFT);  /* ?? */
	p = paint + new_frame->info + 1;
        PushValue(p, new_frame->offset & 0xFFFF);
        PushValue(p, (new_frame->offset >> 16) & 0xFFFF);
        PushValue(p, new_frame->indent);
        PushValue(p, new_frame->width);
	PushValue(p, new_frame->height & 0xFFFF);
	PushValue(p, (new_frame->height >> 16) & 0xFFFF);
	PrintFrameLength(new_frame);
	PrintEndFrame(frame, new_frame); 
	FreeFrames(new_frame);
	new_frame = NULL;
    };
    PixOffset += (int)StyleGet(SI_MARGIN_BOTTOM);
    FormatElementEnd();
}

void ItemNumber(char *buf, int depth, int n)
{
    int w, ones, tens, hundreds, thousands;
    char *p, *q;

    w = depth % 3;

    if (w == 0)
        sprintf(buf, "%d.", n);
    else if (w == 1)
    {
        thousands = n/1000;
        n = n % 1000;
        hundreds = n/100;
        n = n % 100;
        tens = n/10;
        ones = n % 10;

        p = buf;

        while (thousands-- > 0)
               *p++ = 'm';

        if (hundreds)
        {
            q = Hundreds[hundreds-1];
            while ((*p++ = *q++));
            --p;
        }

        if (tens)
        {
            q = Tens[tens-1];
            while ((*p++ = *q++));
            --p;
        }

        if (ones)
        {
            q = Ones[ones-1];
            while ((*p++ = *q++));
            --p;
        }

        *p++ = ')';
        *p = '\0';
    }
    else
        sprintf(buf, "%c)", 'a' + (n-1)%26);
}


void ParsePRE(int implied, Bool initial, Frame *frame, int left, int right)
{
    int ch, n, emph, font, lastTokenValue; /* janet: not used: WordLen */

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    FormatElementStart(Token, class, class_len);

    if (!implied) {
/*        SwallowAttributes();*/

	ParseClassAttrs(&class, &class_len);
/*
	StyleRegisterSequential(Token, class, class_len);
	StyleRegisterHierarchical(Token, class, class_len);
*/
/*	FormatElementStart(Token, class, class_len);*/
	font = (int)StyleGet(SI_FONT);
    }


    if (initial) {
	left = (int)StyleGet(SI_MARGIN_LEFT);
    }

    Here = left;

    preformatted = 1;
    emph = EMPH_NORMAL;
/*    font = IDX_FIXEDFONT;*/
    ch = CHWIDTH(font);

    /* skip leading newline */

    if (*bufptr == '\r')
        ++bufptr;

    if (*bufptr == '\n')
        ++bufptr;

    /* howcome 12/10/94: moved the two lines below from before the ++bufptr statements */

    StartOfLine = StartOfWord = bufptr;
    LineLen = LineWidth = WordStart = 0;

    for (;;)
    {
        lastTokenValue = TokenValue;
        GetToken();

        if (Token == TAG_PRE && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

     /* HR isn't really allowed in PRE, but ... */
        if (Token == TAG_HR)
        {
            FlushLine(BREAK, frame, (int)StyleGet(SI_TEXT_ALIGN), emph, font, left, right);
/*            SwallowAttributes();*/

            if (EndTag)
                continue;

	    ParseClassAttrs(&class, &class_len);
	    FormatElementStart(Token, class, class_len);


/*            error |= ERR_PRE;*/ /* howcome 30/5/95: remember not to flag.. */
/*            RecordError(bufptr, "ERR_PRE"); */ /* howcome 12/3/96: HR *is* allowed within PRE */

            PrintRule(frame, GROOVE, left, right, 0);
            EndOfLine(frame, (int)StyleGet(SI_TEXT_ALIGN));

	    FormatElementEnd();
            continue;
        }

     /* P isn't really allowed in PRE, but ... */
        if (Token == TAG_P)
        {
	    FlushLine(BREAK, frame, (int)StyleGet(SI_TEXT_ALIGN), emph, font, left, right);
	    /* there were two lines here... suspect a bug */
            SwallowAttributes();
            continue;
        }

     /* Ignore header tags in preformatted text */

        if (TokenClass == EN_HEADER)
        {
            error |= ERR_PRE;
            RecordError(bufptr, "ERR_PRE"); /* howcome 11/10/94 */
            SwallowAttributes();
            continue;
        }

        if (TokenClass != EN_TEXT)
        {
            error |= ERR_PRE;
            RecordError(bufptr, "ERR_PRE"); /* howcome 11/10/94 */
            UnGetToken();
            break;
        }

        if (Token == WHITESPACE && TokenValue == '\n')
        {
            FlushLine(BREAK, frame, (int)StyleGet(SI_TEXT_ALIGN), emph, font, left, right);
            continue;
        }

        if (IsTag(Token))
        {
	  /* janet: not used: 	    int s = style->tag; */

            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            FlushLine(NOBREAK, frame, (int)StyleGet(SI_TEXT_ALIGN), emph, font, left, right);
            ParseEmph(frame, ALIGN_LEFT, emph, font, left, right);

	    /* howcome 15/3/95: try to align characters in pre mode */
            if (Token == TAG_ANCHOR)  /* emph & EMPH_ANCHOR)*/
                Here -= 2;
/*
	    if (Token != TAG_BR && Token !=TAG_HR)
		style = FormatElementStart(s, class, class_len);
*/
            continue;
        }

        /* must be PCDATA */

/* CopyLine can't work out how many spaces to use! */
        if (TokenValue == '\t')
        {
            n = LineLen;  /* + (Here - left)/ch; */
            n = 8 - n % 8;

            while (n-- > 0 && LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';
            continue;
        }

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    FlushLine((lastTokenValue == '\n' ? NOBREAK : BREAK), frame,
              (int)StyleGet(SI_TEXT_ALIGN), emph, font, left, right);

    PixOffset += LineSpacing[font]/3; /* fix whitespace bug dsr 16-Nov-94 */ /* howcome applied patch 23/1/94 */
    preformatted = 0;

/*    FormatElementStart(TAG_P, NULL, 0);*/ /* reset to P style after PRE */

    FormatElementEnd();
}



/* advance declaration due for nested lists */
void ParseUL(int implied, Bool initial, Frame *frame, int depth, int align, int left, int right);
void ParseOL(int implied, Bool initial, Frame *frame, int depth, int align, int left, int right);
void ParseDL(int implied, Bool initial, Frame *frame, int left, int align, int right);

void ParseLI(int implied, Frame *frame, int depth, int seq, int align, int left, int right)
{
    int indent, w;
    long y;
    char buf[16];

    if (EndTag)
    {
        SwallowAttributes();
        return;
   }

/*    indent = XTextWidth(Fonts[IDX_NORMALFONT], "mmm", 3);*/


    y = PixOffset;

    FormatElementStart(TAG_LI, class, class_len);

    if (!implied) {
	ParseClassAttrs(&class, &class_len);
/*	FormatElementStart(Token, class, class_len);*/
    }
    indent = (int)StyleGet(SI_MARGIN_LEFT);


    w = left + indent/3;

    if (w < Here + 20)
        EndOfLine(frame, ALIGN_LEFT);

    Here = w;

    if (seq > 0)
    {
        ItemNumber(buf, depth++, seq);
        w = XTextWidth(Fonts[(int)StyleGet(SI_FONT)], buf, strlen(buf));
        PrintSeqText(frame, EMPH_NORMAL, (int)StyleGet(SI_FONT), buf, w);

        if (w + indent/3 > indent - 4)
            indent = 4 + w + indent/3;
    }
    else
        PrintBullet(frame, depth++, (int)StyleGet(SI_FONT), (int)StyleGet(SI_FONT_SIZE));

    if ((w = left + indent) > list_indent)    /* for tables */
        list_indent = w;

    for (;;)
    {
        while (GetToken() == WHITESPACE);
/*	style = FormatElementStart(Token, class, class_len); *//* howcome 15/3/94 */

        if (Token == TAG_LI && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == TAG_UL)
        {
            UnGetToken();

            if (EndTag)
                break;

            ParseUL(0, False, frame, depth, align, left + indent, right);
            continue;
        }

        if (Token == TAG_OL)
        {
            UnGetToken();

            if (EndTag)
                break;

            ParseOL(0, False, frame, depth, align, left + indent, right);
            continue;
        }

        if (Token == TAG_DL)
        {
            UnGetToken();

            if (EndTag)
                break;

            ParseDL(0, False, frame, align, left + indent, right);
            continue;
        }

        if (Token == TAG_DT || Token == TAG_DD)
        {
            UnGetToken();
            ParseDL(1, False, frame, align, left + indent, right);
            continue;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_P)
        {
            Here = left + indent;
/*	    StyleSetFlag(S_INDENT_FLAG,TRUE);
	    StyleSetFlag(S_LEADING_FLAG,TRUE);*/
/*	    PixOffset += style->margin_top;*//* howcome 15/3/95: not a good idea */
            ParsePara(0, False, frame, align, TAG_P, left + indent, right);
	    continue;
        }

        if (Token == TAG_HR)
        {
/*            SwallowAttributes();*/

	    ParseClassAttrs(&class, &class_len);
/*
	    StyleRegisterSequential(Token, class, class_len);
*/
	    FormatElementStart(Token, class, class_len);

            if (EndTag)
                continue;

            PrintRule(frame, GROOVE, left + indent, right, 0);
            EndOfLine(frame, (int)StyleGet(SI_TEXT_ALIGN));
            continue;
        }

	/* howcome 11/8/95: support for PRE added */

        if (Token == TAG_PRE)
        {
            ParsePRE(0, False, frame, left + indent, right);
            continue;
        }

        if (TokenClass == EN_TEXT)
        {
            UnGetToken();
            ParsePara(1, False, frame, align, TAG_P, left + indent, right);
            continue;
        }

     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }    

    /* kludge to cope with an <LI> element with no content */

    if (y == PixOffset)
        PixOffset += (4 * LineSpacing[IDX_NORMALFONT])/3;

    FormatElementEnd();
}

void ParseUL(int implied, Bool initial, Frame *frame, int depth, int align, int left, int right)
{
    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    FormatElementStart(Token, class, class_len);

    if (!implied) {
/*        SwallowAttributes();*/
	ParseClassAttrs(&class, &class_len);
/*
	StyleRegisterSequential(Token, class, class_len);
	StyleRegisterHierarchical(Token, class, class_len);
*/
/*	FormatElementStart(Token, class, class_len);*/
	left += (int)StyleGet(SI_MARGIN_LEFT);
    }

    if (initial)	
	left += (int)StyleGet(SI_MARGIN_LEFT);

    for (;;)
    {
        while (GetToken() == WHITESPACE);
/*	style = FormatElementStart(Token, class, class_len); */ /* howcome 15/3/95 */

        if (Token == TAG_UL && EndTag)
        {
            SwallowAttributes();
/*	    StyleDeregisterHierarchical();*/
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_LI)
        {
            ParseLI(0, frame, depth, 0, align, left, right);
            continue;
        }

        if (Token == TAG_P || Token == TAG_HR || TokenClass == EN_TEXT)
        {
            if (EndTag)
            {
		SwallowAttributes();
                continue;
            }

            UnGetToken();
            ParseLI(1, frame, depth, 0, align, left, right);
            continue;
        }

        if (Token == TAG_UL || Token == TAG_OL ||
             Token == TAG_DL || Token == TAG_DT || Token == TAG_DD)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            ParseLI(1, frame, depth, 0, align, left, right);
            continue;
        }

     /* unexpected tag so terminate element */

        error |= ERR_UL;
        RecordError(bufptr, "ERR_UL"); /* howcome 11/10/94 */
        UnGetToken();
        break;
    }    

    Here = left;

    FormatElementEnd();
}

void ParseOL(int implied, Bool initial, Frame *frame, int depth, int align, int left, int right)
{
    int seq;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    FormatElementStart(Token, class, class_len);

    if (!implied) {
	ParseClassAttrs(&class, &class_len);
/*
	StyleRegisterSequential(Token, class, class_len);
	StyleRegisterHierarchical(Token, class, class_len);
*/
/*	FormatElementStart(Token, class, class_len);*/
    }

    if (initial)
	left += (int)StyleGet(SI_MARGIN_LEFT);

    seq = 0;

    for (;;)
    {
        ++seq;

        while (GetToken() == WHITESPACE);
/*	style = FormatElementStart(Token, class, class_len); *//* howcome 15/3/95 */

        if (Token == TAG_OL && EndTag)
        {
            SwallowAttributes();
/*	    StyleDeregisterHierarchical();*/
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_LI)
        {
            ParseLI(0, frame, depth, seq, align, left, right);
            continue;
        }

        if (Token == TAG_P || Token == TAG_HR || TokenClass == EN_TEXT)
        {
            if (EndTag)
            {
		SwallowAttributes();
                continue;
            }

            UnGetToken();
            ParseLI(1, frame, depth, seq, align, left, right);
            continue;
        }

        if (Token == TAG_UL || Token == TAG_OL ||
             Token == TAG_DL || Token == TAG_DT || Token == TAG_DD)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            ParseLI(1, frame, depth, seq, align, left, right);
            continue;
        }

     /* unexpected tag so terminate element */

        error |= ERR_OL;
        RecordError(bufptr, "ERR_OL"); /* howcome 11/10/94 */
        UnGetToken();
        break;
    }    

    Here = left;

    FormatElementEnd();
}

void ParseDT(int implied, Frame *frame, int align, int left, int right)
{
    int WordLen, emph, font;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    FormatElementStart(Token, class, class_len);

    if (!implied) {
/*        SwallowAttributes();*/
	ParseClassAttrs(&class, &class_len);
    }
/*
    StyleRegisterSequential(Token, class, class_len);
*/

    Here = left;

    emph = EMPH_NORMAL;

    font = (int)StyleGet(SI_FONT);
/*    font = IDX_BNORMALFONT;*/

    /* skip leading white space - subsequently contigous
       white space is compressed to a single space */
    while (GetToken() == WHITESPACE);
    UnGetToken();

    StartOfLine = StartOfWord = bufptr;
    LineLen = LineWidth = WordStart = 0;

    for (;;)
    {
        GetToken();
/*	style = FormatElementStart(Token, class, class_len); *//* howcome 15/3/95 */

        if (Token == TAG_DT && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (TokenClass != EN_TEXT)
        {
            UnGetToken();
            break;
        }

        if (Token == WHITESPACE)
        {
            while (GetToken() == WHITESPACE);
            UnGetToken();

            /* check that we have a word */

            if ((WordLen = LineLen - WordStart) > 0)
                WrapIfNeeded(frame, align, emph, font, left, right);

            if (LineLen < LBUFSIZE - 1)
                LineBuf[LineLen++] = ' ';

            WordStart = LineLen;
            StartOfWord = bufptr;
            continue;
        }

        if (IsTag(Token))
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            FlushLine(NOBREAK, frame, align, emph, font, left, right);
            ParseEmph(frame, align, emph, font, left, right);
/*	    if (Token != TAG_BR && Token !=TAG_HR)
		FormatElementStart(TAG_P, class, class_len); *//* howcome 6/3/95: should there be a revert_tag here ? */
            continue;
        }

        /* must be PCDATA */

        if (LineLen < LBUFSIZE - 1)
            LineBuf[LineLen++] = TokenValue;
    }    

    LineBuf[LineLen] = '\0';
    FlushLine(NOBREAK, frame, align, emph, font, left, right);
    Here += 5; /* +5 pb here? */

    FormatElementEnd();
}

void ParseDD(int implied, Frame *frame, int align, int left, int right)
{
    long y;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    FormatElementStart(Token, class, class_len);

    if (!implied) {
	ParseClassAttrs(&class, &class_len);
    }

    y = PixOffset;

    for (;;)
    {
        while (GetToken() == WHITESPACE);
/*	style = FormatElementStart(Token, class, class_len); *//* howcome 15/3/95 */

        if (Token == TAG_DD && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == TAG_DL && EndTag)
        {
            UnGetToken();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_UL)
        {
            ParseUL(0, False, frame, 0, align, left, right);
            continue;
        }

        if (Token == TAG_OL)
        {
            ParseOL(0, False, frame, 0, align, left, right);
            continue;
        }

        if (Token == TAG_LI)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            ParseUL(1, False, frame, 0, align, left, right);
            continue;
        }

        if (Token == TAG_DL)
        {
            if (PixOffset == y) /* force a line break */
            {
                EndOfLine(frame, align);
                Here = left;
            }

            ParseDL(0, False, frame, align, left, right);
            Here = left;
            continue;
        }

        if (Token == TAG_HR)
        {
	    ParseClassAttrs(&class, &class_len);
	    FormatElementStart(Token, class, class_len);

            if (EndTag)
                continue;

            PrintRule(frame, GROOVE, left, right, 0);
            EndOfLine(frame, (int)StyleGet(SI_TEXT_ALIGN));
            continue;
        }

	/* howcome 25/9/95: support for PRE added */

        if (Token == TAG_PRE)
        {
            ParsePRE(0, False, frame, left + (int)StyleGet(SI_MARGIN_LEFT), right);
            continue;
        }

        if (Token == TAG_P)
        {
	    ParsePara(0, False, frame, align, TAG_P, left, right);
            continue;
        }

        if (TokenClass == EN_TEXT || Token == ENTITY)
        {
            UnGetToken();
            ParsePara(1, False, frame, align, TAG_P, left, right);
            continue;
        }

     /* unexpected tag so terminate element */

        if (Token != TAG_DT) {
            error |= ERR_DL;
            RecordError(bufptr, "ERR_DL"); /* howcome 11/10/94 */
        }
        UnGetToken();
        break;
    }

    /* kludge to cope with an <DD> element with no content */

    if (y == PixOffset)
    {
        PixOffset += LineSpacing[(int)StyleGet(SI_FONT)]/3;
        EndOfLine(frame, (int)StyleGet(SI_TEXT_ALIGN));
    }

    FormatElementEnd();
}

void ParseDL(int implied, Bool initial, Frame *frame, int align, int left, int right)
{
    int indent, LastToken; /* janet: not used: delta */

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    FormatElementStart(Token, class, class_len);

    if (!implied) {
/*        SwallowAttributes();*/
        ParseClassAttrs(&class, &class_len);
/*
	StyleRegisterSequential(Token, class, class_len);
	StyleRegisterHierarchical(Token, class, class_len);
*/
/*	FormatElementStart(Token, class, class_len);*/
	font = (int)StyleGet(SI_FONT);
	left += (int)StyleGet(SI_MARGIN_LEFT);
    }

    if (initial)
	left += (int)StyleGet(SI_MARGIN_LEFT);

    LastToken = TAG_DL;
/*    indent = XTextWidth(Fonts[IDX_NORMALFONT], "mm", 2);*/
    indent = (int)StyleGet(SI_MARGIN_LEFT);



    for (;;)
    {
        while (GetToken() == WHITESPACE);
/*	style = FormatElementStart(Token, class, class_len); *//* howcome 15/3/95 */

        if (Token == TAG_DL && EndTag)
        {
            SwallowAttributes();
/*	    StyleDeregisterHierarchical();*/

            if (LastToken == TAG_DT)
                EndOfLine(frame, align);
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_DT )
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            if (LastToken == TAG_DT)
                EndOfLine(frame, align);

            ParseDT(0, frame, align, left, right);
            LastToken = TAG_DT;
            continue;
        }

        if (Token == TAG_DD )
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            if (LastToken != TAG_DT) {
                error |= ERR_DL;
                RecordError(bufptr, "ERR_DL"); /* howcome 11/10/94 */
            }

            ParseDD(0, frame, align, left + indent, right);
            LastToken = TAG_DD;
            continue;
        }

        if (Token == TAG_P || Token == TAG_HR || TokenClass == EN_TEXT ||
            Token == TAG_UL || Token == TAG_LI || Token == TAG_OL || Token == TAG_DL)
        {
            if (EndTag)
            {
		SwallowAttributes();
                continue;
            }

            error |= ERR_DL;
            RecordError(bufptr, "ERR_DL"); /* howcome 11/10/94 */

            UnGetToken();
            ParseDD(1, frame, align, left + indent, right);
            LastToken = TAG_DD;
            continue;
        }

     /* unexpected tag so terminate element */

        error |= ERR_DL;
        RecordError(bufptr, "ERR_DL"); /* howcome 11/10/94 */

        UnGetToken();
        break;
    }

    Here = left;    

    FormatElementEnd();
}

void ParseCaption(int implied, Frame *frame, int figure, int left, int right)
{
    int align;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
        SwallowAttributes();

    align = (figure ? ALIGN_LEFT : ALIGN_CENTER);

    for (;;)
    {
        while (GetToken() == WHITESPACE);

        if (Token == TAG_CAPTION && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_P)
        {
            Here = left;
	    /*  PixOffset += (int)StyleGet(SI_MARGIN_TOP); */ 
            ParsePara(0, False, frame, align, TAG_P, left, right);
            continue;
        }

        if (TokenClass == EN_TEXT)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            Here = left;
	    PixOffset += (int)StyleGet(SI_MARGIN_TOP);
            ParsePara(1, False, frame, align, TAG_P, left, right);
            continue;
        }

     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }    
}

/* skip alt text for non-graphical displays */
void ParseAlt(int implied)
{
    if (!implied)
        SwallowAttributes();

    for (;;)
    {
        GetToken();

        if (Token == TAG_ALT && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_P)
        {
            SwallowAttributes();
            continue;
        }

        if (TokenClass == EN_TEXT)
        {
            if (IsTag(Token))
                SwallowAttributes();

            continue;
        }

     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }    
}

void ParseFigure(int implied, Frame *frame, int left, int right)
{
    int align, font, hreflen, indent;
    char *href;
    Frame *figframe;
    Image *image;
    long FigureStart;
    int width, height;

/*
    frame = Pop();
    if(frame)
	Push(frame);
    else
	frame = eframe;
	*/
    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    align = ALIGN_CENTER;
    image = NULL;
    font = IDX_NORMALFONT;
    Here = left;
    indent = 0;

    if (!implied)
    {
        ParseFigureAttrs(&href, &hreflen, &align, &width, &height);
        if ((image = GetImage(href, hreflen, CurrentDoc->pending_reload))) {
	    width = image->width;
	    height = image->height;
	}

	if (! (width && height)) {
	    width = IMG_WIDTH;
	    height = IMG_HEIGHT;
	}
    }

/*    if (image)*/
    {
        if (align == ALIGN_RIGHT)
        {
            indent = frame->width - right - frame->rightmargin -
                        width - frame->leftmargin;
        }
        else if (align == ALIGN_CENTER)
        {
            indent = (frame->width - right - frame->rightmargin -
                        width - frame->leftmargin)/2;
        }
        else if (align == ALIGN_BLEEDLEFT)
            indent = 0;
        else if (align == ALIGN_BLEEDRIGHT)
        {
            indent = frame->width - frame->rightmargin -
                        width - frame->leftmargin;
        }
        else
/*            indent = left; */
	    indent = (int)StyleGet(SI_MARGIN_LEFT);

        if (indent > 0 && indent < left)
            indent = left;

        FigureStart = PixOffset;
        figframe = BeginFrame(frame, 0, 0, indent, indent+width,NULL);
	Push(figframe);
	
        if (align == ALIGN_LEFT || align == ALIGN_BLEEDLEFT)
        {
            frame->leftcount += 1;
            figframe->pushcount = frame->leftcount;
            figframe->oldmargin = frame->leftmargin;
            figframe->flow = ALIGN_LEFT;
        }
        else if (align == ALIGN_RIGHT || align == ALIGN_BLEEDRIGHT)
        {
            frame->rightcount += 1;
            figframe->pushcount = frame->rightcount;
            figframe->oldmargin = frame->rightmargin;
            figframe->flow = ALIGN_RIGHT;
        }

        Here = 0; /* position image flush left in figframe */
        PrintImage(figframe, 0, 0, 0, image, width, height);
	above = 0;
        below = height;
        EndOfLine(figframe, ALIGN_LEFT);
#if 0
	if (align == ALIGN_LEFT || align == ALIGN_BLEEDLEFT)
	    AddBox(frame, frame->leftmargin, FigureStart, figframe->width, figframe->height);
	else
	    AddBox(frame, frame->width - frame->rightmargin - figframe->width, PixOffset, figframe->width, figframe->height);
#endif
	above = 0;
	Pop();
    }

    Here = 0;

    for (;;)
    {
        while (GetToken() == WHITESPACE);

        if (Token == TAG_FIG && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_CAPTION)
        {
            ParseCaption(0, figframe, 1, 0, 0);
            continue;
        }

        if (Token == TAG_ALT)
        {
            ParseAlt(0);
            continue;
        }

        if (Token == TAG_P)
        {
            UnGetToken();
            ParseAlt(1);
            continue;
        }

        if (TokenClass == EN_TEXT)
        {
            UnGetToken();
            ParseAlt(1);
            continue;
        }

     /* unexpected tag so terminate element */

        error |= ERR_FIG;
        RecordError(bufptr, "ERR_FIG"); /* howcome 11/10/94 */
        UnGetToken();
        break;
    }    

/*    if (image)*/
    {
        figframe->height = PixOffset - FigureStart;
        figframe->length = paintlen - figframe->info - FRAMESTLEN;
        PrintFrameLength(figframe);
        EndFrame(frame, figframe);
	
        /* FigureEnd = PixOffset -= LineSpacing[font]/3; */

        if (align == ALIGN_LEFT || align == ALIGN_BLEEDLEFT)
        {
	    frame->leftmargin += width + 5;
            PixOffset = FigureStart;
        }
        else if (align == ALIGN_RIGHT || align == ALIGN_BLEEDRIGHT)
        {
	    frame->rightmargin += /*width*/ + 5;
            PixOffset = FigureStart;
        }
	if (align == ALIGN_LEFT || align == ALIGN_BLEEDLEFT)
	    AddBox(frame, frame->leftmargin-3, FigureStart-3, figframe->width+6, figframe->height+6);
	else
	    AddBox(frame, frame->width - frame->rightmargin - figframe->width-3, PixOffset-3, figframe->width+6, figframe->height+6);
    }
}

void ParseBlock(int implied, Frame *frame, int tag, int align, int font, int left, int right)
{
    int indent;
    long offset;
    Frame *blockframe;
    Image *image = NULL;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }


    FormatElementStart(Token, class, class_len);

    align = -1;

    if (!implied)
    {
        if (tag == TAG_NOTE)
            image = ParseNoteAttrs();
        else {
/*            SwallowAttributes();*/
	    ParseParaAttrs(&align, &class, &class_len);
	}

    }

    left += (int)StyleGet(SI_MARGIN_LEFT);
    right += (int)StyleGet(SI_MARGIN_RIGHT);
    PixOffset += (int)StyleGet(SI_MARGIN_TOP);
    font = (int)StyleGet(SI_FONT);
    if (align < 0)
	align = (int)StyleGet(SI_TEXT_ALIGN);
 

    Here = left;
    indent = 0;

    if (tag == TAG_NOTE)
    {
#if 0
        PrintRule(frame, GROOVE, left, right, 0);
        EndOfLine(frame, StyleGet(SI_TEXT_ALIGN));
#endif
        offset = PixOffset;

        if (image)
        {
            indent = 20 + image->width;
            blockframe = BeginFrame(frame, 0, 0, left, left+indent, NULL);
            Here = 0;
            above = 0;
            below = image->height;    
            PrintImage(blockframe, 0, 0, 0, image, image->width, image->height);
            EndOfLine(blockframe, (int)StyleGet(SI_TEXT_ALIGN));
            blockframe->height = image->height;
            blockframe->length = paintlen - blockframe->info - FRAMESTLEN;
            PrintFrameLength(blockframe);
            EndFrame(frame, blockframe);
/*            frame->leftcount += 1;
            blockframe->pushcount = frame->leftcount;
	    */     
	    blockframe->flow = ALIGN_NOTE;
	    AddBox(frame, Here, PixOffset, image->width, image->height);
	    /*  left += indent;*/
            Here = LeftMargin(frame, left, PixOffset);
            PixOffset = offset;
        }
    }

    for (;;)
    {
        while (GetToken() == WHITESPACE);

        if (Token == tag && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_P)
        {
            Here = left;
/*	    PixOffset += StyleGet(SI_MARGIN_TOP);*/
            ParsePara(0, False, frame, align, TAG_P, left, right);
            continue;
        }

        if (TokenClass == EN_TEXT)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
/*	    PixOffset += StyleGet(SI_MARGIN_TOP); */
            ParsePara(1, False, frame, align, tag, left, right);
            continue;
        }

     /* unexpected tag so terminate element */

        error |= ERR_BLOCK;
        RecordError(bufptr, "ERR_BLOCK"); /* howcome 11/10/94 */

        UnGetToken();
        break;
    }    

    if (tag == TAG_NOTE)
    {
     /* ensure image ends before closing rule */
        if (image && PixOffset < offset + image->height)
            PixOffset = offset + image->height;
#if 0
        PrintRule(frame, GROOVE, left-indent, right, 0);
        EndOfLine(frame, StyleGet(SI_TEXT_ALIGN));
#endif
    }

    PixOffset += (int)StyleGet(SI_MARGIN_BOTTOM);
    FormatElementEnd();
}


/* tag is TAG_TH or TAG_TD, col is column number starting from 1 upwards, returns cell height */
long ParseTableCell(int implied, Frame *frame, int row, Frame **cells, int border,
                      ColumnWidth **pwidths, int *pcol, int tag, int left, int right, BG_Style *bg)
{
    int align, nowrap, col, rowspan, colspan, m, font;
    long cellTop, cellHeight;
    Frame *cellframe = NULL, *newframes, preframe;
    ColumnWidth *widths;
    
    if (EndTag)
    {
        SwallowAttributes();
        return 0;
    }

    newframes = NULL;
    col = *pcol;
    widths = *pwidths;
    
    align = ALIGN_CENTER;
    nowrap = 0;

    if (prepass)
    {
        preframe.next = preframe.child = NULL;
	preframe.box_list = NULL;
        preframe.indent = 0;
        preframe.height = 0;
        preframe.width = 0;
        preframe.leftmargin = 0;
        preframe.rightmargin = 0;
        cellframe = &preframe;
    }

    list_indent = min_width = max_width = 0;
    rowspan = colspan = 1;

    if (!implied)
        ParseCellAttrs(&rowspan, &colspan, &align, &nowrap);

    *pcol += colspan;

    if (!prepass)
    {
        font = (tag == TAG_TH ? IDX_H3FONT : IDX_NORMALFONT);
        cellTop = PixOffset;
        left = widths[col].left;
        right = widths[col + colspan - 1].right;
        cellframe = BeginFrame(frame, 0, border, left-3, right+4, bg);
        cellframe->lastrow = row + rowspan - 1;

     /* left and right now adjusted to indents from frame margins */
        left = 3;  /* style */
        right = 4;

        /* try to make TH and TD baselines match for first line */
/*
        if (tag == TAG_TH)
            PixOffset += ASCENT(IDX_NORMALFONT) - ASCENT(IDX_H3FONT);

        PixOffset += LineSpacing[IDX_NORMALFONT]/3
*/

        PixOffset += LineSpacing[(int)StyleGet(SI_FONT)]/3;

    }

    for (;;)
    {
        while (GetToken() == WHITESPACE);
/*	style = FormatElementStart(Token, class, class_len);*/

/*
	if (Token == TAG_TABLE && !EndTag)
	{
	       Push(bufptr); 
	       if(prepass) 
		   ParseTable(0, cellframe, left, right);
	       else
	       {
		   UnGetToken();
		   ParseTable(1, cellframe, left, right);
	       }
	       bufptr = (char *)Pop();
	       printf(", after : %X\n", bufptr);
	       while(strncasecmp(bufptr,"/TABLE",5))
	       {
		   bufptr++;
	       }
	       bufptr += 6;
	       printf("and then... %X\n", bufptr); 
	       break;
	}
*/	
        if (Token == TAG_DD && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (Token == TAG_DL && EndTag)
        {
            UnGetToken();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_UL)
        {
            ParseUL(0, False, cellframe, 0, align, left, right);
            continue;
        }

        if (Token == TAG_OL)
        {
            ParseOL(0, False, cellframe, 0, align, left, right);
            continue;
        }

        if (Token == TAG_LI)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            ParseUL(1, False, cellframe, 0, align, left, right);
            continue;
        }

        if (Token == TAG_DL)
        {
            ParseDL(0, False, cellframe, align, left, right);
            Here = left;
            continue;
        }

        if (Token == TAG_HR)
        {
/*            SwallowAttributes();*/

	    ParseClassAttrs(&class, &class_len);
/*
	    StyleRegisterSequential(Token, class, class_len);
*/
	    FormatElementStart(Token, class, class_len);

            if (EndTag)
                continue;

            PrintRule(cellframe, GROOVE, left, right, 0);
            EndOfLine(cellframe, (int)StyleGet(SI_TEXT_ALIGN));
	    FormatElementEnd();
            continue;
        }

        if (Token == TAG_P)
        {
            Here = left;
	    PixOffset += (int)StyleGet(SI_MARGIN_TOP);
/*            ParsePara(0, False, cellframe, align,
                      (tag == TAG_TH ? IDX_H3FONT : IDX_NORMALFONT), left, right); */
            ParsePara(0, False, cellframe, align, tag, left, right);
            continue;
        }

        if (TokenClass == EN_HEADER)
        {
            UnGetToken();
            ParseHeader(cellframe, align, left, right, 0);
            continue;
        }

        if (TokenClass == EN_TEXT || Token == ENTITY)
        {
            UnGetToken();
            Here = left;
	    PixOffset += (int)StyleGet(SI_MARGIN_TOP);
            ParsePara(1, False, cellframe, align, tag, left, right);
            continue;
        }

        if (Token == TAG_PRE)
        {
            UnGetToken();
            ParsePRE(0, False, cellframe, left, right);
            continue;
        }

	if(TokenClass == EN_BLOCK)
	{
	    ParseBlock(0, cellframe, Token, ALIGN_LEFT, IDX_NORMALFONT, left, right);
	    Here = left;
            continue;
        }
     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }

    if (col > COLS(widths))   /* update table column count */
        COLS(widths) = col;

    m = MAXCOLS(widths);

    if (col + colspan > m)   /* double array size as needed */
    {
        while (col + colspan> m)
           m = m << 1;

	if (TABLE_TRACE)
	    fprintf(stderr,"ParseTableCell: realloc, size = %d\n",(m + 1) * sizeof(ColumnWidth));
        widths = *pwidths = (ColumnWidth *)realloc(widths, (m + 1) * sizeof(ColumnWidth));
	MAXCOLS(widths) = m;

        while (m >= col)   /* zero widths for newly allocated elements */
        {
            widths[m].Cmin = 0;
            widths[m].Cmax = 0;
            widths[m].rows = 0;
            --m;
        }
    }

    max_width = cellframe->width + left + right;
    min_width += list_indent;

    if (nowrap && max_width > min_width)
        min_width = max_width;

    if (colspan > 1)  /* apportion widths evenly */
    {
        min_width = min_width/colspan;
        max_width = max_width/colspan;

        for (m = 0; m < colspan; ++m)
        {
            if (min_width > widths[col + m].Cmin)
                widths[col + m].Cmin = min_width;

            if (max_width > widths[col + m].Cmax)
                widths[col + m].Cmax = max_width;
        }
    }
    else
    {
        if (min_width > widths[col].Cmin)
            widths[col].Cmin = min_width;

        if (max_width > widths[col].Cmax)
            widths[col].Cmax = max_width;
    }

    widths[col].rows = rowspan - 1;

    if (!prepass)
    {
        cellHeight = (PixOffset - cellTop)/rowspan;
        cellframe->height = cellHeight;  /* for debug only */
        widths[col].Cmin = cellHeight;
        FlushPending(frame);
        cellframe->length = paintlen - cellframe->info - FRAMESTLEN ; 
        PrintFrameLength(cellframe);
        InsertCell(cells, cellframe);
        return cellHeight;
    }

    return 0;
}

void DummyCell(Frame *frame, int row, Frame **cells,
               int border, ColumnWidth *widths, int col)
{
    int left, right;
    Frame *cellframe;

    left = widths[col].left;
    right = widths[COLS(widths)].right;
    cellframe = BeginFrame(frame, 0, border, left-3, right+4, NULL);
    cellframe->lastrow = row;
    cellframe->length = paintlen - cellframe->info - FRAMESTLEN;
    PrintFrameLength(cellframe);
    InsertCell(cells, cellframe);
}

void ParseTableRow(int implied, Frame *frame, int row, Frame **cells,
                    int border, ColumnWidth **pwidths, int left, int right, BG_Style *bg)
{
    int cols = 1;
    long rowTop, rowHeight, cellHeight;
    /* janet: not used:    char *row_bufptr; */
    ColumnWidth *widths;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
        SwallowAttributes();

    Here = left;
    rowTop = PixOffset;
    rowHeight = 0;

    for (;;)
    {
	PixOffset = rowTop;
	widths=*pwidths;
	
        /* if this cell spans more than one row */
        if (widths[cols].rows > 0)
        {
            widths[cols].rows -= 1;  /* decrement span count */

            if (!prepass) /* does spanned cell effect rowBottom? */
            {
                if (widths[cols].Cmin > rowHeight)
                    rowHeight = widths[cols].Cmin;
            }

            ++cols;
            continue;
        }

        while (GetToken() == WHITESPACE);

        if (Token == TAG_TR && EndTag)
        {
            if (EndTag)
            {
                SwallowAttributes();
                break;
            }

            UnGetToken();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_TH)
        {
            cellHeight = ParseTableCell(0, frame, row, cells, border,
                                        pwidths, &cols, TAG_TH, left, right, bg);

            if (cellHeight > rowHeight)
                rowHeight = cellHeight;
            continue;
        }

        if (Token == TAG_TD)
        {
            cellHeight = ParseTableCell(0, frame, row, cells, border,
                                        pwidths, &cols, TAG_TD, left, right, bg);

            if (cellHeight > rowHeight)
                rowHeight = cellHeight;
            continue;
        }

        if (Token == TAG_TABLE)
        {
            UnGetToken();
            break;
        }
	
        if (TokenClass == EN_LIST || TokenClass == EN_TEXT ||
            TokenClass == EN_HEADER || TokenClass == EN_BLOCK)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            cellHeight = ParseTableCell(1, frame, row, cells, border,
                                        pwidths, &cols, TAG_TD, left, right, bg);

            if (cellHeight > rowHeight)
                rowHeight = cellHeight;
            continue;
        }

     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }    
    
    widths = *pwidths; /* Just in case it changes again */
    if (!prepass && cols <= COLS(widths))
        DummyCell(frame, row, cells, border, widths, cols);

    PixOffset = rowTop + rowHeight;

    if (!prepass)
        FlushCells(frame, row, cells);  /* calls EndFrame() for cells ending this row */
}

void ParseTable(int implied, Frame *frame, int left, int right)
{
    int row, border, i, w, W, x, ptmin, ptmax, spare, max_width; /* janet: not used: cols, field_count, new */
    long table_offset;
    char *table_bufptr;
    Frame *cells;
    Frame *new_frame;

    ColumnWidth *widths;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    if (!implied)
        ParseTableAttrs(&border);

    FormatElementStart(Token, NULL, 0);

    new_frame = (Frame *)Pop();
    Push(new_frame);

    damn_table = 1; /* debug table format */

    Here = left;
    ProcessTableForm(1);   /* what the HECK is THIS ??!! */

    widths = (ColumnWidth *)malloc((NCOLS + 1) * sizeof(ColumnWidth));

    COLS(widths) = 0;               /* current number of columns */
    MAXCOLS(widths) = NCOLS;        /* space currently allocated */
    widths[0].rows = 0;

    for (i = NCOLS; i > 0; --i)     /* zero widths for allocated elements */
    {
        widths[i].Cmin = 0;
        widths[i].Cmax = 0;
        widths[i].rows = 0;
    }

    prepass = 1;
    table_bufptr = bufptr;          /* note parse position for second pass */
    table_offset = PixOffset;

 draw_table:

    row = 0;
    cells = NULL;

    for (; row < 1000 ;) 
    {
        while (GetToken() == WHITESPACE);

	if (Token == TAG_TABLE && !EndTag)
        {
	    while(strncasecmp(bufptr,"/TABLE",5))
		bufptr++;
	    bufptr+=6;
	    break;
	}
	
	if (Token == TAG_TABLE && EndTag)
        {
            if (EndTag)
            {
                SwallowAttributes();
                break;
            }

            UnGetToken();
            break;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_CAPTION)
        {
            ParseCaption(0, frame, 0, 0, 0);
            continue;
        }

        if (Token == TAG_TR)
        {
            ++row;
            ParseTableRow(0, new_frame, row, &cells, border, &widths, left, right,(BG_Style *)StyleGet(SI_BACKGROUND));
            continue;

        }

        if (TokenClass == EN_LIST || TokenClass == EN_TEXT || TokenClass == EN_TABLE ||
            TokenClass == EN_HEADER || TokenClass == EN_BLOCK)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            ++row;
            ParseTableRow(1, new_frame, row, &cells, border, &widths, left, right, (BG_Style *)StyleGet(SI_BACKGROUND));
            continue;
        }

     /* unexpected tag so terminate element */

        UnGetToken();
        break;
    }    

    if (prepass)   /* assign actual column widths */
    {
        for (i = 1, ptmin = 3, ptmax = 3, W = 0; i <= COLS(widths); ++i)
        {
            ptmin += 7 + widths[i].Cmin;
            ptmax += 7 + widths[i].Cmax;
            W += widths[i].Cmax - widths[i].Cmin;
        }

        /* if max fits in window then use max widths */

        max_width = new_frame->width - right - left;

        if (ptmax <= max_width)
        {
            x = left + (max_width - ptmax)/2;

            for (i = 1; i <= COLS(widths); ++i)
            {
                widths[i].left = x + 3;
                x += 7 + widths[i].Cmax;
                widths[i].right = x - 4;
            }
        }
        else if (ptmin < max_width)
        {
            x = left;
            spare = max_width - ptmin;

            for (i = 1; i <= COLS(widths); ++i)
            {
                w = widths[i].Cmax - widths[i].Cmin;
                widths[i].left = x + 3;
                x += 7 + widths[i].Cmin + (spare * w)/W;
                widths[i].right = x - 4;
            }
        }
        else /* assign minimum column widths */
        {
            x = left;

            for (i = 1; i <= COLS(widths); ++i)
            {
                widths[i].left = x + 3;
                x += 7 + widths[i].Cmin;
                widths[i].right = x - 4;
            }
        }

        /* and do second pass to draw table */

        bufptr = table_bufptr;
        PixOffset = table_offset;
        prepass = 0;
        ProcessTableForm(2);
        goto draw_table;
    }
   
    Free(widths);   /* free column widths */
    Here = left;
    PixOffset += (int)StyleGet(SI_MARGIN_BOTTOM);
    FormatElementEnd();
    damn_table = 0;
}

/* left and right are the indents from the current LHS, RHS respectively */
void ParseBody(int implied, Frame *frame, int left, int right, int margin)
{
    int indent;

    if (EndTag)
    {
        SwallowAttributes();
        return;
    }

    indent = margin;

    if (!implied)
        SwallowAttributes();

    FormatElementStart(TAG_HTML, NULL, 0); /* howcome 3/4/95: first call to FormatElementStart */

    for (;;)
    {
        while (GetToken() == WHITESPACE);

        if (Token == TAG_BODY && EndTag)
        {
            SwallowAttributes();
            break;
        }

        if (IsTag(Token) && TokenClass == EN_HEADER)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

	    ParseHeader(frame, ALIGN_LEFT, left, right, 0);
            continue;
        }

        if (Token == UNKNOWN)
        {
            SwallowAttributes();
            continue;
        }

        if (Token == TAG_UL)
        {
	    ParseUL(0, True, frame, 0, ALIGN_LEFT, left, right);
            continue;
        }

        if (Token == TAG_OL)
        {
            ParseOL(0, True, frame, 0, ALIGN_LEFT, left, right);
            continue;
        }

        if (Token == TAG_LI)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            error |= ERR_LI;
            RecordError(bufptr, "ERR_LI"); /* howcome 11/10/94 */
            UnGetToken();
            ParseUL(1, True, frame, 0, ALIGN_LEFT, left, right); /* howcoem 11/8/95: ALIGN_CENTER -> ALIGN_LEFT */
            continue;
        }

        if (Token == TAG_DL)
        {
/*            ParseDL(0, frame, StyleGet(SI_TEXT_ALIGN), StyleGet(SI_MARGIN_LEFT), right+margin);*/
            ParseDL(0, True, frame, ALIGN_LEFT, left, right + margin);
            continue;
        }

        if (TokenClass == EN_TABLE)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            error |= ERR_TABLE;
            RecordError(bufptr, "ERR_TABLE"); /* howcome 11/10/94 */
            UnGetToken();
            ParseTable(1, frame, left, right);
            continue;
        }

        if (Token == TAG_TABLE)
        {
            ParseTable(0, frame, left, right+margin);
            continue;
        }

        if (Token == TAG_DT || Token == TAG_DD)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            error |= ERR_DL;
            RecordError(bufptr, "ERR_DL"); /* howcome 11/10/94 */
            UnGetToken();
            ParseDL(1, True, frame, ALIGN_LEFT, left, right);
            continue;
        }

        if (Token == TAG_HR)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

/*            SwallowAttributes();*/


	    ParseClassAttrs(&class, &class_len);
/*
	    StyleRegisterSequential(Token, class, class_len);
*/
	    FormatElementStart(Token, class, class_len);
            PrintRule(frame, GROOVE, LeftMargin(frame, left, PixOffset) , RightMargin(frame, right, PixOffset), 0);
            EndOfLine(frame, (int)StyleGet(SI_TEXT_ALIGN));
	    FormatElementEnd();
            continue;
        }

        if (Token == TAG_PRE)
        {
            ParsePRE(0, True, frame, left, right);
            continue;
        }

        if (Token == TAG_P)
        {
            Here = left+margin;
/*	    PixOffset += top;*/
            ParsePara(0, True, frame, ALIGN_LEFT, TAG_P, left, right);
            continue;
        }

        if (TokenClass == EN_TEXT || Token == ENTITY)
        {
            if (EndTag)
            {
                SwallowAttributes();
                continue;
            }

            UnGetToken();
            Here = left + margin;
/*	    style = FormatElementStart(Token, class, class_len); *//* howcome 6/3/94: need to reset */

	    if (REGISTER_TRACE)
		fprintf(stderr,"implied: ");
#if 0
	    StyleRegisterSequential(TAG_P, NULL, 0); /* reason to live?? */
	    FormatElementStart(TAG_P, NULL, 0);
#endif
            ParsePara(1, True, frame, ALIGN_LEFT, TAG_P, left, right);
            continue;
        }

        if (Token == TAG_FIG)
        {
            ParseFigure(0, frame, left, right);
            continue;
        }

        if (Token == TAG_ADDRESS)
        {
/*	    style = FormatElementStart(Token, class, class_len);*/
            ParseBlock(0, frame, Token, ALIGN_LEFT, IDX_NORMALFONT, left, right);
            Here = left;
            continue;
        }

        if (Token == TAG_ABSTRACT)
        {
            ParseBlock(0, frame, Token, ALIGN_LEFT, IDX_NORMALFONT, left, right);
            Here = left;
            continue;
        }

        if (Token == TAG_QUOTE)
        {
            ParseBlock(0, frame, Token, ALIGN_CENTER, IDX_NORMALFONT, left, right);
            Here = left;
            continue;
        }

        if (Token == TAG_NOTE)
        {
            ParseBlock(0, frame, Token, ALIGN_LEFT, IDX_NORMALFONT, left, right);
            Here = left;
            continue;
        }

	if (Token == TAG_STYLE)
	{
	    UnGetToken();
	    ParseStyle(frame);
	    continue;
	}

        if (Token == ENDDATA)
        {
            UnGetToken();
            break;
        }

        if (Token == TAG_BODY || TokenClass == EN_SETUP)
        {
            error |= ERR_SETUP;
            RecordError(bufptr, "ERR_SETUP"); /* howcome 11/10/94 */
            SwallowAttributes();
            continue;
        }
	/* must add form */

        /* unexpected but known tag name */
        error |= ERR_BODY;
        RecordError(bufptr, "ERR_BODY");
        SwallowAttributes();
    }
    FormatElementEnd();
}

void InitBackgroundFrame()
{
 /* initialise background frame */

    background.next = NULL;
    background.child = NULL;
    background.box_list = NULL;
    background.top = paint + FRAMESTLEN;

    if (TAG_TRACE)
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr,"parsehtml.c: ParseHTML: paint = %p background.top = %p\n", paint, background.top);
#else
	fprintf(stderr,"parsehtml.c: ParseHTML: paint = %lx background.top = %lx\n", paint, background.top);
#endif /* PRINTF_HAS_PFORMAT */

    background.offset = 0;
    background.indent = 0;
    background.height = 1;  /* zero might cause problems?? */
    background.width = WinWidth;
    background.info = 0;
    background.length = 3;
    background.style = 0;
    background.border = 0;
    background.leftmargin = MININDENT;
    background.rightmargin = MININDENT;
}

long ParseHTML(int *width, BOOL style_parse)
{
    int margin; /* janet: not used: WordLen, c */
    Byte *p;
    char *bg_name;
    int  bg_len;
    BG_Style *bg;
    Image *image;
    char *quoted_name;
    int i;

    PixOffset = 5;
    LastBufPtr = bufptr = buffer+hdrlen;
    error = prepass = preformatted = 0;
    paintlen = 0;
    IsIndex = 0;
    start_figure = figure = 0;
    font = paintStartLine = -1;
    FigureEnd = LeftMarginIndent = RightMarginIndent = 0;
    form = NULL;
    margin = 0; /* howcome 21/2/95 */

/*    margin = XTextWidth(Fonts[IDX_NORMALFONT], "mmm", 2);*/

    if (paintbufsize == 0)
    {
        paintbufsize = 8192;
        paint = (Byte *)malloc(paintbufsize);
    }

    InitBackgroundFrame();

    Push(&background); 

 /* Reserve space for background's begin frame object
    which is needed to simplify display and scrolling routines */

    MakeRoom(FRAMESTLEN);

    SkipSpace();

    if (Token == TAG_BASE)
    {
        ParseBase(&background);
        SkipSpace();
    }

    if (Token == TAG_HEAD)
        ParseSetUp(0, &background);
    else
    {
        UnGetToken();
        ParseSetUp(1, &background);
    }

    if (style_parse)
    {
	StyleParse();
	FormatElementStart(TAG_HTML, NULL, 0);
	background.leftmargin = (int)StyleGet(SI_MARGIN_LEFT);
	background.rightmargin = (int)StyleGet(SI_MARGIN_RIGHT);
	PixOffset = (int)StyleGet(SI_MARGIN_TOP);
	background.offset = PixOffset;
    }
    else
	FormatElementStart(TAG_HTML, NULL, 0);

 /* kludge to cope with multiple BODY elements */
    while (SkipSpace() != ENDDATA)
    {
        if (Token == TAG_BODY)
	{
#ifndef NO_BACKGROUND_IN_HTML
	    ParseBodyAttrs(&bg_name, &bg_len);
	    if(bg_name)
	    {
		bg = (BG_Style *)StyleGet(SI_BACKGROUND);
		if(!bg)
		{
		    char *tmpchar;
		    
		    quoted_name = (char *)calloc(bg_len+32, sizeof(char));
		    strcpy(quoted_name,"HTML { background: \"");
		    tmpchar = quoted_name + strlen(quoted_name);
		    for(i=0; i<bg_len; i++)
			*tmpchar++ = *(bg_name+i);
		    strcpy(tmpchar,"\" }");
		    if(!CurrentDoc->style)
			CurrentDoc->style = StyleCopy(context->style);
		    StyleChew(CurrentDoc->style, quoted_name, S_READER); 
		    FormatElementEnd();
		    FormatElementStart(TAG_HTML, NULL , 0);
		    Free(quoted_name);
		}
		else 
		{
		    if(image = GetImage(bg_name, bg_len, CurrentDoc->pending_reload))
		    {
			bg->image = image;
			bg->flag |= SI_BACKGROUND_IMAGE;
		    }
		}
	    }
#endif /* NO_BACKGROUND_IN_HTML */
	    ParseBody(0, &background, MININDENT, MININDENT, margin);
	}
	else
	{
	    UnGetToken();
	    ParseBody(1, &background, MININDENT, MININDENT, margin);
	}
    }

    FlushAllFrames(NULL, background.child); /* flush remaining end of frames */
    background.child = NULL;
    PrintEndFrame(NULL, &background);
    *width = background.width;

 /* update background frame length */

    if (PixOffset > background.height)
        background.height = PixOffset;

    background.length = paintlen - FRAMESTLEN;

 /* and fill in begin frame object at start of paint buffer */

    if (TAG_TRACE)
#if defined PRINTF_HAS_PFORMAT
	fprintf(stderr,"parsehtml.c: ParseHTML: starting buffer at %p\n",paint);
#else
	fprintf(stderr,"parsehtml.c: ParseHTML: starting buffer at %lx\n",paint);
#endif /* PRINTF_HAS_PFORMAT */

 /* major bug fix: howcome found it in desparation 3/12/94 */

    background.top = paint + FRAMESTLEN;

    p = paint;
    *p++ = BEGIN_FRAME;
    PushValue(p, background.offset & 0xFFFF);
    PushValue(p, (background.offset >> 16) & 0xFFFF);
    PushValue(p, background.indent);
    PushValue(p, background.width);
    PushValue(p, background.height);
    PushValue(p, 0); 
    *p++ = background.style;
    *p++ = background.border;
#ifdef STYLE_COLOR_BORDER		/* changed from COLOR_STYLE 15/5/95 */
    *p++ = background.cb_ix;
#endif
#ifdef STYLE_BACKGROUND
    PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
    FormatElementEnd();
    PushValue(p, background.length);
    
    /* printf("Pushing now at 0x%xh BEGIN FRAME -> offest %d, indent %d,width %d,height %d\n",paint,
	  background.offset,background.indent,background.width,background.height);  */

    TopObject = paint;   /* obsolete */

    html_width = background.width;
    Pop();
    return background.height;
}


/* ParseImage doesn't really parse the image. It does, however, for
   the equivalent for an image that ParseHTML does for HTML: create a
   paintstream. In the case of a single image, the paint stream is
   very simple. Alternativley, one could have wrapped the image in
   dummy html and formatted the document as normal. This would have
   given greater flexibiliy (e.g. to put it into a table), but would
   have been more expensive. howcome 13/2/95 */

long ParseImage(Doc *doc, int *width)
{
    int margin;
    Byte *p;

    FormatElementStart(TAG_HTML, NULL, 0);
    FormatElementStart(TAG_IMG, NULL, 0);

    LastBufPtr = bufptr = buffer+hdrlen;
    error = prepass = preformatted = 0;
    paintlen = 0;
    IsIndex = 0;
    start_figure = figure = 0;
    font = paintStartLine = -1;
    FigureEnd = LeftMarginIndent = RightMarginIndent = 0;
    form = NULL;
    margin = XTextWidth(Fonts[IDX_NORMALFONT], "mmm", 2);

    if (paintbufsize == 0)
    {
        paintbufsize = 8192;
        paint = (Byte *)malloc(paintbufsize);
    }

    InitBackgroundFrame();
    Here = 0; /* background.leftmargin;*/
    PixOffset = 5;

    MakeRoom(FRAMESTLEN);

    PrintImage(&background, 0, 0, NULL, doc->image, doc->image->width, doc->image->height);
    EndOfLine(&background, ALIGN_LEFT);
    PrintEndFrame(NULL, &background);

    background.width = background.leftmargin + doc->image->width + background.rightmargin;
    background.height = 5 + doc->image->height + 5;
    background.length = paintlen - FRAMESTLEN;
    background.top = paint + FRAMESTLEN;

    p = paint;
    *p++ = BEGIN_FRAME;
    PushValue(p, background.offset & 0xFFFF);
    PushValue(p, (background.offset >> 16) & 0xFFFF);
    PushValue(p, background.indent);
    PushValue(p, background.width);
    PushValue(p, background.height);
    PushValue(p,0); /* --Spif 23-Oct-95 is this the source of all pbs ? */
    *p++ = background.style;
    *p++ = background.border;
#ifdef STYLE_COLOR_BORDER  		/* changed from COLOR_STYLE 15/5/95 */
    *p++ = background.cb_ix;
#endif
#ifdef STYLE_BACKGROUND
    PutPointer(&p, (void *)StyleGet(SI_BACKGROUND)); /* must be set to the structures defined by the style --Spif 18-Oct-95 */
#endif
    PushValue(p, background.length);

/*    TopObject = paint;   *//* obsolete */

    html_width = background.width;
    *width = background.width;

    FormatElementEnd();
    FormatElementEnd();

    return background.height;
}
