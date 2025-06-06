/*                                                                      Rich Hypertext object
                                  RICH HYPERTEXT OBJECT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the C interface to the Objective-C (or whatever) Style-oriented HyperText
   class. It is used when a style-oriented text object is available or craeted in order to
   display hypertext.
   
   This module is a part of the  W3C Reference Library. However, it is NOT implemented
   anywhere in the library, as it is client (and often platform) specific code. Hence
   these functions must be implemented in the client. The Line Mode Browser has the
   implementation in the GridText module.
   
 */
#ifndef HTEXT_H
#define HTEXT_H

#include "HTAnchor.h"
#include "HTStyle.h"
#include "HTStruct.h"

#ifndef HyperText               /* Objective C version defined HyperText */
typedef struct _HText HText;    /* Normal Library */
#else
@class HText;                   /* Objective C version for NeXTStep */
#endif

extern HText * HTMainText;              /* Pointer to current main text */
extern HTParentAnchor * HTMainAnchor;   /* Pointer to current text's anchor */
/*

Creation and deletion

  CREATE HYPERTEXT OBJECT
  
   There are several methods depending on how much you want to specify. The output stream
   is used with objects which need to output the hypertext to a stream.  The structure is
   for objects which need to refer to the structure which is kep by the creating stream.
   
 */
extern HText *  HText_new       (HTRequest * request, HTParentAnchor * anchor);

extern HText *  HText_new2      (HTRequest *            request,
                                 HTParentAnchor *       anchor,
                                 HTStream *             output_stream);

extern HText *  HText_new3      (HTRequest *            request,
                                 HTStream *             output_stream,
                                 HTStructured *         structure);
/*

  FREE HYPERTEXT OBJECT
  
 */
extern void     HText_free (HText * me);
/*

Object Building methods

   These are used by a parser to build the text in an object HText_beginAppend must be
   called, then any combination of other append calls, then HText_endAppend.  This allows
   optimised handling using buffers and caches which are flushed at the end.
   
 */
extern void HText_beginAppend (HText * text);

extern void HText_endAppend (HText * text);
/*

  SET THE STYLE FOR FUTURE TEXT
  
 */
extern void HText_setStyle (HText * text, HTStyle * style);

/*

  ADD ONE CHARACTER
  
 */
extern void HText_appendCharacter (HText * text, char ch);

/*

  ADD A ZERO-TERMINATED STRING
  
 */
extern void HText_appendText (HText * text, CONST char * str);

/*

  NEW PARAGRAPH
  
   and similar things
   
 */
extern void HText_appendParagraph (HText * text);

extern void HText_appendLineBreak (HText * text);

extern void HText_appendHorizontalRule (HText * text);
/*

  START/END SENSITIVE TEXT
  
   The anchor object is created and passed to HText_beginAnchor. The senstive text is
   added to the text object, and then HText_endAnchor is called. Anchors may not be
   nested.
   
 */
extern void HText_beginAnchor (HText * text, HTChildAnchor * anc);
extern void HText_endAnchor (HText * text);


/*

  SET THE NEXT FREE IDENTIFIER
  
   The string must be of the form aaannnnn where aaa is the prefix for automatically
   generated ids, normally "z", and nnnn is the next unused number.  If not present,
   defaults to z0.  An editor should pick up both the a and n bits, and increment n when
   generating ids. This ensures that old ids are not reused, even if the elements using
   them have been deleted from the document.
   
 */
extern void HText_setNextId (
        HText *         text,
        CONST char *    s);

/*

  APPEND AN INLINE IMAGE
  
   The image is handled by the creation of an anchor whose destination is the image
   document to be included. The semantics is the intended inline display of the image.
   
   An alternative implementation could be, for example, to begin an anchor, append the
   alternative text or "IMAGE", then end the anchor. This would simply generate some text
   linked to the image itself as a separate document.
   
 */
extern void HText_appendImage (
        HText *         text,
        HTChildAnchor * anc,
        CONST char *    alternative_text,
        CONST char *    alignment,
        BOOL            isMap);

/*

  HTEXT_APPENDOBJECT:  APPEND AN OBJECT WHICH DOES ITS OWN WORK
  
   The SGML parameters are passed untouched. Currently this is only used for empty
   elements.  Extensions could be (1) to pass CDATA contents as well as any attributes and
   (2) to pass the whole structured stream until the compound object has parsed itself.
   
 */
extern void HText_appendObject (
        HText *                 text,
        int                     element_number,
        CONST BOOL *            present,
        CONST char * CONST *    value);

/*

  HTEXT_UNIMPLEMENTED: WARN THAT OBJECT IS NOT COMPLETELY RENDERED.
  
   If the parser realises that it has incompletely parsed an object, then it can call this
   to flag it to the object.  This will for example prevent editing or writing back.
   
 */
extern void HText_unimplemented (
        HText *         text);


/*

Utilities

  DUMP DIAGNOSTICS TO STDERR
  
 */
extern void HText_dump (HText * me);

/*

  RETURN THE ANCHOR ASSOCIATED WITH THIS NODE
  
 */
extern HTParentAnchor * HText_nodeAnchor (HText * me);


/*

Browsing functions

 */

/*

  BRING TO FRONT AND HIGHLIGHT IT
  
 */
extern BOOL HText_select (HText * text);
extern BOOL HText_selectAnchor (HText * text, HTChildAnchor* anchor);

/*

Editing functions

   These are called from the application. There are many more functions not included here
   from the orginal text object. These functions NEED NOT BE IMPLEMENTED in a browser
   which cannot edit.
   
 */
/*      Style handling:
*/
/*      Apply this style to the selection
*/
extern void HText_applyStyle (HText * me, HTStyle *style);

/*      Update all text with changed style.
*/
extern void HText_updateStyle (HText * me, HTStyle *style);

/*      Return style of  selection
*/
extern HTStyle * HText_selectionStyle (
        HText * me,
        HTStyleSheet* sheet);

/*      Paste in styled text
*/
extern void HText_replaceSel (HText * me,
        CONST char *aString,
        HTStyle* aStyle);

/*      Apply this style to the selection and all similarly formatted text
**      (style recovery only)
*/
extern void HTextApplyToSimilar (HText * me, HTStyle *style);

/*      Select the first unstyled run.
**      (style recovery only)
*/
extern void HTextSelectUnstyled (HText * me, HTStyleSheet *sheet);


/*      Anchor handling:
*/
extern void             HText_unlinkSelection (HText * me);
extern HTAnchor *       HText_referenceSelected (HText * me);
extern HTAnchor *       HText_linkSelTo (HText * me, HTAnchor* anchor);


#endif /* HTEXT_H */
/*

   end */
