/*                                                                              Anchor Object
                               THE ANCHOR AND LINK OBJECTS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is the private part of the anchor object. It has the functions declarations
   that are private to the Library and that shouldn't be used by applications. See also
   the public part of the declarition in the HTAnchorModule.
   
 */
#ifndef HTANCMAN_H
#define HTANCMAN_H

#include "HTAnchor.h"
#include "HTList.h"
#include "HTAtom.h"
#include "HTMethod.h"
/*

The Link Object

   Link Objects bind together anchor objects
   
 */
struct _HTLink {
    HTAnchor *          dest;              /* The anchor to which this leads */
    HTLinkType          type;                      /* Semantics of this link */
    HTMethod            method;            /* Method for this link, e.g. PUT */
    HTLinkResult        result;    /* Result of any attempt to get this link */
};
/*

The Anchor Object

   We have a set of Anchor objects which of course should be sub classes - but - hey -
   this is C - what do you expect!
   
  GENERIC ANCHOR TYPE
  
   This is the super class of anchors. We often use this as an argument to the functions
   that both accept parent anchors and child anchors. We separate the first link from the
   others to avoid too many small mallocs involved by a list creation. Most anchors only
   point to one place.
   
 */
struct _HTAnchor {
  HTLink        mainLink;       /* Main (or default) destination of this */
  HTList *      links;          /* List of extra links from this, if any */
  HTParentAnchor * parent;      /* Parent of this anchor (self for adults) */
};
/*

  ANCHOR FOR A PARENT OBJECT
  
   These anchors points to the whole contents of a graphic object (document). The parent
   anchor of a parent anchor is itself. The parent anchor now contains all meta
   information about the object. This is largely the entity headers in the HTTP
   specification.
   
 */
struct _HTParentAnchor {
  /* Common part from the generic anchor structure */
  HTLink        mainLink;       /* Main (or default) destination of this */
  HTList *      links;          /* List of extra links from this, if any */
  HTParentAnchor * parent;      /* Parent of this anchor (self) */

  /* ParentAnchor-specific information */
  HTList *      children;       /* Subanchors of this, if any */
  HTList *      sources;        /* List of anchors pointing to this, if any */
  void *        document;       /* The document within this is an anchor */
  char *        physical;       /* Physical address */
  BOOL          cacheHit;       /* Yes, if cached object found */
  char *        address;        /* Absolute address of this node */
  BOOL          isIndex;        /* Acceptance of a keyword search */

  /* Entity header fields */
  BOOL          header_parsed;  /* Are we done parsing? */

  char *        title;
  int           methods;        /* Allowed methods (bit-flag) */

  HTEncoding    content_encoding;
  HTLanguage    content_language;       /* @@@ SHOULD BE LIST @@@ */
  long int      content_length;
  HTCte         cte;            /* Content-Transfer-Encoding */
  HTFormat      content_type;
  HTCharset     charset;        /* Parameter to content-type */
  HTLevel       level;          /* Parameter to content-type `text/html' */

  time_t        date;           /* When was the request issued */
  time_t        expires;        /* When does the copy expire */
  time_t        last_modified;

  char *        derived_from;   /* Opaque string */
  char *        version;        /* Opaque string */

  /* List of unknown headers coming in from the network. Use the field in the
     request structure for sending test headers. */
  HTList *      extra_headers;
};
/*

  ANCHOR FOR A CHILD OBJECT
  
   A child anchor is a anchor object that points to a subpart of a graphic object
   (document)
   
 */
struct _HTChildAnchor {
  /* Common part from the generic anchor structure */
  HTLink        mainLink;       /* Main (or default) destination of this */
  HTList *      links;          /* List of extra links from this, if any */
  HTParentAnchor * parent;      /* Parent of this anchor */

  /* ChildAnchor-specific information */
  char *        tag;            /* Address of this anchor relative to parent */
};
/*

 */
#endif /* HTANCMAN_H */
/*

    */
