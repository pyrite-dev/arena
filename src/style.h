#ifndef _ARENA_STYLE_H_
#define _ARENA_STYLE_H_

#ifndef _ARENA_WWW_H_
#include "www.h"
#endif /* _ARENA_WWW_H_ */

extern BOOL StyleGetFlag(int);
extern void StyleSetFlag(int, BOOL);
extern void FreeStyleSheet(StyleSheet *);
extern void StyleParse();
extern void StyleZoomChange(double);
extern char *StyleLoad(char *, int, BOOL);
extern long StyleGet(StyleInternalProperty);
extern StyleSheet *StyleGetInit();
extern void StyleClearDoc();
extern void FormatElementStart(int, char *, int);
extern void FormatElementEnd();
extern StyleSheet *StyleCopy(StyleSheet *);

#endif /* _ARENA_STYLE_H_ */
