/*                                                                 Format Negotiation Manager
                                    THE FORMAT MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   Here we describe the functions of the HTFormat module which handles conversion between
   different data representations. (In MIME parlance, a representation is known as a
   content-type. In WWW the term format is often used as it is shorter). The content of
   this module is:
   
      Converters
      
      Generic preferences (media type, language, charset etc.)
      
      Global Preferences
      
      Content Negotiation
      
      The Stream Stack
      
   This module is implemented by HTFormat.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTFORMAT_H
#define HTFORMAT_H

#include "HTUtils.h"
#include "HTStream.h"
#include "HTAtom.h"
#include "HTList.h"
#include "HTAnchor.h"
#include "HTReq.h"
/*

Stream Converters

   A converter is a stream with a special set of parameters and which is registered as
   capable of converting from a MIME type to something else (maybe another MIME-type). A
   converter is defined to be a function returning a stream and accepting the following
   parameters. The content type elements are atoms for which we have defined a prototype.
   
 */
typedef HTStream * HTConverter  (HTRequest *    request,
                                 void *         param,
                                 HTFormat       input_format,
                                 HTFormat       output_format,
                                 HTStream *     output_stream);
/*

Generic Preferences

   The Library contains functionality for letting the application (or user) express the
   preferences for the rendition of a given data object when issuing a request. The
   categories supported are:
   
      Content type (media type)
      
      Encoding
      
      Language
      
      Charset
      
  REGISTRATION OF ACCEPTED CONTENT TYPES
  
   A presenter is a module (possibly an external program) which can present a graphic
   object of a certain MIME type to the user. That is, presenters are normally used to
   present objects that the converters are not able to handle. Data is transferred to the
   external program using for example the HTSaveAndExecute stream which writes to a local
   file. Both presenters and converters are of the type HTConverter.
   
 */
typedef struct _HTPresentation {
    HTFormat    rep;                         /* representation name atomized */
    HTFormat    rep_out;                         /* resulting representation */
    HTConverter *converter;           /* The routine to gen the stream stack */
    char *      command;                               /* MIME-format string */
    char *      test_command;                          /* MIME-format string */
    double      quality;                     /* Between 0 (bad) and 1 (good) */
    double      secs;
    double      secs_per_byte;
} HTPresentation;
/*

    Predefined Content Types
    
   These macros (which used to be constants) define some basic internally referenced
   representations. The www/xxx ones are of course not MIME standard. They are internal
   representations used in the Library but they can't be exported to other apps!
   
 */
#define WWW_RAW         HTAtom_for("www/void")   /* Raw output from Protocol */
/*

   WWW_RAW is an output format which leaves the input untouched exactly as it is received
   by the protocol module. For example, in the case of FTP, this format returns raw ASCII
   objects for directory listings; for HTTP, everything including the header is returned,
   for Gopher, a raw ASCII object is returned for a menu etc.
   
 */
#define WWW_SOURCE      HTAtom_for("*/*")   /* Almost what it was originally */
/*

   WWW_SOURCE is an output format which leaves the input untouched exactly as it is
   received by the protocol module IF not a suitable converter has been registered with a
   quality factor higher than 1 (for example 2). In this case the SUPER CONVERTER is
   preferred for the raw output. This can be used as a filter effect that allows
   conversion from, for example raw FTPdirectory listings into HTML but passes a MIME body
   untouched.
   
 */
#define WWW_PRESENT     HTAtom_for("www/present")   /* The user's perception */
/*

   WWW_PRESENT represents the user's perception of the document.  If you convert to
   WWW_PRESENT, you present the material to the user.
   
 */
#define WWW_DEBUG       HTAtom_for("www/debug")
/*

   WWW_DEBUG represents the user's perception of debug information, for example sent as a
   HTML document in a HTTP redirection message.
   
 */
#define WWW_UNKNOWN     HTAtom_for("www/unknown")
/*

   WWW_UNKNOWN is a really unknown type. It differs from the real MIME type
   "application/octet-stream" in that we haven't even tried to figure out the content type
   at this point.
   
   These are regular MIME types defined. Others can be added!
   
 */
#define WWW_HTML        HTAtom_for("text/html")
#define WWW_PLAINTEXT   HTAtom_for("text/plain")

#define WWW_MIME        HTAtom_for("message/rfc822")
#define WWW_MIME_HEAD   HTAtom_for("message/x-rfc822-head")

#define WWW_AUDIO       HTAtom_for("audio/basic")

#define WWW_VIDEO       HTAtom_for("video/mpeg")

#define WWW_GIF         HTAtom_for("image/gif")
#define WWW_PNG         HTAtom_for("image/png")

#define WWW_BINARY      HTAtom_for("application/octet-stream")
#define WWW_POSTSCRIPT  HTAtom_for("application/postscript")
#define WWW_RICHTEXT    HTAtom_for("application/rtf")
/*

   We also have some MIME types that come from the various protocols when we convert from
   ASCII to HTML.
   
 */
#define WWW_GOPHER_MENU HTAtom_for("text/x-gopher")
#define WWW_CSO_SEARCH  HTAtom_for("text/x-cso")

#define WWW_FTP_LNST    HTAtom_for("text/x-ftp-lnst")
#define WWW_FTP_LIST    HTAtom_for("text/x-ftp-list")

#define WWW_NNTP_LIST   HTAtom_for("text/x-nntp-list")
#define WWW_NNTP_OVER   HTAtom_for("text/x-nntp-over")
#define WWW_NNTP_HEAD   HTAtom_for("text/x-nntp-head")

#define WWW_HTTP        HTAtom_for("text/x-http")
/*

   Finally we have defined a special format for our RULE files as they can be handled by a
   special converter.
   
 */
#define WWW_RULES       HTAtom_for("application/x-www-rules")
/*

    Add a Presenter
    
   This function creates a presenter object and adds to the list of conversions.
   
  conversions            The list of conveters and presenters
                         
  rep_in                 the MIME-style format name
                         
  rep_out                is the resulting content-type after the conversion
                         
  converter              is the routine to call which actually does the conversion
                         
  quality                A degradation faction [0..1]
                         
  maxbytes               A limit on the length acceptable as input (0 infinite)
                         
  maxsecs                A limit on the time user will wait (0 for infinity)
                         
 */
extern void HTPresentation_add (HTList *        conversions,
                                CONST char *    representation,
                                CONST char *    command,
                                CONST char *    test_command,
                                double          quality,
                                double          secs,
                                double          secs_per_byte);
/*

    Delete a list of Presenters
    
 */
extern void HTPresentation_deleteAll    (HTList * list);
/*

    Add a Converter
    
   This function creates a presenter object and adds to the list of conversions.
   
  conversions            The list of conveters and presenters
                         
  rep_in                 the MIME-style format name
                         
  rep_out                is the resulting content-type after the conversion
                         
  converter              is the routine to call which actually does the conversion
                         
  quality                A degradation faction [0..1]
                         
  maxbytes               A limit on the length acceptable as input (0 infinite)
                         
  maxsecs                A limit on the time user will wait (0 for infinity)
                         
 */
extern void HTConversion_add   (HTList *        conversions,
                                CONST char *    rep_in,
                                CONST char *    rep_out,
                                HTConverter *   converter,
                                double          quality,
                                double          secs,
                                double          secs_per_byte);
/*

    Delete a list of Converters
    
 */
extern void HTConversion_deleteAll      (HTList * list);
/*

  REGISTRATION OF ACCEPTED CONTENT ENCODINGS
  
   Encodings are the HTTP extension of transfer encodings. Encodings include compress,
   gzip etc.
   
 */
typedef struct _HTAcceptNode {
    HTAtom *    atom;
    double      quality;
} HTAcceptNode;
/*

    Predefined Encoding Types
    
 */
#define WWW_ENC_7BIT            HTAtom_for("7bit")
#define WWW_ENC_8BIT            HTAtom_for("8bit")
#define WWW_ENC_BINARY          HTAtom_for("binary")
#define WWW_ENC_BASE64          HTAtom_for("base64")
#define WWW_ENC_COMPRESS        HTAtom_for("compress")
#define WWW_ENC_GZIP            HTAtom_for("gzip")
/*

    Register an Encoding
    
 */
extern void HTEncoding_add (HTList *            list,
                            CONST char *        enc,
                            double              quality);
/*

    Delete a list of Encoders
    
 */
extern void HTEncoding_deleteAll (HTList * list);
/*

  ACCEPTED CHARSETS
  
    Register a Charset
    
 */
extern void HTCharset_add (HTList *             list,
                           CONST char *         charset,
                           double               quality);
/*

    Delete a list of Charsets
    
 */
extern void HTCharset_deleteAll (HTList * list);
/*

  ACCEPTED CONTENT LANGUAGES
  
    Register a Language
    
 */
extern void HTLanguage_add (HTList *            list,
                            CONST char *        lang,
                            double              quality);
/*

    Delete a list of Languages
    
 */
extern void HTLanguage_deleteAll (HTList * list);
/*

Global Registrations

   There are two places where these preferences can be registered: in a global list valid
   for all requests and a local list valid for a particular request only. These are valid
   for all requests. See the Request Manager fro local sets.
   
  CONVERTERS AND PRESENTERS
  
   The global list of specific conversions which the format manager can do in order to
   fulfill the request.  There is also a local list of conversions which contains a
   generic set of possible conversions.
   
 */
extern void HTFormat_setConversion      (HTList *list);
extern HTList * HTFormat_conversion     (void);
/*

  CONTENT ENCODINGS
  
 */
extern void HTFormat_setEncoding        (HTList *list);
extern HTList * HTFormat_encoding       (void);
/*

  CONTENT LANGUAGES
  
 */
extern void HTFormat_setLanguage        (HTList *list);
extern HTList * HTFormat_language       (void);
/*

  CONTENT CHARSETS
  
 */
extern void HTFormat_setCharset         (HTList *list);
extern HTList * HTFormat_charset        (void);
/*

  DELETE ALL GLOBAL LISTS
  
   This is a convenience function that might make life easier.
   
 */
extern void HTFormat_deleteAll (void);
/*

Ranking of Accepted Formats

   This function is used when the best match among several possible documents is to be
   found as a function of the accept headers sent in the client request.
   
 */
typedef struct _HTContentDescription {
    char *      filename;
    HTAtom *    content_type;
    HTAtom *    content_language;
    HTAtom *    content_encoding;
    int         content_length;
    double      quality;
} HTContentDescription;

extern BOOL HTRank (HTList * possibilities,
                    HTList * accepted_content_types,
                    HTList * accepted_content_languages,
                    HTList * accepted_content_encodings);
/*

The Stream Stack

   This is the routine which actually sets up the conversion. It currently checks only for
   direct conversions, but multi-stage conversions are forseen.  It takes a stream into
   which the output should be sent in the final format, builds the conversion stack, and
   returns a stream into which the data in the input format should be fed. If guess is
   true and input format is www/unknown, try to guess the format by looking at the first
   few bytes of the stream.
   
 */
extern HTStream * HTStreamStack (HTFormat       rep_in,
                                 HTFormat       rep_out,
                                 HTStream *     output_stream,
                                 HTRequest *    request,
                                 BOOL           guess);
/*

Cost of a Stream Stack

   Must return the cost of the same stack which HTStreamStack would set up.
   
 */
extern double HTStackValue      (HTList *       conversions,
                                 HTFormat       format_in,
                                 HTFormat       format_out,
                                 double         initial_value,
                                 long int       length);

#endif /* HTFORMAT */
/*

   End of declaration module  */
