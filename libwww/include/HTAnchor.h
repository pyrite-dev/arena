/*                                                                       Public Anchor Object
                                   PUBLIC ANCHOR OBJECT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   An anchor represents a region of a hypertext document which is linked to another anchor
   in the same or a different document. As always we must emulate the fancy features of
   C++ by hand :-(. In this module you find:
   
      Creation and deletion methods
      
      Manipulation of Links
      
      Access Methods for information
      
   This module is implemented by HTAnchor.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTANCHOR_H
#define HTANCHOR_H

#include "HTList.h"
#include "HTAtom.h"
#include "HTMethod.h"
/*

Types defined by the Anchor Object

   This is a set of videly used type definitions used through out the Library:
   
 */
typedef struct _HTLink          HTLink;

typedef HTAtom * HTFormat;
typedef HTAtom * HTLevel;                      /* Used to specify HTML level */
typedef HTAtom * HTEncoding;                             /* Content Encoding */
typedef HTAtom * HTCte;                         /* Content transfer encoding */
typedef HTAtom * HTCharset;
typedef HTAtom * HTLanguage;

typedef struct _HTAnchor        HTAnchor;
typedef struct _HTParentAnchor  HTParentAnchor;
typedef struct _HTChildAnchor   HTChildAnchor;
/*

The Link Object

   Anchors are bound together using link objects. Each anchor can be the source or
   destination of zero, one, or more links from and to other anchors.
   
  LINK DESTINATION
  
 */
extern BOOL HTLink_setDestination (HTLink * link, HTAnchor * dest);
extern HTAnchor * HTLink_destination (HTLink * link);
/*

  LINK RESULT
  
   When a link has been used for posting an object from a source to a destination link,
   the result of the operation is stored as part of the link information.
   
 */
typedef enum _HTLinkResult {
    HT_LINK_INVALID = -1,
    HT_LINK_NONE = 0,
    HT_LINK_ERROR,
    HT_LINK_OK
} HTLinkResult;

extern BOOL HTLink_setResult (HTLink * link, HTLinkResult result);
extern HTLinkResult HTLink_result (HTLink * link);
/*

  LINK METHOD
  
   The method used in a link can be PUT, or POST, for example
   
 */
extern BOOL HTLink_setMethod (HTLink * link, HTMethod method);
extern HTMethod HTLink_method (HTLink * link);
/*

  LINK TYPE
  
   This is used for typed links.
   
 */
typedef HTAtom * HTLinkType;

extern BOOL HTLink_setType (HTLink * link, HTLinkType type);
extern HTLinkType HTLink_type (HTLink * link);
/*

Relations between Links and Anchors

  LINK THIS ANCHOR TO ANOTHER GIVEN ONE
  
   A single anchor may have many outgoing links of which the default is the main link. If
   one already exists then this new link is simply added to the list.
   
 */
extern BOOL HTAnchor_link       (HTAnchor *     source,
                                 HTAnchor *     destination,
                                 HTLinkType     type,
                                 HTMethod       method);
/*

  FIND THE LINK OBJECT THAT BINDS TWO ANCHORS
  
   If the destination anchor is a target of a link from the source anchor then return the
   link object, else NULL.
   
 */
extern HTLink * HTAnchor_findLink (HTAnchor *src, HTAnchor *dest);
/*

  FIND DESTINATION WITH GIVEN RELATIONSHIP
  
   Return the anchor with a given typed link.
   
 */
extern HTAnchor * HTAnchor_followTypedLink (HTAnchor *me, HTLinkType type);
/*

  HANDLING THE MAIN LINK
  
   Any outgoing link can at any time be the main destination.
   
 */
extern BOOL HTAnchor_setMainLink        (HTAnchor * anchor, HTLink * link);
extern HTLink * HTAnchor_mainLink       (HTAnchor * anchor);

extern HTAnchor * HTAnchor_followMainLink (HTAnchor * anchor);
/*

  HANDLING THE SUB LINKS
  
 */
extern BOOL HTAnchor_setSubLinks        (HTAnchor * anchor, HTList * list);
extern HTList * HTAnchor_subLinks       (HTAnchor * anchor);
/*

  MOVE LINK INFORMATION
  
   Move all link information form one anchor to another. This is useful when we get a
   redirection on a request and want to inherit the link information to the new anchor and
   change the link information in the old one to "redirect".
   
 */
extern BOOL HTAnchor_moveAllLinks       (HTAnchor *src, HTAnchor *dest);
/*

  REMOVE LINK INFORMATION
  
   Delete link information between two or more anchors
   
 */
extern BOOL HTAnchor_removeLink         (HTAnchor *src, HTAnchor *dest);
extern BOOL HTAnchor_removeAllLinks     (HTAnchor * me);
/*

  MOVE A CHILD ANCHOR TO THE HEAD OF THE LIST OF ITS SIBLINGS
  
   This is to ensure that an anchor which might have already existed is put in the correct
   order as we load the document.
   
 */
extern void HTAnchor_makeLastChild      (HTChildAnchor *me);
/*

Anchor Objects

   We have three variants of the Anchor object - I guess some would call them superclass
   and subclasses ;-)
   
  GENERIC ANCHOR TYPE
  
   This is the super class of anchors. We often use this as an argument to the functions
   that both accept parent anchors and child anchors. We separate the first link from the
   others to avoid too many small mallocs involved by a list creation. Most anchors only
   point to one place.
   
  ANCHOR FOR A PARENT OBJECT
  
   These anchors points to the whole contents of a graphic object (document). The parent
   anchor of a parent anchor is itself. The parent anchor now contains all meta
   information about the object. This is largely the entity headers in the HTTP
   specification.
   
  ANCHOR FOR A CHILD OBJECT
  
   A child anchor is a anchor object that points to a subpart of a graphic object
   (document)
   
Creation and Deletion Methods

   After we have defined the data structures we must define the methods that can be used
   on them. All anchors are kept in an internal hash table so that they are easier to find
   again.
   
  FIND/CREATE A PARENT ANCHOR
  
   This one is for a reference (link) which is found in a document, and might not be
   already loaded. The parent anchor returned can either be created on the spot or is
   already in the hash table.
   
 */
extern HTAnchor * HTAnchor_findAddress          (CONST char * address);
/*

  FIND/CREATE A CHILD ANCHOR
  
   This one is for a new child anchor being edited into an existing document. The parent
   anchor must already exist but the child returned can either be created on the spot or
   is already in the hash table. The tag is the part that's after the '#' sign in a URI.
   
 */
extern HTChildAnchor * HTAnchor_findChild       (HTParentAnchor *parent,
                                                 CONST char *   tag);
/*

  FIND/CREATE A CHILD ANCHOR AND LINK TO ANOTHER PARENT
  
   Find a child anchor anchor with a given parent and possibly a tag, and (if passed) link
   this child to the URI given in the href. As we really want typed links to the caller
   should also indicate what the type of the link is (see HTTP spec for more information).
   The link is relative to the address of the parent anchor.
   
 */
extern HTChildAnchor * HTAnchor_findChildAndLink
                (HTParentAnchor * parent,               /* May not be 0 */
                CONST char * tag,                       /* May be "" or 0 */
                CONST char * href,                      /* May be "" or 0 */
                HTLinkType ltype);                      /* May be 0 */
/*

  DELETE AN ANCHOR
  
   All outgoing links from parent and children are deleted, and this anchor is removed
   from the sources list of all its targets. We also delete the targets. If this anchor's
   source list is empty, we delete it and its children.
   
 */
extern BOOL HTAnchor_delete     (HTParentAnchor *me);
/*

  DELETE ALL ANCHORS
  
   Deletes all anchors and return a list of all the objects (hyperdoc) hanging of the
   parent anchors found while doing it. The application may keep its own list of
   HyperDocs, but this function returns it anyway.  It is always for the application to
   delete any HyperDocs. If NULL then no hyperdocs are returned. Return YES if OK, else
   NO.
   
   Note: This function is different from cleaning up the history list!
   
 */
extern BOOL HTAnchor_deleteAll  (HTList * objects);
/*

Access Methods of an Anchor

   These functions should be used to access information within the anchor structures.
   
  RELATIONS TO OTHER ANCHORS
  
    Who is Parent?
    
   For parent anchors this returns the anchor itself
   
 */
extern HTParentAnchor * HTAnchor_parent (HTAnchor *me);
/*

    Does it have any Anchors within it?
    
 */
extern BOOL HTAnchor_hasChildren        (HTParentAnchor *me);
/*

  BINDING A DATA OBJECT TO AN ANCHOR
  
   A parent anchor can have a data object bound to it. This data object does can for
   example be a parsed version of a HTML that knows how to present itself to the user, or
   it can be an unparsed data object. It's completely free for the application to use this
   possibility, but a typical usage would to manage the data object as part of a memory
   cache.
   
 */
extern void HTAnchor_setDocument        (HTParentAnchor *me, void * doc);
extern void * HTAnchor_document         (HTParentAnchor *me);
/*

  URI INFORMATION OF ANCHORS
  
   There are two addresses of an anchor. The URI that was passed when the anchor was
   crated and the physical address that's used when the URI is going to be requested. The
   two addresses may be different if the request is going through a proxy or a gateway.
   
    Get URI Address
    
   Returns the full URI of the anchor, child or parent as a malloc'd string to be freed by
   the caller as when the anchor was created.
   
 */
extern char * HTAnchor_address          (HTAnchor *me);
/*

  CACHE INFORMATION
  
   If the cache manager finds a cached object, it is registered in the anchor object. This
   way the file loader knows that it is a MIME data object. The cache manager does not
   know whether the data object is out of date (for example if a Expires: header is in the
   MIME header. This is for the MIME parser to find out.
   
 */
extern BOOL HTAnchor_cacheHit           (HTParentAnchor *me);
extern void HTAnchor_setCacheHit        (HTParentAnchor *me, BOOL cacheHit);
/*

  PHYSICAL ADDRESS
  
   Contains the physical address after we haved looked for proxies etc.
   
 */
extern char * HTAnchor_physical         (HTParentAnchor * me);

extern void HTAnchor_setPhysical        (HTParentAnchor * me, char * protocol);
/*

  IS THE ANCHOR SEARCHABLE?
  
 */
extern void HTAnchor_clearIndex         (HTParentAnchor *me);
extern void HTAnchor_setIndex           (HTParentAnchor *me);
extern BOOL HTAnchor_isIndex            (HTParentAnchor *me);
/*

  TITLE HANDLING
  
   We keep the title in the anchor as we then can refer to it later in the history list
   etc. We can also obtain the title element if it is passed as a HTTP header in the
   response. Any title element found in an HTML document will overwrite a title given in a
   HTTP header.
   
 */
extern CONST char * HTAnchor_title      (HTParentAnchor *me);

extern void HTAnchor_setTitle           (HTParentAnchor *me,
                                         CONST char *   title);

extern void HTAnchor_appendTitle        (HTParentAnchor *me,
                                         CONST char *   title);
/*

  MEDIA TYPES (CONTENT-TYPE)
  
 */
extern HTFormat HTAnchor_format         (HTParentAnchor *me);
extern void HTAnchor_setFormat          (HTParentAnchor *me,
                                         HTFormat       form);
/*

  CHARSET PARAMETER TO CONTENT-TYPE
  
 */
extern HTCharset HTAnchor_charset       (HTParentAnchor *me);
extern void HTAnchor_setCharset         (HTParentAnchor *me,
                                         HTCharset      charset);
/*

  LEVEL PARAMETER TO CONTENT-TYPE
  
 */
extern HTLevel HTAnchor_level           (HTParentAnchor * me);
extern void HTAnchor_setLevel           (HTParentAnchor * me,
                                         HTLevel        level);
/*

  CONTENT LANGUAGE
  
 */
extern HTLanguage HTAnchor_language     (HTParentAnchor *me);
extern void HTAnchor_setLanguage        (HTParentAnchor *me,
                                         HTLanguage     language);
/*

  CONTENT ENCODING
  
 */
extern HTEncoding HTAnchor_encoding     (HTParentAnchor *me);
extern void HTAnchor_setEncoding        (HTParentAnchor *me,
                                         HTEncoding     encoding);
/*

  CONTENT TRANSFER ENCODING
  
 */
extern HTCte HTAnchor_cte               (HTParentAnchor *me);
extern void HTAnchor_setCte             (HTParentAnchor *me,
                                         HTCte          cte);
/*

  CONTENT LENGTH
  
 */
extern long int HTAnchor_length         (HTParentAnchor *me);
extern void HTAnchor_setLength          (HTParentAnchor *me,
                                         long int       length);
/*

  ALLOWED METHODS (ALLOW)
  
 */
extern int HTAnchor_methods             (HTParentAnchor *me);
extern void HTAnchor_setMethods         (HTParentAnchor *me,
                                         int            methodset);
extern void HTAnchor_appendMethods      (HTParentAnchor *me,
                                         int            methodset);
/*

  VERSION
  
 */
extern char * HTAnchor_version  (HTParentAnchor * me);
extern void HTAnchor_setVersion (HTParentAnchor * me, CONST char * version);
/*

  DATE
  
   Returns the date that was registered in the RFC822 header "Date"
   
 */
extern time_t HTAnchor_date             (HTParentAnchor * me);
extern void HTAnchor_setDate            (HTParentAnchor * me, CONST time_t date);
/*

  LAST MODIFIED DATE
  
   Returns the date that was registered in the RFC822 header "Last-Modified"
   
 */
extern time_t HTAnchor_lastModified     (HTParentAnchor * me);
extern void HTAnchor_setLastModified    (HTParentAnchor * me, CONST time_t lm);
/*

  EXPIRES DATE
  
 */
extern time_t HTAnchor_expires          (HTParentAnchor * me);
extern void HTAnchor_setExpires         (HTParentAnchor * me, CONST time_t exp);
/*

  DERIVED FROM
  
 */
extern char * HTAnchor_derived  (HTParentAnchor *me);
extern void HTAnchor_setDerived (HTParentAnchor *me, CONST char *derived_from);
/*

  EXTRA HEADERS
  
   List of unknown headers coming in from the network. Do not use the HTAnchor_addExtra()
   function to extra headers here, but use the field in the request structure for sending
   test headers.
   
 */
extern HTList * HTAnchor_Extra          (HTParentAnchor *me);
extern void HTAnchor_addExtra           (HTParentAnchor *me,
                                         CONST char *   header);
/*

  STATUS OF HEADER PARSING
  
   These are primarily for internal use
   
 */
extern BOOL HTAnchor_headerParsed       (HTParentAnchor *me);
extern void HTAnchor_setHeaderParsed    (HTParentAnchor *me);
/*

  WE WANT TO CLEAR THE HEADER INFORMATION...
  
 */
extern void HTAnchor_clearHeader        (HTParentAnchor *me);

#endif /* HTANCHOR_H */
/*

    */
