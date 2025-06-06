/*                                                                 Access manager  for libwww
                                      ACCESS MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is the application interface module to the Request Manager. You can use the
   Request Manager directly but this module makes it easier to use by providing a lot of
   small request functions using the Request Manager in different ways. It contains help
   functions for accessing documents and for uploading documents to a remote server.
   
   This module is implemented by HTAccess.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTACCESS_H
#define HTACCESS_H

#include "HTReq.h"
#include "HTAnchor.h"
/*

Generic Library Functions

   These are functions that affect the overall behavior of the Library.
   
  [IMAGE] INITIALIZING AND TERMINATING THE LIBRARY
  
   These two functions initiates memory and settings for the Library and cleans up memory
   kept by the Library when about to exit the application. They must be used!
   
 */
extern BOOL HTLibInit (CONST char * AppName, CONST char * AppVersion);
extern BOOL HTLibTerminate (void);
/*

  LIBRARY NAME AND VERSION
  
   You can get the generic name of the Library and the version by using the following
   functions:
   
 */
extern CONST char * HTLib_name (void);
extern CONST char * HTLib_version (void);
/*

  APPLICATION NAME AND VERSION
  
   Returns the name of the application and the version number that was passed to the
   HTLibInit() function.
   
 */
extern CONST char * HTLib_appName (void);
extern CONST char * HTLib_appVersion (void);
/*

  ACCESSING THE LOCAL FILE SYSTEM
  
   The Library does normally use the local file system for dumping unknown data objects,
   file cache etc. In some situations this is not desired and we can therefore turn it
   off. This mode also prevents you from being able to access other resources where you
   have to log in, fr example telnet.
   
 */
extern BOOL HTLib_secure (void);
extern void HTLib_setSecure (BOOL mode);
/*

LOAD Request Methods

  REQUEST A DOCUMENT FROM ABSOLUTE NAME
  
   Request a document referencd by an absolute URL.  Returns YES if request accepted, else
   NO
   
 */
extern BOOL HTLoadAbsolute (CONST char * url, HTRequest* request);
/*

  REQUEST A DOCUMENT FROM ABSOLUTE NAME TO STREAM
  
   Request a document referencd by an absolute URL and sending the data down a stream.
   This is _excactly_ the same as HTLoadAbsolute as the ourputstream is specified using
   the function. HTRequest_setOutputStream(). 'filter' is ignored!  Returns YES if request
   accepted, else NO
   
 */
extern BOOL HTLoadToStream (CONST char * url, BOOL filter, HTRequest *request);
/*

  REQUEST A DOCUMENT FROM RELATIVE NAME
  
   Request a document referenced by a relative URL. The relative URL is made absolute by
   resolving it relative to the address of the 'base' anchor. Returns YES if request
   accepted, else NO
   
 */
extern BOOL HTLoadRelative (CONST char *        relative,
                            HTParentAnchor *    base,
                            HTRequest *         request);
/*

  REQUEST AN ANCHOR
  
   Request the document referenced by the anchor Returns YES if request accepted, else NO
   
 */
extern BOOL HTLoadAnchor (HTAnchor * anchor, HTRequest * request);
/*

  REQUEST AN ANCHOR
  
   Same as HTLoadAnchor but any information in the Error Stack in the request object is
   kept, so that any error messages in one This function is almost identical to
   HTLoadAnchor, but it doesn't clear the error stack so that the information in there is
   kept.  Returns YES if request accepted, else NO
   
 */
extern BOOL HTLoadAnchorRecursive (HTAnchor * anchor, HTRequest * request);
/*

  SEARCH AN ANCHOR
  
   Performs a keyword search on word given by the user. Adds the keyword to the end of the
   current address and attempts to open the new address.  The list of keywords must be a
   space-separated list and spaces will be converted to '+' before the request is issued.
   Search can also be performed by HTLoadAbsolute() etc.  Returns YES if request accepted,
   else NO
   
 */
extern BOOL HTSearch (CONST char *      keywords,
                      HTParentAnchor *  base,
                      HTRequest *       request);
/*

  SEARCH A DOCUMENT FROM ABSOLUTE NAME
  
   Request a document referencd by an absolute URL appended with the keywords given. The
   URL can NOT contain any fragment identifier!  The list of keywords must be a
   space-separated list and spaces will be converted to '+' before the request is issued.
   Returns YES if request accepted, else NO
   
 */
extern BOOL HTSearchAbsolute (CONST char *      keywords,
                              CONST char *      url,
                              HTRequest *       request);
/*

POST Request Methods

  COPY AN ANCHOR
  
   Fetch the URL from either local file store or from a remote HTTP server and send it
   using either PUT or POST to the remote destination using HTTP. The caller can decide
   the exact method used and which HTTP header fields to transmit by setting the user
   fields in the request structure.  If posting to NNTP then we can't dispatch at this
   level but must pass the source anchor to the news module that then takes all the refs
   to NNTP and puts into the "newsgroups" header Returns YES if request accepted, else NO
   
 */
extern BOOL HTCopyAnchor (HTAnchor * src_anchor, HTRequest * main_req);
/*

  UPLOAD AN ANCHOR
  
   This function can be used to send data along with a request to a remote server. It can
   for example be used to POST form data to a remote HTTP server - or it can be used to
   post a newsletter to a NNTP server. In either case, you pass a callback function which
   the request calls when the remote destination is ready to accept data. In this callback
   you get the current request object and a stream into where you can write data. It is
   very important that you return the value returned by this stream to the Library so that
   it knows what to do next. The reason is that the outgoing stream might block or an
   error may occur and in that case the Library must know about it. If you do not want to
   handle the stream interface yourself then you can use the HTUpload_callback which is
   declared below.
   
   The source anchor represents the data object in memory and it points to the destination
   anchor by using the POSTWeb method. The source anchor contains metainformation about
   the data object in memory and the destination anchor represents the reponse from the
   remote server.  Returns YES if request accepted, else NO
   
 */
extern BOOL HTUploadAnchor (HTAnchor *          source_anchor,
                            HTRequest *         request,
                            HTPostCallback *    callback);
/*

  POST CALLBACK HANDLER
  
   Is you do not want to handle the stream interface on your own, you can use this
   "middleman" function which does the actual writing to the target stream for the anchor
   upload and also handles the return value from the stream. Now, your application is
   called via the callback function that you may associate with a request object. You
   indicate when you have sent all the data you want by returning HT_LOADED from the
   callback.
   
 */
PUBLIC int HTUpload_callback (HTRequest * request, HTStream * target);
/*

 */
#endif /* HTACCESS_H */
/*

   End of Declaration */
