/* www.h function declarations and general defines */
#ifndef _ARENA_WWW_H_
#define _ARENA_WWW_H_

#ifndef POPUP
#define	POPUP
#endif

#ifndef PNG
#define	PNG
#endif

#ifndef JPEG
#define	JPEG
#endif

#ifndef STYLE_COLOR
#define	STYLE_COLOR
#endif

#ifndef STYLE_BACKGROUND
#define STYLE_BACKGROUND
#endif

#ifndef ZOOM
#define	ZOOM
#endif

#ifdef SOLARIS
#define	SIGWAITING_IGN
#endif

#ifndef SELECTION
#define	SELECTION
#endif


#define VERSION         "beta-3b"      /* REMEMBER!!! also update DEFAULT_URL and HELP_URL and INITIAL_HTML*/
#define BANNER 		"Arena"     /* also used as APPNAME !!!!! */

/* janet: added, as they are needed within www.h  is this the right place (line)? */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef __hpux
#include <X11/HPkeysym.h>
#endif

#include "types.h"

#if 0
#include "skyblue.h"    /* the constraint solver */
#endif

#include "machdep.h"	/* machine dependent definitions wm 24.Mar.95 */

#include <stdarg.h>
#include "HTUtils.h"
#include "tcp.h"
#include "HTString.h"
#include "HTList.h"
#include "HTAccess.h"
/* #include "HTEvntrg.h" */
#include "HTFormat.h"
#include "HTAnchor.h"
#include "HTAncMan.h"
#include "HTArray.h"


/* janet: this one is not in HTList.h, where it should be */
/* extern void * HTList_removeObjectAt ARGS2 (HTList *,me, int,position);*/

#define COMPACTDEFLISTS 1   /* comment out for old style */

#define HELP_URL   "http://www.w3.org/pub/WWW/Arena/beta-3"
#define DEFAULT_URL "http://info.cern.ch/hypertext/WWW/TheProject.html"
#define PRINTER     "alljet"

#if defined POINTER_IS_64BIT
#define POINTERSIZE 8
#else
#define POINTERSIZE 4
#endif

/* HTML error codes displayed in Octal */

#define ERR_ANCHOR  1 << 0
#define ERR_EMPH    1 << 1
#define ERR_HEADER  1 << 2

#define ERR_UL      1 << 3
#define ERR_OL      1 << 4
#define ERR_DL      1 << 5

#define ERR_SELECT  1 << 6
#define ERR_BLOCK   1 << 7
#define ERR_PRE     1 << 8
#define ERR_FIG     1 << 13

#define ERR_LI      1 << 9
#define ERR_TABLE   1 << 10
#define ERR_SETUP   1 << 11
#define ERR_BODY    1 << 12

/* fonts and colors set in www.c GetResources(void) */

#define H1FONT      "-*-helvetica-bold-r-normal--*-180-75-75-*-*-iso8859-1"
#define H2FONT      "-*-helvetica-bold-r-normal--*-140-75-75-*-*-iso8859-1"
#define H3FONT      "-*-helvetica-bold-r-normal--*-120-75-75-*-*-iso8859-1"
#define H4FONT      "-*-helvetica-bold-r-normal--*-100-75-75-*-*-iso8859-1"
#define LABELFONT   "6x13"

#define SUBSCRFONT  "-*-new century schoolbook-medium-r-normal--*-80-75-75-*-*-iso8859-1"
#define NORMALFONT  "-*-new century schoolbook-medium-r-normal--*-120-75-75-*-*-iso8859-1"
#define ITALICFONT  "-*-new century schoolbook-medium-i-normal--*-120-75-75-*-*-iso8859-1"
#define BINORMFONT  "-*-new century schoolbook-bold-i-normal--*-120-75-75-*-*-iso8859-1"
#define BOLDFONT    "-*-new century schoolbook-bold-r-normal--*-120-75-75-*-*-iso8859-1"

#define IFIXEDFONT  "-*-courier-medium-o-normal--*-120-75-75-*-*-iso8859-1"
#define BFIXEDFONT  "-*-courier-bold-r-normal--*-120-75-75-*-*-iso8859-1"
#define BIFIXEDFONT "-*-courier-bold-o-normal--*-120-75-75-*-*-iso8859-1"
#define RFIXEDFONT  "-*-courier-medium-r-normal--*-120-75-75-*-*-iso8859-1"

#define SYMFONT     "-*-symbol-medium-r-normal--*-140-75-75-*-*-*-*"
#define SUBSYMFONT  "-*-symbol-medium-r-normal--*-100-75-75-*-*-*-*"

#define H1FONTL      "-*-helvetica-bold-r-normal--*-240-75-75-*-*-iso8859-1"
#define H2FONTL      "-*-helvetica-bold-r-normal--*-180-75-75-*-*-iso8859-1"
#define H3FONTL      "-*-helvetica-bold-r-normal--*-140-75-75-*-*-iso8859-1"
#define H4FONTL      "-*-helvetica-bold-r-normal--*-100-75-75-*-*-iso8859-1"

#define SUBSCRFONTL  "-*-new century schoolbook-medium-r-normal--*-100-75-75-*-*-iso8859-1"
#define NORMALFONTL  "-*-new century schoolbook-medium-r-normal--*-140-75-75-*-*-iso8859-1"
#define ITALICFONTL  "-*-new century schoolbook-medium-i-normal--*-140-75-75-*-*-iso8859-1"
#define BINORMFONTL  "-*-new century schoolbook-bold-i-normal--*-140-75-75-*-*-iso8859-1"
#define BOLDFONTL    "-*-new century schoolbook-bold-i-normal--*-140-75-75-*-*-iso8859-1"

#define IFIXEDFONTL  "-*-courier-medium-o-normal--*-140-75-75-*-*-iso8859-1"
#define BFIXEDFONTL  "-*-courier-bold-r-normal--*-140-75-75-*-*-iso8859-1"
#define BIFIXEDFONTL "-*-courier-bold-o-normal--*-140-75-75-*-*-iso8859-1"
#define RFIXEDFONTL  "-*-courier-medium-r-normal--*-140-75-75-*-*-iso8859-1"

#define SYMFONTL     "-*-symbol-medium-r-normal--*-180-75-75-*-*-*-*"
#define SUBSYMFONTL  "-*-symbol-medium-r-normal--*-140-75-75-*-*-*-*"

/* largest font is 24 point :-( */

#define H1FONTG      "-*-helvetica-bold-r-normal--*-300-75-75-*-*-iso8859-1"
#define H2FONTG      "-*-helvetica-bold-r-normal--*-260-75-75-*-*-iso8859-1"
#define H3FONTG      "-*-helvetica-bold-r-normal--*-220-75-75-*-*-iso8859-1"
#define H4FONTG      "-*-helvetica-bold-r-normal--*-180-75-75-*-*-iso8859-1"

#define SUBSCRFONTG  "-*-helvetica-medium-r-normal--*-120-75-75-*-*-iso8859-1"
#define NORMALFONTG  "-*-helvetica-medium-r-normal--*-180-75-75-*-*-iso8859-1"
#define ITALICFONTG  "-*-helvetica-medium-o-normal--*-180-75-75-*-*-iso8859-1"
#define BINORMFONTG  "-*-helvetica-bold-o-normal--*-180-75-75-*-*-iso8859-1"
#define BOLDFONTG    "-*-helvetica-bold-o-normal--*-180-75-75-*-*-iso8859-1"

#define IFIXEDFONTG  "-*-courier-medium-o-normal--*-180-75-75-*-*-iso8859-1"
#define BFIXEDFONTG  "-*-courier-bold-r-normal--*-180-75-75-*-*-iso8859-1"
#define BIFIXEDFONTG "-*-courier-bold-o-normal--*-180-75-75-*-*-iso8859-1"
#define RFIXEDFONTG  "-*-courier-medium-r-normal--*-180-75-75-*-*-iso8859-1"

#define SYMFONTG     "-*-symbol-medium-r-normal--*-240-75-75-*-*-*-*"
#define SUBSYMFONTG  "-*-symbol-medium-r-normal--*-180-75-75-*-*-*-*"


#define IMG_WIDTH 32
#define IMG_HEIGHT 32
#define IMG_INDICATOR 3

/* fonts are held in an array to allow them to be identified by a 4 bit index */

/*#define FONTS          16*/

#define IDX_H1FONT              0
#define IDX_H2FONT              1
#define IDX_H3FONT              2
#define IDX_H4FONT              3
#define IDX_LABELFONT           4
#define IDX_NORMALFONT          5
#define IDX_INORMALFONT         6
#define IDX_BNORMALFONT         7
#define IDX_BINORMALFONT        8
#define IDX_FIXEDFONT           9
#define IDX_IFIXEDFONT         10
#define IDX_BFIXEDFONT         11
#define IDX_BIFIXEDFONT        12
#define IDX_SMALLFONT          13
#define IDX_SYMBOLFONT         14
#define IDX_SUBSYMFONT         15

/*
The paint stream consists of a sequence of overlapping frames:

    a) TextLines which contain elements appearing on a single line
       e.g. STRING, LITERAL, RULE, BULLET, IMAGE. The TextLine
       is large enough to contain all objects on the line. TextLines
       never overlap except when they occur in different frames.

    b) Frames which contain TextLines and other Frames
       (used for figures, tables and sidebars)

Frames are designed for efficient painting and scrolling,
regardless of the length of a document. The frames are strictly
ordered with respect to increasing pixel offset from the top of
the document. Objects with the same offset are sorted with
respect to increasing indent.

This property is vital to the scrolling and painting
mechanism. The ordering may be broken for elements within LINEs,
for which a left to right ordering following that in the HTML
source simplifies the Find menu action.

FRAMEs are broken into BEGIN and END frame objects. The END frame
objects are positioned in the sequence according the offset at
which the frames end. This is needed to safely terminate search
for objects which intersect the top of the window, when scrolling
back through the document.
*/

#define TEXTLINE    11   /* frame containing elements on a line */
#define BEGIN_FRAME 12   /* begining of a frame */
#define END_FRAME   13   /* end of a frame */

#ifdef STYLE_COLOR_BORDER
#define FRAME_STLEN  18  /* number of bytes in start of frame marker */
#else
#define FRAME_STLEN  17  /* number of bytes in start of frame marker */
#endif

#ifdef STYLE_BACKGROUND
#define FRAMESTLEN (FRAME_STLEN+POINTERSIZE)
#else
#define FRAMESTLEN FRAME_STLEN
#endif

#define FRAMENDLEN  5   /* number of bytes in end of frame marker */
#define TXTLINLEN   11  /* number of bytes in TEXTLINE header */

/* parameters in frame-like objects:

Each object starts with an 8 bit tag and ends with a 2 byte
size field that permits moving back up the list of objects.
The size is set to the number of bytes from the tag up to
the size field itself.

    tag = BEGIN_FRAME
    offset (4 bytes)
    indent (2 bytes)
    width  (2 bytes)
    height (4 bytes)
    style  (1 byte)
    border (1 byte)
#ifdef STYLE_COLOR_BORDER
    color_border (1 byte)
#endif
#ifdef STYLE_BACKGROUND
    background pointer (4 or 8 bytes)
#endif
    length (2 bytes)
    zero or more elements
    size   (2 bytes)

The length parameter gives the number of bytes until
the end of the frame's data. It is used to skip quickly
to objects following this frame. This requires a simple
stack in DisplayHTML() to handle nested frames.

The pixel offset for the end of the frame is marked by
the END_FRAME object which specifies the number of bytes
back to the corresponding BEGIN_FRAME object.

    tag = END_FRAME
    start  (2 bytes)
    size   (2 bytes)

A frame for a line of text composed of multiple elements:

TEXTLINE:

    tag == TEXTLINE
    offset (4 bytes)
    baseline (two bytes)
    indent (two bytes)
    height (two bytes)
    zero or more elements
    1 byte marker (= 0)
    size (two bytes)
*/

/*
When parsing, we need to defer printing the END_FRAME marker
until we come to print an object with the same or larger offset
and at the same level of frame nesting. We also need to examine
the widths and position of active frames to decide where to flow
text and graphics. In this phase the Frame struct only uses
the next, nesting, info and offset fields.

When displaying, we need a list of frames which intersect the
top of the window, plus a pointer to the background flow. Once
this list has been dealt with, display continues in the background
flow and subsequent frames are dealt with one at a time.

When scrolling towards the start of the document, the code needs
to take care in case a text line object intersects the top of
the window, otherwise the first fully visible line will do.
*/

struct frame_struct
    {
        struct frame_struct *next;  /* linked list of peer frames */
        struct frame_struct *child; /* linked list of nested frames */
        unsigned char *top;         /* where to start displaying from */
        long offset;                /* from start of document */
        unsigned int indent;        /* from left of document */
        long height;                /* documents can be very long */
        unsigned int width;
        int leftmargin;             /* indent from lhs of frame */
        int rightmargin;            /* indent from rhs of frame */
        unsigned int info;    /* to BEGIN_FRAME object in paint stream */
        unsigned int length;        /* byte count of frame's data */
        unsigned int lastrow;       /* used in parsing tables */
        unsigned char style;        /* frame's background style */
        unsigned char border;       /* frame's border style */
        int flow;    /* ALIGN_LEFT, ALIGN_NOTE, ALIGN_CENTER or ALIGN_RIGHT */
        int oldmargin;     /* previous value of parent's margin */
        int pushcount;     /* save count value for restore */
        int leftcount;     /* of frames with ALIGN_LEFT or ALIGN_NOTE */
        int rightcount;    /* of frames with ALIGN_RIGHT */
        box_link *box_list; /* a generalization of the previous push/restore margin */
#ifdef STYLE_COLOR
	Byte cb_ix; 	   /* howcome 15/3/95: the index to the forgrround color of tables etc */
#endif
    };

typedef struct frame_struct Frame;

/* stack of nested frames starting after top of windo */

typedef struct sframe_struct
    {
        unsigned char *p_next;
        unsigned char *p_end;
    } StackFrame;

/* elements which can appear in a TEXTLINE & must be non-zero */

#define STRING      1       /* tag name for string values */
#define BULLET      2       /* tag name for bullet */
#define RULE        3       /* horizontal rule */
#define SEQTEXT     4       /* explicit text, e.g. "i)" */
#define IMAGE       5       /* a pixmap (character-like) */
#define INPUT       6       /* text input field */
#define LINE        7       /* staalesc: line */

#ifdef STYLE_COLOR
#define RULE_FLEN 8
#else
#define RULE_FLEN 7
#endif

#ifdef STYLE_BACKGROUND
#define RULEFLEN (POINTERSIZE + RULE_FLEN)
#else
#define RULEFLEN    RULE_FLEN
#endif

#ifdef STYLE_COLOR
#define LINE_FLEN 10
#else
#define LINE_FLEN 9
#endif

#ifdef STYLE_BACKGROUND
#define LINEFLEN (POINTERSIZE + LINE_FLEN)
#else
#define LINEFLEN LINE_FLEN
#endif

#ifdef STYLE_COLOR
#define BULLET_FLEN  8 
#else
#define BULLET_FLEN  7
#endif

#ifdef STYLE_BACKGROUND
#define BULLETFLEN (BULLET_FLEN + POINTERSIZE)
#else
#define BULLETFLEN BULLET_FLEN
#endif

#ifdef STYLE_COLOR
#define STRING_FLEN  (12 + POINTERSIZE + POINTERSIZE)     /* must bold 8byte Pointers wm 20.Jan.95 */
#else
#define STRING_FLEN  (11 + POINTERSIZE + POINTERSIZE)     /* must bold 8byte Pointers wm 20.Jan.95 */
#endif

#ifdef STYLE_BACKGROUND
#define IMAGEFLEN (9+3*POINTERSIZE)
#define INPUTFLEN (1+2*POINTERSIZE)
#define STRINGFLEN (STRING_FLEN + POINTERSIZE)
#else
#define IMAGEFLEN (13+POINTERSIZE)
#define INPUTFLEN (1+POINTERSIZE)
#define STRINGFLEN STRING_FLEN
#endif

#ifdef STYLE_BACKGROUND
#define SEQTEXTLEN (POINTERSIZE+9)
#else
#define SEQTEXTLEN 9
#endif

#ifdef STYLE_COLOR
#define SEQTEXTFLEN(len) (SEQTEXTLEN+ 1 + len) /* JS 9.6.95 was 9 */
#else
#define SEQTEXTFLEN(len) (SEQTEXTLEN+ 1 + len) /* JS 9.6.95 was 7 */
#endif

#define B_SIZE 10 /* 6 */     /* size of bullet graphic */

#ifndef max
#define max(a, b)   ((a) > (b) ? (a) : (b))
#define min(a, b)   ((a) < (b) ? (a) : (b))
#endif
#define str_cmp(a, b) ((a && b) ? strcasecmp(a, b) : (a || b))


#define CHWIDTH(font)    XTextWidth(Fonts[font], " ", 1)
#define SPACING(font)    (2 + font->max_bounds.ascent + font->max_bounds.descent)
#define BASELINE(font)   (1 + font->max_bounds.ascent)
#define STRIKELINE(font) (font->max_bounds.ascent - font->max_bounds.descent + 1)
#define LINETHICKNESS(font) (1 + font->ascent / 10)

#define ASCENT(font)     (1 + Fonts[font]->max_bounds.ascent)

#define DESCENT(font)    (1 + Fonts[font]->max_bounds.descent)
#define WIDTH(font, str, len) XTextWidth(Fonts[font], str, len)

#define EMPH_NORMAL      0
/*#define EMPH_ANCHOR     (1 << 0)*/
#define EMPH_UNDERLINE  (1 << 1)
#define EMPH_STRIKE     (1 << 2)
#define EMPH_HIGHLIGHT  (1 << 3)

#define EMPH_OVERLINE   (1 << 4)
#define EMPH_OVERLARR	(1 << 5)
#define EMPH_OVERRARR	(1 << 6)
#define EMPH_OVERTILDE	(1 << 7)
#define EMPH_OVERHAT	(1 << 8)
#define EMPH_OVERDOT    (1 << 9)
#define EMPH_OVERDDOT   (1 << 10)

#define EMPH_UNDERLARR	(1 << 11)
#define EMPH_UNDERRARR	(1 << 12)
#define EMPH_UNDERTILDE	(1 << 13)
#define EMPH_UNDERHAT	(1 << 14)

#define EMPH_ROOTLINE	(1 << 15)
/* --Spif */
#define EMPH_INPUT      0x50 /* many crashes.. so ANCHOR || 0x40 ... ;) --Spif */

/* note that PRE_TEXT is passed to display code with tag *not* emph */

#define EMPH_ANCHOR     0x10
#define ISMAP           0x80
#define PRE_TEXT        0x80     /* implies preformatted text */
#define GROOVE          0x80     /* draw horizontal rule as 3d groove */
#define HLINE           0x40     /* width instead of right coord */

#define IsWhite(c)  (c == ' ' || c == '\n' || c == '\t' || c == '\f' || c == '\r')

/* imaging capability */

typedef struct
    {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char grey;
    } Color;

#define COLOR888    888     /* DirectColor 24 bit systems */
#define COLOR444    444     /* DirectColor 12 bit systems */
#define COLOR232    232     /* 128 shared colors from default visual */
#define GREY4        4      /* 16 shared colors from default visual */
#define MONO         1      /*  when all else fails */

#define BUTTONUP     1      /* used by WhichAnchor() */
#define BUTTONDOWN   2
#define MOVEUP       3      /* Mouse move in button up state */
#define MOVEDOWN     4      /* Mouse move in button down state */

/*--------------------------------------------------------------------------------------*/

/* document tags */
/*
#define REMOTE  0
#define LOCAL   1
*/
/* GATEWAY isn't a tag but is used with
   REMOTE in calls to GetAuthorization */

#define GATEWAY 5

/* document types */

#define TEXT_DOCUMENT    "text/plain"
#define HTML_DOCUMENT    "text/html"
#define HTML3_DOCUMENT   "text/x-html3"
#define HTML_LEVEL3_DOCUMENT   "text/html; level=3"
#define GIF_DOCUMENT     "image/gif"
#define JPEG_DOCUMENT    "image/jpeg"
#define PNG_DOCUMENT     "image/png"
#define PNG_EXP_DOCUMENT "image/x-png"
#define XPM_DOCUMENT     "image/x-xpixmap"
#define XBM_DOCUMENT     "image/x-xbitmap"


/* useful macro for freeing heap strings */

#define Free(s)    {if (s) free(s); s = NULL;};  

/* Block structure used to simulate file access */

typedef struct memblock_struct
        {
                char *buffer;
                size_t next;
                size_t size;
        } Block;

/* Doc structure used for new and current document */


/* the html token codes, form parsehtml.c */

#define ENTITY          -4
#define WHITESPACE      -3     /* the specific char */
#define PCDATA          -2     /* the specific char */
#define ENDDATA         -1

#define ENDTAG          (1<<7) /*  ORed with TAG code */
#define IsTag(tag)      (tag >= 0)

#define UNKNOWN         0
#define TAG_ANCHOR      1       /* EN_TEXT */
#define TAG_BOLD        2       /* EN_TEXT */
#define TAG_DL          3       /* EN_LIST */
#define TAG_DT          4       /* EN_DEFLIST */
#define TAG_DD          5       /* EN_DEFLIST */
#define TAG_H1          6       /* EN_HEADER */
#define TAG_H2          7       /* EN_HEADER */
#define TAG_H3          8       /* EN_HEADER */
#define TAG_H4          9       /* EN_HEADER */
#define TAG_H5          10      /* EN_HEADER */
#define TAG_H6          11      /* EN_HEADER */
#define TAG_ITALIC      12      /* EN_TEXT */
#define TAG_IMG         13      /* EN_TEXT */
#define TAG_LI          14      /* EN_LIST */
#define TAG_OL          15      /* EN_LIST */
#define TAG_P           16      /* EN_BLOCK */
#define TAG_TITLE       17      /* EN_SETUP */
#define TAG_UNDERLINE   18      /* EN_TEXT */
#define TAG_UL          19      /* EN_LIST */
#define TAG_HEAD        20      /* EN_SETUP */
#define TAG_BODY        21      /* EN_MAIN */
#define TAG_HR          22      /* EN_BLOCK */
#define TAG_ADDRESS     23      /* EN_BLOCK */
#define TAG_BR          24      /* EN_TEXT */
#define TAG_STRIKE      25      /* EN_TEXT */
#define TAG_PRE         26      /* EN_BLOCK */
#define TAG_CITE        27      /* EN_TEXT */
#define TAG_CODE        28      /* EN_TEXT */
#define TAG_TT          29      /* EN_TEXT */
#define TAG_EM          30      /* EN_TEXT */
#define TAG_STRONG      31      /* EN_TEXT */
#define TAG_KBD         32      /* EN_TEXT */
#define TAG_SAMP        33      /* EN_TEXT */
#define TAG_DFN         34      /* EN_TEXT */
#define TAG_Q           35      /* EN_TEXT */
#define TAG_QUOTE       36      /* EN_BLOCK */
#define TAG_ISINDEX     37      /* EN_SETUP */
#define TAG_FIG         38      /* EN_BLOCK */
#define TAG_INPUT       39      /* EN_TEXT */
#define TAG_SELECT      40      /* EN_TEXT */
#define TAG_OPTION      41      /* EN_TEXT */
#define TAG_TEXTAREA    42      /* EN_TEXT */
#define TAG_TABLE       43      /* EN_BLOCK */
#define TAG_TR          44      /* EN_TABLE */
#define TAG_TH          45      /* EN_TABLE */
#define TAG_TD          46      /* EN_TABLE */
#define TAG_CAPTION     47      /* EN_BLOCK */
#define TAG_ADDED       48      /* EN_TEXT */
#define TAG_REMOVED     49      /* EN_TEXT */
#define TAG_MATH        50      /* EN_TEXT */
#define TAG_MARGIN      51      /* EN_TEXT */
#define TAG_ABSTRACT    52      /* EN_BLOCK */
#define TAG_BLOCKQUOTE  53      /* EN_BLOCK */
#define TAG_VAR         54      /* EN_TEXT */
#define TAG_BASE        55      /* EN_SETUP */
#define TAG_SUP         56      /* EN_TEXT */
#define TAG_SUB         57      /* EN_TEXT */
#define TAG_SMALL       58      /* EN_TEXT */
#define TAG_NOTE        59      /* EN_BLOCK */
#define TAG_ALT         60      /* EN_BLOCK */
#define TAG_STYLE       61      /* EN_SETUP */ /* howcome 26/2/95 */
#define TAG_LINK	62	/* EN_SETUP */ /* howcome 25/4/95 */
#define TAG_OVERLINE    63      /* MATH */
#define TAG_HTML_SOURCE	64	/* 1/7/95: HTML source display, for use with style sheets */
/* #define TAG_ALL		65 */	/* howcome 7/3/95: for use in style.c */
#define TAG_HTML	65	/* howcome 27/8/85: for use in style.c */
#define TAG_LAST        66	/* howcome 21/8/95: to know the size of element array */
#define TAG_FORM        67      /* --Spif 10/10/95: forms (needed to build form structure) */










/* style.c */


#define   S_FONT_FAMILY_HELVETICA	1
#define   S_FONT_FAMILY_TIMES		2



typedef enum _StyleExternalProperty {
    SE_UNKNOWN, 

    SE_FONT_SIZE,
    SE_FONT_FAMILY,
    SE_FONT_WEIGHT,
    SE_FONT_STYLE,
    SE_LINE_HEIGHT,
    SE_FONT,

    SE_COLOR,
    SE_BACKGROUND,
    SE_BG_BLEND_DIRECTION,
    SE_BG_STYLE,
    SE_BG_POSITION,

    SE_WORD_SPACING,
    SE_LETTER_SPACING,
    SE_TEXT_DECORATION,

    SE_VERTICAL_ALIGN,
    SE_TEXT_TRANSFORM,
    SE_TEXT_ALIGN,
    SE_TEXT_INDENT,

    SE_PADDING,
    SE_MARGIN_TOP,
    SE_MARGIN_RIGHT,
    SE_MARGIN_BOTTOM,
    SE_MARGIN_LEFT,
    SE_MARGIN,

    SE_DISPLAY,
    SE_WIDTH,
    SE_HEIGHT,
    SE_FLOAT,
    SE_CLEAR,
    SE_PACK,

    SE_BORDER_STYLE,
    SE_BORDER_STYLE_INTERNAL,
    SE_BORDER_WIDTH,
    SE_BORDER_WIDTH_INTERNAL,
    SE_BORDER_COLOR,
    SE_BORDER_COLOR_INTERNAL,

    SE_LIST_STYLE,
    SE_MAGNIFICATION,
    SE_WHITE_SPACE,

    SP_NPROPERTY
} StyleExternalProperty;



typedef enum _StyleInternalProperty {
    SI_UNKNOWN, 

    SI_FONT_SIZE,
    SI_FONT_FAMILY,
    SI_FONT_WEIGHT,
    SI_FONT_STYLE_SLANT,
    SI_FONT_STYLE_SMALL_CAPS,
    SI_LINE_HEIGHT,
    SI_FONT,

    SI_COLOR,
    SI_BACKGROUND,            /* soon obsolete */
    SI_BACKGROUND_COLOR,
    SI_BACKGROUND_IMAGE,

    SI_BACKGROUND_BLEND_DIRECTION,

    SI_BACKGROUND_STYLE_REPEAT,
    SI_BACKGROUND_STYLE_SCROLL,

    SI_BACKGROUND_POSITION_X,
    SI_BACKGROUND_POSITION_Y,

    SI_WORD_SPACING,
    SI_LETTER_SPACING,
    SI_TEXT_DECORATION,

    SI_VERTICAL_ALIGN,
    SI_TEXT_TRANSFORM,
    SI_TEXT_ALIGN,
    SI_TEXT_INDENT,

    SI_PADDING_TOP,
    SI_PADDING_RIGHT,
    SI_PADDING_BOTTOM,
    SI_PADDING_LEFT,

    SI_MARGIN_TOP,
    SI_MARGIN_RIGHT,
    SI_MARGIN_BOTTOM,
    SI_MARGIN_LEFT,

    SI_DISPLAY,
    SI_WIDTH,
    SI_HEIGHT,
    SI_FLOAT,
    SI_CLEAR,
    SI_PACK,

    SI_BORDER_STYLE_TOP,
    SI_BORDER_STYLE_RIGHT,
    SI_BORDER_STYLE_BOTTOM,
    SI_BORDER_STYLE_LEFT,

    SI_BORDER_WIDTH_TOP,
    SI_BORDER_WIDTH_RIGHT,
    SI_BORDER_WIDTH_BOTTOM,
    SI_BORDER_WIDTH_LEFT,

    SI_BORDER_COLOR_TOP,
    SI_BORDER_COLOR_RIGHT,
    SI_BORDER_COLOR_BOTTOM,
    SI_BORDER_COLOR_LEFT,

    SI_LIST_STYLE,
    SI_MAGNIFICATION,
    SI_WHITE_SPACE,

    SI_NPROPERTY
} StyleInternalProperty;




/* howcome 22/3/95: added strengths for Arena style sheets */


#define S_LENS		3
#define S_READER        2
#define S_AUTHOR        1
#define S_FALLBACK      0

#define S_IMPORTANT     1
#define S_LEGAL         1
#define S_NORMAL        0

#define S_SPECIFIC	0
#define S_WILDCARD	1
#define S_ANY		2

#define S_BACKGROUND_COLOR 1
#define S_BACKGROUND_IMAGE 2
#define S_BACKGROUND_FIXED 4
#define S_BACKGROUND_X_REPEAT 8
#define S_BACKGROUND_Y_REPEAT 16
#define S_BACKGROUND_ORIGIN 32


#if 0

typedef struct s_variable {
    long value;
    int strength;
} *Variable;

typedef struct s_style_elem {
    int tag;
    int influence;
    char *class;
    HTList *pattern;
    struct s_style_elem *parent;
    HTList *children;
    Variable var[S_NPROPERTY];
    
    Bool flattened;
    Bool computed[S_NPROPERTY];
    long value[S_NPROPERTY];
} StyleElem;

#endif


#define SV_NORMAL 		0

#define SV_FONT_FAMILY_SYMBOL		1
#define SV_FONT_FAMILY_HELVETICA	2
#define SV_FONT_FAMILY_TIMES		3
#define SV_FONT_FAMILY_COURIER	4
#define SV_FONT_FAMILY_TERMINAL	5
#define SV_FONT_FAMILY_FIXED		6
#define SV_FONT_FAMILY_CHARTER	7
#define SV_FONT_FAMILY_LUCIDA		8
#define SV_FONT_FAMILY_LUCIDABRIGHT	9
#define SV_FONT_FAMILY_NCS		10


#define SV_FONT_WEIGHT_NORMAL 	        0x0
#define SV_FONT_WEIGHT_EXTRA_LIGHT	0x1
#define SV_FONT_WEIGHT_LIGHT		0x1
#define SV_FONT_WEIGHT_DEMI_LIGHT	0x1
#define SV_FONT_WEIGHT_MEDIUM		0x2
#define SV_FONT_WEIGHT_DEMI_BOLD	0x3
#define SV_FONT_WEIGHT_BOLD		0x4
#define SV_FONT_WEIGHT_EXTRA_BOLD	0x5

#define SV_FONT_STYLE_NORMAL		0x0
#define SV_FONT_STYLE_ITALIC		0x1
#define SV_FONT_STYLE_ROMAN		0x2
#define SV_FONT_STYLE_OBLIQUE		0x3
#define SV_FONT_STYLE_SMALL_CAPS	0x5


#define SV_TEXT_DECORATION_NONE		0x1
#define SV_TEXT_DECORATION_UNDERLINE	0x2
#define SV_TEXT_DECORATION_OVERLINE	0x3
#define SV_TEXT_DECORATION_LINE_THROUGH	0x4
#define SV_TEXT_DECORATION_BLINK	0x5

#define SV_TEXT_TRANSFORM_NONE		0x0
#define SV_TEXT_TRANSFORM_CAPITALIZE	0x1
#define SV_TEXT_TRANSFORM_UPPERCASE       	0x2
#define SV_TEXT_TBRANSFORM_LOWERCASE	0x3

#define SV_F_SMALLCAPS_IGNORE		0x0
#define SV_F_SMALLCAPS_OFF		0x1
#define SV_F_SMALLCAPS_ON		0x2

#define SV_F_BIGINITIAL_IGNORE		0x0
#define SV_F_BIGINITIAL_OFF		0x1
#define SV_F_BIGINITIAL_ON		0x2

#define SV_F_DROPPED_IGNORE		0x0
#define SV_F_DROPPED_OFF		0x1
#define SV_F_DROPPED_ON		0x2


#define SV_MARGIN_AUTO 1

#define SW_NORMAL	0
#define SW_IMPORTANT	1 
#define SW_INSIST	2
#define SW_LEGAL	3

/* new style.h */

typedef enum {
    StringVal,                  /* Simple value: a string */
    NumVal,			    /* Simple value: a number */
    KeyVal,			    /* Simple value: a keyword */
    PlusVal,		    /* `+' */
    MinusVal,		    /* `-' */
    TimesVal,		    /* `*' */
    DivVal,			    /* `/' */
    ListVal,		    /* left=head, right=tail */
    InterLoVal,		    /* `<<' */
    InterHiVal		    /* '>>' */
} ValTp;

typedef union _Value {
    ValTp tp;
    char *str;                  /* Only if tp = StringVal */
    float num;		    /* Only if tp = NumVal */
    char *key;		    /* Only if tp = KeyVal */
    union _Value *left, *right; /* Otherwise */
} *Value;


#define val2num(v) (assert(v && v->tp == NumVal), v->num)
#define val2str(v) (assert(v && v->tp == StringVal), v->str)
#define val2key(v) (assert(v && v->tp == KeyVal), v->key)
#define val2head(v) (assert(v && v->tp == ListVal), v->left)
#define val2tail(v) (assert(v && v->tp == ListVal), v->right)


typedef enum _StyleType {
  ST_UNKNOWN,
  ST_FLAG,
  ST_FLOAT,
  ST_RGB,
  ST_PX,
  ST_PT,
  ST_PERCENT,
  ST_EM,
  ST_STR,
  ST_BGSTYLE
} StyleType;

typedef struct _StyleValue {
  long value;
  Byte type;
  StyleInternalProperty iproperty;
} StyleValue;


#define S_INITIAL_FLAG 0x4
#define S_LEADING_FLAG 0x5
#define S_INDENT_FLAG 0x6
#define S_MARGIN_TOP_FLAG 0x7


#define S_UNSET 0x0
#define S_INITIALIZED 0x1
#define S_INHERITED 0x2
#define S_INFERRED 0x3


typedef struct _StyleFlat {
  StyleValue value[SI_NPROPERTY];
  Byte weight[SI_NPROPERTY];
  Byte origin[SI_NPROPERTY];
  Byte specificity[SI_NPROPERTY];
/*  Byte status[SI_NPROPERTY]; /* S_UNSET, S_INITIALIZED,  S_INFERRED, S_INHERITED */
} StyleFlat;



typedef struct _StyleSimpleSelector {
    int element;                  /* GI or class */
    char *class;		  /* or None */
/*     AttribSpec attr; */
    StyleFlat *flat;              /* if associated with a flattened style */
} StyleSimpleSelector;



typedef struct _StyleSelector {
    StyleSimpleSelector ss;
    int specificity;
    struct _StyleSelector *pred;
    struct _StyleSelector *ancestor;
} StyleSelector;

typedef struct _StyleRule {
    StyleSelector *selector;
    StyleInternalProperty iproperty;
    StyleValue *value_p;
    Byte weight;
    Byte origin;
    Byte specificity;
    char *warning;
} StyleRule;

typedef struct _AttribSpec {
    char *name;
    Value val;
    struct _AttribSpec *next;
} *AttribSpec;

#if 0
typedef struct _Weight {      /* E.g., !legal "WBvS art. 36.5" */
    int value;                   /* 0 (default) - 3 (!legal) */
    char *warning;
} Weight;
#endif

typedef struct _StyleSheet {
    BOOL initial_flag;
    BOOL leading_flag;
    BOOL indent_flag;
    BOOL margin_top_flag;
    HTList *iproperty[SI_NPROPERTY]; /* internal property is the entry point in the style sheet */
} StyleSheet;




#if 0
typedef struct _StyleStackElement {
    StyleSimpleSelector ss;
} StyleStackElement;
#endif


/* end new style.h */





/* the main document structure */


#define DOC_NOTREGISTERED 0
#define DOC_PENDING 1
#define DOC_LOADED 2
#define DOC_PROCESSED 3
#define DOC_ABORTED 4
#define DOC_REJECTED 5  /* e.g. if image can't be decoded */
#define DOC_EXTERNAL 6  /* externally viewed */

typedef struct s_doc
{
    HTRequest *request;
    HTParentAnchor *anchor;
    char *href;			/* as found in the <a href > construct, must be freed */
    char *url;			/* points into anchor structure */
    char *title;
    char *content_buffer;
    char *tag;			/* current tag pointing into history stack */
    char *link_style;
    char *head_style;
    char *base;
    StyleSheet *style;
    long loaded_length;
    long parsed_lenght;
    unsigned int height, width;
    int state;  /* DOC_NOREGISTER DOC_PENDING, DOC_LOADED, DOC_PROCESSED, */

/*    struct s_doc *old_doc;*/
    BOOL pending_reload;       /* the document is being reloaded and should be freed when the fresh copy comes in */

    BOOL nofree;
    BOOL main_doc;             /* main doc, as opposed to inline doc */
    BOOL user_style; 		/* the document contains the user's preferred style sheet */
    HTList *host_anchors;	       /* if inline: list of hosts where referenced */
    HTList *inline_anchors;       /* if host doc: list of inline docs, if inline: list of hosts */

    BOOL show_raw;             /* for html docs, show raw data? */
    Image *image;              /* if doc is an image */
    char *paint_stream;
    long paint_lenght;
    EditorBuffer *source_editor;
    EditorBuffer *field_editor;
    Field *edited_field;
    HTList *bad_flags;
    BOOL already_displayed;
} Doc;



/* history.c */


#define HISTORY_NOTREGISTERED 0
#define HISTORY_REGISTERED 1
#define HISTORY_VERIFIED 2
#define HISTORY_DELETED 3

typedef struct s_history
{
    int state; 
    HTAnchor *anchor;
    char *tag;
    long y;
    char *title;
    /* time etc */
} History;

typedef struct s_context
{
    HTList *registered_anchors;
#if 0
    HTList *pending_docs;
    HTList *pending_anchors;
#endif
    HTList *reqs;
    HTList *conversions;
    HTParentAnchor *home_anchor;
    StyleSheet *style;
    HTList *history;
    int history_pos;		/* position in linked list */ /* probably obsolete now that list hav both previous and next */
    History *current_history;
    HTList *memory_leaks;
} Context;


#define TABSIZE         8

/* codes returned by button down handlers to indicate
   interest in receiving the corresponding button up event */

#define VOID            0
#define WINDOW          1
#define SCROLLBAR       2
#define TOOLBAR         3
#define STATUS          4

typedef struct s_button
    {
        int x;
        int y;
        unsigned int w;
        unsigned int h;
        char *label;
    } Button;


#if 0
/* constraints.c */

void set_variable_with_strength(Variable var, long val, Strength strength);
void set_variable(Variable var, long val);
#endif

/* www.c */

int GuiEvents(int s, HTRequest * rq, int f);
int CloneSelf(void);
void Redraw(int x, int y, int w, int h);
void SetBanner(char *title);
void PollEvents(int block);
void BackDoc();
int Exit(int); /* wm 18.Jan.95 Exit exits value, must be int not void */
Context *NewContext();
void ShowBusy();
void HideBusy();
void SetSelection(char *s);
int GetColor(int red, int green, int blue, unsigned long *pix);

/* file.c */

char *Uncompress(char *buf, long *len);
int HasXVSuffix(char *name);
int NewDocumentType(void);
char *ParentDirCh(char *dir);
char *GetFile(char *name);
char *CurrentDirectory(void);

/* display.c */

#define StatusTop (ToolBarHeight)
#define StatusFontOffset 5
#define ToolbarTop 5

#define WinTop    (ToolBarHeight + statusHeight)  /* howcome 7/10/94 */
#define WinBottom (win_height - sbar_width) /* howcome 7/10/94 */

#define WinWidth  (win_width - sbar_width)
#define WinHeight (WinBottom - WinTop)
#define WinLeft   0
#define WinRight (WinLeft + WinWidth)
#define Abs2Win(n) (WinTop + n - PixelOffset)
#define Win2Abs(n) ((long)n + PixelOffset - WinTop)

void SetDisplayWin(Window aWin);
void SetDisplayGC(GC aGC);
void SetDisplayFont(XFontStruct *pFont_info);
void SetColor(GC gc, int color_text_ix, int color_background_ix);
void SetFont(GC gc, int fontIndex);
void SetEmphFont(GC gc, XFontStruct *pFont, XFontStruct *pNormal);
void NewBuffer(Doc *d);
void DisplaySizeChanged(int all);
void DisplayAll();
Image *ProcessLoadedImage(Doc *doc);

char *TextLine(char *txt);
long CurrentHeight(char *buf);
/*long DocHeight(char *buf, int *width);*/
void DocDimension(Doc *doc, int *width, long *height);
int LineLength(char *buf);

int DeltaTextPosition(long h);
void MoveVDisplay(long h);
void MoveHDisplay(int indent);
void MoveLeftLine();
void MoveRightLine();
void MoveLeftPage();
void MoveRightPage();
void MoveToLeft();
void MoveToRight();
void MoveUpLine();
void MoveDownLine();
void MoveUpPage();
void MoveDownPage();
void MoveToStart();
void MoveToEnd();
void SlideHDisplay(int slider, int scrollExtent);
void SlideVDisplay(int slider, int scrollExtent);
void DisplayDoc(int x, int y, unsigned int w, unsigned int h);
void FindString(char *pattern, char **next);
void ToggleView(void);

/* scrollbar.c */

void SetScrollBarHPosition(int indent, int buffer_width);
void SetScrollBarVPosition(long offset, long buf_height);
void SetScrollBarWidth(int buffer_width);
void SetScrollBarHeight(long buff_height);

void SetScrollBarWin(Window aWin);
void SetScrollBarGC(GC aGC);

int ScrollButtonDown(int x, int y);
void ScrollButtonUp(int x, int y);
void ScrollButtonDrag(int x, int y);

void MoveHSlider(int indent, int buffer_width);
void MoveVSlider(long offset, long buf_height);
void DisplaySlider();
void DisplayScrollBar();

int AtStart(long offset);
int AtEnd(long offset);
int AtLeft(int indent);
int AtRight(int indent);

/* status.c */

void SelectStatus(int x_in, int y_in);
void ClearStatusSelection();

void HideAuthorizeWidget();
void ShowAbortButton(int n);
int StatusButtonDown(int button, int x, int y);
void StatusButtonUp(int x, int y);
void Beep();
void Announce(char *args, ...);
void Warn(char *args, ...);
void SetStatusWin(Window aWin);
void SetStatusGC(GC aGC);
void SetStatusFont(XFontStruct *pf);
int StatusActive(void);

void DrawOutSet(Window aWin, GC gc, int x, int y, unsigned int w, unsigned int h);
void DrawInSet(Window aWin, GC gc, int x, int y, unsigned int w, unsigned int h);
void DrawOutSetCircle(GC gc, int x, int y, unsigned int w, unsigned int h);
void DrawInSetCircle(GC gc, int x, int y, unsigned int w, unsigned int h);
void DisplayStatusBar();
void SetStatusString(char *s);
void ClearStatus();
void RestoreStatusString(void);
void SaveStatusString(void);
int IsEditChar(char c);
void EditChar(char c);
void MoveStatusCursor(int key);
void GetAuthorization(int mode, char *host);
char *UserName(char *who);
char *PassStr(char *who);

/* toolbar.c */

#define ICON_WIDTH 70
#define ICON_HEIGHT (statusHeight + ToolBarHeight)

void PaintVersion(int bad);
void SetToolBarWin(Window aWin);
void SetToolBarGC(GC aGC);
void SetToolBarFont(XFontStruct *pf);
void DisplayToolBar();
int ToolBarButtonDown(int x, int y);
void ToolBarButtonUp(int x, int y);
void PrintDoc();
void SaveDoc(char *file);
void DisplayExtDocument(char *buffer, long length, int type, char *path);

/* parsehtml.c */

#define MAXVAL  1024

/* alignment codes */
#define ALIGN_TOP       0
#define ALIGN_MIDDLE    1
#define ALIGN_BOTTOM    2
#define ALIGN_LEFT      3
#define ALIGN_CENTER    4
#define ALIGN_RIGHT     5
#define ALIGN_JUSTIFY   6
#define ALIGN_NOTE      7      /* used only for text flow mechanism */
#define ALIGN_BLEEDLEFT 8
#define ALIGN_BLEEDRIGHT 9

/* entity classes */

#define EN_UNKNOWN         0
#define EN_TEXT         (1 << 0)
#define EN_BLOCK        (1 << 1)
#define EN_LIST         (1 << 2)
#define EL_DEFLIST      (1 << 3)
#define EN_HEADER       (1 << 4)
#define EN_SETUP        (1 << 5)
#define EN_MAIN         (1 << 6)
#define EN_TABLE        (1 << 7)

/* form actions --Spif 11-Oct-95 */

#define GET             0
#define POST            1

/* input field types */

#define TEXTFIELD       0x00
#define CHECKBOX        0x10
#define RADIOBUTTON     0x20
#define OPTIONLIST      0x40
#define SUBMITBUTTON    0x60
#define RESETBUTTON     0x70
#define HIDDEN          0x80
#define PASSWD          0x90

#define CHECKED         0x80    /* flags field as 'active' */
#define IN_ERROR        0x40    /* field flagged as in error */
#define DISABLED        0x20    /* field is greyed out */
#define MULTIPLE        0x10    /* multiple selections are allowed */

/* window margin and indents */

#define MAXMARGIN       (win_width - sbar_width - 4)
#define MININDENT       0 /* 4 */
#define NESTINDENT      5
#define GLINDENT        30
#define OLINDENT        3
#define LINDENT         8
#define GLGAP           4


typedef struct h_link
        {
            int x;
            int y;
            unsigned int w;
            unsigned int h;
            int continuation;
            char *href;
            int reflen;
        } HyperLink;

#define MAXLINKS        512

/* structure for table widths - index from 1
   upwards as zeroth entry used for book keeping */

typedef struct t_width
        {
            int left;
            int right;
            int Cmin;   /* reused for row height in 2nd pass */
            int Cmax;   /* we track min/max width of columns */
            int rows;  /* row span count for each column */
        } ColumnWidth;

/* store current/max number of columns in first element */

#define COLS(widths)    widths->Cmin
#define MAXCOLS(widths) widths->Cmax
#define NCOLS   15

int GetWord(char *, int *, int);
void SwallowAttributes();
int GetToken(void);
char *TitleText(char *buf);
void ClipToWindow(void);
void DisplayHTML(int x, int y, unsigned int w, unsigned int h);
void ParseSGML(int mode, long *offset, char **data, long window, long bottom, char *target);
long DeltaHTMLPosition(long h);
long ParseImage(Doc *doc, int *width);
void ParseImageAttrs(char **href, int *hreflen, int *align, int *ismap, int *width, int *height);
void ParseAnchorAttrs(char **href, int *hreflen, char **name, int *namelen, char **class_p, int *class_len_p);
char *ParseAttribute(int *len);
char *ParseValue(int *len);
int TextWidth(int font, char *str, int len, int *up, int *down);
void FontSize(int font, int *ascent, int *descent);
void PutText(Frame *frame, unsigned int emph, int font, char *s, int len, int x, int y);
void PrintRule(Frame *frame, int type, int left, int right, int dy);
void PrintLine(Frame *frame, int left, int right, int top, int bottom); /* staalesc */

void OpenDoc(char *name);
void ReloadDoc(char *name);
int WindowButtonDown(int x, int y);
void WindowButtonUp(int shifted, int x, int y);
void SearchIndex(char *keywords);
char *FindEndComment(char *ptr);

/* html.c */

long ParseHTML(int *width, BOOL style_parse);
char *TopStr(Frame *frame);

/* http.c */

#define HTTP_PORT       80
#define OLD_PORT        2784
#define GOPHERPORT      70

#define SCHSIZ 12
#define HOSTSIZ 48
#define PATHSIZ 512
#define ANCHORSIZ 64

char *safemalloc(int n);
int IsHTMLDoc(char *p, int len);
char *MyHostName(void);
void InitCurrent(char *path);
char *UnivRefLoc(Doc *doc);
char *ParseReference(char *s, int local);
int HeaderLength(char *buf, int *type);
char *GetDocument(char *href, char *who, int local);
char *SearchRef(char *keywords);

/* cache.c */

int CloneHistoryFile(void);
void SetCurrent();
int ViewStack(void);
int PushDoc(long offset);
char *PopDoc(long *where);
char *GetCachedDoc(void);
int StoreNamePW(char *who);
char *RetrieveNamePW(void);

/* ftp.c */

char *CloseFTP(void);
char *GetFileData(int socket);
char *GetFTPdocument(char *host, char *path, char *who);

/* nntp.c */

char *GetNewsDocument(char *host, char *path);

/* tcp.c */

#define GATEWAYPORT 2785

void Pause(int delay);
int XPSend(int skt, char *data, int len, int once);
int XPRecv(int skt, char *msg, int len);
int Connect(int s, char *host, int port, int *ViaGateway);
char *GetData(int socket, int *length);

/* entities.c */

int entity(char *name, int *len);
void InitEntities(void);

/* image.c */

int InitImaging(int ColorStyle);
unsigned long GreyColor(unsigned int grey);
unsigned long StandardColor(unsigned char red, unsigned char green, unsigned char blue);
unsigned char *CreateBackground(unsigned int width, unsigned int height, unsigned int depth);
unsigned char *Transparent(unsigned char *p, int i, int j);
Image *GetImage(char *href, int hreflen, BOOL reload);
void FreeImages(int cloned);
void ReportStandardColorMaps(Atom which_map);
void ReportVisuals(void);
Visual *BestVisual(int class, int *depth);
void PaintFace(int happy);
void MakeFaces(unsigned int depth);
void MakeIcons(unsigned int depth);
Image *DefaultImage();

/* dither.c */
unsigned long magic2color(Byte ix);
long ix2color(int ix);

Byte rgb2magic(int r, int g, int b);
Byte rgb2ix(int status, int r, int g, int b, Bool rw);
long rgb2color(int status, int r, int g, int b, Bool rw);
void rgbClear();

/* gif.c */

unsigned char *LoadGifImage(Image *image, Block *bp, unsigned int depth);

/* jpeg.c */
unsigned char *LoadJPEGImage(Image *image, Block *bp, unsigned int depth);

/* png.c */
unsigned char *LoadPNGImage(Image *image, Block *bp, unsigned int depth);


/* forms.c */

char *strsav(char *s);
char *strdup2(char *s, int len);
void FreeForms(void);
void FreeFrames(Frame *frame);
void ScrollForms(long delta);
void ResizeForm(void);
void ProcessTableForm(int pass);
Form *GetForm(int method, char *url, int len);
Field *GetField(Form *form, int type, int x, char *name, int nlen,
               char *value, int vlen, int rows, int cols, int flags);
void AddOption(Field *field, int flags, char *label, int len);
int ClickedInField(GC gc, int indent, int baseline, Field *field, int x, int y, int event);
void PaintField(GC gc, int indent, int baseline, Field *field);
void PaintDropDown(GC gc, Field *field, int indent);
int ClickedInDropDown(GC gc, Field *field, int indent, int event, int x, int y);
Form *DefaultForm(void);
void PaintCross(GC gc, int x, int y, unsigned int w, unsigned int h);
void PaintTickMark(GC gc, int x, int y, unsigned int w, unsigned int h);
void ShowPaint(int npaint, int nobjs);
int RegisterDoc(char *buf);
int QueryCacheServer(char *command, char **response);

/* bridge.c */

/* #define HTList_destroy HTList_delete */
void HTList_destroy(HTList *me);
#if 0
void HTAlert (CONST char * Msg);
#endif
Doc *GetInline(char *href, int hreflen, BOOL reload);
BOOL DocType(char *a, char *b);
BOOL DocHTML(Doc *d);
Doc *NewDoc();
BOOL DocImage(Doc *d);
BOOL DocRawText(Doc *d);

void libEntry();
void libExit(int pCache);
void libEventLoop(int c, char *href);
void libSane();
void DisplayUrl();
BOOL HTCheckPresentation(HTList * conversions, char * content_type, double quality);


HTAnchor *libExpandHref(char *href, size_t hreflen);
Doc *libLoadAnchor(HTAnchor *anchor, Doc *host_doc, BOOL main_doc, BOOL nofree, BOOL record, BOOL reload, BOOL block, BOOL forreal);
HTAnchor *libGetDocument(char *href, size_t hreflen, Doc *host_doc, BOOL main_doc, BOOL nofree, BOOL record, BOOL reload, BOOL block);

/*void ConnectAnchorDocument(HTAnchor *anchor, HTRequest *request);*/
void ConnectAnchorDocument(HTAnchor *anchor, HTRequest *request, BOOL terminate);

/* editor.c */

void EditCurrentBuffer();

typedef struct s_bad_flag
{
    char *p;
    char *error_text;
} BadFlag;

void RecordError(char * p, char *s);
void ForkEditor(char **b);

void FormEditChar(char c);
void FormMoveCursor(int key);

/* from phb */

#define EDIT_CHARACTER_TYPE_NORMAL       0x0
#define EDIT_CHARACTER_TYPE_PARA_BREAK   0x1
#define EDIT_CHARACTER_TYPE_WORD_BREAK   0x2

#define EXTRA_LENGTH 40

typedef enum {
    edit_cursor_movement_relative,
    edit_cursor_movement_absolute
} edit_cursor_movement;

typedef enum {
    edit_cursor_unit_pixel,
    edit_cursor_unit_character,
    edit_cursor_unit_word,
    edit_cursor_unit_paragraph,
    edit_cursor_unit_page,
} edit_cursor_unit;

typedef int edit_pixels;
typedef int edit_characters;


typedef struct s_edit_buffer 
{
    HTList 	*paras;
    int		width;
    BOOL	auto_break;
    int		break_table_size;
    char 	*break_table;
    BOOL	soft_break;
} EditBuffer;

typedef struct s_edit_paragraph 
{
    char	*buffer;
    int		buffer_used;
    int		buffer_size;
    int		width;
    int		height;

    HTList 	*lines;
} EditParagraph;

typedef struct s_edit_line
{
    int		index;
    int		pixel_width;
    int		pixel_height;
} EditLine;

typedef struct s_edit_view 
{
    int		width;
    int		height;

    int		(*display_line)();
    int		(*display_cursor)();
    int		(*display_scroll)();

    EditBuffer	*main_buffer;
    EditBuffer	*cut_section_buffer;
    EditBuffer	*cut_line_buffer;
    EditBuffer	*cut_word_buffer;
    EditBuffer	*cut_character_buffer;

    int		cursor_row;
    int		cursor_column;
    int		cursor_x;
    int		cursor_y;
} EditView;



/* util.c */

void HTList_addObjectFirst (HTList *me, void *newObject);
char *www_strndup(char *s, int n);
char *str_tok(char *a, char *b, char **c);
char *chop_str(char *p);
Byte hex2byte(char c);


/* icon.c */

void SetIconWin(Window aWin);
void SetIconGC(GC aGC);
void DisplayIcon();
void LoadIcon();


/* history.c */

History *NewHistory();
void BackDoc();
void ForwardDoc();
void HomeDoc();
void HistoryVerify(HTAnchor *a);
void HistoryRecord(HTAnchor *a);
void HistoryDelete(HTAnchor *a);

/* mailcap.c */

void register_mailcap(char *mailcap);
void register_mailcaps();


#define MAILCAPS "/etc/mailcap:/usr/etc/mailcap:/usr/local/etc/mailcap"



#define VERBOSE_TRACE (debug & 1)
#define VERBOSE VERBOSE_TRACE 

#define X_TRACE (debug & 2)
#define BRIDGE_TRACE (debug & 4)
#define OPTION_TRACE (debug & 8)
#define IMAGE_TRACE (debug & 16)
#define SCROLL_TRACE (debug & 32)
#define BADFLAG_TRACE (debug & 64)
#define HISTORY_TRACE (debug & 128)
#define MENU_TRACE (debug & 128)
#define STATUS_TRACE (debug & 256)
#define TAG_TRACE (debug & 512)
#define TABLE_TRACE (debug & 1024)
#define MAILCAP_TRACE (debug & 2048)
#define STYLE_TRACE (debug & 4096)
#define CONNECT_TRACE (debug & 8192)
#define REGISTER_TRACE (debug & 16384)
#define FONT_TRACE (debug & 32768)
#define COLOR_TRACE (debug & 65536)

/* math.c */
void ParseMath(Frame *frame, int *up, int *down);

/* x11.c */

#define FONTS  256

int GetFont(char *family_l, long px_size, long weight, long style, BOOL small_caps);
void *Pop(void);
void Push(void *);
/*
int LibraryCallback( SOCKET s, HTRequest * rq, SockOps f);
int GuiEvents( SOCKET s, HTRequest * rq, SockOps f);
*/

/* menu.h */

/* boundaries */
#define MAX_ITEMLENGTH 40
#define MAX_ITEMS 10

/* titles */
#define ANCHOR_POPUP_TITLE "Anchor"
#define HISTORY_POPUP_TITLE "History"
#define POPUP_ANCHOR 0
#define POPUP_HISTORY 1

/* actions */
#define NO_ACTION 0
#define VISIT_HISTORY 1

#ifdef EXTENDED_MENUS
#define COPY_TO_CLIPBOARD 3
#define ADD_AS_BKMRK 4
/*  more can be added... */
#endif
/* status of buttons */
#define INACTIVE_BUTTON 0
#define ACTIVE_BUTTON 1
    
/* structures */
typedef struct MenuItem
{
    struct MenuItem *next;	/* next menu item */
    char *label;			    /* the character string displayed on left*/
    char *url;                  /* url if action is GOTO */
    int  action;		        /* action to be performed */
    unsigned int item_pos;        /* item number of this menu */ 
    int x;			    /* x coordinate for item*/
    int y;		        /* y coordinate for item */
    short state;		    /* state, ACTIVE_BUTTON or INACTIVE_BUTTON */
    unsigned int strlen;		    /* strlen(item) */
#ifdef SUB_MENUS
    struct MenuRoot *menu;    /* sub-menu */
#endif
} MenuItem;

typedef struct MenuRoot
{
    struct MenuItem *first;	/* first item in menu */
    struct MenuItem *last;	/* last item in menu */
    char *name;			    /* name of root, title */
    int x;
    int y;
    int x_root;
    int y_root;
    unsigned int w;		    /* width of the menu */
    unsigned int h;		    /* height of the menu,  */
    unsigned int items;		    /* number of items in the menu */
    unsigned int y_offset;          /* where initial item begins (below separator)*/
} MenuRoot;

typedef struct BG_Style          /* --Spif 20-Oct-95 style background struct */
{
    int            flag;           /* flag -> no bg, use color, use pixmap, repeat mode... */
    int            x_pos;
    int            y_pos;
    unsigned char  r, g, b;         /* color using RGB , must add the same thing for text or foreground color */
    Image          *image;
} BG_Style;




#define INITIAL_HTML "<HTML>\n<HEAD>\n<TITLE>welcome to arena</TITLE>\n<STYLE>\nh1 { font-style: small-caps;\
 font-weight: bold; color: DarkGreen; font-size: 22 } \nh2 { color: #0A0 } \nem { color: #A00 }\
 \nmath { color: #05C }\na { background: #DD0 }\n</STYLE>\n</HEAD>\n<BODY>\n\n<H1>Welcome to Arena</H1>\
\n\n<P>You are using prerelease beta3. This page should disappear when\nArena loads the initial page.  Unless \
otherwise specified on the\ncommand line or through the WWW_HOME environment variable, Arena will\nload its \
<A HREF=\"http://www.w3.org/pub/WWW/Arena/beta-3\">release page</A>. If you are behind\na firewall, this page\
 may be inaccessible and you will have to point\nArena to a SOCKSified http-server (e.g. CERN's httpd) to get\
 to\nfreedom.\n\n<P>While waiting, you may take a quick look at some of the things\nHTML3 can do for you: \n\n\
<table><tr><td align=middle><EM>Math without GIF's:</EM><tr><td\nalign=middle> <math>&thetav;<sub>0 </sub>(x,q)\
&nbsp; = 1 + 2 sum\n<sub>n=1 </sub><sup>&inf;</sup>(-1) <sup>n </sup>q <sup>n <sup>2\n</sup></sup>cos2n &pi;x, \
</math><tr> </table>\n\n<P>The example above is a table (with non-visible borders) containing\nmath markup.  The\
 colors (if you're on a color screen) come from a\nstyle sheet. You will find more style sheet examples behind \
the\nhelp button.\n\n</BODY>\n</HTML>"

/* XGetHClrs.c */
Status XAllocColors(register Display *dpy,Colormap cmap, 
		    XColor *defs, int ndefs, Status *statuses);

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#endif /* _ARENA_WWW_H_ */
