/*                                                               Structured Stream Definition
                               STRUCTURED STREAM DEFINITION
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is a part of the W3C Reference Library.
   
 */
#ifndef HTSTRUCT_H
#define HTSTRUCT_H

#include "HTStream.h"
#include "HTList.h"
/*

   A structured object is something which can reasonably be represented in for eaxmple
   SGML.  I'll rephrase that.  A structured object is am ordered tree-structured
   arrangement of data which is representable as text. An example is the SGML parser which
   outputs to a Structured Object. A Structured object can output its contents to another
   Structured Object. It's a kind of typed stream. The architecure is largely Dan
   Conolly's. Elements and entities are passed to the sob by number, implying a knowledge
   of the DTD.
   
   The Streuctured Stream is a subclass of a Generic Stream Object. As always, we don't
   have classes in basic C so we have to do this by hand!
   
   NOTE: The put_block method was write, but this upset systems which had macros for
   write(). See the Generic Stream Definition for valid return codes.
   
 */
typedef struct _HTStructured HTStructured;

typedef struct _HTStructuredClass {

    char * name;

    int (*flush)        (HTStructured * me);

    int (*_free)        (HTStructured * me);

    int (*abort)        (HTStructured * me, HTList * errorlist);

    int (*put_character)(HTStructured * me, char ch);

    int (*put_string)   (HTStructured * me, CONST char * str);

    int (*put_block)    (HTStructured * me, CONST char * str, int len);
/*

   See the Generic Stream Definition for an explanation of these methods. Note that they
   all have a HTStructured object a the parameter, not a generic stream. This is to avoid
   incompatible pointer warnings
   
 */
    void (*start_element)(HTStructured *me,
                          int           element_number,
                          CONST BOOL *  attribute_present,
                          CONST char ** attribute_value);

    void (*end_element) (HTStructured * me, int element_number);

    void (*put_entity)  (HTStructured * me, int entity_number);
                
} HTStructuredClass;

#endif
/*

   End of Structured Stream definition */
