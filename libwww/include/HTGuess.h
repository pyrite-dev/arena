/*                                                                            Guessing Stream
                                   CONTENT-TYPE GUESSER
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This interface provides functionality for guessing unknown media types from magic
   words. The stream is a one that reads first a chunk of stuff, tries to figure out the
   format, and calls HTStreamStack(). This is a kind of lazy-evaluation of
   HTStreamStack().
   
   This could be extended arbitrarily to recognize all the possible file formats in the
   world, if someone only had time to do it.
   
   This module is implemented by HTGuess.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTGUESS_H
#define HTGUESS_H

#include "HTStream.h"
#include "HTFormat.h"

extern HTConverter HTGuess_new;

#endif  /* !HTGUESS_H */

/*

   End of file HTGuess.h. */
