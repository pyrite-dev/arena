#ifndef _ARENA_TYPES_H_
#define _ARENA_TYPES_H_

#define BANK_SIZE 511
#define SPLIT_SIZE 128

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef __hpux
#include <X11/HPkeysym.h>
#endif

typedef struct cell_ {
    char *bank;
    struct cell_ *next;
    struct cell_ *prev;
    int size;
} cell;

typedef struct buff_struct_ {
    cell *cell;
    int n_bank;
    int pos;
    int size;
} EditorBuffer;


typedef struct image_struct
{
    struct image_struct *next;		/* only used for internal images, icons etc  */
    char *url;				/* only used for internal images, icons etc  */
    Pixmap pixmap;
    Pixmap mask;                        /* for future use (transparency) */
    unsigned long *pixels;
    int npixels;           
    unsigned int width;
    unsigned int height;
} Image;

#ifndef Byte
typedef unsigned char Byte;
#endif

#define TEXT_FIELD_SCRATCH_LENGTH 4

typedef struct text_field_struct
{
    char *buffer;
    int length;
    int used;
    char *b, *e;

    char scratch[TEXT_FIELD_SCRATCH_LENGTH];
} TextField;

typedef struct option_struct
{
    struct option_struct *next; /* linked list of options */
    unsigned char flags;        /* CHECKED, DISABLED */
    char *label;
    int j;                      /* option index number */
} Option;

typedef  struct field_struct
{
    struct field_struct *next;  /* linked list of fields */
    struct form_struct  *form;  /* the parent form */
    Option *options;            /* linked list of options */
    unsigned char type;         /* RADIOBUTTON, CHECKBOX, ... */
    unsigned char flags;        /* CHECKED, IN_ERROR, DISABLED */
    char *name;                 /* field name attribute */
    int nlen;                   /* name lenght --Spif */
    char *value;                /* malloc'ed buffer */
    Image *image;               /* SUBMIT & RESET can be images --Spif */
    int bufsize;                /* current buffer size */
    int buflen;                 /* length of useful data */
    int above;                  /* above baseline */
    int width;                  /* in pixels */
    int height;                 /* in pixels */
    int x;                      /* from left of document */
    int x_indent;               /* for text fields */
    int y_indent;
    long baseline;              /* from start of document */
    int object;                 /* offset in paint stream */
    int frame_indent;           /* this is a kludge !!!! */
    TextField *text_field;      /* howcome 20/1/95 */
    
} Field;

typedef struct form_struct
{
    struct form_struct *next;   /* linked list of forms */
    char *action;               /* URL from ACTION attriubte */
    int  alen;                  /* size of URL --Spif */
    int  method;                /* Method */
    Field *fields;              /* linked list of fields */
} Form;

typedef struct output_image_data_struct {
    int			depth;
    int			bytes_per_row;
    unsigned char 	*data;
    int			width;
    int			height;

    int			row;
} output_image_data;


typedef struct {
    int			width;
    int			height;
    int			colour_space;
    int			mode;

    int			components;
    unsigned char	*buffer;
    int			rows;
    int			row_index;
    int			row_interlace;
} image_data;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} box;

typedef struct box_struct_ {
    box *box;
    struct box_struct_ *next;
} box_link;

#endif
