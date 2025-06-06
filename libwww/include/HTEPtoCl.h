/*                           EPtoClient: interface between the ExtParse module and the Client
              EPTOCLIENT: INTERFACE BETWEEN THE XPARSE MODULE AND THE CLIENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module contains the interface between the XParse module and the client. The dummy
   function is only here so that clients that use the XParse module can overwrite it. See
   also HTXParse
   
   This module is implemented by HTEPtoCl.c, and it is a part of the W3C Reference
   Library.
   
 */
#ifndef HTEPTOCLIENT_H
#define HTEPTOCLIENT_H

#include "HTStream.h"
#include "HTXParse.h"

extern CallClient HTCallClient;

#endif

/*

   end */
