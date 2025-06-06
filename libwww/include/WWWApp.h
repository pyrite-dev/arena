/*                                                     Non-Mandatory Application Include File
                          NON-MANDATORY APPLICATION INCLUDE FILE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   In addition top the basic W3C Reference Library include file called WWWLib.h you can
   also include this file called WWWApp.h depending on the needs of your application.
   However, it is not required and none of the files included below are ever used in the
   core part of the Library itself. Only if this file is included, the extra modules will
   get included in the linked object code. It is also possible to include only a subset of
   the files below if the functionality you are after is covered by them.
   
 */
#ifndef WWWAPP_H
#define WWWAPP_H
/*

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

Generating the First Anchor

   This module provides some "make life easier" functions in order to get the application
   going. They help you generate the first anchor, also called the home anchor. It also
   contains a nice set of default WWW addresses.
   
 */
#include "HTHome.h"
/*

User Dialogs and Messages

   You can register a set of callback functions to handle user prompting, error messages,
   confimations etc. Here we give a set of functions that can be used on almost anu
   thinkable platform. If you want to provide your own platform dependent implementation
   then fine :-)
   
 */
#include "HTDialog.h"
/*

After Terminating a Request

   When a request is terminated, the application often has to do some action as a result
   of the request (and of the result of the request). The Application part of the Library
   provides two following modules to handle logging and history management. You can
   register a POST request handler in the Net Manager as described in the User's Guide.
   
   You can find a function called HTLoadTerminate in the HTHome module that enables all
   the functionality for handling a request result.
   
  LOGGING
  
   Often it is required to log the requests issued to the Library. This can either be the
   case if the application is a server or it can also be useful in a client application.
   This module provides a simple logging mechanism which can be enabled if needed.
   
 */
#include "HTLog.h"
/*

  HISTORY MANAGEMENT
  
   Another type of logging is keeping track of which documents a user has visited when
   browsing along on the Web. The Library history manager provides a basic set of
   functionality to keep track of a linear history list.
   
 */
#include "HTHist.h"
/*

   End of application specific modules
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWAPP definition  */
