/*                                             Escape and Unescape Illegal Characters in URIs
                      ESCAPE AND UNESCAPE ILLEGAL CHARACTERS IN URIS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module have been spawned from HTParse, as it then can be used in utility programs
   without loading a whole bunch of the library code. It contains functions for escaping
   and unescaping a URI for reserved characters in URIs.
   
   This module is implemented by HTEscape.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTESCAPE_H
#define HTESCAPE_H

/*

Encode Unacceptable Characters using %xy

   This funtion takes a string containing any sequence of ASCII characters, and returns a
   malloced string containing the same infromation but with all "unacceptable" characters
   represented in the form %xy where x and y are two hex digits.
   
 */
typedef enum _HTURIEncoding {
    URL_XALPHAS         = 0x1,
    URL_XPALPHAS        = 0x2,
    URL_PATH            = 0x4
} HTURIEncoding;

extern char * HTEscape (CONST char * str, HTURIEncoding mask);
/*

Decode %xy Escaped Characters

   This function takes a pointer to a string in which character smay have been encoded in
   %xy form, where xy is the acsii hex code for character 16x+y. The string is converted
   in place, as it will never grow.
   
 */
extern char * HTUnEscape (char * str);
/*

 */
#endif  /* HTESCAPE_H */
/*

   End of HTEscape Module */
