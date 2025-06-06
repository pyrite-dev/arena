/*                                                                             Request Object
                                      REQUEST OBJECT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is the private part of the request object. It has the functions
   declarations that are private to the Library and that shouldn't be used by
   applications. The module has been separated from the old HTAccess module. See also the
   public part of the declarition in the HTReq Module.
   
   This module is implemented by HTReqMan.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTREQMAN_H
#define HTREQMAN_H

#include "HTReq.h"
#include "HTList.h"
#include "HTFormat.h"
#include "HTAnchor.h"
#include "HTMethod.h"
#include "HTAABrow.h"
#include "HTStream.h"
#include "HTSocket.h"
#include "HTNet.h"
/*

The Request structure

   When a request is handled, all kinds of things about it need to be passed along
   together with a request. It is intended to live as long as the request is still active,
   but can be deleted as soon as it has terminated. Only the anchor object stays around
   after the request itself is terminated.
   
 */
struct _HTRequest {

    BOOL                internal;      /* Does the app knows about this one? */

    HTMethod            method;
    HTReload            reload;

    char *              boundary;                 /* MIME multipart boundary */
    int                 retrys;               /* Number of automatic reloads */
    time_t              retry_after;             /* Absolut time for a retry */
    HTNet *             net;                /* Information about socket etc. */
    HTPriority          priority;               /* Priority for this request */
/*

  ACCEPT HEADERS
  
 */
    HTList *            conversions;
    BOOL                conv_local;

    HTList *            encodings;
    BOOL                enc_local;

    HTList *            languages;
    BOOL                lang_local;

    HTList *            charsets;
    BOOL                char_local;
/*

  HEADERS AND HEADER INFORMATION
  
 */
    HTGnHd              GenMask;
    HTRsHd              ResponseMask;
    HTRqHd              RequestMask;
    HTEnHd              EntityMask;

    HTList *            parsers;
    BOOL                pars_local;

    HTList *            generators;
    BOOL                gens_local;
/*

  ANCHORS
  
 */
    HTParentAnchor *    anchor;        /* The Client anchor for this request */

    HTChildAnchor *     childAnchor;        /* For element within the object */
    HTParentAnchor *    parentAnchor;                   /* For referer field */
/*

    Redirection
    
   If we get a redirection back then we return the new destination for this request to the
   application using this anchor.
   
 */
    HTAnchor *          redirectionAnchor;                /* Redirection URL */
/*

  STREAMS FROM NETWORK TO APPLICATION
  
 */
    HTStream *          output_stream;
    HTFormat            output_format;

    HTStream*           debug_stream;
    HTFormat            debug_format;
/*

  STREAMS FROM APPLICATION TO NETWORK
  
 */
    HTStream *          input_stream;
    HTFormat            input_format;
/*

  CALLBACK FUNCTION FOR GETTING DATA DOWN THE INPUT STREAM
  
 */
    HTPostCallback *    PostCallback;
/*

  CONTEXT SWAPPING
  
 */
    HTRequestCallback * callback;
    void *              context;
/*

  OTHER FLAGS
  
 */
    BOOL                preemptive;
    BOOL                ContentNegotiation;
    BOOL                using_proxy;
/*

  ERROR MANAGER
  
 */
    HTList *            error_stack;                       /* List of errors */
/*

  POSTWEB INFORMATION
  
 */
    HTRequest *         source;              /* Source for request or itself */
    HTParentAnchor *    source_anchor;            /* Source anchor or itself */

    HTRequest *         mainDestination;             /* For the typical case */
    HTList *            destinations;            /* List of related requests */
    int                 destRequests;      /* Number of destination requests */
    int                 destStreams;        /* Number of destination streams */
/*

  ACCESS AUTHENTICATION INFORMATION
  
   This will go into its own structure
   
 */
    char *      WWWAAScheme;            /* WWW-Authenticate scheme */
    char *      WWWAARealm;             /* WWW-Authenticate realm */
    char *      WWWprotection;          /* WWW-Protection-Template */
    char *      authorization;          /* Authorization: field */
    HTAAScheme  scheme;                 /* Authentication scheme used */
    HTList *    valid_schemes;          /* Valid auth.schemes             */
    HTAssocList **      scheme_specifics;/* Scheme-specific parameters    */
    char *      authenticate;           /* WWW-authenticate: field */
    char *      prot_template;          /* WWW-Protection-Template: field */
    HTAASetup * setup;                  /* Doc protection info            */
    HTAARealm * realm;                  /* Password realm                 */
    char *      dialog_msg;             /* Authentication prompt (client) */
/*

  WINDOWS SPECIFIC INFORMATION
  
 */
#ifdef WWW_WIN_ASYNC
    HWND                hwnd;           /* Windows handle for MSWindows   */
    unsigned long       winMsg;         /* msg number of Windows eloop    */
#endif /* WWW_WIN_ASYNC */
/*

 */
};
/*

Post Web Management

   These functions are mainly used internally in the Library but there is no reason for
   them not to be public.
   
 */
extern BOOL HTRequest_addDestination (HTRequest * src, HTRequest * dest);
extern BOOL HTRequest_removeDestination (HTRequest * dest);
extern BOOL HTRequest_destinationsReady (HTRequest * me);

extern BOOL HTRequest_linkDestination (HTRequest * dest);
extern BOOL HTRequest_unlinkDestination (HTRequest * dest);

extern BOOL HTRequest_removePostWeb (HTRequest * me);
extern BOOL HTRequest_killPostWeb (HTRequest * me);

#define HTRequest_mainDestination(me) \
        ((me) && (me)->source ? (me)->source->mainDestination : NULL)
#define HTRequest_isDestination(me) \
        ((me) && (me)->source && (me) != (me)->source)
#define HTRequest_isMainDestination(me) \
        ((me) && (me)->source && \
        (me) == (me)->source->mainDestination)
#define HTRequest_isSource(me) \
        ((me) && (me)->source && (me) == (me)->source)
#define HTRequest_isPostWeb(me) ((me) && (me)->source)
#define HTRequest_source(me) ((me) ? (me)->source : NULL)
/*

   End of Declaration
   
 */
#endif /* HTREQMAN_H */
/*

   end of HTAccess */
