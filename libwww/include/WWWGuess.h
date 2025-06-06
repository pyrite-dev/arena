/*                                          Declaration of W3C Reference Content Guess MODULE
                    DECLARATION OF W3C REFERENCE CONTENT GUESS MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This interface provides functionality for guessing unknown media types from magic
   words. The stream is a one that reads first a chunk of stuff, tries to figure out the
   format, and calls HTStreamStack(). This is a kind of lazy-evaluation of
   HTStreamStack(). This could be extended arbitrarily to recognize all the possible file
   formats in the world, if someone only had time to do it.
   
 */
#ifndef WWWGUESS_H
#define WWWGUESS_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTGuess.h"
/*

   End of GUESS module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWGUESS API definition  */
