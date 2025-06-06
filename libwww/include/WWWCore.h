/*                                                  Declaration of W3C Reference Library CORE
                        DECLARATION OF W3C REFERENCE LIBRARY CORE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the basic include file for the core of the W3C Reference Library. Togrther with
   the WWWUtil module it contains all core specific modules which are required to compile
   and build the Library.
   
   The core part of the Library is designed as a set of registration modules with no real
   functionality in itself. Instead all the functionality comes when the applciation
   registeres the modules that provides a desired functionaly, for example accessing HTTP
   servers or the local file system. The Library has a special include file called
   WWWApp.h which contains all converters, protocol modules, and a lot of other "sugar"
   modules that can make the core a very powerful Web interface. You can include this one
   if the application is to use all the functionality of the Library.
   
 */
#ifndef WWWCORE_H
#define WWWCORE_H
/*

System dependencies

 */
#include "tcp.h"
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

Core Modules

 */
#include "HTAccess.h"                   /* Document access network code */
#include "HTAlert.h"                    /* User Messages and Dialogs */
#include "HTAnchor.h"                   /* Anchor class Definition */
#include "HTBind.h"                     /* Binding to file suffixes */
#include "HTConLen.h"                   /* Content Length Counter */
#include "HTDNS.h"                      /* Host name cache */
#include "HTError.h"                    /* Error manager */
#include "HTEscape.h"                   /* Escape and unescape URLs */
#include "HTEvntrg.h"                   /* Event manager */
#include "HTFWrite.h"                   /* Write to an ANSI file descriptor */
#include "HTFormat.h"                   /* Stream Stack and content neg. */
#include "HTMethod.h"                   /* Request methods like PUT, GET */
#include "HTNet.h"                      /* public part of Net Manager */
#include "HTParse.h"                    /* Parse URLs */
#include "HTProt.h"                     /* Protocol Manager */
#include "HTReq.h"                      /* public part of Request Manager */
#include "HTSocket.h"                   /* public part of Net Manager */
#include "HTTCP.h"                      /* GetXbyY functions */
#include "HTTee.h"                      /* T streem for splitting a stream */
#include "HTWWWStr.h"                   /* Web Related String Functions */
#include "HTWriter.h"                   /* Write to a unix file descriptor */
/*

   End of Core modules
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWCore API definition  */
