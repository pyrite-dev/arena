/*                                                        Non-Mandatory Initialization Module
                           NON-MANDATORY INITIALIZATION MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

 */
#ifndef WWWINIT_H
#define WWWINIT_H
/*

   The core parts of the Library is a framework for adding functionality. It has hooks for
   adding protocol modules, like for example HTTP, FTP, and also for adding streams that
   can convert from one media type to some other type, or presenting the result to the
   user. In the distribution file of the Library you will find a large set of protocol
   modules and streams already implemented. However, in order to use these you need to
   initialize them. This can be done by using the files in the HTInit module. You can
   modify this module as you like to fit your particular needs.
   
 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTInit.h"
#include "HTBInit.h"
/*

 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWINIT declaration  */
