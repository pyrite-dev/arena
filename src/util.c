
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "www.h"
 
/* duplicate  string s and ensure it is zero terminated
   where n is the maximum number of chars copied from s */

void HTList_destroy(HTList *me)
{
  HTList *current;
  while ((current = me)) {
    if (me->object) {            /* howcome 14/2/95 */
	Free(me->object);
    }
    me = me->next;
    Free (current);
  }
}

/* "first" is temporal, not logical */


void HTList_addObjectFirst (HTList *me, void *newObject) /* howcome 26/1/95 */
{
  while (me->next) {
    me = me->next;
  }

  if (me) {
    HTList *newNode = (HTList *)malloc (sizeof (HTList));
    if (newNode == NULL) outofmem(__FILE__, "HTList_addObject");
    newNode->object = newObject;
    newNode->next = NULL;
    me->next = newNode;
  }
}



char *www_strndup(char *s, int n)
{
    char *pp;

    pp = (char *)malloc(n+1);

    if (!pp)
    {
        Beep();
        fprintf(stderr, "ERROR: malloc failed, memory exhausted!\n");
        exit(1);
    }

    strncpy(pp, s, n);
    pp[n] = '\0';

    return pp;
}

/* str_tok: a reentrant strtok */

char * str_tok(char *a, char *b, char **c)
{
    char *d;
    char *start, *stop;

    if (!b)
        return(NULL);

    d = (a) ? a : *c;
    if (!d)
	return NULL;
    while (*d && strchr(b,(int)*d))
        d++;
    start = d;
    while (*d && !strchr(b,(int)*d))
        d++;
    stop = d;
    if (start == stop)
       return(NULL);

    if (*d == '\0') {
	*c = d;
	return (start);
    }
	
    *d = '\0';
    d++;
    *c = d;
    return(start);
}


char *chop_str(char *p)
{
    char *p_start, *p_end;	/* janet 21/07/95: not used: *p_return */

    if (p==NULL)
        return NULL;

    p_start = p;
    while (isspace((int)*p_start))
        p_start++;

    p_end = p + strlen(p) - 1;

    while(isspace((int)*p_end)) {
	p_end--;
    }

    /* how should we handle empty stuff?? */

    if (p_start > p_end)
	return (strdup(p));
    else
	return((char *)www_strndup(p_start, p_end - p_start + 1));
}

Byte hex2byte(char c)
{
    if ((c <= '9') && (c >= '0'))
	return((int)(c - '0'));

    if ((c <= 'F') && (c >= 'A'))
	return((int)(c - 'A' + 10));

    if ((c <= 'f') && (c >= 'a'))
	return((int)(c - 'a' + 10));

    return 0;
}

#ifdef NO_STRERROR   /* most SVR4 machines */
char *strerror(int a)
{
    return NULL;
}
#endif

#ifdef NO_STRDUP   /* utlrix */

char *strdup(char *s)
{
    char * n;

    if (!s)
	return NULL;
    
    n = (char *) malloc (strlen(s) + 1);
    if (!n)
	return NULL;

    strcpy(n,s);
    return(n);
}
#endif
