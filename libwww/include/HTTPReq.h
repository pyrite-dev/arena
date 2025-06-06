/*                                                                        HTTP Request Stream
                                   HTTP REQUEST STREAM
                                             
   The HTTP Request stream generates a HTTP request header and writes it to the target
   which is normally a HTWriter stream.
   
   This module is implemented by HTTPReq.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTTPREQ_H
#define HTTPREQ_H

#include "HTStream.h"
#include "HTReq.h"
/*

  STREAMS DEFINITION
  
   This stream makes a HTTP request header before it goes into transparent mode. If
   endHeader is YES then we send an empty CRLF in order to end the header.
   
 */
extern HTStream * HTTPRequest_new (HTRequest * request, HTStream * target,
                                   BOOL endHeader);
/*

 */
#endif
/*

   End of declaration */
