/*                                                                       RFC822 Header Parser
                                       MIME PARSER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   The MIME parser stream presents a MIME document. It recursively invokes the format
   manager to handle embedded formats.
   
   As well as stripping off and parsing the headers, the MIME parser has to parse any
   weirld MIME encodings it may meet within the body parts of messages, and must deal with
   multipart messages.
   
   This module is implemented to the level necessary for operation with WWW, but is not
   currently complete for any arbitrary MIME message.
   
   Check the source for latest additions to functionality.
   
    The MIME parser is complicated by the fact that WWW allows real binary to be sent, not
   ASCII encoded.  Therefore the netascii decoding is included in this module. One cannot
   layer it by converting first from Net to local text, then decoding it. Of course, for
   local files, the net ascii decoding is not needed.  There are therefore two creation
   routines.
   
   This module is implemented by HTMIME.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTMIME_H
#define HTMIME_H

#include "HTStream.h"
#include "HTFormat.h"
/*

Stream Converters in this Module

  MIME HEADER PARSER STREAM
  
   This stream parses a complete MIME header and if a content type header is found then
   the stream stack is called. Any left over data is pumped right through the stream.
   
 */
extern HTConverter HTMIMEConvert;
/*

  MIME HEADER ONLY PARSER STREAM
  
   This stream parses a complete MIME header and then returnes HT_PAUSE. It does not set
   up any streams and resting data stays in the buffer. This can be used if you only want
   to parse the headers before you decide what to do next. This is for example the case in
   a server app.
   
 */
extern HTConverter HTMIMEHeader;
/*

 */
#endif
/*

   End of HTMIME declaration */
