/*                                                                              Style Manager
                              STYLE DEFINITION FOR HYPERTEXT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   Styles allow the translation between a logical property of a piece of text and its
   physical representation. A StyleSheet is a collection of styles, defining the
   translation necessary to represent a document. It is a linked list of styles.
   
Overriding this module

   Why is the style structure declared in the HTStyle.h module, instead of having the user
   browser define the structure, and the HTStyle routines just use sizeof() for copying?
   
   It's not obvious whether HTStyle.c should be common code.  It's useful to have common
   code for loading style sheets, especially if the movement toward standard style sheets
   gets going.
   
   If it is common code, then both the hypertext object and HTStyle.c must know the
   structure of a style, so HTStyle.h is a suitable place to put that.
   
   If we take it out of the library, then of course HTStyle could be declared as an
   undefined structure. The only references to it are in the structure-flattening code
   HTML.c and HTPlain.c, which only use HTStypeNamed().
   
   You can in any case override this function in your own code, which will prevent the
   HTStyle from being loaded.  You will be able to redefine your style structure in this
   case without problems, as no other moule needs to know it.
   
   This module is implemented by HTStyle.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTStyle_H
#define HTStyle_H

#include "HTFormat.h"
#include "HTAnchor.h"
#include "HTML.h"

typedef long int HTFont;        /* Dummy definition instead */

#ifdef DEFINE_STRUCT_HTSTYLE
#ifdef NeXT_suppressed
#include <appkit/appkit.h>
typedef NXCoord HTCoord;
#define HTParagraphStyle NXTextStyle
#define HTCoord NXCoord
typedef struct _color {
        double  grey;
        int     RGBColor;
} HTColor;
#else

typedef double HTCoord;

typedef struct _HTParagraphStyle {
    HTCoord     left_indent;            /* @@@@ junk! etc etc*/
} HTParagraphStyle;

typedef int HTColor;            /* Sorry about the US spelling! */

#endif

typedef struct {
    short               kind;           /* only NX_LEFTTAB implemented*/
    HTCoord             position;       /* x coordinate for stop */
} HTTabStop;

#else
typedef struct _HTParagraphStyle HTParagraphStyle;
#endif



#define STYLE_NAME_LENGTH       80      /* @@@@@@@@@@@ */
        

/*      The Style Structure
**      -------------------
*/

typedef struct _HTStyle HTStyle;

/*

Style Creation

  HTSTYLEMODIFY
  
   This routine is passed the style for a particular SGML nesting state, and the element
   number of a new element whithin that state. The routine returns a suitable style for
   text within the new element. It is passed a popinter tothe nesting state so that it can
   link the style back to the nesting state for later manipulation of the SGML nesting
   tree.
   
 */
extern HTStyle * HTStyleModify (
                HTStyle *       style,
                HTNesting*      nesting,
                int             element_number);



/*      Style functions:
*/
extern HTStyle * HTStyleNew (void);
extern HTStyle* HTStyleNewNamed (CONST char * name);
extern HTStyle * HTStyleFree (HTStyle * self);
#ifdef SUPRESS
extern HTStyle * HTStyleRead (HTStyle * self, HTStream * stream);
extern HTStyle * HTStyleWrite (HTStyle * self, HTStream * stream);
#endif
/*              Style Sheet
**              -----------
*/
typedef struct _HTStyleSheet {
        char *          name;
        HTStyle *       styles;
} HTStyleSheet;


/*      Stylesheet functions:
*/
extern HTStyleSheet * HTStyleSheetNew (void);
extern HTStyleSheet * HTStyleSheetFree (HTStyleSheet * self);
extern HTStyle * HTStyleNamed (HTStyleSheet * self, CONST char * name);
extern HTStyle * HTStyleMatching (HTStyleSheet *self, HTStyle * style);
/* extern HTStyle * HTStyleForRun (HTStyleSheet *self, NXRun * run); */
extern HTStyleSheet * HTStyleSheetAddStyle (HTStyleSheet * self,
        HTStyle * style);
extern HTStyleSheet * HTStyleSheetRemoveStyle (HTStyleSheet * self,
        HTStyle * style);
#ifdef SUPPRESS
extern HTStyleSheet * HTStyleSheetRead (HTStyleSheet * self,
                                                HTStream * stream);
extern HTStyleSheet * HTStyleSheetWrite (HTStyleSheet * self,
                                                HTStream * stream);
#endif
#define CLEAR_POINTER ((void *)-1)      /* Pointer value means "clear me" */
#endif /* HTStyle_H */


/*

    */
