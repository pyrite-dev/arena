/*                                                                    Initialization Routines
                                  INITIALIZATION MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module resisters all the plug & play software modules which will be used in the
   program on client side (The server has it's own initialization module). None of the
   functions in this module are called from the Library. They are uniquely a help for the
   application programmer in order to set up the functionality of the Library.
   
   The initialization consists of definiting the following bindings:
   
      Between a source media type and a dest media type (conversion)
      
      Between media types and external viewers/presenters
      
      Between an access method and a protocol module
      
   This module is implemented by HTInit.c, and it is a part of the  W3C Reference Library.
   
   [IMAGE] THE APPLICATION MAY USE THESE FUNCTIONS BUT IT IS NOT REQUIRED
   
   [IMAGE] PLEASE SEE THE HTBInit MODULE FOR STANDARD BINDINGS BETWEEN FILE EXTENSIONS AND
   MEDIA TYPES.
   
 */
#ifndef HTINIT_H
#define HTINIT_H
#include "WWWLib.h"
/*

Media Type Conversions

   The Converters are used to convert a media type to another media type, or to present it
   on screen. This is a part of the stream stack algorithm. The Presenters are also used
   in the stream stack, but are initialized separately.
   
 */
#include "HTML.h"                       /* Uses HTML/HText interface */
#include "HTPlain.h"                    /* Uses HTML/HText interface */

#include "HTTeXGen.h"
#include "HTMLGen.h"
#include "HTMIME.h"
#include "HTBound.h"
#include "HTGuess.h"
#include "HTRules.h"
#include "HTWSRC.h"
#include "HTFWrite.h"
#include "HTNewsLs.h"

extern void HTConverterInit     (HTList * conversions);
/*

Presenters

   The Presenters are used to present a media type to the use by calling an external
   program, for example a post script viewer. This is a part of the stream stack
   algorithm. The Converters are also used in the stream stack, but are initialized
   separately.
   
 */
extern void HTPresenterInit     (HTList * conversions);
/*

Converters and Presenters

   This function is only defined in order to preserve backward compatibility.
   
 */
extern void HTFormatInit        (HTList * conversions);
/*

Protocol Modules

   Set up default bindings between access schemes and the set of protocol modules in the
   Library.
   
 */
#include "HTIcons.h"
#include "WWWHTTP.h"
#include "HTFile.h"
#include "HTFTP.h"
#include "HTGopher.h"
#include "HTTelnet.h"
#include "HTNews.h"

#ifdef HT_DIRECT_WAIS
#include "HTWAIS.h"
#endif

extern void HTAccessInit (void);
/*

File Suffix Setup

   This functions defines a basic set of file suffixes and the corresponding media types.
   
 */
extern void HTFileInit (void);
/*

Register Callbacks For the NET manager

   We register two often used callback functions: a BEFORE and a AFTER callback Not done
   automaticly - may be done by application!
   
 */
#include "HTHome.h"

extern void HTNetInit (void);
/*

Register Callbacks for the ALERT Manager

   We register a set of alert messages Not done automaticly - may be done by application!
   
 */
#include "HTDialog.h"

extern void HTAlertInit (void);
/*

 */
#endif
/*

   End of HTInit Module. */
