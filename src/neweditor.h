#ifndef _ARENA_EDITOR_H_
#define _ARENA_EDITOR_H_

#include "types.h"

extern EditorBuffer *CreateBuffer();
extern cell *CreateCell();
extern void FreeCell(cell *);
extern void FreeBuffer(EditorBuffer *);
extern void SplitCell(cell *);
extern void MergeCell(cell *);
extern int DeleteChar(EditorBuffer *, int);
extern int InsertChar(EditorBuffer *, int, char);
extern int InsertnChar(EditorBuffer *, int, char *, int);
extern int InsertString(EditorBuffer *, int , char *);
extern int AppendChar(EditorBuffer *, char);
extern int AppendnChar(EditorBuffer *, char *, int);
extern int AppendString(EditorBuffer *, char *);
extern EditorBuffer *Str2Buffer(char *);
extern char *Buffer2Str(EditorBuffer *);
extern int LineNumber(EditorBuffer *);
extern int ColNumber(EditorBuffer *);
extern void NextLine(EditorBuffer *);
extern void PrevLine(EditorBuffer *);

#endif /* _ARENA_EDITOR_H_ */








