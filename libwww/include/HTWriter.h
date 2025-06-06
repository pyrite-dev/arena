/*                                                                       Socket Writer Stream
                          UNIX FILE DESCRIPTOR OR SOCKET WRITER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This version of the stream object just writes to a socket. The socket is assumed open
   and closed afterward.There are two versions (identical on ASCII machines) one of which
   converts to ASCII on output. We have to have the Net Manager involved as we want to
   have control of how many sockets we are using simultanously. This means that
   applications should use the ANSI C FILE writer stream for writing to an output. Proxy
   servers will have to go through the Net Manager anyway, so this will not be a problem
   for them.
   
   This module is implemented by HTWriter.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTWRITE_H
#define HTWRITE_H

#include "HTStream.h"
#include "HTNet.h"
/*

Unbuffered output

   This is a non-buffered output stream which remembers state using the write_pointer. As
   normally we have a big buffer somewhere else in the stream chain an extra output buffer
   will often not be needed. There is also a small buffer stream that can be used if this
   is not the case.
   
 */
extern HTStream * HTWriter_new (HTNet *net, BOOL leave_open);
/*

Buffered Output

   This is a buffer output stream writing to a socket. However, it uses a "one-time"
   buffer in that you can specify the total amount of bytes to be buffered. From that
   point it goes into transparent mode. If buf_size > 0 then we set up buffered output
   used for at most buf_size bytes. Otherwise we'll use nonbuffered output.
   
 */
extern HTStream * HTBufWriter_new (HTNet *net, BOOL leave_open, int buf_size);
/*

ASCII Stream Converter

   If you are on a non-ASCII machine then this stream converts to ASCII before data is
   written to the socket.
   
 */
#ifdef NOT_ASCII
extern HTStream * HTASCIIWriter (HTNet *net, BOOL leave_open);
#endif

#endif
/*

   End of socket stream declaration  */
