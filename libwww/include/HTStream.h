/*                                                        The Generic Stream Class Definition
                                 STREAM OBJECT DEFINITION
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   A Stream object is something which accepts a stream of text. See also the Structured
   stream definition.. Generally streams are cascaded into stream pipes where the output
   of a stream is directed into the input of the down stream object. However, it is not
   required that a stream has a target, it might as well be a black hole that just
   swallows data.
   
   The creation methods will vary on the type of Stream Object.  All creation methods
   return a pointer to the stream object below.
   
   This module is a part of the W3C Reference Library. NOTE: All methods in the stream
   definition do now return an integer. The general return codes from the methods are:
   
      HT_WOULD_BLOCK
      
      HT_ERROR
      
      HT_OK
      
      >0 - any return greater than 0 will result in that the return code will be parsed on
      from the HTSocketRead. This can be used by inidividual stream to introduce their own
      return codes.
      
   As streams now have a return value, it is very important that these return values are
   passed upstream to all the other streams in the stream pipe.
   
   It is not relevant to return how much data has been written in the stream, as there
   often will be a relationship other than 1:1 between indata and outdata. However, it is
   important that a stream keeps state (eitheron the indata or the outdata) so that it can
   accept a HT_WOULD_BLOCK and continue at a later time when the blocking situation has
   stopped.
   
 */
#ifndef HTSTREAM_H
#define HTSTREAM_H

#include "HTList.h"

typedef struct _HTStream HTStream;

typedef struct _HTStreamClass {

    char * name;
/*

   This field is for diagnostics only
   
 */
    int (*flush)        (HTStream * me);
/*

   The flush method is introduced in order to allow the stream to put any buffered data
   down the stream pipe but without taking the stream pipe down. It is for the stream to
   decide whether it has buffered data or not. In some situations, the stream might not
   want to send buffered data down the target as the date might be relevant for this
   stream only.
   
 */
    int (*_free)        (HTStream * me);
/*

   The free method is like the flush method but it also frees the current stream object
   and all stream objects down stream. When the free method has been called, the whole
   stream pipe (not only this obejct) should not accept any more data.
   
 */
    int (*abort)        (HTStream * me, HTList * errorlist);
/*

   The abort method should only be used if a stream is interrupted, for example by the
   user, or an error occurs.
   
 */
    int (*put_character)(HTStream * me, char ch);
                                
    int (*put_string)   (HTStream * me, CONST char * str);

    int (*put_block)    (HTStream * me, CONST char * str, int len);
/*

   These methods are for actually putting data down the stream. It is important that the
   most efficient method is chosen (often put_block). There is no guarantee that a stream
   won't change method, for example from put_character to put_block
   
 */
} HTStreamClass;

#endif /* HTSTREAM_H */

/*

   End of HTStream definition */
