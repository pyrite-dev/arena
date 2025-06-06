/*                                                                             History module
                                     HISTORY MANAGER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is a simple history module for a WWW client.  It keeps a linear history, with a
   destructive or non-destructive backtrack, and list sequencing (previous, next)
   operations.
   
   If you are building a client, you don't have to use this: just don't call it.  This
   module is not used by any other modules in the libwww, so if you don't refer to it you
   don't get it in your linked application.
   
   This module is implemented by HTHist.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTHISTORY_H
#define HTHISTORY_H

#include "HTAnchor.h"
/*

Creation and Deletion Methods

   The history module can handle multiple history lists which can be useful in a
   multithreaded environment with several open windows in the client application. A new
   histrory lidt is referenced by the handle returned from the creation method.
   
 */
typedef struct _HTHistory HTHistory;

extern HTHistory *HTHistory_new         (void);
extern BOOL HTHistory_delete            (HTHistory *old);
/*

Add and delete History Elements

  RECORD AN ENTRY IN A LIST
  
   Registers the object in the linear list. The first entry is the home page. No check is
   done for duplicates.  Returns YES if ok, else NO
   
 */
extern BOOL HTHistory_record            (HTHistory *hist, HTAnchor *cur);
/*

  REPLACE LIST WITH NEW ELEMENT
  
   Iserts the new element at the current position and removes all any old list from
   current position. For example if c is cur pos
   
      before: a b c d e
      
      after : a b f
      
   Returns YES if ok, else NO
   
 */
extern BOOL HTHistory_replace           (HTHistory *hist, HTAnchor *cur);
/*

  DELETE LAST ENTRY IN A LIST
  
   Deletes the last object from the list Returns YES if OK, else NO
   
 */
extern BOOL HTHistory_removeLast        (HTHistory *hist);
/*

  REMOVE THE HISTORY LIST FROM POSITION
  
   Deletes the history list FROM the entry at position 'cur' (excluded). Home page has
   position 1.  Returns YES if OK, else NO
   
 */
extern BOOL HTHistory_removeFrom        (HTHistory *hist, int pos);
/*

  NUMBER OF ELEMENTS STORED
  
   Returns the size of the history list or -1 if none.
   
 */
extern int HTHistory_count              (HTHistory *hist);
/*

  CURRENT POSITION
  
   Returns the current position or -1 on error
   
 */
extern int HTHistory_position           (HTHistory *hist);
/*

  FIND AND RE-REGISTER VISITED ANCHOR
  
   Finds already registered anchor at given position and registers it again EXCEPT if last
   entry. This allows for `circular' history lists with duplicate entries. Position 1 is
   the home anchor. The current position is updated.
   
 */
extern HTAnchor * HTHistory_recall      (HTHistory *hist, int pos);
/*

  FIND ENTRY AT POSITION
  
   Entry with the given index in the list (1 is the home page). Like HTHistory_recall but
   without re-registration. Un success, the current position is updated to the value 'pos'
   value.
   
 */
extern HTAnchor * HTHistory_find        (HTHistory *hist, int pos);
/*

  LIST THE HISTORY LIST
  
   This function is like HTHistory_find() but does not update the current position
   
 */
extern HTAnchor * HTHistory_list        (HTHistory *hist, int pos);
/*

Navigation

  CAN WE BACK IN HISTORY
  
   Returns YES if the current anchor is not the first entry (home page)
   
 */
extern BOOL HTHistory_canBacktrack      (HTHistory *hist);
/*

  BACKTRACK WITH DELETION
  
   Returns the previous object and erases the last object. This does not allow for
   'forward' as we are always at the end of the list. If no previous object exists, NULL
   is returned so that the application knows that no previous object was found. See also
   HTHistory_back().
   
 */
extern HTAnchor * HTHistory_backtrack   (HTHistory *hist);
/*

  BACKTRACK WITHOUT DELETION
  
   Returns the previos object but does not erase the last object. This does not allow for
   'forward'. If no previous object exists, NULL is returned so that the application knows
   that no previous object was found. See also HTHistory_backtrack()
   
 */
extern HTAnchor * HTHistory_back        (HTHistory *hist);
/*

  CAN WE GO FORWARD
  
   Returns YES if the current anchor is not the last entry
   
 */
extern BOOL HTHistory_canForward        (HTHistory *hist);
/*

  FORWARD
  
   Return the next object in the list or NULL if none
   
 */
extern HTAnchor * HTHistory_forward     (HTHistory *hist);

#endif /* HTHISTORY_H */
/*

    */
