/*                                                                        Dynamic Array Class
                                   DYNAMIC ARRAY CLASS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module implements a flexible array. It is a general utility module.  A chunk is a
   structure which may be extended.  These routines create and append data to chunks,
   automatically reallocating them as necessary.  It is garanteed that the array is '\0'
   terminated at all times, so the terminating function, HTChunkTerminate is only
   necessary to adjust the size in the chunk structure (the '\0' counts as a character
   when counting the size of the chunk.
   
   This module is implemented by HTChunk.c, and it is a part of the  W3C Reference
   Library.
   
   NOTE The names without a "_" (made as a #define's) are only provided for backwards
   compatibility and should not be used.
   
 */
#ifndef HTCHUNK_H
#define HTCHUNK_H

/*

Private Data Structure

   This structure should not be referenced outside this module!
   
 */
typedef struct {
        int     size;           /* In bytes                     */
        int     growby;         /* Allocation unit in bytes     */
        int     allocated;      /* Current size of *data        */
        char *  data;           /* Pointer to malloced area or 0 */
} HTChunk;
/*

Create new chunk

   Create a new chunk and specify the number of bytes to allocate at a time when the chunk
   is later extended. Arbitrary but normally a trade-off time vs. memory
   
 */
#define HTChunkCreate(growby) HTChunk_new(growby)
extern HTChunk * HTChunk_new (int growby);
/*

Free a chunk

   Free a chunk created by HTChunkCreatefrom memory
   
 */
#define HTChunkFree(ch) HTChunk_delete(ch)
extern void HTChunk_delete (HTChunk * ch);
/*

Clear a chunk

   Keep the chunk in memory but clear all data kept inside.
   
 */
#define HTChunkClear(ch) HTChunk_clear(ch)
extern void HTChunk_clear (HTChunk * ch);
/*

Ensure a chunk has a certain space in

   Make sure that a chunk has a certain size. If this is not the case then the chunk is
   expanded. Nothing is done if the current size if bigger than the size requested.
   
 */
#define HTChunkEnsure(ch, s) HTChunk_ensure(ch, s)
extern void HTChunk_ensure (HTChunk * ch, int s);
/*

Append a character to a chunk

   Add the character and increment the size of the chunk by one character
   
 */
#define HTChunkPutc(ch, c) HTChunk_putc(ch, c)
extern void HTChunk_putc (HTChunk * ch, char c);
/*

Append a string to a  chunk

   Add the string and increment the size of the chunk by the length of the string (without
   the trailing zero)
   
 */
#define HTChunkPuts(ch, str) HTChunk_puts(ch, str)
extern void HTChunk_puts (HTChunk * ch, CONST char *str);
/*

Append a block to a chunk

   Add the block and increment the size of the chunk by the len
   
 */
extern void HTChunk_putb (HTChunk * ch, CONST char *block, int len);
/*

Zero Terminate a chunk

   As a chunk often is a dynamic string, it needs to be terminated by a zero in order to
   be used in C. However, by default any chunk is always zero terminated, so the only
   purpose of this function is to increment the size counter with one corresponding to the
   zero.
   
 */
#define HTChunkTerminate(ch)    HTChunk_terminate(ch)
#define HTChunk_terminate(ch)   HTChunk_putc((ch), '\0')
/*

Return Pointer to Data

   This define converts a chunk to a normal char pointer so that it can be parsed to any
   ANSI C string function.
   
 */
#define HTChunkData(me)         ((me) ? (me)->data : NULL)
#define HTChunk_data(me)         ((me) ? (me)->data : NULL)
/*

Return Current Size

   Returns the current size of the chunk
   
 */
#define HTChunkSize(me)         ((me) ? (me)->size : -1)
#define HTChunk_size(me)         ((me) ? (me)->size : -1)
/*

 */
#endif
/*

   End of Declaration  */
