/*                                                                              Event Manager
                                      EVENT MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is the application interface to the multi-threaded functionality in the
   Library. It contains a set of functions that the application can either use as are or
   they can be overwritten by the application.
   
   This module is implemented by HTEvntrg.c, and it is a part of the W3C Reference
   Library.
   
 */
#ifndef HTEVNTRG_H
#define HTEVNTRG_H
#include "HTReq.h"
#include "tcp.h"
/*

Windows Specific Handles

 */
#if defined(WWW_WIN_ASYNC) || defined(WWW_WIN_DLL)
extern BOOL HTEvent_winHandle (HTRequest * request);
extern BOOL HTEvent_setWinHandle (HWND window, unsigned long message);
extern HWND HTEvent_getWinHandle (unsigned long * pMessage);
extern LRESULT CALLBACK AsyncWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
);
#endif
/*

Event Handlers

   The appplication registers a set of event handlers to be used on a specified set of
   sockets. The eventhandlers must be defined as follows:
   
 */
typedef u_long SockOps;

#define FD_NONE 0
#define FD_ALL (FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT |FD_CLOSE)
#define FD_UNREGISTER (((FD_ALL) << 1) & (~(FD_ALL)))

typedef enum _HTPriority {
    HT_PRIORITY_INV = -1,
    HT_PRIORITY_OFF = 0,
    HT_PRIORITY_MIN = 1,
    HT_PRIORITY_MAX = 20
} HTPriority;

typedef int HTEventCallback (SOCKET, HTRequest *, SockOps);
/*

  REGISTER A TTY EVENT HANDLER
  
   Register the tty (console) as having events. If the TTY is select()-able (as is true
   under Unix), then we treat it as just another socket. Otherwise, take steps depending
   on the platform. This is the function to use to register user events!
   
 */
extern int HTEvent_RegisterTTY  (SOCKET, HTRequest *, SockOps,
                                 HTEventCallback *, HTPriority);
/*

  UNREGISTER A TTY EVENT HANDLER
  
   Unregisters TTY I/O channel. If the TTY is select()-able (as is true under Unix), then
   we treat it as just another socket.
   
 */
extern int HTEvent_UnRegisterTTY (SOCKET, SockOps);
/*

  REGISTER AN EVENT HANDLER
  
   For a given socket, reqister a request structure, a set of operations, a
   HTEventCallback function, and a priority. For this implementation, we allow only a
   single HTEventCallback function for all operations. and the priority field is ignored.
   
 */
extern int HTEvent_Register     (SOCKET, HTRequest *,
                                 SockOps, HTEventCallback *,
                                 HTPriority);
/*

  UNREGISTER AN EVENT HANDLER
  
   Remove the registered information for the specified socket for the actions specified in
   ops. if no actions remain after the unregister, the registered info is deleted, and, if
   the socket has been registered for notification, the HTEventCallback will be invoked.
   
 */
extern int HTEvent_UnRegister   (SOCKET, SockOps);
/*

  UNREGISTER ALL EVENT HANDLERS
  
   Unregister all sockets. N.B. we just remove them for our internal data structures: it
   is up to the application to actually close the socket.
   
 */
extern int HTEvent_UnregisterAll (void);
/*

Handler for Timeout on Sockets

   This function sets the timeout for sockets in the select() call and registers a timeout
   function that is called if select times out. This does only works on NON windows
   platforms as we need to poll for the console on windows If tv = NULL then timeout is
   disabled. Default is no timeout. If always=YES then the callback is called at all
   times, if NO then only when Library sockets are active. Returns YES if OK else NO.
   
 */
typedef int HTEventTimeout (HTRequest *);

extern BOOL HTEvent_registerTimeout (struct timeval *tp, HTRequest * request,
                                     HTEventTimeout *tcbf, BOOL always);
/*

Start the Event Loop

   That is, we wait for activity from one of our registered channels, and dispatch on
   that. Under Windows/NT, we must treat the console and sockets as distinct.  That means
   we can't avoid a busy wait, but we do our best.
   
 */
extern int HTEvent_Loop (HTRequest * request);
/*

Stop the Event Loop

   Stops the (select based) event loop. The function does not guarantee that all requests
   have terminated. This is for the app to do
   
 */
extern void HTEvent_stopLoop (void);
/*

 */
#endif /* HTEvent_H */
/*

   End of declartion module  */
