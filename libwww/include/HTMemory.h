/*                                                                          Memory Management
                                 DYNAMIC MEMORY INTERFACE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module implements a dynamic memory API thata is used in the Library. There are
   three situations that are covered by this module:
   
      Handling of allocation and deallocation of dynamic memory
      
      Recovering from temporary lack of available memory
      
      Panic handling in case a new allocation fails
      
   This module is implemented by HTMemory.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTMEMORY_H
#define HTMEMORY_H
/*

Dynamic Memory Allocation and Deallocation

   These are the functions for handling dynamic allocation and deallcation.
   
 */
extern void* HTMemory_malloc(size_t size);
extern void* HTMemory_calloc(size_t count, size_t size);
extern void* HTMemory_realloc(void * ptr, size_t size);
extern void HTMemory_free(void* ptr);
/*

  MEMORY MACROS
  
   We use the following set of macros throughout the code. If you don't wany any memory
   management beyond normal malloc and alloc then you can just use that instead of the
   HTMemory_* function.
   
 */
#ifndef __FILE__
#define __FILE__ ""
#endif

#ifndef __LINE__
#define __LINE__ 0L
#endif

#define HT_MALLOC(size)         HTMemory_malloc(size)
#define HT_CALLOC(count, size)  HTMemory_calloc(count, size)
#define HT_REALLOC(ptr, size)   HTMemory_realloc(ptr, size)
#define HT_FREE(pointer)        {HTMemory_free(pointer);(pointer)=NULL;}
/*

Memory Freer Functions

   The dynamic memory freer functions are typically functions that are capable of freeing
   large chunks of memory. In case a new allocation fails, the allocation method looks for
   any freer functions to call. There can be multriple freer functions and after each
   call, the allocation method tries again to allocate the desired amount of dynamic
   memory. The freer functions are called in reverseorder meaning that the last one
   registered gets called first. That way, it is easy to add temporary free functions
   which then are guaranteed to be called first if a methods fails.
   
  ADD A FREER FUNCTION
  
   You can add a freer function by using the following method. The Library itself
   registeres a set of free functions during initialization. If the application does not
   register any freer functions then the Library looks how it can free internal memory.
   
 */
typedef void HTMemoryCallback(size_t size);

extern BOOL HTMemoryCall_add (HTMemoryCallback * cbf);
/*

  DELETE A FREER FUNCTION
  
   Freer functions can be deleted at any time in which case they are not called anymore.
   
 */
extern BOOL HTMemoryCall_delete (HTMemoryCallback * cbf);
extern BOOL HTMemoryCall_deleteAll (void);
/*

Panic Handling

   If the freer functions are not capable of deallocation enough memory then the
   application must have an organized way of closing down. This is done using the panic
   handler. In the libwww, each allocation is  tested and HT_OUTOFMEM is called if a NULL
   was returned. HT_OUTOFMEM is a macro which calls HTMemory_outofmem. This function calls
   an exit function defined by the app in a call to HTMemory_setExit. If the app has not
   defined this function, HTMemory_outofmem TTYPrints the error message and calls exit(1).
   
 */
typedef void HTMemory_exitCallback(char *name, char *file, unsigned long line);

extern void HTMemory_setExit(HTMemory_exitCallback * pExit);
extern HTMemory_exitCallback * HTMemory_exit(void);
/*

  CALL THE EXIT HANDLER
  
   If an allocation fails then this function is called. If the application has registered
   it's own panic handler then this is called diretly from this function. Otherwise, the
   default behavior is to write a small message to stderr and then exit.
   
 */
#define outofmem(file, name)    HT_OUTOFMEM(name)
#define HT_OUTOFMEM(name)       HTMemory_outofmem((name), __FILE__, __LINE__)

extern void HTMemory_outofmem(char * name, char * file, unsigned long line);
/*

 */
#endif /* HTMEMORY_H */
/*

   End of declaration */
