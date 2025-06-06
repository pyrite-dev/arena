/*                                                  Atoms - an easy way of organizing strings
                                          ATOMS
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   Atoms are strings which are given representative pointer values so that they can be
   stored more efficiently, and compaisons for equality done more efficiently. The list of
   atoms are stored in a has table, so when asking for a new atom you might infact get
   back an existing one.
   
   There are a whole bunch of MIME-types defined as atoms, so please use them!
   
   This module is implemented by HTAtom.c, and it is a part of the  W3C Reference Library.
   
 */
#ifndef HTATOM_H
#define HTATOM_H

#include "HTList.h"

typedef struct _HTAtom HTAtom;
struct _HTAtom {
        HTAtom *        next;
        char *          name;
}; /* struct _HTAtom */
/*

Public Functions

   The following methods are available for this class:
   
  GET AN ATOM
  
   This function returns a representative value (an atom) such that it will always (within
   one run of the program) return the same value for the same given string. The former is
   case sensitive, the latter is case insensitive.
   
 */
extern HTAtom * HTAtom_for      (CONST char * string);
extern HTAtom * HTAtom_caseFor  (CONST char * string);
/*

  GET CONTENT OF AN ATOM
  
 */
#define HTAtom_name(a) ((a) ? (a)->name : NULL)
/*

   This macro returns the string pointed to by the atom.
   
  SEARCH FOR ATOMS
  
   Returns a list of atoms which matches the template given. It is especially made for
   MIME-types so that for example a template like text<slash><star> returns a list of all
   MIME-types of type text.
   
 */
extern HTList * HTAtom_templateMatches (CONST char * templ);
/*

  CLEANUP MEMORY
  
   In order to cleanup memory, call this function. This is done automaticly from the
   HTLibTerminate function.
   
 */
extern void HTAtom_deleteAll (void);

#endif
/*

   End of HTAtom definition.  */
