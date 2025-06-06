/*                                                                         Socket I/O Manager
                      MANAGES READ AND WRITE TO AND FROM THE NETWORK
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module defines the read and write functions to and from the network. As we are
   having reentrant function and a smarter network I/O this will get very small :-)
   
   This module is implemented by HTSocket.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTSOCKET_H
#define HTSOCKET_H

#include "tcp.h"
#include "HTReq.h"
#include "HTStream.h"
#include "HTAnchor.h"
#include "HTEvntrg.h"
/*

Create an Input Buffer

   This function allocates a input buffer and binds it to the socket descriptor given as
   parameter. The size of the buffer, INPUT_BUFFER_SIZE, is a compromise between speed and
   memory. Here it is chosen as the default TCP High Water Mark (sb_hiwat) for receiving
   data.
   
 */
#define INPUT_BUFFER_SIZE 8192

typedef struct _HTInputSocket HTInputSocket;

extern HTInputSocket* HTInputSocket_new (SOCKET file_number);
/*

Free an Input Buffer

 */
extern void HTInputSocket_free (HTInputSocket * isoc);
/*

Load Data from a Socket

   This function is a wrapper around the HTSocketRead()declared below. It provides a
   callback function for the event loop so that a socket can be loaded using non-blocking
   I/O. The function requires an opensocket. It will typically be used in server
   applications or in a client application that can read directly from stdin.
   
 */
extern HTEventCallback HTLoadSocket;
/*

Read Data from a Socket

   This function has replaced many other functions for doing read from a socket. It
   automatically converts from ASCII if we are on a NON-ASCII machine. This assumes that
   we do not use this function to read a local file on a NON-ASCII machine. The following
   type definition is to make life easier when having a state machine looking for a <CRLF>
   sequence.
   
 */
typedef enum _HTSocketEOL {
    EOL_ERR = -1,
    EOL_BEGIN = 0,
    EOL_FCR,
    EOL_FLF,
    EOL_DOT,
    EOL_SCR,
    EOL_SLF
} HTSocketEOL;

extern FILE * HTSocket_DLLHackFopen (const char * filename, const char * mode);
extern int HTSocketRead (HTRequest * request, HTNet * net);
/*

Read from an ANSI file Descriptor

   This function has replaced the HTParseFile() and HTFileCopy functions for read from an
   ANSI file descriptor.
   
 */
extern int HTFileRead   (HTRequest * request, HTNet * net, FILE * fp);
/*

 */
#endif
/*

   End of declaration module  */
