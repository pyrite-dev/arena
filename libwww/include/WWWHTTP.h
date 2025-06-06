/*                                                   Declaration of W3C Reference HTTP MODULE
                         DECLARATION OF W3C REFERENCE HTTP MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the include file for all HTTP access including the server side and the client
   side. It can be used together with the core of the W3C Reference Library. It contains
   all HTTP specific modules which are required to compile and build the HTTP DLL.
   
 */
#ifndef WWWHTTP_H
#define WWWHTTP_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTTPUtil.h"                   /* Basic things */
#include "HTTP.h"                       /* HTTP client state machine */
#include "HTTPServ.h"                   /* HTTP server state machine */

#include "HTTPGen.h"                    /* General HTTP Header Stream */
#include "HTTPReq.h"                    /* Stream for generating requests */
#include "HTTPRes.h"                    /* Stream for gererating responses */

#include "HTAAUtil.h"                   /* Access authentication */
#include "HTAABrow.h"                   /* Access authentication */
/*

   End of HTTP module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWHTTP API definition  */
