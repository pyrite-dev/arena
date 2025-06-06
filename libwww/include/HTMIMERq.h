/*                                                                        MIME Request Stream
                                   MIME REQUEST STREAM
                                             
   The MIME Request stream generates a MIME request header and writes it to the target
   which is normally a HTWriter stream.
   
   This module is implemented by HTMIMERq.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTMIMERQ_H
#define HTMIMERQ_H

#include "HTStream.h"
#include "HTReq.h"
/*

  STREAMS DEFINITION
  
   This stream makes a MIME header before it goes into transparent mode. If endHeader is
   YES then we send an empty CRLF in order to end the header.
   
 */
extern HTStream * HTMIMERequest_new    (HTRequest * request, HTStream * target,
                                        BOOL endHeader);

#endif
/*

   End of HTMIMERq */
