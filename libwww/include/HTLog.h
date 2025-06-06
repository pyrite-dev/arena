/*                                                                                Log Manager
                                       LOG MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module maintaines logs of request to a file.
   
   This module is implemented by HTLog.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTLIBLOG_H
#define HTLIBLOG_H

#include "HTReq.h"
/*

Enable the log

   Open the log file and start doing log. The time used in the log file is either GMT or
   local dependent on local.
   
 */
extern BOOL HTLog_open (CONST char * filename, BOOL local, BOOL append);
/*

Disable the log

   Close the log file and do more log
   
 */
extern BOOL HTLog_close (void);
/*

In log Enabled?

 */
extern BOOL HTLog_isOpen (void);
/*

Log a Request

   This functions logs the result of a request.
   
 */
extern BOOL HTLog_add (HTRequest * request, int status);

#endif
/*

   End of declaration */
