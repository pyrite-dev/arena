/*                                                                      Generic News Browsing
                                  GENERIC NEWS BROWSING
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This news browser generates listings for NNTP reponses. This module contains the
   protocol independent code and it produces the HTML object. It is only included if the
   NNTP.
   
   This module is implemented by HTNDir.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTNDIR_H
#define HTNDIR_H
#include "HTReq.h"
/*

What Should the Listings Look Like?

   You can set how the listings are to be sorted using the following flags.
   
 */
typedef enum _HTNewsDirKey {
    HT_NDK_NONE         = 0,                    /* Unsorted */
    HT_NDK_INDEX        = 1,                    /* Sort by Message Number */
    HT_NDK_DATE         = 2,                    /* Sort by date */
    HT_NDK_SUBJECT      = 3,                    /* Sort by Subject */
    HT_NDK_FROM         = 4,                    /* Sort by Sender */
    HT_NDK_GROUP        = 5                     /* Sort group listing */
} HTNewsDirKey;
/*

The News Listing Object

   The news listing object handles the generation of a news listing. It is a bit like a
   stream in that it accept data, but it must be formatted in a special way which makes
   the normal stream architecture inadequate.
   
  CREATE A NEWS LISTING OBJECT
  
   Creates a structured stream object and sets up the initial HTML stuff Returns the dir
   object if OK, else NULL
   
 */
typedef struct _HTNewsDir HTNewsDir;
extern HTNewsDir * HTNewsDir_new (HTRequest * request, CONST char * title,
                                  HTNewsDirKey key);
/*

  ADD A LINE TO THE LIST
  
   This function accepts a news line. Everything except dir and nama can can be 0 or NULL.
   Returns YES if OK, else NO
   
 */
extern BOOL HTNewsDir_addElement (HTNewsDir * dir, int index, char * subject,
                                  char * from, time_t date, char * name,
                                  int refs);
/*

  FREE A NEWS LISTING OBJECT
  
   If we are sorting then do the sorting and put out the list, else just append the end of
   the list.
   
 */
extern BOOL HTNewsDir_free (HTNewsDir * dir);
/*

 */
#endif /* HTNDIR */
/*

   End of declaration */
