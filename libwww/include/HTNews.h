/*                                                                                NNTP Access
                                       NEWS ACCESS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the NEWS load module that handles all communication with NEWS-servers.
   
   This module is implemented by HTNews.c, and it is a part of the W3C Reference Library.
   
 */
#ifndef HTNEWS_H
#define HTNEWS_H
#include "HTEvntrg.h"
/*

   We define the max NNTP line as rather long as the result coming from a XOVER command
   needs it. If the line is longer then it is chopped, but we will almost have received
   the information we're looking for.
   
 */
#define MAX_NEWS_LINE           512

extern HTEventCallback HTLoadNews;
/*

Setting Number of Articles to Show

   You can set the number of news articles to be shown at a time. If you set the number to
   none (0) then all articles are shown at once. This is also the default behavior.
   
 */
extern BOOL HTNews_setMaxArticles (int new_max);
extern int HTNews_maxArticles (void);
/*

Handling the News Host

   The default news host is "news" but you can get ans set the value here.
   
 */
extern BOOL HTNews_setHost (CONST char * newshost);
extern CONST char *HTNews_host (void);
extern void HTFreeNewsHost (void);
/*

 */
#endif /* HTNEWS_H */
/*

   End of News declaration  */
