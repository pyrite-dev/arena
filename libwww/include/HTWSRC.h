/*                                                             A parser for WAIS source files
                                 WAIS SOURCE FILE PARSER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This converter returns a stream object into which a WAIS source file can be written.
   The result is put via a structured stream into whatever format was required for the
   output stream.
   
    See also: HTWAIS protocol interface module.
   
   This module is implemented by HTWSRC.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTWSRC_H
#define HTWSRC_H

#include "HTFormat.h"

extern  HTConverter HTWSRCConvert;
/*

Escaping Strings

   HTDeSlash takes out the invlaid characters in a URL path ELEMENT by converting them
   into hex-escaped characters.  HTEnSlash does the reverse.
   
    Each returns a pointer to a newly allocated string which must eventually be freed by
   the caller.
   
 */
extern char * HTDeSlash (CONST char * str);

extern char * HTEnSlash (CONST char * str);

#endif

/*

                                                                                    Tim BL
                                                                                          
    */
