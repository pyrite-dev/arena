#ifndef _ARENA_FORM_H_
#define _ARENA_FORM_H_

#ifndef _ARENA_WWW_H_
#include "www.h"
#endif /* _ARENA_WWW_H_ */

extern void PaintFieldCursorBuffer(GC, unsigned long, EditorBuffer *, Field *);
extern Form *FindForm(Form *,char *);
extern Field *FindField(Form *,int , char *, int);
extern int FieldCount(Form *);
extern void FreeForms(void);
extern Form *GetForm(int, char *, int);
extern Form *DefaultForm(void);
extern Field *GetField(Form *, int, int, char *, int, char *, int, int, int, int);
extern void AddOption(Field *, int, char *, int);
extern char *WWWEncode(char *); 
extern void SubmitForm(Form *, int, char *, int, int,  char *, int, char *, int, Image *, int, int, char *); 

/* ... */



#endif /* _ARENA_FORM_H_ */


