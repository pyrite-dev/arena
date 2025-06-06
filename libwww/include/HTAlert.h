/*                                     Library Interface for Displaying and Promting Messages
                  LIBRARY INTERFACE FOR DISPLAYING AND PROMTING MESSAGES
                                             
 */
/*
**      (c) COPYRIGHT MIT 1995.
**      Please first read the full copyright statement in the file COPYRIGH.
*/
/*

   This module is a platform independent and language independent interface to User
   messages and prompting. The Library does not provide any messages on its own as they
   must be language independent. These can be provided by the application in a language
   that suits the user. The module is a registration of call back functions to the
   application.
   
   This module is implemented by HTAlert.c, and it is a part of the  W3C Reference
   Library.
   
 */
#ifndef HTALERT_H
#define HTALERT_H

#include "HTReq.h"
/*

Declaration of Callback Function

   The callback functions are defined as a generic callback where the caller can pass a
   set of input parameters and the callee can return a set of outptu parameters. Also note
   that all the *_PROG_*opcodes are a subset of HT_A_PROGRESS. This means that you easily
   can register a callback for all progress reports.
   
 */
typedef enum _HTAlertOpcode {
    HT_PROG_DNS         = 0x1,          /* Doing DNS resolution */
    HT_PROG_CONNECT     = 0x2,          /* Connecting Active */
    HT_PROG_ACCEPT      = 0x4,          /* Connecting Passive */
    HT_PROG_READ        = 0x8,          /* Read data */
    HT_PROG_WRITE       = 0x10,         /* Write data */
    HT_PROG_DONE        = 0x20,         /* Request finished */
    HT_PROG_WAIT        = 0x40,         /* Wait for socket */
    HT_A_PROGRESS       = 0xFF,         /* Send a progress report - no reply */

    /* First word are reserved for progresss notifications */

    HT_A_MESSAGE        = 0x1<<8,       /* Send a message - no reply */
    HT_A_CONFIRM        = 0x2<<8,       /* Want YES or NO back */
    HT_A_PROMPT         = 0x4<<8,       /* Want full dialog */
    HT_A_SECRET         = 0x8<<8,       /* Secret dialog (e.g. password) */
    HT_A_USER_PW        = 0x10<<8       /* Atomic userid and password */
} HTAlertOpcode;

typedef struct _HTAlertPar HTAlertPar;

typedef BOOL HTAlertCallback   (HTRequest * request, HTAlertOpcode op,
                                int msgnum, CONST char * dfault, void * input,
                                HTAlertPar * reply);
/*

   If you don't expect any return values then reply can be NULL. The return value of the
   callback function can be used to indicate confirmation on a prompt (Yes or No).
   
String Messages

   This is an enumerated list of messages that can be converted into a string table etc.
   
 */
typedef enum _HTAlertMsg {
    HT_MSG_NULL = -1,
    HT_MSG_UID = 0,
    HT_MSG_PW,
    HT_MSG_FILENAME,
    HT_MSG_ACCOUNT,
    HT_MSG_METHOD,
    HT_MSG_MOVED,
    HT_MSG_RULES,
    HT_MSG_ELEMENTS                         /* This MUST be the last element */
} HTAlertMsg;
/*

Public Methods

  ENABLE OR DISABLE MESSAGES
  
   If you really don't want the library to prompt for anything at all then enable this
   constant. The default value is Interactive.
   
 */
extern void HTAlert_setInteractive      (BOOL interative);
extern BOOL HTAlert_interactive         (void);
/*

  ADD A CALLBACK FUNCTION TO A LIST
  
   Register a call back function that is to be called when generating messages, dialog,
   prompts, progress reports etc. The opcode signifies which call back function to call
   depending of the type of the message. Opcode can be any combination of the bitflags
   defined by HTAlertOpcode. If you register one callback for HT_A_PROGRESS then this will
   get called on all progress notifications.
   
 */
extern BOOL HTAlertCall_add (HTList * list, HTAlertCallback * cbf,
                             HTAlertOpcode opcode);
/*

  DELETE A CALLBACK FUNCTION FROM A LIST
  
   Unregister a call back function from a list
   
 */
extern BOOL HTAlertCall_delete (HTList * list, HTAlertCallback * cbf);
/*

  DELETE A LIST OF CALLBACK FUNCTIONS
  
   Unregisters all call back functions
   
 */
extern BOOL HTAlertCall_deleteAll (HTList * list);
/*

  FIND A CALLBACK FUNCTION FROM A LIST
  
   Finds a callback function corresponding to the opcode. If none has been registered then
   NULL is returned.
   
 */
extern HTAlertCallback * HTAlertCall_find(HTList * list, HTAlertOpcode opcode);
/*

Handle the Reply Structure

   Create and delete...
   
 */
extern HTAlertPar * HTAlert_newReply    (void);
extern void HTAlert_deleteReply         (HTAlertPar * old);
/*

  HANDLE THE REPLY MESSAGE
  
   These methods provide the API for handling the reply message. There are two ways of
   assigning a message to the reply message - either by copying the buffer or by reusing
   the same buffer. In the latter case, the caller must make sure not to free the reply
   message before it has been used.
   
 */
extern BOOL HTAlert_setReplyMessage     (HTAlertPar * me, CONST char *message);
extern BOOL HTAlert_assignReplyMessage  (HTAlertPar * me, char * message);
/*

   You can get the data back again by using this method:
   
 */
extern char * HTAlert_replyMessage      (HTAlertPar * me);
/*

 */
extern char * HTAlert_replySecret       (HTAlertPar * me);
extern BOOL HTAlert_setReplySecret      (HTAlertPar * me, CONST char * secret);

extern void * HTAlert_replyOutput       (HTAlertPar * me);
extern BOOL HTAlert_setReplyOutput      (HTAlertPar * me, void * output);
/*

Global set of Callback Functions

   A list can be assigned as being global for all messages.
   
 */
extern void HTAlert_setGlobal   (HTList * list);
extern HTList * HTAlert_global  (void);
/*

   You can also assign a callback directly to the global list. In this case you do not
   need to worry about creating the list - it will be created automatically.
   
 */
extern BOOL HTAlert_add         (HTAlertCallback * cbf, HTAlertOpcode opcode);
extern BOOL HTAlert_delete      (HTAlertCallback * cbf);
extern HTAlertCallback * HTAlert_find (HTAlertOpcode opcode);
/*

 */
#endif
/*

   End of declaration  */
