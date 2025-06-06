/*                                                                         ANSI C FILE Stream
                              WRTING TO A FILE USING ANSI C
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   It is useful to have both FWriter and Writer for environments in which fdopen() doesn't
   exist for example. The module contains the following parts:
   
      Basic Utility Streams
      
      An ANSI C File Writer Stream
      
      Various Converters using the File Writer Stream
      
   This module is implemented by HTFWrite.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTFWRITE_H
#define HTFWRITE_H

#include "HTStream.h"
#include "HTFormat.h"
/*

Basic Utility Streams

   These streams can be plugged in everywhere in a stream pipe.
   
  BLACK HOLE STREAM
  
   This stream simply absorbs data without doing anything what so ever. The
   HTBlackHoleConverter declaration can be used in the stream stack as a converter.
   
 */
extern HTStream * HTBlackHole (void);
extern HTConverter HTBlackHoleConverter;
/*

  THROUGH LINE
  
   This stream just pumps data right through.
   
 */
extern HTConverter HTThroughLine;
/*

  GENERIC ERROR STREAM
  
   The Error stream simply returns HT_ERROR on all methods. This can be used to stop a
   stream as soon as data arrives, for example from the network.
   
 */
extern HTStream * HTErrorStream (void);
/*

An ANSI C File Writer Stream

   This function puts up a new stream given an open file descripter. If the file is not to
   be closed afterwards, then set leave_open = NO.
   
 */
extern HTStream * HTFWriter_new (HTRequest * request,
                                 FILE * fp,
                                 BOOL leave_open);
/*

Various Converters using the File Writer Stream

   This is a set of functions that can be registered as converters. They all use the basic
   ANSI C file writer stream for writing out to the local file system.
   
 */
extern HTConverter HTSaveAndExecute, HTSaveLocally, HTSaveAndCallback;
/*

  HTSaveLocally          Saves a file to local disk. This can for example be used to dump
                         date objects of unknown media types to local disk. The stream
                         prompts for a file name for the temporary file.
                         
  HTSaveAndExecute       Creates temporary file, writes to it and then executes system
                         command (maybe an external viewer) when EOF has been reached. The
                         stream finds a suitable name of the temporary file which
                         preserves the suffix. This way, the system command can find out
                         the file type from the name of the temporary file name.
                         
  HTSaveAndCallback      This stream works exactly like the HTSaveAndExecutestream but in
                         addition when EOF has been reached, it checks whether a callback
                         function has been associated with the request object in which
                         case, this callback is being called. This can be use by the
                         application to do some processing after the system command has
                         terminated. The callback function is called with the file name of
                         the temporary file as parameter.
                         
  LOCATION OF TEMPORARY FILES
  
   The destination for temporary files can be managed by the following functions:
   
 */
extern BOOL  HTTmp_setRoot              (CONST char * tmp_root);
extern CONST char * HTTmp_getRoot       (void);
extern void  HTTmp_freeRoot             (void);
/*

   The HTTmp_freeRoot is called by the HTLibTerminate function. The default value is
   defined in HTReq.html
   
 */
#endif
/*

   End of declaration module */
