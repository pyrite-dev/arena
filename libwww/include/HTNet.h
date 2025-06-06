/*                                                              Asyncronous Socket Management
                              ASYNCRONOUS SOCKET MANAGEMENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module contains the routines for handling the set of active sockets currently in
   use by the multithreaded clients. It is an internal module to the Library, the
   application interface is implemented in the Event Module. Look for more information in
   the Multithread Specifications.
   
   This module is implemented by HTNet.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTNET_H
#define HTNET_H
#include "HTEvntrg.h"
#include "HTReq.h"
/*

The HTNet Object

   The HTNet object is the core of the request queue management. This object contains
   information about the socket descriptor, the input read buffer etc. required to
   identify and service a request.
   
 */
typedef struct _HTNet HTNet;
/*

Request Call Back Functions

   Callback functions can be registered to be called before and after a request has either
   been started or has terminated. The following functions are the generic registration
   mechanisms where we use lists as the basic data container. Then there is two methods
   for binding a list of callback functions to the set which is called before and to the
   set set which is called after
   
   In both cases there can be more than one callback function which are called on turn and
   each callback function can be associated with a status code of the request. For example
   one callback function can be registered for HT_LOADED, another for HT_ERROR etc.
   
  REGISTER A REQUEST CALLBACK
  
   Register a call back function that is to be called on every termination of a request.
   Several call back functions can be registered in which case all of them are called in
   the reverse order of which they were registered (last one first). We name the calling
   mechansm of calling the functions for the before loop and the after loop.
   
   In case the callback function is registered as being called after the request has
   terminated the result of the request is passed to the fucntion. The status signifies
   which call back function to call depending of the result of the request. This can be
   
  HT_ERROR               An error occured
                         
  HT_LOADED              The document was loaded
                         
  HT_NO_DATA             OK, but no data
                         
  HT_RETRY               Retry request after at a later time
                         
  HT_REDIRECT            The request has been redirected and we send back the new URL
                         
  HT_ALL                 All of above
                         
   Any callback function any code it likes, but IF NOT the code is HT_OK, then the
   callback loop is stopped. If we are in the before loop and a function returns anything
   else than HT_OK then we immediately jump to the after loop passing the last return code
   from the before loop.
   
 */
typedef int HTNetCallback (HTRequest * request, int status);

extern BOOL HTNetCall_add (HTList * list, HTNetCallback *cbf, int status);
/*

  DELETE A SINGLE CALLBAK
  
   Removes a callback function from a list
   
 */
extern BOOL HTNetCall_delete (HTList * list, HTNetCallback *cbf);
/*

  DELETE A LIST OF CALLBACKS
  
   Unregisters all call back functions in the list
   
 */
extern BOOL HTNetCall_deleteAll (HTList * list);
/*

  CALL LIST OF REGISTERED CALLBACK FUNCTIONS
  
   Call all the call back functions registered in the list IF not the status is HT_IGNORE.
    The callback functions are called in the order of which they were registered. At the
   moment an application callback function is called, it can free the request object - it
   is no longer used by the Library. Returns what the last callback function returns
   
 */
extern int HTNetCall_execute (HTList * list, HTRequest * request, int status);
/*

  BEFORE CALLBACKS
  
   Global set of callback functions BEFORE the request is issued. The list can be NULL.
   
 */
extern BOOL HTNetCall_addBefore (HTNetCallback *cbf, int status);
extern BOOL HTNet_setBefore     (HTList * list);
extern HTList * HTNet_before    (void);
extern int HTNet_callBefore     (HTRequest *request, int status);
/*

  AFTER CALLBACKS
  
   Global set of callback functions AFTER the request is issued. The list can be NULL
   
 */
extern BOOL HTNetCall_addAfter  (HTNetCallback *cbf, int status);
extern BOOL HTNet_setAfter      (HTList * list);
extern HTList * HTNet_after     (void);
extern int HTNet_callAfter      (HTRequest *request, int status);
/*

Request Queue

   The request queue ensures that no more than a fixed number of TCP connections are open
   at the same time. If more requests are handed to the Library, they are put into the
   pending queue and initiated when sockets become free.
   
  NUMBER OF SIMULTANOUS OPEN TCP CONNECTIONS
  
   Set the max number of simultanous sockets. The default value is HT_MAX_SOCKETS which is
   6. The number of persistent connections depend on this value as a deadlock can occur if
   all available sockets a persistent (see the DNS Manager for more information on setting
   the number of persistent connections). The number of persistent connections can never
   be more than the max number of sockets-2, so letting newmax=2 prevents persistent
   sockets.
   
 */
extern BOOL HTNet_setMaxSocket (int newmax);
extern int  HTNet_maxSocket (void);
/*

  LIST ACTIVE QUEUE
  
   Returns the list of active requests that are currently having an open connection.
   Returns list of HTNet objects or NULL if error.
   
 */
extern HTList *HTNet_activeQueue (void);
extern BOOL HTNet_idle (void);
/*

  ARE WE ACTIVE?
  
   We have some small functions that tell whether there are registered requests in the Net
   manager. There are tree queues: The active, the pending, and the persistent. The active
   queue is the set of requests that are actively sending or receiving data. The pending
   is the requests that we have registered but which are waiting for a free socket. The
   Persistent queue are requets that are waiting to use the same socket in order to save
   network resoures (if the server understands persistent connections).
   
    Active Reqeusts?
    
   Returns whether there are requests in the active queue or not
   
 */
extern BOOL HTNet_idle (void);
/*

    Registered Requests?
    
   Returns whether there are requests registered in any of the lists or not
   
 */
extern BOOL HTNet_isEmpty (void);
/*

  LIST PENDING QUEUE
  
   Returns the list of pending requests that are waiting to become active. Returns list of
   HTNet objects or NULL if error
   
 */
extern HTList *HTNet_pendingQueue (void);
/*

Create an Object

   You can create a new HTNet object as a new request to be handled. If we have more than
   HTMaxActive connections already then put this into the pending queue, else start the
   request by calling the call back function registered with this access method.  Returns
   YES if OK, else NO
   
 */
extern BOOL HTNet_newClient (HTRequest * request);
/*

   You can create a new HTNet object as a new request to be handled. If we have more than
   HTMaxActive connections already then return NO. Returns YES if OK, else NO
   
 */
extern BOOL HTNet_newServer (HTRequest * request, SOCKET sockfd, char *access);
/*

   And you can create a plain new HTNet object using the following method:
   
 */
extern HTNet * HTNet_new (HTRequest * request, SOCKET sockfd);
/*

  DUPLICATE AN EXISTING OBJECT
  
   Creates a new HTNet object as a duplicate of the same request. Returns YES if OK, else
   NO.
   
 */
extern HTNet * HTNet_dup (HTNet * src);
/*

HTNet Object Methods

  MAKE AN OBJECT WAIT
  
   Let a net object wait for a persistent socket. It will be launched from the
   HTNet_delete() function when the socket gets free.
   
 */
extern BOOL HTNet_wait (HTNet *net);
/*

  PRIORITY MANAGEMENT
  
   Each HTNet object is created with a priority which it inherits from the Request
   manager. However, in some stuations it is useful to be to change the current priority
   after the request has been started. These two functions allow you to do this. The
   effect will show up the first time (which might be imidiately) the socket blocks and
   control returns to the event loop. Also have a look at how you can do this before the
   request is issued in the request manager.
   
 */
extern HTPriority HTNet_priority (HTNet * net);
extern BOOL HTNet_setPriority (HTNet * net, HTPriority priority);
/*

  DELETE AN OBJECT
  
   Deletes the HTNet object from the list of active requests and calls any registered call
   back functions IF not the status is HT_IGNORE. This is used if we have internal
   requests that the app doesn't know about. We also see if we have pending requests that
   can be started up now when we have a socket free. The callback functions are called in
   the reverse order of which they were registered (last one first);
   
 */
extern BOOL HTNet_delete (HTNet * me, int status);
/*

  DELETE ALL HTNET OBJECTS
  
   Deletes all HTNet object that might either be active or pending We DO NOT call the call
   back functions - A crude way of saying goodbye!
   
 */
extern BOOL HTNet_deleteAll (void);
/*

  KILL A REQUEST
  
   Kill the request by calling the call back function with a request for closing the
   connection. Does not remove the object. This is done by HTNet_delete() function which
   is called by the load routine.  Returns OK if success, NO on error
   
 */
extern BOOL HTNet_kill (HTNet * me);
/*

  KILL ALL REQUESTS
  
   Kills all registered (active+pending) requests by calling the call back function with a
   request for closing the connection. We do not remove the HTNet object as it is done by
   HTNet_delete().  Returns OK if success, NO on error
   
 */
extern BOOL HTNet_killAll (void);
/*

Data Access Methods

  SOCKET DESCRIPTOR
  
 */
extern BOOL HTNet_setSocket (HTNet * net, SOCKET sockfd);
extern SOCKET HTNet_socket (HTNet * net);
/*

 */
#endif /* HTNET_H */
/*

   End of declaration module */
