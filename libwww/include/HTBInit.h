/*                                                         Media Type Initialization Routines
                             MEDIA TYPE INITIALIZATION MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module resisters all the plug & play software modules which will be used in the
   program on client side (The server has it's own initialization module). None of the
   functions in this module are called from the Library. They are uniquely a help for the
   application programmer in order to set up the functionality of the Library.
   
   The initialization consists of definiting the following bindings:
   
      Between an access method and a protocol module
      
   This module is implemented by HTBInit.c, and it is a part of the  W3C Reference
   Library.
   
   [IMAGE] THE APPLICATION MAY USE THESE FUNCTIONS BUT IT IS NOT REQUIRED
   
 */
#ifndef HTBINIT_H
#define HTBINIT_H
/*

 */
extern void HTFileInit (void);
/*

 */
#endif
/*

   End of HTBinit Module. */
