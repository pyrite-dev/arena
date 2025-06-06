/*                                        HTTP Communalities between Server and Client Module
                   HTTP COMMUNALITIES BETWEEN SERVER AND CLIENT MODULE
                                             
   The HTTP client module and the server module has a few things in common which we keep
   in this file.
   
 */
#ifndef HTTPUTIL_H
#define HTTPUTIL_H
/*

  HTTP VERSION MANAGEMENT
  
 */
typedef enum _HTTPVersion {
    HTTP = 0,
    HTTP_09,
    HTTP_10,
    HTTP_11,
    HTTP_12
} HTTPVersion;
/*

  CURRENT VERSION OF HTTP
  
 */
#define HTTP_VERSION    "HTTP/1.0"
/*

 */
#endif
/*

   End of declaration */
