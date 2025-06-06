/*                                                                              Error Manager
                       REPORTING ERRORS AND MESSAGES TO THE CLIENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the implementaion of an error message reporting system that reports errors
   occured either in a stream module (structured streams inclusive) or in a protocol
   module. A list of errors are put into the a list which can be bound to a request object
   or to a stream.
   
   This module is implemented by HTError.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTERROR_H
#define HTERROR_H
#include "HTReq.h"
/*

Error Codes and Messages

   Note:  All non-HTTP error codes have index numbers > HTERR_HTTP_CODES, and they will
   not be shown in the error-message generated.
   
 */
typedef enum _HTErrorElement {
        HTERR_OK = 0,                                           /* 200 */
        HTERR_CREATED,                                          /* 201 */
        HTERR_ACCEPTED,                                         /* 202 */
        HTERR_PARTIAL,                                          /* 203 */
        HTERR_NO_CONTENT,                                       /* 204 */
        HTERR_MOVED,                                            /* 301 */
        HTERR_FOUND,                                            /* 302 */
        HTERR_METHOD,                                           /* 303 */
        HTERR_NOT_MODIFIED,                                     /* 304 */
        HTERR_BAD_REQUEST,                                      /* 400 */
        HTERR_UNAUTHORIZED,                                     /* 401 */
        HTERR_PAYMENT_REQUIRED,                                 /* 402 */
        HTERR_FORBIDDEN,                                        /* 403 */
        HTERR_NOT_FOUND,                                        /* 404 */
        HTERR_NOT_ALLOWED,                                      /* 405 */
        HTERR_NONE_ACCEPTABLE,                                  /* 406 */
        HTERR_PROXY,                                            /* 407 */
        HTERR_TIMEOUT,                                          /* 408 */
        HTERR_INTERNAL,                                         /* 500 */
        HTERR_NOT_IMPLEMENTED,                                  /* 501 */
        HTERR_BAD_GATE,                                         /* 502 */
        HTERR_DOWN,                                             /* 503 */
        HTERR_GATE_TIMEOUT,                                     /* 504 */
        HTERR_HTTP_CODES_END,    /* Put all non-HTTP status codes after this */
        HTERR_NO_REMOTE_HOST,
        HTERR_NO_HOST,
        HTERR_NO_FILE,
        HTERR_FTP_SERVER,
        HTERR_FTP_NO_RESPONSE,
        HTERR_TIME_OUT,
        HTERR_GOPHER_SERVER,
        HTERR_INTERRUPTED,
        HTERR_CON_INTR,
        HTERR_CSO_SERVER,
        HTERR_HTTP09,
        HTERR_BAD_REPLY,
        HTERR_UNKNOWN_AA,
        HTERR_NEWS_SERVER,
        HTERR_FILE_TO_FTP,
        HTERR_MAX_REDIRECT,
        HTERR_EOF,
        HTERR_WAIS_OVERFLOW,
        HTERR_WAIS_MODULE,
        HTERR_WAIS_NO_CONNECT,
        HTERR_SYSTEM,
        HTERR_CLASS,
        HTERR_ACCESS,
        HTERR_LOGIN,
        HTERR_ELEMENTS                      /* This MUST be the last element */
} HTErrorElement;
/*

Creation and Deletion Methods

  ADD AN ERROR
  
   Add an error message to the error list. `par' and `where' might be set to NULL. If par
   is a string, it is sufficient to let length be unspecified, i.e., 0. If only a part of
   the string is wanted then specify a length inferior to strlen((char *) par).  The
   string is '\0' terminated automaticly.  See also HTError_addSystem for system errors.
   Returns YES if OK, else NO.
   
 */
typedef struct _HTError HTError;

typedef enum _HTSeverity {
    ERR_FATAL             = 0x1,
    ERR_NON_FATAL         = 0x2,
    ERR_WARN              = 0x4,
    ERR_INFO              = 0x8
} HTSeverity;

extern BOOL HTError_add (HTList *       list,
                         HTSeverity     severity,
                         BOOL           ignore,
                         int            element,
                         void *         par,
                         unsigned int   length,
                         char *         where);

/*

  ADD A SYSTEM ERROR
  
   Add a system error message to the error list. syscall is the name of the system call,
   e.g. "close". The message put to the list is that corresponds to the error number
   passed. See also HTError_add. Returns YES if OK, else NO.
   
 */
extern BOOL HTError_addSystem (HTList *         list,
                               HTSeverity       severity,
                               int              errornumber,
                               BOOL             ignore,
                               char *           syscall);
/*

  DELETE AN ENTIRE ERROR STACK
  
   Deletes all errors in a list.
   
 */
extern BOOL HTError_deleteAll (HTList * list);
/*

  DELETES THE LAST EDDED ENTRY
  
   Deletes the last error entry added to the list. Return YES if OK, else NO
   
 */
extern BOOL HTError_deleteLast (HTList * list);
/*

How Should Errors be Presented

   This variable dictates which errors should be put out when generating the message to
   the user. The first four enumerations make it possible to see `everything as bad or
   worse than' this level, e.g. HT_ERR_SHOW_NON_FATAL shows messages of type
   HT_ERR_SHOW_NON_FATAL and HT_ERR_SHOW_FATAL.
   
   Note: The default value is made so that it only puts a message to stderr if a `real'
   error has occurred. If a separate widget is available for information and error
   messages then probably HT_ERR_SHOW_DETAILED would be more appropriate.
   
 */
typedef enum _HTErrorShow {
    HT_ERR_SHOW_FATAL     = 0x1,
    HT_ERR_SHOW_NON_FATAL = 0x3,
    HT_ERR_SHOW_WARNING   = 0x7,
    HT_ERR_SHOW_INFO      = 0xF,
    HT_ERR_SHOW_PARS      = 0x10,
    HT_ERR_SHOW_LOCATION  = 0x20,
    HT_ERR_SHOW_IGNORE    = 0x40,
    HT_ERR_SHOW_FIRST     = 0x80,
    HT_ERR_SHOW_LINKS     = 0x100,
    HT_ERR_SHOW_DEFAULT   = 0x13,
    HT_ERR_SHOW_DETAILED  = 0x1F,
    HT_ERR_SHOW_DEBUG     = 0x7F
} HTErrorShow;

extern HTErrorShow HTError_show (void);
extern BOOL HTError_setShow (HTErrorShow mask);
/*

  SHOW THE ERROR ENTRY?
  
   Should we show this entry in the list or just continue to the next one?
   
 */
extern BOOL HTError_doShow (HTError * info);
/*

Ignore last Added Error

   Turns on the `ignore' flag for the most recent error entered the error list. Returns
   YES if OK else NO
   
 */
extern BOOL HTError_ignoreLast  (HTList * list);
extern BOOL HTError_setIgnore   (HTError * info);
/*

Methods for Accessing the Error Object

   The definition of the error code and the error message is not part of the core Libray.
   Only the definition of the Error object itself is. This object has an index number
   defined by the HTErrorElement above. The mapping from this index to an erro code and a
   message is for the application to do. The Library provides a default implementation in
   the HTDialog module. These are the methods for accessing the error object:
   
 */
extern int HTError_index                (HTError * info);
extern HTSeverity HTError_severity      (HTError * info);
extern void * HTError_parameter         (HTError * info, int * length);
extern CONST char * HTError_location    (HTError * info);
/*

 */
#endif
/*

   End of Declaration Manager */
