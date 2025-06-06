/*                                                      Public Declaration of Request Manager
                          PUBLIC DECLARATION OF REQUEST MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   The request manager consists of two parts: a public part and a private part. The public
   part contains all the information needed to define a request the parameters to be used
   when requesting a resource from the network or local file system. When a request is
   handled, all kinds of things about it need to be passed along together with a request.
   
   This module is implemented by HTReqMan.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTREQ_H
#define HTREQ_H

typedef long HTRequestID;
typedef struct _HTRequest HTRequest;

#include "HTList.h"
#include "HTFormat.h"
#include "HTStream.h"
#include "HTEvntrg.h"
#include "HTError.h"
#include "HTNet.h"
/*

Request a resource

   This is an internal routine, which has an address AND a matching anchor.  (The public
   routines are called with one OR the other.)
   
 */
extern BOOL HTLoad (HTRequest * request, BOOL recursive);
/*

Creation and Deletion Methods

   The request object is intended to live as long as the request is still active, but can
   be deleted as soon as it has terminatedk, for example in one of the request termination
   callback functions as described in the Net Manager. Only the anchor object stays around
   after the request itself is terminated.
   
  CREATE NEW OBJECT
  
   Creates a new request object with a default set of options -- in most cases it will
   need some information added which can be done using the methods in this module, but it
   will work as is for a simple request.
   
 */
extern HTRequest * HTRequest_new (void);
/*

  CLEAR A REQUEST OBJECT
  
   Clears all protocol specific information so that the request object can be used for
   another request. It should be use with care as application specific information is not
   re-initialized. Returns YES if OK, else NO.
   
 */
extern BOOL HTRequest_clear (HTRequest * me);
/*

  CREATE A DUPLICATE
  
   Creates a new HTRequest object as a duplicate of the src request. Returns YES if OK,
   else NO
   
 */
extern HTRequest * HTRequest_dup (HTRequest * src);
/*

    Create a duplicate for Internal use
    
   Creates a new HTRequest object as a duplicate of the src request.  The difference to
   the HTRequest_dup function is that we don't copy the error_stack and other information
   that the application keeps in its copy of the request object. Otherwise it will be
   freed multiple times. Returns YES if OK, else NO
   
 */
extern HTRequest * HTRequest_dupInternal (HTRequest * src);
/*

  DELETE OBJECT
  
   This function deletes the object and cleans up the memory.
   
 */
extern void HTRequest_delete (HTRequest * request);
/*

Bind an Anchor to a Request Object

   Every request object has an anchor associated with it. The anchor normally lives until
   the application terminates but a request object only lives as long as the request is
   being serviced.
   
 */
extern void HTRequest_setAnchor (HTRequest *request, HTAnchor *anchor);
extern HTParentAnchor * HTRequest_anchor (HTRequest *request);
/*

Bind an Access to a request

   This is done if you are a server. In this case you do not need an anchor
   
 */
#if 0
extern void HTRequest_setAccess (HTRequest * request, char * access);
extern CONST char * HTRequest_access (HTRequest * request);
#endif
/*

Set the Method

   The Method is the operation to be executed on the requested object. The default set if
   the set of operations defined by the HTTP protocol, that is "GET", "HEAD", "PUT",
   "POST", "LINK", "UNLINK", and "DELETE" but many of these can be used in other protocols
   as well. The important thing is to think of the requested element as an object on which
   you want to perform an operation. Then it is for the specific protocol implementation
   to try and carry this operation out. However, not all operations can be implemented (or
   make sense) in all protocols.
   
   Methods are handled by the Method Module, and the default value is "GET".
   
 */
extern void HTRequest_setMethod (HTRequest *request, HTMethod method);
extern HTMethod HTRequest_method (HTRequest *request);
/*

Update, Reload, or Refresh a Document

   The Library has two concepts of caching: in memory and on file. When loading a
   document, this flag can be set in order to define who can give a response to the
   request. IMS means that a "If-Modified-Since" Header is used in a HTTP request.
   
 */
typedef enum _HTReload {
    HT_ANY_VERSION      = 0x0,          /* Use any version available */
    HT_MEM_REFRESH      = 0x1,          /* Reload from file cache or network */
    HT_CACHE_REFRESH    = 0x2,          /* Update from network with IMS */
    HT_FORCE_RELOAD     = 0x4           /* Update from network with no-cache */
} HTReload;

extern void HTRequest_setReloadMode (HTRequest *request, HTReload mode);
extern HTReload HTRequest_reloadMode (HTRequest *request);
/*

Redirections

   When a redirection response is returned to the Library, for example from a remote HTTP
   server, this code is passed back to the application. The application can then decide
   whether a new request should be established or not. These two methods return the
   redirection information required to issue a new request, that is the new anchor and any
   list of keywords associated with this anchor.
   
 */
extern HTAnchor * HTRequest_redirection (HTRequest * request);
/*

Max number of Retrys for a Down Load

   Automatic reload can happen in two situations:
   
      The server sends a redirection response
      
      The document has expired
      
   In order to avoid the Library going into an infinite loop, it is necessary to keep
   track of the number of automatic reloads. Loops can occur if the server has a reload to
   the same document or if the server sends back a Expires header which has already
   expired. The default maximum number of automatic reloads is 6.
   
 */
extern BOOL HTRequest_setMaxRetry (int newmax);
extern int  HTRequest_maxRetry (void);
extern BOOL HTRequest_retry (HTRequest *request);
/*

Retry Request After

   Some services, for example HTTP, can in case they are unavailable at the time the
   request is issued send back a time and date stamp to the client telling when they are
   expected to back online. In case a request results in a HT_RETRY status, the
   application can use any time indicated in this field to retry the request at a later
   time. The Library does not initiate any request on its own - it's for the application
   to do. The time returned by this function is in calendar time or -1 if not available.
   
 */
extern time_t HTRequest_retryTime (HTRequest * request);
/*

Accept Headers

   The Accept family of headers is an important part of HTTP handling the format
   negotiation. The Library supports both a global set of accept headers that are used in
   all HTTP requests and a local set of accept headers that are used in specific requests
   only. The global ones are defined in the Format Manager.
   
   Each request can have its local set of accept headers that either are added to the
   global set or replaces the global set of accept headers. Non of the headers have to be
   set. If the global set is sufficient for all requests then this us perfectly fine. If
   the parameter "override" is set then only local accept headers are used, else both
   local and global headers are used.
   
  CONTENT TYPES
  
   The local list of specific conversions which the format manager can do in order to
   fulfill the request.  It typically points to a list set up on initialisation time for
   example by HTInit(). There is also a global list of conversions which contains a
   generic set of possible conversions.
   
 */
extern void HTRequest_setConversion (HTRequest *request, HTList *type, BOOL override);
extern HTList * HTRequest_conversion (HTRequest *request);
/*

  CONTENT ENCODINGS
  
   The list of encodings acceptable in the output stream.
   
 */
extern void HTRequest_setEncoding (HTRequest *request, HTList *enc, BOOL override);
extern HTList * HTRequest_encoding (HTRequest *request);
/*

  CONTENT-LANGUAGES
  
   The list of (human) language values acceptable in the response. The default is all
   languages.
   
 */
extern void HTRequest_setLanguage (HTRequest *request, HTList *lang, BOOL override);
extern HTList * HTRequest_language (HTRequest *request);
/*

  CHARSET
  
   The list of charsets accepted by the application
   
 */
extern void HTRequest_setCharset (HTRequest *request, HTList *charset, BOOL override);
extern HTList * HTRequest_charset (HTRequest *request);
/*

Handling Metainformation (RFC822 Headers)

   The Library supports a large set of headers that can be sent along with a request (or a
   response for that matter). All headers can be either disabled or enabled using bit
   flags that are defined in the following.
   
  GENERAL HTTP HEADER MASK
  
   There are a few header fields which have general applicability for both request and
   response mesages, but which do not apply to the communication parties or theentity
   being transferred. This mask enables and disables these headers. If the bit is not
   turned on they are not sent. All headers are optional and the default value is NO
   GENERAL HEADERS
   
 */
typedef enum _HTGnHd {
    HT_G_DATE           = 0x1,
    HT_G_FORWARDED      = 0x2,
    HT_G_MESSAGE_ID     = 0x4,
    HT_G_MIME           = 0x8,
    HT_G_CONNECTION     = 0x10,
    HT_G_NO_CACHE       = 0x20                                     /* Pragma */
} HTGnHd;

#define DEFAULT_GENERAL_HEADERS         HT_G_CONNECTION

extern void HTRequest_setGnHd (HTRequest *request, HTGnHd gnhd);
extern void HTRequest_addGnHd (HTRequest *request, HTGnHd gnhd);
extern HTGnHd HTRequest_gnHd (HTRequest *request);
/*

  REQUEST HEADERS
  
   The request header fields allow the client to pass additional information about the
   request (and about the client itself) to the server. All headers are optional but the
   default value is all request headers if present except From and Pragma.
   
 */
typedef enum _HTRqHd {
    HT_C_ACCEPT_TYPE    = 0x1,
    HT_C_ACCEPT_CHAR    = 0x2,
    HT_C_ACCEPT_ENC     = 0x4,
    HT_C_ACCEPT_LAN     = 0x8,
    HT_C_FROM           = 0x10,
    HT_C_IMS            = 0x20,
    HT_C_HOST           = 0x40,
    HT_C_REFERER        = 0x80,
    HT_C_USER_AGENT     = 0x200
} HTRqHd;

#define DEFAULT_REQUEST_HEADERS \
        HT_C_ACCEPT_TYPE+HT_C_ACCEPT_CHAR+ \
        HT_C_ACCEPT_ENC+HT_C_ACCEPT_LAN+HT_C_REFERER+HT_C_USER_AGENT

extern void HTRequest_setRqHd (HTRequest *request, HTRqHd rqhd);
extern void HTRequest_addRqHd (HTRequest *request, HTRqHd rqhd);
extern HTRqHd HTRequest_rqHd (HTRequest *request);
/*

  RESPONSE HEADERS
  
   The response header fields allow the server to pass additional information about the
   response (and about the server itself) to the client. All headers are optional.
   
 */
typedef enum _HTRsHd {
    HT_S_LOCATION       = 0x1,
    HT_S_PROXY_AUTH     = 0x2,
    HT_S_PUBLIC         = 0x4,
    HT_S_RETRY_AFTER    = 0x8,
    HT_S_SERVER         = 0x10,
    HT_S_WWW_AUTH       = 0x20
} HTRsHd;

#define DEFAULT_RESPONSE_HEADERS HT_S_SERVER

extern void HTRequest_setRsHd (HTRequest * request, HTRsHd rshd);
extern void HTRequest_addRsHd (HTRequest * request, HTRsHd rshd);
extern HTRsHd HTRequest_rsHd (HTRequest * request);
/*

  ENTITY HEADER MASK
  
   The entity headers contain information about the object sent in the HTTP transaction.
   See the Anchor module, for the storage of entity headers. This flag defines which
   headers are to be sent in a request together with an entity body. All headers are
   optional but the default value is ALL ENTITY HEADERS IF PRESENT
   
 */
typedef enum _HTEnHd {
    HT_E_ALLOW          = 0x1,
    HT_E_CONTENT_ENCODING = 0x2,
    HT_E_CONTENT_LANGUAGE = 0x4,
    HT_E_CONTENT_LENGTH = 0x8,
    HT_E_CTE            = 0x10,                 /* Content-Transfer-Encoding */
    HT_E_CONTENT_TYPE   = 0x20,
    HT_E_DERIVED_FROM   = 0x40,
    HT_E_EXPIRES        = 0x80,
    HT_E_LAST_MODIFIED  = 0x200,
    HT_E_LINK           = 0x400,
    HT_E_TITLE          = 0x800,
    HT_E_URI            = 0x1000,
    HT_E_VERSION        = 0x2000
} HTEnHd;

#define DEFAULT_ENTITY_HEADERS          0xFFFF                        /* all */

extern void HTRequest_setEnHd (HTRequest *request, HTEnHd enhd);
extern void HTRequest_addEnHd (HTRequest *request, HTEnHd enhd);
extern HTEnHd HTRequest_enHd (HTRequest *request);
/*

  REFERER FIELD
  
   If this parameter is set then a `Referer: <parent address> can be generated in the
   request to the server, see Referer field in a HTTP Request
   
 */
extern void HTRequest_setParent (HTRequest *request, HTParentAnchor *parent);
extern HTParentAnchor * HTRequest_parent (HTRequest *request);
/*

  SENDING DATA TO THE NETWORK
  
   The Library supports two ways of posting a data object to a remote destination: Input
   comes from a socket descriptor or from memory. In the case where you want to copy a
   URL, for example from local file system or from a remote HTTP server then you must use
   the POSTWeb design. This model operates by using at least two request objects which
   gets linked to eachother as part of the POSTWeb model. However, if you are posting from
   memory, we only use one request object to perform the operation. In order to do this,
   the application must register a callback function that can be called when the HTTP
   client module is ready for accepting data. be included as part of the body and/or as
   extra metainformation. In the latter case you need to register a callback function of
   the following type using the methods provided in the next section.
   
 */
typedef int HTPostCallback (HTRequest * request, HTStream * target);
/*

  EXTRA HEADERS
  
   Extra header information can be send along with a request using this variable. The text
   is sent as is so it must be preformatted with <CRLF> line terminators. This will get
   changed at some point so that you can register a header together with a handler in the
   MIME parser.
   
 */
extern void HTRequest_setGenerator (HTRequest *request, HTList *gens, BOOL override);
extern HTList * HTRequest_generator (HTRequest *request, BOOL *override);

extern void HTRequest_setParser (HTRequest *request, HTList *pars, BOOL override);
extern HTList * HTRequest_parser (HTRequest *request, BOOL *override);
/*

Streams From Network to Application

  DEFAULT OUTPUT STREAM
  
   The output stream is to be used to put data down to as they come in from the network
   and back to the application. The default value is NULL which means that the stream goes
   to the user (display).
   
 */
extern void HTRequest_setOutputStream (HTRequest *request, HTStream *output);
extern HTStream *HTRequest_outputStream (HTRequest *request);
/*

   The desired format of the output stream. This can be used to get unconverted data etc.
   from the library. If NULL, then WWW_PRESENT is default value.
   
 */
extern void HTRequest_setOutputFormat (HTRequest *request, HTFormat format);
extern HTFormat HTRequest_outputFormat (HTRequest *request);
/*

  DEBUG STREAM
  
   All object bodies sent from the server with status codes different from 200 OK will be
   put down this stream. This can be used for redirecting body information in status codes
   different from "200 OK" to for example a debug window. If the value is NULL (default)
   then the stream is not set up.
   
 */
extern void HTRequest_setDebugStream (HTRequest *request, HTStream *debug);
extern HTStream *HTRequest_debugStream (HTRequest *request);
/*

   The desired format of the error stream. This can be used to get unconverted data etc.
   from the library. The default value if WWW_HTML as a character based only has one
   WWW_PRESENT.
   
 */
extern void HTRequest_setDebugFormat (HTRequest *request, HTFormat format);
extern HTFormat HTRequest_debugFormat (HTRequest *request);
/*

Context Swapping

   In multi threaded applications it is often required to keep track of the context of a
   request so that when the Library returns a result of a request, it can be put into the
   context it was in before the request was first passed to the Library. This call back
   function allows the application to do this.
   
 */
typedef int HTRequestCallback (HTRequest * request, void *param);

extern void HTRequest_setCallback (HTRequest *request, HTRequestCallback *cb);
extern HTRequestCallback *HTRequest_callback (HTRequest *request);
/*

   The callback function can be passed an arbitrary pointer (the void part) which can
   describe the context of the current request structure. If such context information is
   required then it can be set using the following methods:
   
 */
extern void HTRequest_setContext (HTRequest *request, void *context);
extern void *HTRequest_context (HTRequest *request);
/*

Using a proxy server

   As a HTTP request looks different when it is directed to a proxy server than to a
   origin server, we need to know whether we are using a proxy for this particular request
   or not. These two methods can be used to set and check the current state whether we are
   going to a proxy or not.
   
 */
extern void HTRequest_setProxying (HTRequest * request, BOOL proxying);
extern BOOL HTRequest_proxying (HTRequest * request);
/*

Preemptive or Non-preemptive Access

   A access scheme is defined with a default for using either preemptive (blocking I/O) or
   non-premitve (non-blocking I/O). This is basically a result of the implementation of
   the protocol module itself. However, if non-blocking I/O is the default then some times
   it is nice to be able to set the mode to blocking instead. For example when loading the
   first document (the home page) then blocking can be used instead of non-blocking.
   
 */
extern void HTRequest_setPreemptive (HTRequest *request, BOOL mode);
extern BOOL HTRequest_preemptive (HTRequest *request);
/*

Priority Management

   The request can be assigned an initial priority which then gets inherited by all HTNet
   objects and other requests objects created as a result of this one. You can also assign
   a separate priority to an indicidual HTNet object by using the methods in the Net
   manager.
   
 */
extern HTPriority HTRequest_priority (HTRequest * request);
extern BOOL HTRequest_setPriority (HTRequest * request, HTPriority priority);
/*

Get an set the HTNet Object

   If a request is actually going on the net then the Net MAnager is contacted to handle
   the request. The Net manager creates a HTNEt object and links it to the Request object.
   You can get to the HTNet object using the following functions.
   
 */
extern HTNet * HTRequest_net (HTRequest * request);
extern BOOL HTRequest_setNet (HTRequest * request, HTNet * net);
/*

Format Negotiation

   When accessing the local file system, the Library is capable of performing content
   negotioation as described by the HTTP protocol. This is mainly for server applications,
   but some client applications might also want to use content negotiation when accessing
   the local file system. This method enables or disables content negotiation - the
   default value is ON.
   
 */
extern void HTRequest_setNegotiation (HTRequest *request, BOOL mode);
extern BOOL HTRequest_negotiation (HTRequest *request);
/*

Error Manager

   Errors are like almost anything kept in lists and a error list can be associated with a
   request using the following functions. In order to make life easier, there are also
   some easy mapping functions to the real HTError module, so that you can add an error
   directly to a request object.
   
 */
extern HTList * HTRequest_error (HTRequest * request);
extern void HTRequest_setError  (HTRequest * request, HTList * list);
/*

   These are the cover functions that go directly to the Error manager
   
 */
extern BOOL HTRequest_addError (HTRequest *     request,
                                HTSeverity      severity,
                                BOOL            ignore,
                                int             element,
                                void *          par,
                                unsigned int    length,
                                char *          where);

extern BOOL HTRequest_addSystemError (HTRequest *       request,
                                      HTSeverity        severity,
                                      int               errornumber,
                                      BOOL              ignore,
                                      char *            syscall);
/*

Bytes Read or Written in a Request

   This function returns the bytes read in the current request. For a deeper description
   of what the current request is, please read the user's guide. This function can be used
   in for example the HTAlert module to give the number of bytes read or written in a
   progress message.
   
 */
extern long HTRequest_bytesRead (HTRequest * request);
extern long HTRequest_bytesWritten (HTRequest * request);
/*

Kill a Request

   This function kills this particular request, see HTNet module for a function that kills
   them all.
   
 */
extern BOOL HTRequest_kill(HTRequest * request);
/*

 */
#endif /* HTREQ_H */
/*

   End of declaration */
