/*                                                   Declaration of W3C Reference NEWS MODULE
                         DECLARATION OF W3C REFERENCE NEWS MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the include file for the basic NNTP module that can be used together with the
   core of the W3C Reference Library. It contains all News specific modules which are
   required to compile and build the News DLL.
   
 */
#ifndef WWWNEWS_H
#define WWWNEWS_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTNews.h"                     /* NNTP client state machine */
#include "HTNewsLs.h"                   /* Streams for parsing News output */
#include "HTNDir.h"                     /* HTML Generator */
/*

   End of News module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWNews API definition  */
