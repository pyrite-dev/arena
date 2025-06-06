/*                                                                                HTTP Access
                     MULTI THREADED HYPERTEXT TRANFER PROTOCOL MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is actually a very small definition file as almost everything is set up elsewhere.
   
   This module is implemented by HTTP.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTTP_H
#define HTTP_H

#include "HTEvntrg.h"
#include "HTStream.h"
#include "HTFormat.h"

extern HTEventCallback HTLoadHTTP;
extern HTConverter HTTPStatus_new;

#endif /* HTTP_H */
/*

   End of HTTP module declaration */
