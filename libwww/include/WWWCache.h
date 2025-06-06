/*                                       Declaration of W3C Reference PERSISTENT CACHE MODULE
                   DECLARATION OF W3C REFERENCE PERSISTENT CACHE MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

  CACHE MANAGER
  
   Caching is a required part of any efficient Internet access applications as it saves
   bandwidth and improves access performance significantly in almost all types of
   accesses.  The Library supports two different types of cache: The memory cache and the
   file cache. The two types differ in several ways which reflects their two main
   purposes: The memory cache is for short term storage of graphic objects whereas the
   file cache is for intermediate term storage of data objects. Often it is desirable to
   have both a memory and a file version of a cached document, so the two types do not
   exclude each other.
   
   The cache contains details of temporary disk files which contain the contents of remote
   documents.  There is also a list of cache items for each URL in its anchor object.
   
   There are various ways of handling Expires header when met in a history list. Either it
   can be ignored all together, the user can be notified with a warning, or the document
   can be reloaded automatically. This flag decides what action to be taken. The default
   action is HT_EXPIRES_IGNORE. In HT_EXPIRES_NOTIFY mode you can specify a message to
   tell the user (NULL is valid in which case my own message pops up - watch out it might
   be in Danish ;-))
   
   The Library has two notions of a local cache: a file cache and a memory cache. The
   memory cache is handled by the client and the file cache is handled by the Library.
   Often the format of a object cached in memory is in the form of a hypertext object
   ready to be displayed (that is, it's already parsed).
   
 */
#ifndef WWWCACHE_H
#define WWWCACHE_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTCache.h"
/*

   End of CACHE module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWCACHE API definition  */
