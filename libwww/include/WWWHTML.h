/*                                                   Declaration of W3C Reference HTML MODULE
                         DECLARATION OF W3C REFERENCE HTML MODULE
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is the include file for the basic HTML module that can be used together with the
   core of the W3C Reference Library. It contains all HTML specific modules which are
   required to compile and build the HTML DLL.  Please note that as the HText is not
   included in this interface. The reason is that the HText interface only is declared by
   the Library but must be defined by the application. Therefore it can not be part of a
   DLL but must be included directly in the application.
   
 */
#ifndef WWWHTML_H
#define WWWHTML_H
/*

Library Includes

 */
#ifdef __cplusplus
extern "C" {
#endif
/*

 */
#include "HTMLPDTD.h"
#include "SGML.h"
#include "HTMLGen.h"
#include "HTTeXGen.h"
/*

   End of HTML module
   
 */
#ifdef __cplusplus
} /* end extern C definitions */
#endif

#endif
/*

   End of WWWHTML API definition  */
