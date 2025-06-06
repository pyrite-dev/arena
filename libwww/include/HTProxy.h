/*                                                                  Proxy and gateway Manager
                                PROXY AND GATEWAY MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module keeps a list of proxies and gateways to be contacted on a request in stead
   of requesting it directly from the origin server. The module replaces the old system of
   environment variables for gateways and proxies. However for backward compatibility
   there is a function that reads the environment variables at start up. Note that there
   is a difference between a proxy and a gateway - the difference is the way the URL is
   set up in the RequestLine of the HTTP request. If the original, full URL looks like
   "http://www.w3.org/test.html" then the result will for a proxy is
   "http://www.w3.org/test.html" and a gateway "/www.w3.org/test.html"
   
   The module is implemented by HTProxy.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTPROXY_H
#define HTPROXY_H

#include "HTList.h"
/*

Registering a proxy server

   A proxy server is registered with a corresponding access method, for example http, ftp
   etc. The `proxy' parameter should be a fully valid name, like http://proxy.w3.org:8001
   but domain name is not required. If an entry exists for this access then delete it and
   use the new one.
   
 */
extern BOOL HTProxy_add         (CONST char * access, CONST char * proxy);
extern BOOL HTProxy_deleteAll   (void);
/*

   The remove function removes all registered proxies. This is automatically done in
   HTLibTerminate()
   
Registering a host or domain with no proxy

   The noproxy list is a list of host names and domain names where we don't contact a
   proxy even though a proxy is in fact registered for this particular access method .
   When registering a noproxy item, you can specify a specific port for this access method
   in which case it isvalid only for requests to this port. If `port' is '0' then it
   applies to all ports and if `access' is NULL then it applies to to all access methods.
   Examples of host names are w3.org and www.close.com
   
 */
extern BOOL HTNoProxy_add       (CONST char * host, CONST char * access,
                                 unsigned port);
extern BOOL HTNoProxy_deleteAll (void);
/*

   The remove function removes all entries in the list. This is automatically done in
   HTLibTerminate()
   
Look for a Proxy server

   This function evaluates the lists of registered proxies and if one is found for the
   actual access method and it is not registered in the `noproxy' list, then a URL
   containing the host to be contacted is returned to the caller. This string must be
   freed be the caller.
   
 */
extern char * HTProxy_find      (CONST char * url);
/*

Registering a gateway

   A gateway is registered with a corresponding access method, for example http, ftp etc.
   The `gate' parameter should be a fully valid name, like http://gateway.w3.org:8001but
   domain name is not required. If an entry exists for this access then delete it and use
   the new one.
   
 */
extern BOOL HTGateway_add       (CONST char * access, CONST char * gate);
extern BOOL HTGateway_deleteAll (void);
/*

   The remove function removes all registered proxies. This is automatically done in
   HTLibTerminate()
   
Look for a Gateway

   This function evaluates the lists of registered gateways and if one is found for the
   actual access method then it is returned and must be freed by the caller.
   
 */
extern char * HTGateway_find    (CONST char * url);
/*

Backwards Compability with Environment Variables

   This function maintains backwards compatibility with the old environment variables and
   searches for the most common values: http, ftp, news, wais, and gopher
   
 */
extern void HTProxy_getEnvVar   (void);
/*

 */
#endif /* HTPROXY_H */
/*

   End of HTProxy declaration  */
