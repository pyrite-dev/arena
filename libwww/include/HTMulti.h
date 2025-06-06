/*                                                             Multiformat handling in libwww
                                   MULTIFORMAT HANDLING
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is implemented by HTMulti.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTMULTI_H
#define HTMULTI_H

#include "HTReq.h"

/*
**      Set default file name for welcome page on each directory.
*/
extern void HTAddWelcome (char * welcome_name);

/*
**      Perform multiformat handling
*/
extern char * HTMulti (HTRequest *      req,
                      char *            path,
                      struct stat *     stat_info);

#endif /* HTMULTI_H */

/*

   End of HTMulti. */
