/* Experimental html-math parser for X11 browser */

/* Written by Dave Ragget, 1995 */

/*

Janne Saarela
janne.saarela@hut.fi
28.7.1995

I took the liberty of extending the mathematical text to hold
the emphasis attribute. The different EMPH-* macros
correspond to different emphasis one can give to different
symbols and text using <above> and <below> elements.

Therefore each math_box or Box now also holds the emph
as an unsigned int.

The parsing of mathematical markup was also improved to
take the attributes into account. Before they were simply
dropped out. This is why in ParseExpr() all shortrefs have
to be carefully handled. This means in TAG_SUP, TAG_SUB
and TAG_BOX the *bufptr has to be checked for the >
character.

Generally, the ParseExpr() routine is the trickies one
to understand in this file. It's highly recursive and
generates, as it proceeds, a list of boxes. In addition
one should update the x value which corresponds to the
x-coordinate of each box. The w member of the box
holds the width of each box and can be used to calculate
the x.

The next variable is created in each case: and its next
member is made to link to box variable which is always the
list of previously created boxes. This is why the box = next
assignment is also performed.

The root and sqrt elements are parsed but the corresponding
symbols and their alignments are not of satisfactory, though.

*/

/*
  .
Stale Schumacher
staalesc@usit.uio.no / stale@hypnotech.com
15.8.95

I have extended the math support and corrected a few minor
bugs. These are the most important changes:

  - Added support for the <ATOP> tag
  - Added new entity &lim; (limit function)
  - Functions (&sin;, &cos;, &tan;, etc.) are now rendered in
    an upright font, _not_ italic as they used to be. Furtermore,
    all unknown entities are treated as functions. This avoids
    the problem of defining one FUNC code for each mathematical
    function (and believe me, there are quite a few if you take
    into account things like arctan, sinh, arccosh etc.) This
    solution also enables the HTML author to "invent" new
    functions as an alternative to using <t>...</t>:
      myfunc   - rendered in italics
      &myfunc; - rendered in an upright font
    Misuse of this "feature" is discouraged... :-)
  - The characters '!' and ':' are now treated as SYMBOLs
  - Added new macro IsISOLetter to prevent Arena from hanging if
    the math markup contains national characters
  - Curly brackets (braces: '{' and '}') may now be stretched
    using <left> and <right>, just as '[', ']', '(', ')', and '|'
  - Commented out some obsolete code, including FuncCode
  - Reimplemented &prod; in MathEntity

Changes 13.12.95:

  - Changed <sqrt> and <root> to draw the radical graphically
    rather than using the symbol font
  - Changed the behaviour of <above>, <below>, <vec>, <bar> etc.
    to draw _one_ contiguous line or arrow
  - Fixed/improved a large number of alignment/placement problems
  - Added new entity &dots;

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "www.h"

#define LATIN1  0
#define SYMSET  1


#define HLINE           0x40     /* width instead of right coord */

#define NORMAL  0
#define SMALL   1

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

#define EMPH_ROOTLINE   (1 << 15)

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

#if 0
/*  janet: commented, already defined in www.h  */
/* reproduce Frame struct here for quick hack interface */

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
     }; 

typedef struct frame_struct Frame; 
#endif

extern int Here;
extern char *bufptr, *lastbufptr;
extern XFontStruct *Fonts[FONTS]; /* staalesc 13/12/95 */


/******** end of interface to X11 browser ********/

/* staalesc 15/08/95: these are now obsolete
#define SIN             1
#define COS             2
#define TAN             3
#define LOG             4
#define EXP             5
*/

/* integral, summation and product signs */
#define INT             100
#define SUM             101
#define PROD            102

/* staalesc 15/08/95: limit function */
#define LIM             103

/* staalesc 13/12/95: horizontal dots */
#define DOTS            104

#define LBRACKET        200
#define RBRACKET        201
#define LSQBRACKET      202
#define RSQBRACKET      203
#define LBRACE          204
#define RBRACE          205

/* janet 1/8/95: #define UNKNOWN        -2     */
#define BUFFEND        -1
#define NONE            0
#define SPACE           1
#define NAME            2
#define FUNC            3
#define OPERATOR        4
#define DELIM           5
#define SYM             6
#define BOX             7
#define DIVIDER         8
#define TAG_BOX         9
/* janet 1/8/95: #define TAG_SUP        10 */
/* janet 1/8/95: #define TAG_SUB        11 */
#define TAG_OVER       12
#define TAG_LEFT       13
#define TAG_RIGHT      14
/* janet 1/8/95: #define TAG_MATH       15 */
#define TAG_ROOT       16
#define TAG_ABOVE      17
#define TAG_BELOW      18
#define TAG_UPRIGHT    19
#define TAG_BUPRIGHT   20
#define TAG_BOLDTEXT   21
#define TAG_OF         22
#define TAG_VEC        23
#define TAG_BAR        24
#define TAG_DOT        25
#define TAG_DDOT       26
#define TAG_HAT        27
#define TAG_TILDE      28
#define TAG_SQRT       29
#define TAG_ATOP       30   /* staalesc 15/08/95 */
#define RADICAL        31   /* staalesc 13/12/95 */

#define STARTTAG        0
/* janet 1/8/95: #define ENDTAG          1 */
#define SHORTREF        2

#define EXPR            1
#define RSUP            2
#define RSUB            3
#define LDELIM          4
#define CSUP            5
#define CSUB            6
#define RDELIM          7
#define LSUP            8
#define LSUB            9

/* janet 1/8/95: #define ALIGN_LEFT      0 */
/* janet 1/8/95: #define ALIGN_CENTER    1 */
/* janet 1/8/95: #define ALIGN_RIGHT     2 */

#define ROWS 24
/* janet 1/8/95: #define COLS 80 */

#ifndef max
#define     max(a, b)   ((a) > (b) ? (a) : (b))
#endif

struct math_box
{
    int role;   /* EXPR, RSUP, RSUB, LDELIM, CSUP, CSUB, RDELIM, LSUP, LSUB */
    int type;   /* NAME, FUNC, OPERATOR, DELIM, BOX */
    struct math_box *next;
    struct math_box *base;
    struct math_box *indices;
    int font;
    char *str;
    int len;
    int code;
    int subcode;
    int dx, dy, w, above, below;
    unsigned int emph;
};

typedef struct math_box Box;

static int token_code, token_len, token_width, token_above, token_font,
           token_below, token_align, token_charset, token_subcode;
static char *token_str;


int math_normal_sym_font = 0, math_small_sym_font, math_normal_text_font, 
    math_small_text_font, math_italic_text_font, math_bold_text_font, math_ibold_text_font;

static int current_font_tag = 0;

int TextFont(int size, int emp)
{
    if (size == SMALL) {
	switch (emp) {
	case TAG_UPRIGHT:
	    return math_small_text_font;
	    break;
	case TAG_BUPRIGHT:
	    return math_small_text_font;
	    break;
	case TAG_BOLDTEXT:
	    return math_small_text_font;
	    break;
	default:
	    return math_small_text_font;
	    break;
	}
    } else {
	switch (emp) {
	case TAG_UPRIGHT:
	    return math_normal_text_font;
	    break;
	case TAG_BUPRIGHT:
	    return math_bold_text_font;
	    break;
	case TAG_BOLDTEXT:
	    return math_ibold_text_font;
	    break;
	default:
	    return math_italic_text_font;
	    break;
	}
    }
}

#define SymFont(size) (size == NORMAL ? math_normal_sym_font : math_small_sym_font)
#define UprightTextFont(size) (TextFont(size, TAG_UPRIGHT))

int Space(int font)
{
    int up, down;
    static int myfont = -1, width = 0;

    if (font != myfont)
    {
        myfont = font;
        width = TextWidth(myfont, " ", 1, &up, &down);
    }

    return width;
}

int divide_base(int font) /* staalesc 13/12/95 */
{
    int direction_hint, font_ascent, font_descent;
    XCharStruct overall;

    XTextExtents(Fonts[font], "o", 1,
                    &direction_hint,
                    &font_ascent,
                    &font_descent,
                    &overall);

    return font_ascent / 2;
}

Box *NewBox(int font)
{
    Box *box;

    box = (Box *)malloc(sizeof(Box));
    box->role = EXPR;
    box->next = NULL;
    box->base = NULL;
    box->indices = NULL;
    box->type = NAME;
    box->str = NULL;
    box->len = 0;
    box->dx = box->dy = box->below = 0;
    box->w = box->above = 0;
    box->font = font;
    box->emph = EMPH_NORMAL;

    return (box); /* howcome 12/10/94: the return was missing! */
}

/* janet 1/8/95: defined in www.h: #define IsWhite(c) (c == ' ' || c == '\t' || c == '\n' || c== '\r') */

#define IsLetter(c) (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
#define IsISOLetter(c) (IsLetter(c) || (192 <= c && c <= 255 && c!=215 && c!=247))
#define IsDigit(c) ('0' <= c && c <= '9')
#define IsAlphaNumeric(c) (IsLetter(c) || IsDigit(c))

static void ParseAboveAttrs (int *emph)
{
    int n, m, isSet;	/* janet 21/09/95: not used: c */
    char *attr, *attrval;

    isSet = 0;
    bufptr--;
    for (;;)
    {
        if (*bufptr == '\0')
            break;

        if (*bufptr == '>') {
	    bufptr++;
            break;
	}

        if (IsWhite(*bufptr)) {
	    bufptr++;
            continue;
	}

        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);
        if (n == 3 && strncasecmp(attr, "sym", n) == 0)
        {
            if (m == 3 && strncasecmp(attrval, "hat", m) == 0) {
                *emph |= EMPH_OVERHAT;
		isSet = 1;
		continue;
	    }
            else if (m == 5 && strncasecmp(attrval, "tilde", m) == 0) {
                *emph |= EMPH_OVERTILDE;
		isSet = 1;
		continue;
	    }
            else if (m == 4 && strncasecmp(attrval, "larr", m) == 0) {
                *emph |= EMPH_OVERLARR;
		isSet = 1;
		continue;
	    }
            else if (m == 4 && strncasecmp(attrval, "rarr", m) == 0) {
                *emph |= EMPH_OVERRARR;
		isSet = 1;
		continue;
	    }
	    continue;
        }
    }
    if (!isSet)
	*emph |= EMPH_OVERLINE;
}

static void ParseBelowAttrs (int *emph)
{
    int n, m, isSet;	/* janet 21/07/95: not used: c */
    char *attr, *attrval;

    isSet = 0;
    bufptr--;
    for (;;)
    {
        if (*bufptr == '\0')
            break;

        if (*bufptr == '>') {
	    bufptr++;
            break;
	}

        if (IsWhite(*bufptr)) {
	    bufptr++;
            continue;
	}

        attr = ParseAttribute(&n);
        attrval = ParseValue(&m);
        if (n == 3 && strncasecmp(attr, "sym", n) == 0)
        {
            if (m == 3 && strncasecmp(attrval, "hat", m) == 0) {
                *emph |= EMPH_UNDERHAT;
		isSet = 1;
		continue;
	    }
            else if (m == 5 && strncasecmp(attrval, "tilde", m) == 0) {
                *emph |= EMPH_UNDERTILDE;
		isSet = 1;
		continue;
	    }
            else if (m == 4 && strncasecmp(attrval, "larr", m) == 0) {
                *emph |= EMPH_UNDERLARR;
		isSet = 1;
		continue;
	    }
            else if (m == 4 && strncasecmp(attrval, "rarr", m) == 0) {
                *emph |= EMPH_UNDERRARR;
		isSet = 1;
		continue;
	    }
	    continue;
        }
    }
    if (!isSet)
	*emph |= EMPH_UNDERLINE;
}

static int GetTag(void)
{
    int len;	/* janet 21/07/95: not used: c */
    char *tag;

 /* bufptr -> "sub> or "/sub> etc */

    len = 0;
    token_code = STARTTAG;
    tag = bufptr;

    if (*bufptr == '/')
    {
        token_code = ENDTAG;
        ++tag;
        ++bufptr;
    }
    while (IsAlphaNumeric(*bufptr)) {
        len++;
        bufptr++;
    }

    /* don't skip attributes, Janne Saarela 9.6.1995 */

    /* skip blank space before attributes or end tag character > */
    while (IsWhite(*bufptr))
        bufptr++;

    /* skip the last > character */
/*    if (token_code == ENDTAG)
        bufptr++; */

    if (len == 3 && strncasecmp(tag, "sub", len) == 0)
        return TAG_SUB;

    if (len == 3 && strncasecmp(tag, "sup", len) == 0)
        return TAG_SUP;

    if (len == 3 && strncasecmp(tag, "box", len) == 0)
        return TAG_BOX;

    if (len == 4 && strncasecmp(tag, "over", len) == 0)
        return TAG_OVER;

    if (len == 4 && strncasecmp(tag, "atop", len) == 0)
        return TAG_ATOP;

    if (len == 4 && strncasecmp(tag, "left", len) == 0)
        return TAG_LEFT;

    if (len == 5 && strncasecmp(tag, "right", len) == 0)
        return TAG_RIGHT;

    if (len == 4 && strncasecmp(tag, "math", len) == 0)
        return TAG_MATH;

    if (len == 4 && strncasecmp(tag, "root", len) == 0)
        return TAG_ROOT;

    if (len == 4 && strncasecmp(tag, "sqrt", len) == 0)
        return TAG_SQRT;

    if (len == 2 && strncasecmp(tag, "of", len) == 0)
        return TAG_OF;

    if (len == 5 && strncasecmp(tag, "above", len) == 0)
        return TAG_ABOVE;

    if (len == 5 && strncasecmp(tag, "below", len) == 0)
        return TAG_BELOW;

    if (len == 1 && strncasecmp(tag, "t", len) == 0)
        return TAG_UPRIGHT;

    if (len == 2 && strncasecmp(tag, "bt", len) == 0)
        return TAG_BUPRIGHT;

    if (len == 1 && strncasecmp(tag, "b", len) == 0)
        return TAG_BOLDTEXT;

    if (len == 3 && strncasecmp(tag, "vec", len) == 0)
        return TAG_VEC;

    if (len == 3 && strncasecmp(tag, "bar", len) == 0)
        return TAG_BAR;

    if (len == 3 && strncasecmp(tag, "dot", len) == 0)
        return TAG_DOT;

    if (len == 4 && strncasecmp(tag, "ddot", len) == 0)
        return TAG_DDOT;

    if (len == 3 && strncasecmp(tag, "hat", len) == 0)
        return TAG_HAT;

    if (len == 5 && strncasecmp(tag, "tilde", len) == 0)
        return TAG_TILDE;
    
    return UNKNOWN;
}

/* needs expanding to cover full range of symbols as per TeX
   one unresolve question is the set of tokens like ","  */

static int DelimSymCode(int c, int *ch)
{
    if (c == '[')
    {
        *ch = 91;
        return 234;
    }

    if (c == ']')
    {
        *ch = 93;
        return 250;
    }

    if (c == '(')
    {
        *ch = 40;
        return 231;
    }

    if (c == ')')
    {
        *ch = 41;
        return 247;
    }

    if (c == '{')
    {
        *ch = 123;
        return 237;
    }

    if (c == '}')
    {
        *ch = 125;
        return 253;
    }

    if (c == '|')
    {
        *ch = 124;
        return 239;
    }

    return TRUE; 	/* janet: not of much use here, only for compilation */
}

static int DelimCode(int c, int size)
{
    char ch;

    if (c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}' || c == '|')
    {
        token_font = SymFont(size);
        ch = c;
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        if (c == ')' || c == '|' || c == ']') /* staalesc 13/12/95: added extra space */
            token_width += Space(token_font)/2;
        token_code = DelimSymCode(c, &token_subcode);
        token_charset = SYMSET;
        return c;
    }

/*    return NULL;*/
    return 0;
}

static int OperatorCode(int c, int size)
{
    char ch;

    if (c == '=' || c == '+' || c == '-' || c == '/' ||
        c == '.' || c == ',' || c == '<' || c == '>' ||
        c == '!' || c == ':')
    {
        token_font = SymFont(size);
        token_charset = SYMSET;
        ch = c;
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        if (c != '.') token_width += Space(token_font)/2; /* staalesc 13/12/95 */
        token_code = c;
        return c;
    }

/*    return NULL;*/
    return 0;
}

/* staalesc 15/08/95: FuncCode is never called, its job is done by MathEntity instead
static int FuncCode(char *str, int len, int size)
{
    int up, down;
    char ch;

    token_font = UprightTextFont(size);

    if (len == 3 && strncasecmp(str, "sin", len) == 0)
    {
        token_width = TextWidth(token_font, "sin", 3, &token_above, &token_below);
        token_code = SIN;
        return token_code;
    }

    if (len == 3 && strncasecmp(str, "cos", len) == 0)
    {
        token_width = TextWidth(token_font, "cos", 3, &token_above, &token_below);
        token_code = COS;
        return token_code;
    }

    if (len == 3 && strncasecmp(str, "tan", len) == 0)
    {
        token_width = TextWidth(token_font, "tan", 3, &token_above, &token_below);
        token_code = TAN;
        return token_code;
    }

    if (len == 3 && strncasecmp(str, "log", len) == 0)
    {
        token_width = TextWidth(token_font, "log", 3, &token_above, &token_below);
        token_code = LOG;
        return token_code;
    }

    if (len == 3 && strncasecmp(str, "exp", len) == 0)
    {
        token_width = TextWidth(token_font, "e", 1, &token_above, &token_below);
        token_code = EXP;
        return token_code;
    }

    if (len == 3 && strncasecmp(str, "int", len) == 0)
    {
        token_font = SymFont(size);
        ch = 243;
        token_width = TextWidth(token_font, &ch, 1, &up, &down);
        token_above = up + up + down;
        token_below = down + up + down;
        token_align = ALIGN_CENTER;
        token_code = INT;
        token_charset = SYMSET;
        return token_code;
    }

    if (len == 3 && strncasecmp(str, "sum", len) == 0)
    {
        token_font = SymFont(size);
        ch = 229;        
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        token_align = ALIGN_CENTER;
        token_code = SUM;
        token_charset = SYMSET;
        return token_code;
    }

    if (len == 4 && strncasecmp(str, "prod", len) == 0)
    {
        token_font = SymFont(size);
        ch = 213;        
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        token_align = ALIGN_CENTER;
        token_code = PROD;
        token_charset = SYMSET;
        return token_code;
    }

    return 0;
}
*/

static int MathEntity(int size)
{
    int  c, length;
    char ch;

    token_font = TextFont(size, current_font_tag);

    if (strncmp(bufptr, "sp;", 3) == 0)
    {
        token_width = Space(token_font);
        token_code = ' ';
        bufptr += 3;
        return SPACE;
    }

    if (strncmp(bufptr, "thinsp;", 7) == 0)
    {
        token_width = Space(token_font)/2;
        token_code = ' ';
        bufptr += 7;
        return SPACE;
    }

    if (strncmp(bufptr, "dots;", 5) == 0)
    {
        token_font = SymFont(size);
        token_width = TextWidth(token_font, "... ", 4, &token_above, &token_below);
        token_align = ALIGN_CENTER;
        token_code = DOTS;
	bufptr += 5;
        token_charset = SYMSET;
        return FUNC;
    }

    if (strncmp(bufptr, "int;", 4) == 0)
    {
        token_font = SymFont(size);
        ch = 243;
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        token_above = token_above + token_above + token_below;
        /* token_below = token_below + token_above + token_below; */  /* staalesc 13/12/95 */
        token_align = ALIGN_CENTER;
        token_code = INT;
	bufptr += 4;
        token_charset = SYMSET;
        return FUNC;
    }

    if (strncmp(bufptr, "sum;", 4) == 0)
    {
        token_font = SymFont(size);
        ch = 229;        
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        token_align = ALIGN_CENTER;
        token_code = SUM;
	bufptr += 4;
        token_charset = SYMSET;
        return FUNC;
    }

    if (strncmp(bufptr, "prod;", 5) == 0)
    {
        token_font = SymFont(size);
        ch = 213;        
        token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
        token_align = ALIGN_CENTER;
        token_code = PROD;
	bufptr += 5;
        token_charset = SYMSET;
        return FUNC;
    }

    if (strncmp(bufptr, "lim;", 4) == 0)
    { 
        token_font = UprightTextFont(size);
        token_width = TextWidth(token_font, "lim", 3, &token_above, &token_below);
        token_align = ALIGN_CENTER;
        token_code = LIM;
 	bufptr += 4;
        return FUNC;
    }

    /* Janne Saarela 1/3/95
       All the symbols entities are in the same hash table with the other
       entity names. See entities.c */

    if ( (c = entity(bufptr, &length)) )
    {
      bufptr += length-1;
      if (DelimCode(c, size))
          return DELIM; /* staalesc 15/08/95: entity may be '{' or '}' */
      ch = c;
      token_font = SymFont (size);
      token_width = TextWidth(token_font, &ch, 1, &token_above, &token_below);
      token_width += Space(token_font);
/*      token_code = ch;*/
      token_code = c; /* howcome 8/3/95 */
      token_charset = SYMSET;
      return SYM;
    }

    token_len = 0;
    token_str = bufptr;
    c = *bufptr++;

    while (IsAlphaNumeric(c))
    {
        ++token_len;
        c = *bufptr++;
    }

    if (token_len == 0)
        return UNKNOWN;

    token_font = UprightTextFont(size); /* Functions should not be rendered in italics */
    token_width = TextWidth(token_font, token_str, token_len, &token_above, &token_below);
    token_width += Space(token_font);
    return NAME;
}

static int GetMathToken(int size)
{
  /* janet 21/07/95: not used:    Box *box; */
  /* janet 21/07/95: not used:    char *str; */
    unsigned char c; /* staalesc 15/08/95: changed from int to unsigned char */
    int n;	/* janet 21/07/95: not used: font */

    token_align = ALIGN_RIGHT;
    lastbufptr = bufptr;
    token_charset = LATIN1;
    token_font = TextFont(size, current_font_tag);
    token_code = token_subcode = 0;

    while (c = *bufptr++, IsWhite(c));

    if (c == '\0')
    {
        --bufptr;
        return BUFFEND;
    }

/*    if (c == '&' && (n = MathEntity(font)))*/ /* howcome 28/2/95: font should be size? */

    if (c == '&' && (n = MathEntity(size)))
        return n;

    if (c == '^')
    {
        token_code = SHORTREF;
        return TAG_SUP;
    }

    if (c == '_')
    {
        token_code = SHORTREF;
        return TAG_SUB;
    }

    if (c == '{')
    {
        token_code = STARTTAG;
        return TAG_BOX;
    }

    if (c == '}')
    {
        token_code = ENDTAG;
        return TAG_BOX;
    }

    if ( (c == '<') &&  (bufptr[0] == '/' && IsLetter(bufptr[1]) || IsLetter(bufptr[0])) )
        return GetTag();

    if (DelimCode(c, size))
        return DELIM;

    if (OperatorCode(c, size))
        return OPERATOR;

    /* probably a NAME or FUNC */

    token_len = 0;
    token_str = bufptr - 1;


#if 0

    /* howcome 28/2/94: added more characters to the while test There
       are probably even more of these */

    while (IsAlphaNumeric(c) || (c == '\'') || (c == '*') ||
	   (c == '%') || (c == ':') || (c == ';'))
    {
        ++token_len;
        c = *bufptr++;
    }
    
#endif

    /* howcome 28/2/94: added more characters to the while test There
       are probably even more of these */

    /* jsaarela 7/3/95: had to separate digits from other symbols since
       otherwise letters following digits would be in the symbol font */

    if (IsISOLetter (c) || (c == '\'') || (c == '*') || (c == '%') ||
	       (c == ';') || (c == '.'))
    {
	while (IsISOLetter(c) || (c == '\'') || (c == '*') || (c == '%') ||
	       (c == ';') || (c == '.'))
	{
	    ++token_len;
	    c = *bufptr++;
	}
    } else if (IsDigit(c))
    {
	while (IsDigit(c))
	{
	    ++token_len;
	    c = *bufptr++;
	}
    }

/* howcome 8/3/95: jannes part ends here */

    --bufptr;

    if (token_len == 0)
        return UNKNOWN;

/* staalesc 15/08/95: FuncCode obsolete
    if (FuncCode(token_str, token_len, size))
        return FUNC;
*/

    if (IsDigit(*token_str))
    {
        if (size == SMALL)
            token_font = TextFont(size, current_font_tag);
        else
            token_font = SymFont(size);
    }
    else
        token_font = TextFont(size, current_font_tag);

    token_width = TextWidth(token_font, token_str, token_len, &token_above, &token_below);
    token_width += Space(token_font);
    return NAME;
}

/* janet 21/07/95: function is never used:
static void UnGetMathToken(void)
{
    bufptr = lastbufptr;
}
*/

/* vertically reposition indices following stretching of base */
static void AdjustIndices(Box *base, int *above, int *below)
{
    int up, down;
    Box *box;

    up = base->above;
    down = base->below;

    for (box = base->indices; box; box = box->next)
    {
        if (box->role == LSUP || box->role == CSUP || box->role == RSUP)
        {
            box->dy = base->above + box->below;
            up = max(up, box->dy + box->above);
            continue;
        }

        if (box->role == LSUB || box->role == CSUB || box->role == RSUB)
        {
            box->dy = - (base->below + box->above);
            down = max(down, box->below - box->dy);
            continue;
        }
    }

    *above = up;
    *below = down;
}

int StretchSymbol(int font, int *up, int *down)
{
    int n, H, H1, h, ascent, descent;

    FontSize(font, &ascent, &descent);
    h = ascent + descent;
    H = *up + *down;

    if (H <= h)
        return 0;

    n = ((H + h - 1)/h) - 2;
    n = max(n, 1);
    H1 = (n + 2) * h;
    *up += (H1 - H)/2;
    *down = (H1 - *up);
    return n;
}

static void SetDelimHeight(Box *delim, int above, int below, int *above1, int *below1)
{
    int up, down;
    Box *box;

    for (box = delim; box; box = box->next)
    {
        up = above;
        down = below;

        if (box->type == DELIM)
        {
            box->above = above;
            box->below = below;
            box->len = StretchSymbol(box->font, &(box->above), &(box->below));
            up = max(up, box->above);
            down = max(down, box->below);
        }
        else if (box->type == FUNC)
        {
            if (box->code == INT)
            {
                box->above = max(above, box->above);
                box->below = max(below, box->below);
                box->len = StretchSymbol(box->font, &(box->above), &(box->below));
                AdjustIndices(box, &up, &down);
                up = max(up, box->above);
                down = max(down, box->below);
            }
        }
        else if (box->type == BOX)
            SetDelimHeight(box->base, above, below, &up, &down);

        *above1 = max(up, above);
        *below1 = max(down, below);
    }
}

static Box *ParseExpr(int size, int tag, int emph, int *closetag)
{
    int token, x, hw, above, below, left, right, limits=0, endtag;
    int up, down, asc, desc, oldemph; /* janet 21/07/95: not used: tmp */
    char ch;
    int basex, baseleft, baseright, baseup, basedown;	/* janet 21/07/95: not used: baseh */
    Box *box, *next, *numerator, *denominator, *ldelim; /* janet 21/07/95: not used: *tmpBox */

    oldemph = EMPH_NORMAL;
    box = NULL;
    x = above = below = 0;
    basex = baseleft = baseright = baseup = basedown = 0;
    *closetag = BUFFEND;

    while ((token = GetMathToken(size)) != BUFFEND)
    {
        if (token == tag)
        {
            if (token_code == ENDTAG || token_code == SHORTREF)
            {
                *closetag = token;
		/* don't use swallowattribute as it does not work with } */
		if (*bufptr == '>')
		    bufptr++;
                break;
            }
        }

     /* need to do something smarter based on tag classes */
        if (token == TAG_OVER || token == TAG_ATOP || token == TAG_LEFT ||
	    token == TAG_RIGHT || token == TAG_OF)
        {
            *closetag = token;
	    SwallowAttributes ();
            break;
        }
        switch (token)
        {
            case SPACE:
                basex = x;
                baseleft = baseright = baseup = basedown = 0;
                x += token_width;
                above = max(above, token_above);
                below = max(below, token_below);
                break;

            case TAG_SUP:
/*		SwallowAttributes ();*/

		if (*bufptr == '>')
		    bufptr++;

                if (token_code != ENDTAG)
                {
                    next = ParseExpr(SMALL, TAG_SUP, emph, &endtag);
                    next->role = RSUP;

                    if (!box)
                        box = NewBox(token_font);

                    next->next = box->indices;
                    box->indices = next;

                    if (limits == ALIGN_RIGHT)
                    {
                        baseright = max(baseright, next->w);
                        next->dx = box->w - Space(token_font)/2; /* staalesc 13/12/95 */
                        next->dy = box->above;
                        baseup = max(baseup, next->above);
                        x = basex + baseleft + box->w + baseright;
                    }
                    else if (limits == ALIGN_CENTER)
                    {
                        right = (next->w)/2;
                        hw = (box->w)/2;

                        if (right > baseright + hw)
                            baseright = right + hw;

                        left = next->w - right;
                        hw = box->w - hw;
                        next->dx = hw - left;

                        if (left > baseleft + hw)
                        {
                            baseleft = left - hw;
                            box->dx = basex + baseleft;
                        }

                        next->dy = box->above + next->below;
                        baseup = max(baseup, next->above + next->below);
                        x = basex + box->w + baseright;
                    }

                    above = max(above, box->above + baseup);
                }
                break;

            case TAG_SUB:
/*		SwallowAttributes ();*/

		if (*bufptr == '>')
		    bufptr++;

                if (token_code != ENDTAG)
                {
                    next = ParseExpr(SMALL, TAG_SUB, emph, &endtag);
                    next->role = RSUB;

                    if (!box)
                        box = NewBox(token_font);

                    next->next = box->indices;
                    box->indices = next;

                    if (limits == ALIGN_RIGHT)
                    {
                        baseright = max(baseright, next->w);
                        next->dx = box->w - Space(token_font)/2;  /* staalesc 13/12/95 */
                        next->dy = - (box->below);
                        basedown = max(basedown, next->below);
                        x = baseleft + basex + box->w + baseright;
                    }
                    else if (limits == ALIGN_CENTER)
                    {
                        right = (next->w)/2;
                        hw = (box->w)/2;

                        if (right > baseright + hw)
                            baseright = right + hw;

                        left = next->w - right;
                        hw = box->w - hw;
                        next->dx = hw - left;

                        if (left > baseleft + hw)
                        {
                            baseleft = left - hw;
                            box->dx = basex + baseleft;
                        }

                        next->dy = box->dy - (box->below + next->above);  /* staalesc 13/12/95 */
                        basedown = max(basedown, next->above + next->below);
                        x = baseleft + basex + baseright + Space(token_font);
                    }

                    below = max(below, box->below + basedown - box->dy);
                }
                break;

            case TAG_BOX:
		/* cannot swallow attributes here as they cannot be found
		   in case of { and }. If some day someone introduces
		   attributes to BOX elements, this has to be changed */
		if (*bufptr == '>') {
		    bufptr++;
		}
                if (token_code != ENDTAG)
                {
                    int up1, down1;

                    limits = token_align;
                    basex = x;
                    baseleft = baseright = baseup = basedown = 0;
                    next = ParseExpr(size, TAG_BOX, emph, &endtag);

                    if (endtag == TAG_LEFT)
                    {
                        ldelim = next;

                        if (ldelim)
                            ldelim->role = LDELIM;

                        next = ParseExpr(size, TAG_BOX, emph, &endtag);
                    }
                    else
                        ldelim = NULL;

                    if (endtag == TAG_OVER || endtag == TAG_ATOP)
                    {
                        int previous_endtag = endtag; /* Need to remember this */
                        numerator = next;
                        numerator->role = CSUP;
                        denominator = ParseExpr(size, TAG_BOX, emph, &endtag);
                        denominator->role = CSUB;
                        next = NewBox(SymFont(size));
                        if (previous_endtag == TAG_OVER)
                            next->type = DIVIDER;
                        else /* previous_endtag == TAG_ATOP */
                            next->type = NONE;
                        next->code = 0;
                        next->above = 1;
                        next->below = 2;
                        next->w = 10;
                        next->dy = divide_base(next->font);  /* staalesc 13/12/95 */
                        next->indices = denominator;
                        denominator->next = numerator;

                        if (numerator->w > denominator->w)
                        {
                            right = (numerator->w)/2;
                            left = numerator->w - right;
                            numerator->dx = 2;  /* staalesc 13/12/95 */
                            denominator->dx = 2 + left - denominator->w + (denominator->w)/2;
                        }
                        else
                        {
                            right = (denominator->w)/2;
                            left = denominator->w - right;
                            denominator->dx = 2;  /* staalesc 13/12/95*/
                            numerator->dx = 2 + left - numerator->w + (numerator->w)/2;
                        }

                        numerator->dy = next->dy + next->above + numerator->below;
                        denominator->dy = next->dy - (next->below + denominator->above);
                        up = next->above + (numerator->above + numerator->below);
                        down = next->below + (denominator->above + denominator->below);
                        next->w = left + right + 3;             /* staalesc 13/12/95 */
                        above = max(above, up + next->dy);      /* staalesc 13/12/95 */
                        below = max(below, down - next->dy);    /* staalesc 13/12/95 */
                    }
                    else
                    {
                        up = next->above;
                        down = next->below;
                        above = max(above, up);
                        below = max(below, down);
                    }

                 /* at this point next is the boxed expression */

                    if (next)
                    {
                        next->next = box;
                        box = next;
                        box->dx = x;
                        /* box->dy = 0; */  /* staalesc 13/12/95 */
                    }

                /* insert left delimited in list of indices
                   and adjust position of expression to make room */

                    if (ldelim)
                    {
                        if (!box)  /* avoid core dump */
                        {
                            box = NewBox(SymFont(size));
                            box = next;
                            box->dx = x;
                            box->dy = 0;
                        }

                        SetDelimHeight(ldelim, up, down, &up1, &down1);
                        above = max(above, up1 + box->dy);
                        below = max(below, down1 + box->dy);
                        ldelim->next = box->indices;
                        box->indices = ldelim;
                        ldelim->dx = - ldelim->w;
                        ldelim->dy = 0;
                        box->dx += ldelim->w;
                    }

                    if (box)
                        x = box->dx + box->w + Space(token_font)/2;  /* staalesc 13/12/95 */

                /* and check for a <RIGHT> element */

                    if (endtag == TAG_RIGHT)
                    {
                        next = ParseExpr(size, TAG_BOX, emph, &endtag);

                        if (next)
                        {
                            if (!box)  /* avoid core dump */
                            {
                                box = NewBox(SymFont(size));
                                box = next;
                                box->dx = x;
                                box->dy = 0;
                            }

                            next->role = RDELIM;
                            SetDelimHeight(next, up, down, &up1, &down1);
                            above = max(above, up1 + box->dy);
                            below = max(below, down1 + box->dy);
                            next->next = box->indices;
                            box->indices = next;
                            next->dx = box->w;
                            next->dy = 0;
                            x += next->w;
                        }
                    }
                }
                break;

	    case TAG_UPRIGHT:
		SwallowAttributes ();
                if (token_code != ENDTAG)
                {
		    limits = token_align;
		    basex = x;
		    baseleft = baseright = baseup = basedown = 0;
		    current_font_tag = TAG_UPRIGHT;
		    next = ParseExpr(size, TAG_UPRIGHT, emph, &endtag);
		    if (next)
		    {
			if (box) {
			    next->next = box;
			    box = next;
			} else {
			    box = next;
			}
			next->font = TextFont(size, current_font_tag);
			current_font_tag = 0;
			next->dx = x;
			next->dy = 0;
		    }
                    above = max(above, next->above);  /* staalesc 13/12/95 */
		    x += next->w + 2;
		}
		break;

	    case TAG_BUPRIGHT:
		SwallowAttributes ();
                if (token_code != ENDTAG)
                {
		    limits = token_align;
		    basex = x;
		    baseleft = baseright = baseup = basedown = 0;
		    current_font_tag = TAG_BUPRIGHT;
		    next = ParseExpr(size, TAG_BUPRIGHT, emph, &endtag);
		    if (next)
		    {
			if (box) {
			    next->next = box;
			    box = next;
			} else {
			    box = next;
			}
			next->font = TextFont(size, current_font_tag);
			current_font_tag = 0;
			next->dx = x;
			next->dy = 0;
		    }
                    above = max(above, next->above);  /* staalesc 13/12/95 */
		    x += next->w + 2;
		}
		break;

	    case TAG_BOLDTEXT:
		SwallowAttributes ();
                if (token_code != ENDTAG)
                {
		    limits = token_align;
		    basex = x;
		    baseleft = baseright = baseup = basedown = 0;
		    current_font_tag = TAG_BOLDTEXT;
		    next = ParseExpr(size, TAG_BOLDTEXT, emph, &endtag);
		    if (next)
		    {
			if (box) {
			    next->next = box;
			    box = next;
			} else {
			    box = next;
			}
			next->font = TextFont(size, current_font_tag);
			current_font_tag = 0;
			next->dx = x;
			next->dy = 0;
		    }
                    above = max(above, next->above);  /* staalesc 13/12/95 */
		    x += next->w + 2;
		}
		break;

	    case TAG_SQRT:
		SwallowAttributes ();
                if (token_code != ENDTAG)
                {
                    limits = token_align;
                    basex = x;
                    baseleft = baseright = baseup = basedown = 0;

		    /* Create a box for the square root symbol */
		    next = NewBox (SymFont(size));
		    next->font = SymFont (size);
		    next->type = RADICAL;
		    ch = 214; /* the square root character */
		    next->w = 2 + TextWidth(next->font, &ch, 1,
					&token_above, &token_below);
		    next->dx = x;
		    next->dy = 0;
		    next->above = token_above;
		    next->below = token_below;
		    next->next = box;
		    box = next;
		    x += next->w;
                    next = ParseExpr(size, TAG_SQRT, emph, &endtag);
		    if (next)
		    {
			next->dx = x;
			next->dy = 0;
			x += next->w;

                        /* staalesc 13/12/95: Stretch the radical to fit expression */
			box->above = next->above += 2;
                        box->below = next->below;
			next->next = box;
			box = next;

                        /* staalesc 13/12/95: Place the whole expression inside its own box */
                        next = NewBox(token_font);
                        next->type = BOX;
		        next->emph |= EMPH_ROOTLINE;
                        next->dx = box->dx;
                        next->dy = 0;
                        next->above = box->above;
                        next->below = box->below;
                        next->w = x - box->dx;
                        next->next = box;
                        box = next;

			limits = token_align;
			above = max (above, next->above);
			below = max (below, next->below);
		    }
		}
		break;

	    case TAG_ROOT:
		SwallowAttributes ();
                if (token_code != ENDTAG)
                {
                    limits = token_align;
                    basex = x;
                    baseleft = baseright = baseup = basedown = 0;

		    /* create a box in which we can draw the square root symbol */
		    next = NewBox (SymFont(size));
		    next->font = SymFont (size);
		    next->type = RADICAL;
		    ch = 214; /* the square root character */
		    next->w = 2 + TextWidth(next->font, &ch, 1,
					&token_above, &token_below);
		    next->dx = x;
		    next->dy = 0;
		    next->above = token_above;
		    next->below = token_below;
		    next->next = box;
		    box = next;
		    x += next->w;

                    next = ParseExpr(SMALL, TAG_ROOT, emph, &endtag);
                    if (endtag == TAG_OF)
                    {
			/* place the index of the root over the root sign */
			if (next)
                        {
			    FontSize(next->font, &asc, &desc);
			    next->dx = box->dx-next->w+box->w;
			    /* next->dy = (asc+desc)*0.95; */
			    next->above = token_above;
			    next->below = token_below;
			    next->next = box;
			    box = next;

			    limits = token_align;
			    above = max (above, next->above);
			    below = max (below, next->below);
			}
			next = ParseExpr(size, TAG_ROOT, emph, &endtag);
			if (next)
                        {
			    next->dx = x;
			    next->dy = 0;
			    x += next->w;

                            /* staalesc 13/12/95: Stretch the radical to fit expression */
			    box->next->above = next->above += 2;
                            box->next->below = next->below;
                            box->dy = next->above-asc+2; /* position the index correctly */
			    next->next = box;
			    box = next;

                            /* staalesc 13/12/95: Place the whole expression inside its own box */
                            next = NewBox(token_font);
                            next->type = BOX;
		            next->emph |= EMPH_ROOTLINE;
                            next->dx = box->dx;
                            next->dy = 0;
                            next->above = box->above;
                            next->below = box->below;
                            next->w = x - box->dx;
                            next->next = box;
                            box = next;
			    
			    limits = token_align;
			    above = max (above, next->above);
			    below = max (below, next->below);
			}
 		    } else { /* Missing <OF> tag: treat just like <SQRT> */
			if (next)
			{
			    next->dx = x;
			    next->dy = 0;
			    x += next->w;

                            /* staalesc 13/12/95: Stretch the radical to fit expression */
			    box->above = next->above += 2;
                            box->below = next->below;
			    next->next = box;
			    box = next;

                            /* staalesc 13/12/95: Place the whole expression inside its own box */
                            next = NewBox(token_font);
                            next->type = BOX;
		            next->emph |= EMPH_ROOTLINE;
                            next->dx = box->dx;
                            next->dy = 0;
                            next->above = box->above;
                            next->below = box->below;
                            next->w = x - box->dx;
                            next->next = box;
                            box = next;

			    limits = token_align;
			    above = max (above, next->above);
			    below = max (below, next->below);
			}
		    }
		}
		break;

            case TAG_VEC:
            case TAG_BAR:
            case TAG_DOT:
            case TAG_DDOT:
            case TAG_HAT:
            case TAG_TILDE:
		SwallowAttributes ();
                if (token_code != ENDTAG)
                {
                    limits = token_align;
                    basex = x;
                    baseleft = baseright = baseup = basedown = 0;
                    next = ParseExpr (size, token, emph, &endtag);
                    if (next)
                    {
                        next->dx = x;
                        next->dy = 0;

                        next->above = token_above;
                        next->below = token_below;
                        next->next = box;
                        box = next;
                        x += next->w;

                        oldemph = emph;

                        switch (token) {
                        case TAG_VEC:
                            emph |= EMPH_OVERRARR;
                            break;
                        case TAG_BAR:
                            emph |= EMPH_OVERLINE;
                            break;
                        case TAG_DOT:
                            emph |= EMPH_OVERDOT;
                            break;
                        case TAG_DDOT:
                            emph |= EMPH_OVERDDOT;
                            break;
                        case TAG_HAT:
                            emph |= EMPH_OVERHAT;
                            break;
                        case TAG_TILDE:
                            emph |= EMPH_OVERTILDE;
                            break;
                        default:
                            break;
                        }   

                        /* staalesc 13/12/95: Place the whole expression inside its own box */
                        next = NewBox(token_font);
                        next->type = BOX;
		        next->emph = emph;
                        next->dx = basex;
                        next->dy = 0;
                        next->above = box->above;
                        next->below = box->below;
                        next->w = x - basex;
                        next->next = box;
                        box = next;

                        emph = oldemph;

                        above = max(above, next->above);
                        below = max(below, next->below);
                    }
                }
                break;

	    case TAG_ABOVE:
		if (token_code != ENDTAG)
		{
                    int newemph = 0;
		    ParseAboveAttrs (&newemph);
		    limits = token_align;
		    basex = x;
		    baseleft = baseright = baseup = basedown = 0;
		    next = ParseExpr (size, TAG_ABOVE, emph, &endtag);
		    if (next)
		    {
			next->dx = x;
			next->dy = 0;

			next->above = token_above;
			next->below = token_below;
			next->next = box;
			box = next;
			x += next->w;

                        /* staalesc 13/12/95: Place the whole expression inside its own box */
                        next = NewBox(token_font);
                        next->type = BOX;
		        next->emph = newemph;
                        next->dx = basex;
                        next->dy = 0;
                        next->above = box->above;
                        next->below = box->below;
                        next->w = x - basex;
                        next->next = box;
                        box = next;

			above = max(above, next->above);
			below = max(below, next->below);
		    }
		}
		break;

	    case TAG_BELOW:
		if (token_code != ENDTAG)
		{
                    int newemph = 0;
		    ParseBelowAttrs (&newemph);
		    limits = token_align;
		    basex = x;
		    baseleft = baseright = baseup = basedown = 0;
		    next = ParseExpr (size, TAG_BELOW, emph, &endtag);
		    if (next)
		    {
			next->dx = x;
			next->dy = 0;

			next->above = token_above;
			next->below = token_below;
			next->next = box;
			box = next;
			x += next->w;

                        /* staalesc 13/12/95: Place the whole expression inside its own box */
                        next = NewBox(token_font);
                        next->type = BOX;
		        next->emph = newemph;
                        next->dx = basex;
                        next->dy = 0;
                        next->above = box->above;
                        next->below = box->below;
                        next->w = x - basex;
                        next->next = box;
                        box = next;

			above = max(above, next->above);
			below = max(below, next->below);
		    }
		}
		break;

            case NAME:
                limits = token_align;
                basex = x;
                baseleft = baseright = baseup = basedown = 0;
                next = NewBox(token_font);
                next->font = token_font;
                next->type = NAME;
                next->str = token_str;
                next->len = token_len;
		next->emph = emph;
                next->dx = x;
                next->dy = 0;
                next->above = token_above;
                next->below = token_below;
                next->w = token_width;
                next->next = box;
                box = next;
                x += box->w;
                above = max(above, token_above);
                below = max(below, token_below);
                break;

            case FUNC:
                limits = token_align;
                basex = x;
                baseleft = baseright = baseup = basedown = 0;
                next = NewBox(token_font);
                next->font = token_font;
                next->type = FUNC;
                next->code = token_code;
		next->emph = emph;
                next->dx = x;
                if (next->code == LIM)
                    next->dy = divide_base(next->font);  /* staalesc 13/12/95 */
                else
                    next->dy = 0;
                next->above = token_above;
                next->below = token_below;
                next->w = token_width;
                next->next = box;
                box = next;
                x += box->w;
                above = max(above, token_above + box->dy);
                below = max(below, token_below - box->dy);
                break;

            case OPERATOR:
                if (token_code=='=')
                    token_align = ALIGN_CENTER;  /* staalesc 13/12/95 */
                limits = token_align;
                basex = x;
                baseleft = baseright = baseup = basedown = 0;
                next = NewBox(token_font);
                next->type = OPERATOR;
                next->code = token_code;
		next->emph = emph;
                if (next->code == '.')
                    x -= Space(token_font);  /* staalesc 13/12/95 */
                next->dx = x;
                next->dy = 0;
                next->above = token_above;
                next->below = token_below;
                next->w = token_width;
                next->next = box;
                box = next;
                x += box->w+2;
                above = max(above, token_above);
                below = max(below, token_below);
                break;

            case RADICAL:
            case DELIM:
            case SYM:
                if (token_code==174 || token_code==222 || token_code==219)
                    token_align = ALIGN_CENTER;  /* staalesc 13/12/95 */
                limits = token_align;
                basex = x;
                baseleft = baseright = baseup = basedown = 0;
                next = NewBox(token_font);
                next->type = token;
                next->code = token_code;
                next->subcode = token_subcode;
		next->emph = emph;
                next->dx = x;
                next->dy = 0;
                next->above = token_above;
                next->below = token_below;
                next->w = token_width;
                next->next = box;
                box = next;
                x += box->w;
                above = max(above, token_above);
                below = max(below, token_below);
                break;

	    case TAG_MATH:
	    case TAG_LEFT:
	    case TAG_RIGHT:
		SwallowAttributes ();
		break;
        }
    }

 /* place non-atomic expression in a box */

    if (box)
    {
        if (box->next != NULL || box->indices != NULL)
        {
            next = NewBox(TextFont(size, current_font_tag));
            next->type = BOX;
            next->base = box;
            box = next;
        }

        box->w = x;
        box->above = above;
        box->below = below;
    }

    return box;
}

static void PutChar(Frame *frame, int emph, int font, char c, int x, int y)
{
    PutText(frame, emph, font, &c, 1, x, y);
}

static void PrintStr(Frame *frame, Box *box, int x, int y)
{
    x += box->dx;
    y += box->dy;
    PutText(frame, box->emph, box->font, box->str, box->len, x, y);
}

/* added void before DrawBigSym wm 20.Jan.95 */
static void DrawBigSym(Frame *frame, Box *box, int x, int y, char topch, char middlech, char bottomch)
{
    int len, h, asc, desc, m;

    FontSize(box->font, &asc, &desc);
    h = asc + desc;
    y += box->above - asc;
    PutChar(frame, box->emph, box->font, topch, x, y);
    len = box->len;
    m = len/2;

    while (len-- > 0)
    {
        y -= h;
        /* staalesc 15/08/95: kludge for '{' and '}' */
        if ((len!=m) && ((box->subcode==123) || (box->subcode==125)))
            PutChar(frame, box->emph, box->font, 239, x, y);
        else
            PutChar(frame, box->emph, box->font, middlech, x, y);
    }

     y -= h;
    PutChar(frame, box->emph, box->font, bottomch, x, y);
}

static void PrintFunc(Frame *frame, Box *box, int x, int y)
{
  /* janet 21/07/95: not used:    int i; */
    char *s = NULL;

    x += box->dx;
    y += box->dy;

    switch(box->code)
    {
        case INT:
            DrawBigSym(frame, box, x, y, 243, 244, 245);
            return;

        case SUM:
            PutChar(frame, box->emph, box->font, 229, x, y);
            return;

        case PROD:
            PutChar(frame, box->emph, box->font, 213, x, y);
            return;

        case LIM:
            s = "lim";
            break;

        case DOTS:
            s = "...";
            break;
    }

    PutText(frame, box->emph, box->font, s, strlen(s), x, y);
}

static void PrintSym(Frame *frame, Box *box, int x, int y)
{
  /*  janet 21/07/95: not used:   int i;   */
  /*  janet 21/07/95: not used:   char *s; */

    x += box->dx;
    y += box->dy;
    PutChar(frame, box->emph, box->font, box->code, x, y);
}

static void PrintOperator(Frame *frame, Box *box, int x, int y)
{
    x += box->dx;
    y += box->dy;
    PutChar(frame, box->emph, box->font, box->code, x, y);
}

static void PrintDelim(Frame *frame, Box *box, int x, int y)
{
  /*  janet 21/07/95: not used:   int i; */

    x += box->dx;
    y += box->dy;

    if (box->len > 0)
    {
        if (box->subcode == 124)  /* kludge for '|' */
            DrawBigSym(frame, box, x, y, box->code, box->code, box->code);
        else
            DrawBigSym(frame, box, x, y, box->code-1, box->code, box->code+1);
    }
    else
        PutChar(frame, box->emph, box->font, box->subcode, x, y);
}

static void RenderMath(Frame *frame, Box *box, int x, int y)
{
  /* janet 21/07/95: not used:    Box *indices; */

    if (box == NULL)
        return;

    RenderMath(frame, box->next, x, y);

/* staalesc: Uncomment this for debugging:

    PrintLine (frame,
               x + box->dx,
               x + box->dx + box->w,
	       y + box->dy - box->below,
               y + box->dy - box->below);
    PrintLine (frame,
               x + box->dx,
               x + box->dx + box->w,
	       y + box->dy + box->above,
               y + box->dy + box->above);
    PrintLine (frame,
               x + box->dx,
               x + box->dx,
	       y + box->dy - box->below,
               y + box->dy + box->above);
    PrintLine (frame,
               x + box->dx + box->w,
               x + box->dx + box->w,
	       y + box->dy - box->below,
               y + box->dy + box->above);
*/

    switch(box->type)
    {
        case NAME:
            PrintStr(frame, box, x, y);
            break;

        case FUNC:
            PrintFunc(frame, box, x, y);
            break;

        case OPERATOR:
            PrintOperator(frame, box, x, y);
            break;

        case DELIM:
            PrintDelim(frame, box, x, y);
            break;

        case RADICAL:
            {
		PrintLine (frame,
                           x + box->dx,
                           x + box->dx+2,
			   y + box->dy - box->below + (box->above+box->below)/2,
                           y + box->dy - box->below + (box->above+box->below)/2 + 1);
		PrintLine (frame,
                           x + box->dx+2,
                           x + box->dx+2 + (box->w-2)/3,
			   y + box->dy - box->below + (box->above+box->below)/2,
                           y + box->dy - box->below + 1);
		PrintLine (frame,
                           x + box->dx+2 + (box->w-2)/3,
                           x + box->dx + box->w,
			   y + box->dy - box->below + 1,
                           y + box->dy + box->above);
            }
            break;

        case SYM:
            PrintSym(frame, box, x, y);
            break;

        case DIVIDER:
            PrintRule(frame, HLINE, x + box->dx, box->w, y + box->dy);
            break;

        case BOX:
	    /* staalesc 13/12/95: Handle all the EMPH tags here */
	    if (box->emph & EMPH_ROOTLINE) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy + box->above);
		PrintLine (frame, x + box->dx + box->w, x + box->dx + box->w,
			   y + box->dy + box->above, y + box->dy + box->above - 2);
	    }
	    if (box->emph & EMPH_OVERLINE) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy + box->above - 1);
	    }
	    if (box->emph & EMPH_OVERRARR) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy + box->above - 1);
		PrintLine (frame, x + box->dx + box->w, x + box->dx + box->w - 2,
			   y + box->dy + box->above - 1, y + box->dy + box->above - 3);
	        PrintLine (frame, x + box->dx + box->w, x + box->dx + box->w - 2,
			   y + box->dy + box->above - 1, y + box->dy + box->above + 1);
	    }
	    if (box->emph & EMPH_OVERLARR) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy + box->above - 1);
		PrintLine (frame, x + box->dx, x + box->dx + 2,
			   y + box->dy + box->above - 1, y + box->dy + box->above - 3);
	        PrintLine (frame, x + box->dx, x + box->dx + 2,
			   y + box->dy + box->above - 1, y + box->dy + box->above + 1);
	    }
	    if (box->emph & EMPH_UNDERLINE) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy - box->below - 1);
	    }
	    if (box->emph & EMPH_UNDERRARR) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy - box->below - 1);
		PrintLine (frame, x + box->dx + box->w, x + box->dx + box->w - 3,
			   y + box->dy - box->below - 1, y + box->dy - box->below - 3);
	        PrintLine (frame, x + box->dx + box->w, x + box->dx + box->w - 2,
			   y + box->dy - box->below - 1, y + box->dy - box->below + 1);
	    }
	    if (box->emph & EMPH_UNDERLARR) {
		PrintRule (frame, HLINE, x + box->dx, box->w,
			   y + box->dy - box->below - 1);
		PrintLine (frame, x + box->dx, x + box->dx + 2,
			   y + box->dy - box->below - 1, y + box->dy - box->below - 3);
	        PrintLine (frame, x + box->dx, x + box->dx + 2,
			   y + box->dy - box->below - 1, y + box->dy - box->below + 1);
	    }
	    RenderMath(frame, box->base, x + box->dx, y + box->dy);
            break;
    }

    RenderMath(frame, box->indices, x + box->dx, y);
}

/* shouldn't BoxFree be freeing more than b->next? */
void BoxFree(Box *b)
{
  if (b->next) 
    BoxFree(b->next);	
#if 0
  else {

    Free(b->indices);		/* janet 2/8/95: added  */
    Free(b->base);
    Free(b->str); 
    Free(b);
    return;
  }
*/

  /* janet 2/8/95: added freeing base and indices too  */

  if (b->base)
    BoxFree(b->base);
  else
    return;
  
  if (b->indices)
    BoxFree(b->indices);
  else	
    return;
  
  Free(b->next); 		/* janet: what about free(b->indices), free(b->base), free(b->str), free(b) ?*/
  b->next = NULL;

#endif 

  if (b->indices)
    BoxFree(b->indices);		/* janet 2/8/95: added  */
  if (b->base)
    BoxFree(b->base);

  /*   Free(b->str); */

  Free(b);
}



#define IDX_IFIXEDFONT         10

void ParseMath(Frame *frame, int *up, int *down)
{
    int endtag;
    Box *box;

    box = ParseExpr(NORMAL, TAG_MATH, EMPH_NORMAL, &endtag); /* purify: box and it's children has allocated memory */

    /* howcome 16/3/95: added test for box existance, but why doesn't <MATH></MATH> render properly? */

    if (box) { 
	RenderMath(frame, box, Here, 0);
	*up = box->above;
	*down = box->below;
	Here += box->w;
    }

    BoxFree(box);  /* janet: something is not being freed here, what? (64 bytes)*/
}



