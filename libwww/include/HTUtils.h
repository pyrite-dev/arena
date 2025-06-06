/*                                                                             Utility macros
                                  GENERAL PURPOSE MACROS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is a part of the  W3C Reference Library. See also the system dependent file
   tcp module for system specific information.
   
 */
#ifndef HTUTILS_H
#define HTUTILS_H
/*

Are we using Standard Code?

   We hopefully are...
   
 */
#if defined(__STDC__) || defined(__cplusplus) || defined(WWW_MSWINDOWS)
#define _STANDARD_CODE_
#endif
/*

Debug Message Control

   This is the global flag for setting the WWWTRACE options. The verbose mode is no longer
   a simple boolean but a bit field so that it is possible to see parts of the output
   messages.
   
 */
#ifndef DEBUG
#define DEBUG   /* No one ever turns this off as trace is too important */
#endif
/*

  DEFINITION OF THE GLOBAL TRACE FLAG
  
   The global trace flag variable is available everywhere.
   
 */
#ifdef DEBUG
#ifdef WWW_WIN_DLL
extern int *            WWW_TraceFlag;   /* In DLLs, we need the indirection */
#define WWWTRACE        (*WWW_TraceFlag)
#else
extern int              WWW_TraceFlag;       /* Global flag for all W3 trace */
#define WWWTRACE        (WWW_TraceFlag)
#endif /* WWW_WIN_DLL */
#else
#define WWWTRACE        0
#endif /* DEBUG */
/*

   The WWWTRACE define outputs messages if verbose mode is active according to the
   following rules:
   
 */
typedef enum _HTTraceFlags {
    SHOW_UTIL_TRACE     = 0x1,                          /*                 1 */
    SHOW_APP_TRACE      = 0x2,                          /*                10 */
    SHOW_CACHE_TRACE    = 0x4,                          /*               100 */
    SHOW_SGML_TRACE     = 0x8,                          /*              1000 */
    SHOW_BIND_TRACE     = 0x10,                         /*            1.0000 */
    SHOW_THREAD_TRACE   = 0x20,                         /*           10.0000 */
    SHOW_STREAM_TRACE   = 0x40,                         /*          100.0000 */
    SHOW_PROTOCOL_TRACE = 0x80,                         /*         1000.0000 */
    SHOW_MEM_TRACE      = 0x100,                        /*       1.0000.0000 */
    SHOW_URI_TRACE      = 0x200,                        /*      10.0000.0000 */
    SHOW_ANCHOR_TRACE   = 0x800,                        /*   10.00.0000.0000 */
    SHOW_ALL_TRACE      = 0xFFF                         /*   11.11.1111.1111 */
} HTTraceFlags;
/*

   The flags are made so that they can serve as a group flag for correlated trace
   messages, e.g. showing messages for SGML and HTML at the same time.
   
 */
#define UTIL_TRACE      (WWWTRACE & SHOW_UTIL_TRACE)
#define APP_TRACE       (WWWTRACE & SHOW_APP_TRACE)
#define CACHE_TRACE     (WWWTRACE & SHOW_CACHE_TRACE)
#define SGML_TRACE      (WWWTRACE & SHOW_SGML_TRACE)
#define BIND_TRACE      (WWWTRACE & SHOW_BIND_TRACE)
#define THD_TRACE       (WWWTRACE & SHOW_THREAD_TRACE)
#define STREAM_TRACE    (WWWTRACE & SHOW_STREAM_TRACE)
#define PROT_TRACE      (WWWTRACE & SHOW_PROTOCOL_TRACE)
#define MEM_TRACE       (WWWTRACE & SHOW_MEM_TRACE)
#define URI_TRACE       (WWWTRACE & SHOW_URI_TRACE)
#define ANCH_TRACE      (WWWTRACE & SHOW_ANCHOR_TRACE)
/*

  DESTINATION FOR TRACE MESSAGES
  
   You can send trace messages to various destinations depending on the type of your
   application. By default, on Unix the messages are sent to stderr using fprintf() and if
   we are on Windows and have a windows applications then use the TTYPrint function.
   
 */
#define WWWTRACE_FILE   1               /* Output to file */
#define WWWTRACE_STDERR 2               /* Output to stderr */
#define WWWTRACE_TTY    3               /* Output to TTY */

#ifndef WWWTRACE_MODE
#ifdef WWW_WIN_WINDOW
#define WWWTRACE_MODE WWWTRACE_TTY
#else
#define WWWTRACE_MODE WWWTRACE_STDERR
#endif
#endif

#if WWWTRACE_MODE == WWWTRACE_FILE
extern FILE *WWWTrace;
#ifndef HT_TRACE_FILE
#define HT_TRACE_FILE   "WWWTRACE.TXT"
#endif
#define TTYPrint fprintf
#define TDEST    WWWTrace
#endif

#if WWWTRACE_MODE == WWWTRACE_STDERR
#define TTYPrint fprintf
#define TDEST    stderr
#endif

#if WWWTRACE_MODE == WWWTRACE_TTY
#ifdef WWW_WIN_WINDOW
/* standard windows 3.x and non-console WIN32 programs */
#define TDEST    0
#ifdef WWW_WIN_DLL
typedef int TTYPrint_t(unsigned int target, const char* fmt, ...);
extern TTYPrint_t** PTTYPrint;
#define TTYPrint (**PTTYPrint)
#else
int TTYPrint(unsigned int target, const char* fmt, ...);
#endif
#else
/* if there is a real console, us it */
#define TDEST    stderr
int TTYPrint(FILE* target, const char* fmt, ...);
#endif
#endif
/*

Standard C library for malloc() etc

   Replace memory allocation and free C RTL functions with VAXC$xxx_OPT alternatives for
   VAXC (but not DECC) on VMS. This makes a big performance difference. (Foteos Macrides).
   Also have a look at the Dynamic Memory Module for how to handle malloc and calloc.
   
 */
#ifdef vax
#ifdef unix
#define ultrix  /* Assume vax+unix=ultrix */
#endif
#endif

#ifndef VMS
#ifndef ultrix
#ifdef NeXT
#include <libc.h>       /* NeXT */
#endif

#ifndef Mips
#ifndef MACH /* Vincent.Cate@furmint.nectar.cs.cmu.edu */
#include <stdlib.h>     /* ANSI */
#endif
#endif

#else /* ultrix */
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>   /* ANSI */   /* BSN */
#endif

#else   /* VMS */
#include <stdio.h>
#include <stdlib.h>
#include <unixlib.h>
#include <ctype.h>
#if defined(VAXC) && !defined(__DECC)
#define malloc  VAXC$MALLOC_OPT
#define calloc  VAXC$CALLOC_OPT
#define free    VAXC$FREE_OPT
#define cfree   VAXC$CFREE_OPT
#define realloc VAXC$REALLOC_OPT
#endif /* VAXC but not DECC */
#define unlink remove
#define gmtime localtime
#include <stat.h>
#define S_ISDIR(m)      (((m)&S_IFMT) == S_IFDIR)
#define S_ISREG(m)      (((m)&S_IFMT) == S_IFREG)
#define putenv HTVMS_putenv
#endif
/*

Macros for Function Declarations

 */
#define PUBLIC                  /* Accessible outside this module     */
#define PRIVATE static          /* Accessible only within this module */

#ifdef _STANDARD_CODE_

#if defined(sco) && !defined(_SCO_DS)
#define CONST             /* The pre SCO 5.0 CC compiler does not know const */
#else
#define CONST const                           /* "const" only exists in STDC */
#endif

#else

#define CONST

#endif /* _STANDARD_CODE_ (ANSI) */
/*

Variable Argument Functions

   This is normally one of the more tricky part in portable code as it very often doesn't
   work. If you are going to use it then watch out!
   
 */
#ifdef _STANDARD_CODE_
#include <stdarg.h>
#else
#include <varargs.h>
#endif
/*

Booleans

 */
#ifndef BOOLEAN_DEFINED
typedef char    BOOLEAN;                                    /* Logical value */
#endif

#ifndef CURSES
#ifndef TRUE
#define TRUE    (BOOLEAN)1
#define FALSE   (BOOLEAN)0
#endif
#endif   /*  CURSES  */

#ifndef BOOL
#define BOOL BOOLEAN
#endif
#ifndef YES
#define YES             (BOOL)1
#define NO              (BOOL)0
#endif
/*

NULL Definition

 */
#ifndef NULL
#define NULL ((void *)0)
#endif
/*

Often used Interger Macros

  MIN AND MAX FUNCTIONS
  
 */
#ifndef HTMIN
#define HTMIN(a,b) ((a) <= (b) ? (a) : (b))
#define HTMAX(a,b) ((a) >= (b) ? (a) : (b))
#endif
/*

  DOUBLE ABS FUNCTION
  
 */
#ifndef HTDABS
#define HTDABS(a) ((a) < 0.0 ? (-(a)) : (a))
#endif
/*

Return Codes for Protocol Modules and Streams

   Theese are the codes returned from the protocol modules, and the stream modules.
   Success are (>=0) and failure are (<0)
   
 */
#define HT_OK                   0       /* Generic success */
#define HT_ALL                  1       /* Used by Net Manager */

#define HT_CLOSED               29992   /* The socket was closed */
#define HT_PERSISTENT           29993   /* Wait for persistent connection */
#define HT_IGNORE               29994   /* Ignore this in the Net manager */
#define HT_NO_DATA              29995   /* OK but no data was loaded */
#define HT_RELOAD               29996   /* If we must reload the document */
#define HT_PERM_REDIRECT        29997   /* Redo the retrieve with a new URL */
#define HT_TEMP_REDIRECT        29998   /* Redo the retrieve with a new URL */
#define HT_LOADED               29999   /* Instead of a socket */

#define HT_ERROR                -1      /* Generic failure */

#define HT_NO_ACCESS            -10     /* Access not available */
#define HT_FORBIDDEN            -11     /* Access forbidden */
#define HT_RETRY                -13     /* If service isn't available */

#define HT_INTERNAL             -100    /* Weird -- should never happen. */

#define HT_WOULD_BLOCK          -29997  /* If we are in a select */
#define HT_INTERRUPTED          -29998  /* Note the negative value! */
#define HT_PAUSE                -29999  /* If we want to pause a stream */
/*

Upper- and Lowercase macros

   The problem here is that toupper(x) is not defined officially unless isupper(x) is.
   These macros are CERTAINLY needed on #if defined(pyr) || define(mips) or BDSI
   platforms. For safefy, we make them mandatory.
   
 */
#include <ctype.h>

#ifndef TOLOWER
#define TOLOWER(c) tolower(c)
#define TOUPPER(c) toupper(c)
#endif
/*

Max and Min values for Integers and Floating Point

 */
#ifdef FLT_EPSILON                                  /* The ANSI C way define */
#define HT_EPSILON FLT_EPSILON
#else
#define HT_EPSILON 0.00000001
#endif
/*

White Characters

   Is character c white space?
   
 */
#define WHITE(c) isspace(c)
/*

The local equivalents of CR and LF

   We can check for these after net ascii text has been converted to the local
   representation. Similarly, we include them in strings to be sent as net ascii after
   translation.
   
 */
#define LF   FROMASCII('\012')  /* ASCII line feed LOCAL EQUIVALENT */
#define CR   FROMASCII('\015')  /* Will be converted to ^M for transmission */
/*

Library Dynamic Memory Magement

   The Library has it's own dynamic memory API which is declared in memory management
   module.
   
 */
#include "HTMemory.h"
/*

 */
#endif /* HT_UTILS.h */
/*

   End of utility declarations */
