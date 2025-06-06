/*                                                                          Method Management
                                    METHOD MANAGEMENT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module keeps a list of valid methods to be used on a request obejct. It used to be
   in HTAccess module but for future extensions, it is now in its own module.
   
   This module is implemented by HTmethod.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTMETHOD_H
#define HTMETHOD_H

/*

   These are the valid methods, see HTTP Methods.
   
   NOTE: the anchor list of allowed methods is now a bitflag, not at list.
   
 */
typedef enum {
    METHOD_INVALID      = 0x0,
    METHOD_GET          = 0x1,
    METHOD_HEAD         = 0x2,
    METHOD_POST         = 0x4,
    METHOD_PUT          = 0x8,
    METHOD_DELETE       = 0x10,
    METHOD_LINK         = 0x20,
    METHOD_UNLINK       = 0x40
} HTMethod;
/*

  GET METHOD ENUMERATION
  
   Gives the enumeration value of the method as a function of the (char *) name.
   
 */
extern HTMethod HTMethod_enum (CONST char * name);
/*

  GET METHOD STRING
  
   The reverse of HTMethod_enum()
   
 */
extern CONST char * HTMethod_name (HTMethod method);
/*

  IS METHOD "SAFE"
  
   If a method a safe, it doesn't produce any side effects on the server
   
 */
#define HTMethod_isSafe(me)     ((me) & (METHOD_GET|METHOD_HEAD))
/*

  DOES A METHOD INCLUDE ENTITY
  
   Does a method include an entity to be sent from the client to the server?
   
 */
#define HTMethod_hasEntity(me)  ((me) & (METHOD_PUT | METHOD_POST))
/*

 */
#endif /* HTMETHOD_H */
/*

   End of Declaration module  */
