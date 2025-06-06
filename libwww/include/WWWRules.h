/*                                                                     Rule Files and Proxies
                                  RULE FILES AND PROXIES
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   In addition top the basic W3C Reference Library include file called WWWLib.h you can
   also include this file depending on the needs of your application. However, it is not
   required and none of the files included below are ever used in the core part of the
   Library itself. Only if this file is included, the extra modules will get included in
   the linked object code. It is also possible to include only a subset of the files below
   if the functionality you are after is covered by them.
   
 */
#ifndef WWWRULES_H
#define WWWRULES_H
/*

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

  RULE FILE MANAGEMENT
  
   Another way to initialize applications is to use a rule file, also known as a
   configuration file. This is for example the case with the W3C httpd and the W3C Line
   Mode Browser. This module provides basic support for configuration file management and
   the application can use this is desired. The module is not referred to by the Library.
   Reading a rule file is implemented as a stream converter so that a rule file can come
   from anywhere, even across the network!
   
 */
#include "HTRules.h"
/*

  PROXIES AND GATEWAYS
  
   Applications do not have to provide native support for all protocols, they can in many
   situations rely on the support of proxies and gateways to help doing the job. Proxy
   servers are often used to carry client requests through a firewall where they can
   provide services like corporate caching and other network optimizations. Both Proxy
   servers and gateways can serve as "protocol translators" which can convert a request in
   the main Web protocol, HTTP, to an equivalent request in another protocol, for example
   NNTP, FTP, or Gopher. In case a proxy server or a gateway is available to the
   application, it can therefore by use of HTTP forward all requests to for example a
   proxy server which then handle the communications with the remote server, for example
   using FTP about the document and return it to the application (proxy client) using
   HTTP.
   
 */
#include "HTProxy.h"
/*

   End of application specific modules
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWRules declaration  */
