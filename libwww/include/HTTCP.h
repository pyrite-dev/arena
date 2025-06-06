/*                                                              Generic Network Communication
                              GENERIC NETWORK COMMUNICATION
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module has the common code for handling TCP/IP and DECnet connections etc. The
   main topics of functions in this module are:
   
      Connection establishment
      
      Cache of host names
      
      Errno Messages
      
      Host and mail addresses
      
      Signal Handling
      
   This module is implemented by HTTCP.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTTCP_H
#define HTTCP_H
#include "HTReq.h"
#include "HTNet.h"
/*

Connection Management

   All connections are established through the following functions.
   
  ACTIVE CONNECTION ESTABLISHMENT
  
   This makes an active connect to the specified host. The HTNet structure is parsed in
   order to handle errors. Default port might be overwritten by any port indication in the
   URL specified as <host>:<port> If it is a multihomed host then HTDoConnect measures the
   time to do the connection and updates the calculated weights in the cache of visited
   hosts.
   
 */
extern int HTDoConnect (HTNet * net, char * url, u_short default_port);
/*

  PASSIVE CONNECTION ESTABLISHMENT
  
   This function makes a non-blocking accept on a port. The net must contain a valid
   socket to accept on. If accept is OK then the socket descripter in the Net object is
   swapped to the new one.
   
 */
extern int HTDoAccept (HTNet * net);
/*

  LISTEN ON A SOCKET
  
   Listens on the specified port. 0 means that we don't care and a temporary one will be
   assigned. If master==INVSOC then we listen on all local interfaces (using a wildcard).
   If !INVSOC then use this as the local interface. backlog is the number of connections
   that can be queued on the socket - you can use HT_BACKLOG for a platform dependent
   value (typically 5 on BSD and 32 on SVR4). Returns HT_ERROR or HT_OK.
   
 */
extern int HTDoListen (HTNet * net, u_short port, SOCKET master, int backlog);
/*

System Description of Error Message

   Return error message corresponding to errno number given. We need to pass the error
   number as a parameter as we on some platforms get different codes from sockets and
   local file access.
   
 */
extern CONST char * HTErrnoString       (int errnum);
extern int HTInetStatus                 (int errnum, char * where);
/*

  PARSE A CARDINAL VALUE
  
 */
/*      Parse a cardinal value                                 parse_cardinal()
**      ----------------------
**
** On entry:
**      *pp points to first character to be interpreted, terminated by
**      non 0..9 character.
**      *pstatus points to status already valid,
**      maxvalue gives the largest allowable value.
**
** On exit:
**      *pp points to first unread character,
**      *pstatus points to status updated iff bad
*/

extern unsigned int HTCardinal (int *           pstatus,
                                char **         pp,
                                unsigned int    max_value);
/*

Internet Name Server Functions

   The following functions are available to get information about a specified host.
   
  PRODUCE A STRING FOR AN INTERNET ADDRESS
  
   This function is equivalent to the BSD system call inet_ntoa in that it converts a
   numeric 32-bit IP-address to a dotted-notation decimal string. The pointer returned
   points to static memory which must be copied if it is to be kept.
   
 */
extern CONST char * HTInetString (struct sockaddr_in * sin);
/*

  GET NAME OF THIS MACHINE
  
   This function returns a CONET char pointer to a static location containing the name of
   this host or NULL if not available.
   
 */
extern CONST char * HTGetHostName (void);
/*

  SET NAME OF THIS MACHINE
  
   This function overwrites any other value of current host name. This might be set by the
   user to change the value in the ID value parsed to a news host when posting. The change
   doesn't influence the Mail Address as they are stored in two different locations. If,
   however, the change is done before the first call to HTGetMailAddress() then this
   function will use the new host and domain name.
   
 */
extern void HTSetHostName (char * host);
/*

  CLEANUP MEMORY
  
   Called from HTLibTerminate
   
 */
extern void HTFreeHostName (void);
/*

  GET DOMAIN NAME OF THIS MACHINE
  
   This function rerturns the domain name part of the host name as returned by
   HTGetHostName() function. Changing the domain name requires a call to  HTSetHostname().
   
 */
extern CONST char *HTGetDomainName (void);
/*

  GET USER MAIL ADDRESS
  
   This functions returns a char pointer to a static location containing the mail address
   of the current user. The static location is different from the one of the current host
   name so different values can be assigned. The default value is <USER>@hostname where
   hostname is as returned by HTGetHostName().
   
 */
extern CONST char * HTGetMailAddress (void);
/*

  SET USER MAIL ADDRESS
  
   This function overwrites any other value of current mail address. This might be set by
   the user to change the value in the  From field in the HTTP Protocol.
   
 */
extern void HTSetMailAddress (char * address);
/*

  FREE MEMORY
  
   Called by HTLibTerminate
   
 */
extern void HTFreeMailAddress (void);
/*

Signal Handling

   This is only necessary to compile on a few platforms and only if the application does
   not have its own signal handling. It is required on Solaris 2.3 (and other SVR4
   platforms?) due to a bug in the TCP kernel. When a connect() is tried to a illegal
   port, solaris gives a SIGPIPE signal instead of returning Connection refused.
   
 */
#ifdef WWWLIB_SIG
extern void HTSetSignal (void);
#endif

#endif   /* HTTCP_H */
/*

   End of file */
