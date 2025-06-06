/*                                                                       Extra Header Manager
                                   EXTRA HEADER MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module handles lists of callback functions for generating and parsing protocol
   headers. This works exactly like the lists in HTFormat.
   
   This module is implemented by HTheader.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTHEADER_H
#define HTHEADER_H

#include "HTReq.h"
#include "HTStream.h"
/*

   We have two call back functions: the first is for generating headers. This needs a
   stream to put down the extra headers. This one is defined in the Request Manager. The
   other one is for parsing. This needs the string to parse.
   
 */
typedef int HTParserCallback (HTRequest * request, CONST char * token);
/*

Header Parser Management

   Header Parsers can be registered to handle any header that is not part of the "default
   set" handled by the HTMIME module. This is mainlu the set defined by HTTP/1.1.
   
  ADD A HEADER PARSER
  
   Register a Header parser to be called if we encounter the token in teh protocol
   response. Tokens can contain a wildcard '*' which will match zero or more arbritary
   chars.
   
 */
extern BOOL HTParser_add (HTList *              parsers,
                          CONST char *          token,
                          BOOL                  case_sensitive,
                          HTParserCallback *    callback);
/*

  UNREGISTER A HEADER PARSER
  
 */
extern BOOL HTParser_delete (HTList * parsers, CONST char * token);
/*

  DELETE THE LIST OF REGISTERED HEADER PARSERS.
  
 */
extern BOOL HTParser_deleteAll (HTList * parsers);
/*

  FIND A PARSER
  
   Search registered parsers to find suitable one for this token If a parser isn't found,
   the function returns NULL
   
 */
extern HTParserCallback * HTParser_find (HTList *parsers, CONST char * token);
/*

Header Generator Management

   Header Generators can be use to add additional information to aprotocol request. They
   will all be called.
   
  ADD A HEADER GENERATOR
  
 */
extern BOOL HTGenerator_add (HTList * gens, HTPostCallback * callback);
/*

  UNREGISTER A HEADER GENERATOR
  
 */
extern BOOL HTGenerator_delete (HTList * gens, HTPostCallback * callback);
/*

  DELETE THE LIST OF REGISTERED HEADER GENERATORS.
  
 */
extern BOOL HTGenerator_deleteAll (HTList * gens);
/*

Global List Of Parsers and Generators

   As in HTFormat module you can register a list globally or locally as you like. The
   local registrations is managed by Request Manager
   
  HEADER PARSERS
  
 */
extern void HTHeader_setParser (HTList * list);
extern BOOL HTHeader_addParser (CONST char * token, BOOL case_sensitive,
                                HTParserCallback * callback);
extern BOOL HTHeader_deleteParser (CONST char * token);
extern HTList * HTHeader_parser (void);
/*

  HEADER GENERATION
  
 */
extern void HTHeader_setGenerator (HTList * list);
extern BOOL HTHeader_addGenerator (HTPostCallback * callback);
extern BOOL HTHeader_deleteGenerator (HTPostCallback * callback);
extern HTList * HTHeader_generator (void);
/*

  DELETE ALL GLOBAL LISTS
  
 */
extern void HTHeader_deleteAll (void);
/*

 */
#endif /* HTHEADER_H */
/*

   End of Declaration  */
