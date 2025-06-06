/*                                                    Declaration of W3C Reference FTP MODULE
                         DECLARATION OF W3C REFERENCE FTP MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the include file for the basic FTP module that can be used together with the
   core of the W3C Reference Library. It contains all FTP specific modules which are
   required to compile and build the FTP DLL.
   
 */
#ifndef WWWFTP_H
#define WWWFTP_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTFTP.h"                      /* FTP client state machine */
#include "HTFTPDir.h"                   /* Streams for parsing FTP output */
/*

   End of FTP module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWFTP API definition  */
