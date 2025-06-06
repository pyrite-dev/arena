/*                                                                    Generic String Handling
                                GENERIC STRING MANAGEMENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   These functions provide functionality for case-independent string comparison and
   allocations with copies etc.
   
   This module is implemented by HTString.c, and it is a part of the W3C Reference
   Library.
   
 */
#ifndef HTSTRING_H
#define HTSTRING_H
/*

Dynamic String Manipulation

   These two functions are dynamic versions of strcpy and strcat. They use malloc for
   allocating space for the string. If StrAllocCopy is called with a non-NULL dest, then
   this is freed before the new value is assigned so that only the last string created has
   to be freed by the user. If StrAllocCat is called with a NULL pointer as destination
   then it is equivalent to StrAllocCopy.
   
 */
#define StrAllocCopy(dest, src) HTSACopy (&(dest), src)
#define StrAllocCat(dest, src)  HTSACat  (&(dest), src)

extern char * HTSACopy (char **dest, CONST char *src);
extern char * HTSACat  (char **dest, CONST char *src);
/*

Case-insensitive String Comparison

   The usual routines (comp instead of cmp) had some problem.
   
 */
extern int strcasecomp  (CONST char *a, CONST char *b);
extern int strncasecomp (CONST char *a, CONST char *b, int n);
/*

String Comparison with Wild Card Match

   String comparison function for file names with one wildcard * in the template.
   Arguments are:
   
  tmpl                   is a template string to match the name against.  agaist, may
                         contain a single wildcard character * which matches zero or more
                         arbitrary characters.
                         
  name                   is the name to be matched agaist the template.
                         
   Returns empty string ("") if perfect match, pointer to part matched by wildcard if any,
   or NULL if no match. This is basically the same as YES if match, else NO.
   
 */
extern char * HTStrMatch        (CONST char * tmpl, CONST char * name);
extern char * HTStrCaseMatch    (CONST char * tmpl, CONST char * name);
/*

Case-insensitive strstr

   This works like strstr() but is not case-sensitive.
   
 */
extern char * www_strcasestr (char * s1, char * s2);
/*

Strip white space off a string

   Return value points to first non-white character, or to '/0' if none. All trailing
   white space is OVERWRITTEN with zero.
   
 */
extern char * HTStrip (char * s);
/*

 */
#endif
/*

   End of declaration module  */
