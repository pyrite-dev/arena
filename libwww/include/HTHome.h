/*                                                                          Application Stuff
                                    APPLICATION STUFF
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module provides some "make life easier" functions in order to get the application
   going. The functionality of this module was originally in HTAccess, but now it has been
   moved here as a part of the application interface where the application may use it if
   desired.
   
   This module is implemented by HTHome.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTHOME_H
#define HTHOME_H
#include "WWWLib.h"
/*

Default WWW Addresses

   These control the home page selection. To mess with these for normal browses is asking
   for user confusion.
   
 */
#define LOGICAL_DEFAULT "WWW_HOME"            /* Defined to be the home page */

#ifndef PERSONAL_DEFAULT
#define PERSONAL_DEFAULT "WWW/default.html"             /* in home directory */
#endif

/* If the home page isn't found, use this file: */
#ifndef LAST_RESORT
#define LAST_RESORT     "http://www.w3.org/"
#endif

/* If one telnets to an access point it will look in this file for home page */
#ifndef REMOTE_POINTER
#define REMOTE_POINTER  "/etc/www-remote.url"               /* can't be file */
#endif

/* and if that fails it will use this. */
#ifndef REMOTE_ADDRESS
#define REMOTE_ADDRESS  "http://www.w3.org/"                /* can't be file */
#endif

#ifndef LOCAL_DEFAULT_FILE
#define LOCAL_DEFAULT_FILE "/usr/local/lib/WWW/default.html"
#endif
/*

Home Anchor Generation

   We provide two small functions for finding the first anchor.
   
  GENERATE THE ANCHOR FOR THE HOME PAGE
  
   As it involves file access, this should only be done once when the program first runs.
   This is a default algorithm using the WWW_HOME environment variable.
   
 */
extern HTParentAnchor * HTHomeAnchor (void);
/*

  FIND RELATED NAME
  
   Creates a local file URI that can be used as a relative name when calling HTParse() to
   expand a relative file name to an absolute one. The code for this routine originates
   from the Line Mode Browser and was moved here by howcome@w3.org in order for all
   clients to take advantage.
   
 */
extern char *  HTFindRelatedName (void);
/*

Net Manager Callback Functions

   These two functions are examples of callback functions from the Net Manager which are
   called before and after a request respectively. They do a lot of the stuff an
   application often has to do anyway.
   
  BEFORE REQUEST CALLBACK
  
   This function uses all the functionaly that the app part of the Library gives for URL
   translations BEFORE a request is isseued.  It checks for Cache, proxy, and gateway (in
   that order)
   
 */
extern int HTLoadStart (HTRequest * request, int status);
/*

  AFTER REQUEST CALLBACK
  
   This function uses all the functionaly that the app part of the Library gives for
   handling AFTER a request.
   
 */
extern int HTLoadTerminate (HTRequest * request, int status);
/*

 */
#endif /* HTHOME_H */
/*

   End of Declaration */
