/*                                                                            String Handling
                              WWW RELATED STRING MANAGEMENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is like the generic string utility module but it contains more Web related
   string utility functions. This module is implemented by HTWWWStr.c, and it is a part of
   the  W3C Reference Library.
   
 */
#ifndef HTWWWSTR_H
#define HTWWWSTR_H
/*

Next word or quoted string

   This function returns a RFC822 word separated by space, comma, or semi-colons. pstr
   points to a string containing a word separated by white white space "," ";" or "=". The
   word can optionally be quoted using  or "" Comments surrrounded by '(' ')' are filtered
   out. On exit, pstr has been moved to the first delimiter past the field THE STRING HAS
   BEEN MUTILATED by a 0 terminator.  The function returns a pointer to the first word or
   NULL on error
   
 */
extern char * HTNextField (char** pstr);
/*

RFC1123 Date/Time Stamp String

   Returns a pointer to a static area!
   
 */
extern CONST char *HTDateTimeStr (time_t *calendar, BOOL local);
/*

Date used for directory listings

 */
extern BOOL HTDateDirStr (time_t * time, char * str, int len);
/*

Timezone Offset

   Calculates the offset from GMT in seconds. This is called from HTLibInit().
   
 */
extern long HTGetTimeZoneOffset (void);
/*

Parse a Date/Time String

   Converts a string representation in GMT to a local representation of localtime time_t.
   
 */
extern time_t HTParseTime (CONST char * str);
/*

Unique Message-ID String

 */
extern CONST char *HTMessageIdStr (void);
/*

Converts an Integer to a String using Prefix

   In computer-world 1K is 1024 bytes and 1M is 1024K -- however, sprintf() still formats
   in base-10.  Therefore I output only until 999, and then start using the next unit.
   This doesn't work wrong, it's just a feature.  The conversion is done in "str" which
   must be large enough to contain the result.
   
 */
extern void HTNumToStr (unsigned long n, char *str, int len);
/*

Conversion between URLs and Local File Names

   These are two functions that separate the URL naming syntax from platform dependent
   file naming schemes. If you are porting the code to a new platform, you probably have
   to do some translation here.
   
  CONVERT FILE URLS INTO A LOCAL REPRESENTATION
  
   The URL has already been translated through the rules in get_physical in HTAccess.c and
   all we need to do now is to map the path to a local representation, for example if must
   translate '/' to the ones that turn the wrong way ;-) Returns local file (that must be
   freed by caller) if OK, else NULL.
   
 */
extern char * HTWWWToLocal (CONST char * url, CONST char * base);
/*

  CONVERT A LOCAL FILE NAME INTO A URL
  
   Generates a WWW URL name from a local file name or NULL if error. Returns URL (that
   must be freed by caller) if OK, else NULL.
   
 */
extern char * HTLocalToWWW (CONST char * local);
/*

 */
#endif
/*

   End of declaration module  */
