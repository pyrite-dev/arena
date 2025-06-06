/*                                                                      Access Scheme Manager
                                  ACCESS SCHEME MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module keeps a list of valid protocol (naming scheme) specifiers with associated
   access code. New access protocols may be registered at any time.
   
   This module is implemented by HTProt.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTPROT_H
#define HTPROT_H

#include "HTReq.h"
#include "HTAnchor.h"
#include "HTEvntrg.h"
/*

   After the new architecture based on call back functions managed by an eventloop and
   protocol state machines, the protocol structure has been modified to reflect this call
   back structure. The HTEventCallback is defined in HTEvntrg module.
   
   All the access schemes supported in the Library can be initiated using the function
   HTAccessInit() in HTInit module
   
   An access scheme module takes as a parameter a socket (which is an invalid socket the
   first time the function is called), a request structure containing details of the
   request, and the action by which the (valid) socket was selected in the event loop.
   When the protocol class routine is called, the anchor element in the request is already
   valid (made valid by HTAccess).
   
 */
typedef struct _HTProtocol HTProtocol;
/*

  ADD AN ACCESS SCHEME
  
   This functions registers a protocol module and binds it to a specific access acheme.
   For example HTTP.c is bound to http URLs. The call back function is the function to be
   called for loading. The reason why it is of type HTEventCallback is that it then can be
   used directly in the event loop when used in non-preemptive mode.
   
 */
extern BOOL HTProtocol_add (CONST char *        name,
                            BOOL                preemptive,
                            HTEventCallback *   client,
                            HTEventCallback *   server);
/*

  DELETE AN ACCESS SCHEME
  
   This functions deletes a registered protocol module so that it can not be used for
   accessing a resource anymore.
   
 */
extern BOOL HTProtocol_delete (CONST char * name);
/*

  REMOVE ALL REGISTERED SCHEMES
  
   This is the garbage collection function. It is called by HTLibTerminate()
   
 */
extern BOOL HTProtocol_deleteAll (void);
/*

  FIND A PROTOCOL OBJECT
  
   You can search the list of registered protocol objects as a function of the access
   acheme. If an access scheme is found then the protocol object is returned.
   
 */
extern HTProtocol * HTProtocol_find (HTRequest * request, CONST char * access);
/*

  GET THE CALLBACK FUNCTIONS
  
   You can get the callback functions registered together with a protocol object using the
   following methods.
   
 */
extern HTEventCallback * HTProtocol_client (HTProtocol * protocol);
extern HTEventCallback * HTProtocol_server (HTProtocol * protocol);
/*

  IS ACCESS SCHEME PREEMPTIVE
  
   Returns YES if the implementation of the access scheme supports preemptive access only.
   
 */
extern BOOL HTProtocol_preemptive (HTProtocol * protocol);
/*

 */
#endif /* HTPROT_H */
/*

   End of Declaration  */
