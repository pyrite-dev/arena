/*                                                                       CRLF Stripper Stream
                        NETWORK TELNET TO INTERNAL CHARACTER TEXT
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This is a filter stream suitable for taking text from a socket and passing it into a
   stream which expects text in the local C representation.  It does newline conversion.
   As usual, pass its output stream to it when creating it.
   
   This module is implemented by HTNetTxt.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTNETTXT_H
#define HTNETTXT_H

extern HTStream * HTNetToText (HTStream * target);

#endif
/*

   End of definition module */
