/*                                                                         Disk Cache Manager
                                    DISK CACHE MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   The cache contains details of temporary disk files which contain the contents of remote
   documents.  There is also a list of cache items for each URL in its anchor object.
   
   This module is implemented by HTCache.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTCACHE_H
#define HTCACHE_H

#include "HTStream.h"
#include "HTAnchor.h"
#include "HTFormat.h"
#include "HTReq.h"
/*

Converters

   The cache stream is in fact a stream that can be registered as a converter!
   
 */
extern HTConverter HTCacheWriter;
/*

  HOW DO WE HANDLE EXPIRES HEADER?
  
   There are various ways of handling Expires header when met in a history list. Either it
   can be ignored all together, the user can be notified with a warning, or the document
   can be reloaded automatically. This flag decides what action to be taken. The default
   action is HT_EXPIRES_IGNORE. In HT_EXPIRES_NOTIFY mode you can specify a message to
   tell the user (NULL is valid in which case my own message pops up - watch out it might
   be in Danish ;-))
   
 */
typedef enum _HTExpiresMode {
    HT_EXPIRES_IGNORE = 0,
    HT_EXPIRES_NOTIFY,
    HT_EXPIRES_AUTO
} HTExpiresMode;

extern void HTCache_setExpiresMode (HTExpiresMode mode, char * notify);
extern HTExpiresMode HTCache_expiresMode (char ** notify);
/*

Memory cache Management

   The Library has two notions of a local cache: a file cache and a memory cache. The
   memory cache is handled by the client and the file cache is handled by the Library.
   Often the format of a object cached in memory is in the form of a hypertext object
   ready to be displayed (that is, it's already parsed). However, this is not required,
   and the application can therefore register a memory cache handler that returns an int
   with the following values:
   
      HT_LOADED if the document is in memory and is OK
      
      HT_ERROR if the document is not in memory
      
 */
typedef int HTMemoryCacheHandler        (HTRequest *    request,
                                         HTExpiresMode  mode,
                                         char *         message);

extern int  HTMemoryCache_check         (HTRequest * request);
extern BOOL HTMemoryCache_register      (HTMemoryCacheHandler * cbf);
extern BOOL HTMemoryCache_unRegister    (void);
/*

Find a Reference for a Cached Object

   Verifies if a cache object exists for this URL and if so returns a URL for the cached
   object. It does not verify whether the object is valid or not, for example it might
   have been expired.
   
 */
extern char * HTCache_getReference      (char * url);
/*

Verify if an Object is Valid

   This function checks whether a document has expired or not.  The check is based on the
   metainformation passed in the anchor object The function returns YES or NO.
   
 */
extern BOOL HTCache_isValid             (HTParentAnchor * anchor);
/*

Enable Cache

   If cache_root is NULL then reuse old value or use HT_CACHE_ROOT.  An empty string will
   make '/' as cache root.
   
 */
extern BOOL HTCache_enable              (CONST char * cache_root);
/*

Disable Cache

   Turns off the cache. Note that the cache can be disabled and enabled at any time. The
   cache root is kept and can be reused during the execution.
   
 */
extern BOOL HTCache_disable             (void);
/*

Is Cache Enabled

   Returns YES or NO. Also makes sure that we have a root value (even though it might be
   invalid)
   
 */
extern BOOL HTCache_isEnabled           (void);
/*

Set Cache Root

   If cache_root is NULL then the current value (might be a define) Should we check if the
   cache_root is actually OK? I think not!
   
 */
extern BOOL HTCache_setRoot             (CONST char * cache_root);
/*

Get Cache Root

 */
extern CONST char *HTCache_getRoot      (void);
/*

Clean up memory

   This is done by the Library function HTLibTerminate(). None of these functions tourches
   the disk cache itself - they only manages memory.
   
 */
extern void HTCache_freeRoot            (void);
extern void HTCache_clearMem            (void);
/*

To remove All cache files known to this session

   This function also cleans the entries on disk
   
 */
extern void HTCache_deleteAll           (void);

#endif
/*

   End of declaration module */
