/*                                                                     Content Counter Stream
                                  CONTENT COUNTER STREAM
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This stream also buffers the result to find out the content length. If a maximum buffer
   limit is reached Content-Length is calculated for logs but it is not sent to the client
   -- rather the buffer is flushed right away. Code taken from HTRequest.c written by Ari
   Luotonen and modified to fit new stream model. The buffer stream is a small buffer that
   can be used to optimize net work access in order to prevent multiple writes.
   
   This module is implemented by HTNetTxt.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTCONLEN_H
#define HTCONLEN_H

extern HTStream * HTContentCounter      (HTStream *     target,
                                         HTRequest *    request,
                                         int            max_size);

extern HTStream * HTBuffer_new          (HTStream *     target,
                                         HTRequest *    request,
                                        int             max_size);
/*

   End of definition module
   
 */
#endif /* HTCONLEN_H */
/*

    */
