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
#ifndef HTNETMAN_H
#define HTNETMAN_H
#include "HTNet.h"
#include "HTDNS.h"
#include "HTEvntrg.h"
#include "HTSocket.h"
/*

The HTNet Object

   The HTNet object is the core of the request queue management. This object contains
   information about the socket descriptor, the input read buffer etc. required to
   identify and service a request.
   
 */
typedef enum _TCPState {
    TCP_ERROR           = -2,
    TCP_CONNECTED       = -1,
    TCP_BEGIN           = 0,
    TCP_DNS,
    TCP_NEED_SOCKET,
    TCP_NEED_BIND,
    TCP_NEED_LISTEN,
    TCP_NEED_CONNECT
} TCPState;

struct _HTNet {
    SOCKET              sockfd;                         /* Socket descripter */
    SockA               sock_addr;              /* SockA is defined in tcp.h */
    TCPState            tcpstate;                     /* State in connection */
    HTInputSocket *     isoc;                                /* Input buffer */
    HTdns *             dns;                           /* Entry in DNS table */
    HTStream *          target;                             /* Target stream */
    int                 retry;               /* Counting attempts to connect */
    int                 home;                    /* Current home if multiple */
    time_t              connecttime;             /* Used on multihomed hosts */
    long                bytes_read;               /* Bytes read from network */
    long                bytes_written;           /* Bytes written to network */
    BOOL                preemptive;  /* Eff result from Request and Protocol */
    BOOL                persistent;       /* YES if persistent, otherwise NO */
    HTPriority          priority;        /* Priority of this request (event) */
    HTEventCallback *   cbf;                         /* Library load routine */
    HTRequest *         request;           /* Link back to request structure */
    void *              context;                /* Protocol Specific context */
};

#define HTNet_bytesRead(me)             ((me) ? (me)->bytes_read : -1)
#define HTNet_bytesWritten(me)          ((me) ? (me)->bytes_written : -1)

#define HTNet_setBytesRead(me,l)        ((me) ? (me->bytes_read=(l)) : -1)
#define HTNet_setBytesWritten(me,l)     ((me) ? (me->bytes_written=(l)) :-1)

#define HTNet_dns(me)                   ((me) ? (me)->dns : NULL)
/*

 */
#endif /* HTNETMAN_H */
/*

   End of declaration module */
