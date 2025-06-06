/*                                                                                URI Parsing
                                       URI PARSING
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module contains code to parse URIs and various related things such as:
   
      Parse a URI for tokens
      
      Canonicalization of URIs
      
      Search a URI for illigal characters in order to prevent security holes
      
   This module is implemented by HTParse.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTPARSE_H
#define HTPARSE_H

#include "HTEscape.h"
/*

Parsing URIs

   These functions can be used to get information in a URI.
   
  PARSE A URI RELATIVE TO ANOTHER URI
  
   This returns those parts of a name which are given (and requested) substituting bits
   from the related name where necessary. The aName argument is the (possibly relative)
   URI to be parsed, the relatedName is the URI which the aName is to be parsed relative
   to. Passing an empty string means that the aName is an absolute URI. The following are
   flag bits which may be OR'ed together to form a number to give the 'wanted' argument to
   HTParse.
   
 */
#define PARSE_ACCESS            16
#define PARSE_HOST               8
#define PARSE_PATH               4
#define PARSE_ANCHOR             2
#define PARSE_PUNCTUATION        1
#define PARSE_ALL               31
/*

   where the format of a URI is as follows:
   
 */
/*
        ACCESS :// HOST / PATH # ANCHOR
*/
/*

   PUNCTUATION means any delimiter like '/', ':', '#' between the tokens above. The string
   returned by the function must be freed by the caller.
   
 */
extern char * HTParse  (CONST char * aName, CONST char * relatedName,
                        int wanted);
/*

  CREATE A RELATIVE (PARTIAL) URI
  
   This function creates and returns a string which gives an expression of one address as
   related to another.  Where there is no relation, an absolute address is retured.
   
  ON ENTRY,
  
   Both names must be absolute, fully qualified names of nodes (no anchor bits)
   
  ON EXIT,
  
   The return result points to a newly allocated name which, if parsed by HTParse relative
   to relatedName, will yield aName. The caller is responsible for freeing the resulting
   name later.
   
 */
extern char * HTRelative (CONST char * aName, CONST char *relatedName);
/*

Canonicalization

   Canonicalization of URIs is a difficult job, but it saves a lot of down loads and
   double entries in the cache if we do a good job. A URI is allowed to contain the
   seqeunce xxx/../ which may be replaced by "" , and the seqeunce "/./" which may be
   replaced by "/".  Simplification helps us recognize duplicate URIs. Thus, the following
   transformations are done:
   
       /etc/junk/../fred
      becomes
      /etc/fred
      
       /etc/junk/./fred
      becomes
      /etc/junk/fred
      
   but we should NOT change
   
       http://fred.xxx.edu/../.. or
      
       ../../albert.html
      
   In the same manner, the following prefixed are preserved:
   
       ./
      
       //
      
   In order to avoid empty URIs the following URIs become:
   
       /fred/..
      
      
      becomes /fred/..
      
       /fred/././..
      
      becomes /fred/..
      
       /fred/.././junk/.././
      becomes /fred/..
      
   If more than one set of `://' is found (several proxies in cascade) then only the part
   after the last `://' is simplified.
   
 */
extern char *HTSimplify (char **filename);
/*

Prevent Security Holes

   In many telnet like protocols, it can be very dangerous to allow a full ASCII character
   set to be in a URI. Therefore we have to strip them out. HTCleanTelnetString() makes
   sure that the given string doesn't contain characters that could cause security holes,
   such as newlines in ftp, gopher, news or telnet URLs; more specifically: allows
   everything between hexadesimal ASCII 20-7E, and also A0-FE, inclusive.
   
  str                     the string that is *modified* if necessary.  The string will be
                             truncated at the first illegal character that is encountered.
                         
  returns                 YES, if the string was modified.      NO, otherwise.
                         
 */
extern BOOL HTCleanTelnetString (char * str);
/*

 */
#endif  /* HTPARSE_H */
/*

   End of HTParse Module */
