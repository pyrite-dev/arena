/*                                                                   File Suffix Bind Manager
                                 FILE SUFFIX BIND MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module sets up the binding between a file Bind and a media type, language,
   encoding etc. In a client application the Binds are used in protocols that does not
   support media types etc., like FTP, and in server applications they are used to make
   the bindings between the server and the local file store that the server can serve to
   the rest of the world (well almost). The HTFormat module holds this information against
   the accept headers received in a request and uses if for format negotiation. All the
   binding management can all be replace by a database interface.
   
   This module is implemented by HTBind.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTBIND_H
#define HTBIND_H

#include "HTFormat.h"
#include "HTAnchor.h"
/*

Initialization of the Module

   These functions must be called on startup and termination of the application. This is
   done automatically by HTLibInit() and HTLibTerminate().
   
 */
extern BOOL HTBind_init         (void);
extern BOOL HTBind_deleteAll    (void);
/*

Case Sensitivity

   Should the search for suffixes be case sensitive or not? The default value is case
   sensitive.
   
 */
extern void HTBind_caseSensitive        (BOOL sensitive);
/*

Suffix Delimiters

   Change the set of suffix delimiters. The default is a platform dependent set defined in
   the tcp module.
   
 */
extern CONST char *HTBind_delimiters    (void);
extern void HTBind_setDelimiters        (CONST char * new_suffixes);
/*

Set up Bindings Associated with a File Suffix

   There are three types of bindings:
   
      Content Type (media type)
      
      Language
      
      Content Encoding
      
   And the associated set of methods is defined as:
   
 */
extern BOOL HTBind_add          (CONST char *   suffix,
                                 CONST char *   representation,
                                 CONST char *   encoding,
                                 CONST char *   language,
                                 double         value);

extern BOOL HTBind_addType      (CONST char *   suffix,
                                 CONST char *   format,
                                 double         value);

extern BOOL HTBind_addEncoding  (CONST char *   suffix,
                                 CONST char *   encoding,
                                 double         value);

extern BOOL HTBind_addLanguage  (CONST char *   suffix,
                                 CONST char *   language,
                                 double         value);
/*

   The first method is a "super" method for binding information to a file suffic. Any of
   the string values can be NULL. If filename suffix is already defined its previous
   definition is overridden or modified. For example, a HTBind_setType and
   HTBind_setEncoding can be called with the same suffix.
   
   Calling this with suffix set to "*" will set the default representation. Calling this
   with suffix set to "*.*" will set the default representation for unknown suffix files
   which contain a "."
   
   NOTE: The suffixes can contain characters that must be escaped in a URL. However, they
   should not be encoded when parsed as the suffix parameter.
   
Determine a suitable suffix

   Use the set of bindings to find a suitable suffix (or index) for a certain combination
   of language, media type and encoding given in the anchor. Returns a pointer to a
   suitable suffix string that must be freed  by the caller. If more than one suffix is
   found they are all concatenated. If no suffix is found, NULL is returned.
   
 */
extern char * HTBind_getSuffix  (HTParentAnchor * anchor);
/*

Determine the content of an Anchor

   Use the set of bindings to find the combination of language, media type and encoding of
   a given anchor. If more than one suffix is found they are all searched. The last suffix
   has highest priority, the first one lowest. Returns the anchor object with the
   representations found. See also HTBind_getFormat
   
 */
extern BOOL HTBind_getBindings  (HTParentAnchor * anchor);
/*

Determine the content of File

   Use the set of bindings to find the combination of language, media type and encoding of
   a given anchor. If more than one suffix is found they are all searched. The last suffix
   has highest priority, the first one lowest. Returns the format, encoding, and language
   found. See also HTBind_getBindings.
   
 */
extern BOOL HTBind_getFormat    (CONST char *   filename,
                                 HTFormat *     format,
                                 HTEncoding *   encoding,
                                 HTLanguage *   language,
                                 double *       quality);
/*

Multi Format Management

   Use the set of bindings to find the combination of language, media type and encoding of
   a given anchor. If more than one suffix is found they are all searched. The last suffix
   has highest priority, the first one lowest. Returns the format, encoding, and language
   found. See also HTBind_getBindings, and HTBind_getFormat.
   
 */
#define MULTI_SUFFIX ".multi"   /* Extension for scanning formats */
#define MAX_SUFF 15             /* Maximum number of suffixes for a file */

extern HTContentDescription * HTBind_getDescription (char * file);
/*

   End of declaration module
   
 */
#endif /* HTBIND_H */
/*

    */
