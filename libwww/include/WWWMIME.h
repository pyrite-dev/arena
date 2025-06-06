/*                                                   Declaration of W3C Reference MIME MODULE
                         DECLARATION OF W3C REFERENCE MIME MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the module for basic RFC822/MIME parsing that can be used together with the
   core of the W3C Reference Library. It contains all MIME specific modules which are
   required to compile and build the MIME DLL.
   
 */
#ifndef WWWMIME_H
#define WWWMIME_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTMIME.h"                     /* MIME response parser */
#include "HTMIMERq.h"                   /* MIME request generator */
#include "HTBound.h"                    /* Multipart MIME parser */
#include "HTMulpar.h"                   /* Multipart MIME generator stream */
#include "HTHeader.h"                   /* Extra Header parser and generator */
/*

   End of MIME module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWMIME API definition  */
