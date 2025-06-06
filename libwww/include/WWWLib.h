/*                                                     Include file for W3C Reference Library
                          INCLUDE FILE FOR W3C REFERENCE LIBRARY
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the basic include files and the core include files necessary in order to use
   the W3C Reference Library. It contains all core specific modules which are required to
   compile and build the Library. No converter streams or protocol modules are included in
   this file as they are for the application to set up. The Library has a special include
   file called WWWApp.h which contains all converters and protocol modules known to the
   Library. You can include this one if the application is to use all the functionality of
   the Library.
   
 */
#ifndef WWWLIB_H
#define WWWLIB_H
/*

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

General Utilities

 */
#include "WWWUtil.h"                    /* Basic utility modules */
/*

Core Modules

 */
#include "WWWCore.h"                    /* Core modules */
/*

 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWLib API definition  */
