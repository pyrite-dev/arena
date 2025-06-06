/*                                                                        NNTP Request Stream
                                   NNTP REQUEST STREAM
                                             
   The NNTP Request stream generates a NNTP request header and writes it to the target
   which is normally a HTWriter stream.
   
   This module is implemented by HTNewsRq.c, and it is a part of the W3C Reference
   Library.
   
 */
#ifndef HTNEWSREQ_H
#define HTNEWSREQ_H

#include "HTStream.h"
#include "HTAccess.h"
/*

  STREAMS DEFINITION
  
   This stream makes a HTNews request header before it goes into transparent mode.
   
 */
extern HTStream * HTNewsPost_new (HTRequest * request, HTStream * target);

#endif
/*

   End of HTNewsReq */
