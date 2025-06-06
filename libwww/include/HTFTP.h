/*                                                                                 FTP Access
                                   FTP ACCESS FUNCTIONS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the FTP load module that handles all communication with FTP-servers.
   
   This module is implemented by HTFTP.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTFTP_H
#define HTFTP_H
#include "HTEvntrg.h"

extern HTEventCallback HTLoadFTP;

typedef enum _FTPServerType {
    FTP_GENERIC         = 0x1,
    FTP_MACHTEN         = 0x2,
    FTP_UNIX            = 0x4,
    FTP_VMS             = 0x8,
    FTP_CMS             = 0x10,
    FTP_DCTS            = 0x20,
    FTP_TCPC            = 0x40,
    FTP_PETER_LEWIS     = 0x80,
    FTP_NCSA            = 0x200,
    FTP_WINNT           = 0x400,
    FTP_UNSURE          = 0x8000
} FTPServerType;

#define MAX_FTP_LINE    128                      /* Don't use more than this */
/*

 */
#endif
/*

   end of HTFTP Module */
