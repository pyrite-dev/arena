/*                                                                          Association Pairs
                      ASSOCIATION LIST FOR STORING NAME-VALUE PAIRS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   Lookups from association list are not case-sensitive.
   
   This module is implemented by HTAssoc.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTASSOC_H
#define HTASSOC_H

#include "HTList.h"

typedef HTList HTAssocList;

typedef struct {
    char * name;
    char * value;
} HTAssoc;

extern HTAssocList *HTAssocList_new (void);
extern BOOL HTAssocList_delete  (HTAssocList * alist);

extern BOOL HTAssocList_add     (HTAssocList * alist,
                                 CONST char * name, CONST char * value);

extern char *HTAssocList_lookup (HTAssocList * alist, CONST char * name);

#endif /* not HTASSOC_H */
/*

   End of file HTAssoc.h. */
