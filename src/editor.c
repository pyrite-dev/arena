/* howcome 11/10/94 */

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <X11/Xlib.h>

#include "HTUtils.h"	/* WWW general purpose macros */
#include "tcp.h"
#include "HTString.h"
#include "HTList.h"
#include "HTAccess.h"

#include "www.h"

#ifndef MAIN

extern int debug;
extern Doc *CurrentDoc;
extern char * CacheRoot;
extern char * Editor;
extern int Badflags;
extern char *buffer;

extern long buf_height;
extern int buf_width;
extern int PixelIndent;
extern long PixelOffset;
extern int ToolBarHeight;
extern int statusHeight;
extern unsigned int win_width, win_height;
extern int sbar_width;


/* record position and nature of error to generate listing */

void RecordError(char * p, char *s)
{
    BadFlag *h;

    if (!CurrentDoc)
	return;

    if (!CurrentDoc->bad_flags)
	CurrentDoc->bad_flags = HTList_new();

    if ((h = (BadFlag *) calloc (sizeof(BadFlag), 1)) == NULL)
	return;

    h->p = p + 1;
    h->error_text = s;

    HTList_addObject(CurrentDoc->bad_flags, (void *) h);

    if (BADFLAG_TRACE)
#if defined (__alpha) && defined (__osf__)
	fprintf(stderr,"RecordError: %s %p\n",s,p);
#else
	fprintf(stderr,"RecordError: %s %lx\n",s,p);
#endif /* (__alpha) && (__osf__) */
}

char *WriteBadFlagFile(char *p)
{
    int fd;
    char *n;
    BadFlag *h;
    HTList *l;

    if (CacheRoot){
	n = malloc (strlen(CacheRoot) + 20);
	sprintf(n,"%s/edit.%d.html", CacheRoot, getpid());
    } else {
	n = malloc (25);
	sprintf(n,"/tmp/edit.%d.html", getpid());  /* 16/10/94 use tempnam?? */
    }

    if ((fd = creat(n, S_IRUSR | S_IWUSR)) == -1) {
	return(NULL);
    }

    l = CurrentDoc->bad_flags;

    while ((h = (BadFlag*) HTList_removeFirstObject(l) )) {
	int len = h->p - p;
	if (BADFLAG_TRACE) {
	    char s[1000];
#if defined (__alpha) && defined (__osf__)
	    fprintf(stderr,"WriteBadFlagFile %s %p\n",h->error_text,h->p);
#else
 	    fprintf(stderr,"WriteBadFlagFile %s %lx\n",h->error_text,h->p);
#endif /* (__alpha) && (__osf__) */

	    strncpy(s, p, h->p - p);
	    fprintf(stderr,"WriteBadFlagFile:  %d bytes: \"%s\"\n\n", len, s);
	}

	if (len > 0) {
	    write(fd, p, h->p - p);
	    write(fd, "\n<!-- ", 6);
	    write(fd, h->error_text, strlen(h->error_text));
	    write(fd, " -->\n", 5);
	    p = h->p;
	}
    }

    write(fd, p, strlen(p));

    close(fd);
    return(n);
}


char *ReadBadFlagFile(char * f_name, int * r)
{
    int fd, l; 
    char *buf;
    struct stat s;

    *r = 0;
    
    if ((fd = open(f_name, O_RDONLY)) == -1)
	return NULL;

    fstat(fd, &s);    

    if (s.st_size > 0) {
	
	buf = (char *)malloc(s.st_size + 1);
	l = read(fd, buf, s.st_size);
	buf[l] = 0;
	close(fd);
	if (l != s.st_size) {
	    return NULL;
	}
	*r = l+1; /* Don't forget the terminating 0! Michael Van Biesbrouck <mlvanbie@valeyard.csclub.uwaterloo.ca> */
	return buf;
    }
    close(fd);
    return(NULL);
}



void ForkEditor(char **b)
{

    /* janet: not used:     char *p, *q, *start; */
    /* janet: not used:     long offset, maxOffset, target; */

    if (Editor) {
	char *tmp_file, *cmd, *newbuf;
	int n_read;
	
	tmp_file = WriteBadFlagFile(*b);
	cmd = strdup(Editor);
	StrAllocCat(cmd, " ");
	StrAllocCat(cmd, tmp_file);
/*	    StrAllocCat(cmd," &"); */
	if (debug)
	    printf("forking off \"%s\"\n",cmd);
	system(cmd);
	Free(cmd);
	newbuf = ReadBadFlagFile(tmp_file, &n_read);
	*b = realloc(*b, n_read);
	memcpy(*b, newbuf, n_read);
	Free(newbuf);
	unlink(tmp_file);
    }
}

/* howcome 12/10/94: added support for external editing */

void EditCurrentBuffer()
{
    if (Editor) {
	ForkEditor(&buffer);
	buf_height = ParseHTML(&buf_width, TRUE);

	SetScrollBarWidth(buf_width);
	SetScrollBarHeight(buf_height);
	SetScrollBarHPosition(PixelIndent, buf_width);
	SetScrollBarVPosition(PixelOffset, buf_height);
	DisplayScrollBar();
	DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight);
    } else {
	Warn("No editor has been specified");
    }	
}


#if 0 

/* internal editing! */


void FormDel(TextField *tf)
{
    int len3 = tf->used - (tf->e - tf->buffer);

    if (tf->b == tf->e)
	return;

/*    memmove(tf->b, tf->e, len);*/
    tf->used -= (tf->e - tf->b);
}


void FormAdd(TextField *tf)
{
    int newlen = tf->used + strlen(tf->scratch);
    int len1 = tf->b - tf->buffer;
    int len2 = strlen(tf->scratch);
    int len3 = tf->used - len1;

    if (newlen > (tf->length - 2)) {
	tf->length = newlen + 100;
	tf->buffer = (char *)realloc(tf->buffer, tf->length);
    }

    /* first move the part after insertion point forward */

    memmove(tf->b + len2, tf->b, len3);

    /* then, insert the scratchpad */

    memcpy(tf->buffer + len1, tf->scratch, len2);

    tf->b = tf->e = tf->buffer + len1 + len2;
    tf->scratch[0] = 0;
    tf->used = len1 + len2 + len3;
    tf->buffer[tf->used] = 0;

    fprintf(stderr,"FormAdd: %s\n",tf->buffer);
}

void FormUpdate(TextField *tf)
{
    char *p, *s;

    fprintf(stderr,"FormUpdate: ");
/*
    if (tf->scratch[0] != 0) {
	fprintf(stderr,"%s",tf->scratch);
    }
*/
    if (tf->e) {
	p = strchr(tf->e, '\r');
	if (!p)
	    p = tf->buffer + tf->used;

	s = strndup(tf->e, p - tf->e);
    
	if (tf->scratch[0] != 0) {
	    fprintf(stderr,"%s",s);
	}
    }
    fprintf(stderr,"\n");
}



void FormEditChar(char c)
{
    int l;
    TextField *tf = focus->text_field;

    fprintf(stderr,"FormEditChar: %d %c\n",c,c);

    if (strlen(tf->scratch) > (TEXT_FIELD_SCRATCH_LENGTH - 2)) {
	FormDel(tf);
	FormAdd(tf);
    }

    l = strlen(tf->scratch);
    tf->scratch[l] = c;
    tf->scratch[l+1] = 0;

    switch (c) {
      case ' ':
      case '\r':
	FormAdd(tf);
	break;
      default:
    }

    FormUpdate(tf);

    if (tf->buffer)
	fprintf(stderr,"FormEditChar %s\n",tf->buffer);

}


void FormMoveCursor(int key)
{
    fprintf(stderr,"FormMoveCursor %d\n",key);
}

#endif /* 0 */

#endif /* #ifndef MAIN */

/* ------------------------------------------------- */

typedef int ENTRY_POINT;
#define MAX_CHAR_VALUE 255

ENTRY_POINT EditInitialize ()
{
    /*
    **
    **  Non-Reentrant initialisation routine. 
    **
    **  This must be called prior to performing any edit operations.
    */
    return TRUE; /* janet: for compiler purposes only */


    
}

ENTRY_POINT EditTerminate ()
{
    /*
    **  Non-Reentrant termination routine. 
    **
    **  This must be called on termination.
    */
  
    return TRUE; /* janet: for compiler purposes only */
  

}


void FreeEditBuffer(EditBuffer *eb)
{
    Free(eb);
}


    /*
    **  FUNCTIONAL DESCRIPTION:
    **
    **      An edit buffer structure is created and initialised.
    **
    **      If the input parameters line_break_characters and
    **      word_break_characters are given as NULL the defaults
    **      for ASCII text are used.
    */


ENTRY_POINT EditBufferCreate (
		int	width, 
		int 	height,
                char            *line_break_characters,
                char            *word_break_characters,
		char            *content,
		EditBuffer     **eb_out) 


/*
                int             (* writeahead_log) 
                                        (void *user_data, 
                                        edit_operation *operation),
                void            *user_data;
*/
{
    EditBuffer	*eb = NULL;
    int		i;
    char 	*bt;
    
/*
    ALLOCATE (edit_buffer, parent, &buffer);
    ALLOCATE_ARRAY (char, buffer, MAX_CHAR_VALUE, &buffer->word_break_table);
*/

    eb = (EditBuffer *)malloc(sizeof(EditBuffer));
    if (!eb) 
	return FALSE;

    eb->paras = HTList_new();
    eb->width = width;
    eb->auto_break = TRUE;
    eb->break_table_size = MAX_CHAR_VALUE;
    eb->break_table = (char *)malloc(MAX_CHAR_VALUE);
    if (!eb->break_table) {
	FreeEditBuffer(eb);
	return FALSE;
    }

    bt = eb->break_table;

    /*
    **  Initialise the break table
    */ 
    for (i=0; i<MAX_CHAR_VALUE; i++) {
        bt[i] = EDIT_CHARACTER_TYPE_NORMAL;
    }

    if (line_break_characters != NULL) {
        for (i=0; line_break_characters[i] != 0; i++) {
            bt [line_break_characters[i]] |= EDIT_CHARACTER_TYPE_PARA_BREAK;
	}
    }
    else {
        bt['\n'] |= EDIT_CHARACTER_TYPE_PARA_BREAK;
    }

    if (word_break_characters != NULL) {
        for (i=0; word_break_characters[i] != 0; i++) {
            bt[word_break_characters[i]] |= EDIT_CHARACTER_TYPE_WORD_BREAK;
	}
    }
    else {
        bt[' '] |= EDIT_CHARACTER_TYPE_WORD_BREAK;
        bt['\t'] |= EDIT_CHARACTER_TYPE_WORD_BREAK;
    }

    if (content){
	char *p = content;
	char *old_p = content;

	while (*p) {
	    if (bt[*p] & EDIT_CHARACTER_TYPE_PARA_BREAK) {

		int len;
		EditParagraph *ep=NULL;

		ep = (EditParagraph *)malloc(sizeof(EditParagraph));

		if (!ep) {
		    FreeEditBuffer(eb);
		    return FALSE;
		}

		len = p - old_p;
		
		ep->buffer = (char *) malloc (len + EXTRA_LENGTH);
		if (! ep->buffer) {
		    FreeEditBuffer(eb);
		    return FALSE;
		}

		strncpy(ep->buffer, old_p, len);
		ep->buffer[len] = '\0';
		ep->buffer_used = len;
		ep->buffer_size = len + EXTRA_LENGTH;
		ep->width = width;
		ep->height = height;

		fprintf(stderr,"new para: %s\n",ep->buffer);
		HTList_addObject(eb->paras, ep);
		old_p = p + 1;

	    } else if (bt[*p] & EDIT_CHARACTER_TYPE_WORD_BREAK) {
		/* add new word break */
	    }
	    p++;
	}
    }

    *eb_out = eb;
    return TRUE;
}

ENTRY_POINT EditViewCreate (
                EditBuffer	*buffer,
                int		width,
                int		height,
                int		row, 
                int		column,

                /* font structure ??? for time being fixed ! */
/*
                int (*display_line) (
                        void *user_data, EditLine *line),
                int (*display_cursor) (
                        void *user_data),
                int (*display_scroll) (
                        void *user_data),
*/
                EditView       **view_out)
{
    EditView *ev;

    if (!buffer)
	return FALSE;

    ev = (EditView *) malloc (sizeof(EditView));
    
    if (!ev)
	return FALSE;

    ev->width = width;
    ev->height = height;
    ev->main_buffer = buffer;
    ev->cursor_row = 0;
    ev->cursor_column = 0;
    ev->cursor_x = 0;
    ev->cursor_y = 0;
    /* janet 24/07/95: to have function return a value: added 'return TRUE' */
    return TRUE;
}

#if 0
ENTRY_POINT edit_delete_object (
                edit_object     *object         /* Object to be deleted */
                ) 
{
    /*
    **  Determine the type of the structure then call appropriate destroy
    **  routine
    */
}


/*
**  Routines to initialize the text in a buffer
*/

ENTRY_POINT EditBufferInputOpen (
                EditBuffer     *buffer) 
{
    /*
    **  FUNCTIONAL DESCRIPTION:
    **
    **      An 
    */
    
}

ENTRY_POINT EditBufferInputAdd (EditBuffer *buffer, char *block, int length) 
{
    /*
    **  FUNCTIONAL DESCRIPTION:
    **
    **      
    */
    
}

ENTRY_POINT EditBufferInputClose (
                EditBuffer *buffer) 
{

}

ENTRY_POINT EditBufferOutput (EditBuffer *buffer, HTStream *stream) 
{
    
}


ENTRY_POINT EditCursorMove (
                EditView               *view, 
                edit_cursor_movement    mode, 
                edit_cursor_unit        unit,
                int 			x,
                int	         	y,
                ) 
{
    /*
    **  Moves the editing cursor in the specified view. The display window
    **  is adjusted acordingly.
    **
    **  The cursor may be moved in either absolute or relative modes.
    **  
    **  Pixel coordinates may only be specified in absolute mode however?
    */
/*    
    if (!view)
	return FALSE;
    
    switch (unit) {
      case edit_cursor_unit_character:
	if (mode == edit_cursor_movement_relative) {
	    view->cursor_column += x;
	    if (view->cursor_column < 0)
		view->cursor_column = 0;
	    else if (view->cursor_column > HTList_count(view->main_buffer->)

	    view->cursor_row += y;
	}

	break;
      default:
    }
*/
}

ENTRY_POINT EditCoordinatesConvert (
                EditView		*view, 
                int			input_unit,
                int			input_row,
                int			input_colum,
                int			output_unit,
                int			*output_row,
                int			*outpu_colum
                ) 
{
    /*
    **  Converts from one type of coordinates to another, valid input
    **  and output units are :
    **
    **          pixels -> character
    **          character -> pixels
    */

    


    
}


ENTRY_POINT EditCursorMark (EditView *view) 
{
    
    
    
}



ENTRY_POINT EditViewSet (
                EditView               *view,
                edit_pixels             pixel_row,
                edit_pixels             pixel_colum,
                edit_pixels             pixel_width,
                edit_pixels             pixel_height,
                void *user_data
                ) 
{
    /*
    **  Sets the viewport of a window.
    **  
    **  If any parameter is specified as -1 the previous value is used.
    */
    

}

ENTRY_POINT EditBufferCut (
                EditView       *view,
                EditBuffer     *destination
                ) 
{


}

ENTRY_POINT EditBufferCopy (
                EditView       *view,
                EditBuffer     *destination
                ) 
{
    
}
        
ENTRY_POINT EditBufferPaste (
                EditView       *view,
                EditBuffer     *source
                ) 
{

}

ENTRY_POINT EditBufferInsert (
                EditView	*view,      /* edit view to be affected */
                char		*string,    /* characters to insert */
                int		length      /* if >= 0 length of string
                                            else string is null terminated */
                ) 
{
   
}

#endif /* #if 0 */


#ifdef MAIN

main()
{
    EditBuffer *eb;

    EditBufferCreate(10, 10, NULL, NULL, "A test\nFor now\n", &eb);

}

#endif















