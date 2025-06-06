/*                                                                Domain Name Service Manager
                               DOMAIN NAME SERVICE MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module has the common code for handling DNS access. It maintains a cache of all
   visited hosts so that subsequent connects to the same host doesn't imply a new request
   to the DNS every time.
   
   Multihomed hosts are treated specially in that the time spend on every connect is
   measured and kept in the cache. On the next request to the same host, the IP-address
   with the lowest average connect time is chosen. If one IP-address fails completely,
   e.g. connection refused then it disabled and HTDoConnect tries one of the other
   IP-addresses to the same host.
   
   If the connect fails in the case of at single-homed host then the entry is removed from
   the cache and HTDoConnect tries again asking the DNS.
   
   This module is implemented by HTDNS.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTDNS_H
#define HTDNS_H
#include "tcp.h"
#include "HTEvntrg.h"
/*

The DNS Object

   The DNS object contains information obtained from the DNS system but in addition it
   stores known information about the remote server, for example the type (HTTP/1.0,
   HTTP/1.1, FT etc.) along with information on how the connections can be used (if it
   supports persistent TCP connections, interleaved access etc.)
   
 */
typedef struct _HTdns HTdns;
/*

Cache Timeouts

   When to remove an entry in the cache. The default value is 48h.
   
 */
extern void HTDNS_setTimeout (time_t timeout);
extern time_t HTDNS_timeout  (time_t timeout);
/*

Object Methods

  PERSISTENT SOCKETS
  
 */
extern SOCKET HTDNS_socket      (HTdns *dns);
extern BOOL   HTDNS_setSocket   (HTdns *dns, SOCKET socket);
extern void   HTDNS_clearActive (HTdns *dns);
extern int    HTDNS_socketCount (void);
/*

  PERSISTENT CONNECTION TIMEOUTS
  
   When should we drop a socket. The default value is 1h
   
 */
extern time_t HTDNS_sockTimeout (time_t timeout);
extern void HTDNS_setSockTimeout (time_t timeout);
/*

  PERSISTENT CONNECTION EXPIRATION
  
   Absolute value of when this socket will expire. Default value can be set with the
   function HTDNS_setSockTimeout and is normally 1 h.
   
 */
extern HTEventCallback HTDNS_closeSocket;

extern void HTDNS_setSockExpires (HTdns * dns, time_t expires);
extern time_t HTDNS_sockExpires (HTdns * dns);
/*

  SET THE SERVER CLASS AND TYPE
  
   Define the server class of the server at the other end. A class is a generic
   description of the protocol which is exactly like the access method in a URL, for
   example "http" etc. The server version is a finer distinction (sub-class) between
   various versions of the server class, for example HTTP/0.9, HTTP/1.1 etc. The server
   version is a bit flag that the protocol module can define on its own. That way we don't
   have to change this module when registering a new protocol module. The server type is a
   description of whether we can keep the connection persistent or not.
   
 */
extern char * HTDNS_serverClass         (HTdns * dns);
extern void HTDNS_setServerClass        (HTdns * dns, char * s_class);

extern int  HTDNS_serverVersion         (HTdns * dns);
extern void HTDNS_setServerVersion      (HTdns * dns, int version);

typedef enum _HTTCPType {
    HT_TCP_PLAIN        = 0x0,                      /* One request at a time */
    HT_TCP_BATCH        = 0x1,                         /* Use batch requests */
    HT_TCP_INTERLEAVE   = 0x2                 /* Can we interleave requests? */
} HTTCPType;

extern HTTCPType HTDNS_connection       (HTdns * dns);
extern void HTDNS_setConnection         (HTdns * dns, HTTCPType type);
/*

  RECALCULATING THE TIME-WEIGHTS ON MULTIHOMED HOSTS
  
   On every connect to a multihomed host, the average connect time is updated
   exponentially for all the entries.
   
 */
extern BOOL HTDNS_updateWeigths (HTdns *dns, int cur, time_t deltatime);
/*

  DELETE A HOST ELEMENT FROM CACHE
  
   This function deletes a single cache entry.
   
 */
extern BOOL HTDNS_delete (CONST char * host);
/*

  DELETE ALL HOST ELEMENTS FROM CACHE
  
   This function is called from HTLibTerminate. It can be called at any point in time if
   the DNS cache is going to be flushed.
   
 */
extern BOOL HTDNS_deleteAll (void);
/*

Resolver Functions

   These are the functions that resolve a host name
   
  GET HOST BY SOCKET
  
   This function should have been called HTGetHostByAddr but for historical reasons this
   is not the case.
   
 */
extern char * HTGetHostBySock (int soc);
/*

  GET HOST BY NAME
  
   This function gets the address of the host and puts it in to the socket structure. It
   maintains its own cache of connections so that the communication to the Domain Name
   Server is minimized. Returns the number of homes or -1 if error.
   
 */
extern int HTGetHostByName (struct _HTNet *net, char *host);
/*

 */
#endif
/*

   End of declaration file */
