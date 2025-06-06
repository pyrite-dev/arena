/* forms.c - forms handling for html+

ParseHTML() creates a linked list of forms. Each form has a linked
list of fields. This file includes routines for drawing fields,
handling mouse clicks/drags and key presses on fields, plus sending
the contents of forms to HTTP servers.

When resizing windows, it is important to keep the existing form
data structures as otherwise the current values of fields will be
lost and overridden by the original starting values. This requires
the global new_form to be set to false. It should be set to true
when reading new documents. In addition, The browser history list
needs to preserve the form data structures so that they can be
reused upon backtracking to restore previous field values.

The desire to evolve to a wysiwyg editor means that its a bad idea
to have pointers into the paint stream as this would be expensive
to update when users edit the html document source . Consequently,
one has to search the form structure to find fields that need to
be redrawn under programatic control, e.g. unsetting of radio
buttons. Note that anyway, the y-position of a field is unresolved
when it is added to the paint stream, and only becomes fixed when
the end of the line is reached. A compromise is to cache the
baseline position when painting each field and trash the cache
each time the user alters the document.

Another complication is the need to save current field values
when following a link to another document, as otherwise there
is now way of restoring them when the user backtracks to the
document containing the form. A simple approach is to save the
linked list of forms in memory as part of a memory resident
history list. Note for local links, the same form data should
be used, rather than recreating the list from new. The corollary
is that the list should only be freed when backtracking the
document containing the form or reloading the same.

Note that in many cases a sequence of form interactions with
a server causes a state change in the server (e.g. updating a
database) and hence is not simply reversed by backtracking in
the normal way. In this case it makes sense to avoid pushing
intermediate steps onto the history stack, e.g. when submitting
the form. This probably requires the document or server to
disable the history nechanism via suitable attributes.

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "www.h"
#include "types.h"
#include "neweditor.h"

extern Display *display;
extern int screen;
extern Window win;
extern GC disp_gc, gc_fill;
extern Cursor hourglass;
extern int UsePaper;

extern int debug;  /* controls display of errors */
extern int document;  /* HTMLDOCUMENT or TEXTDOCUMENT */
extern int busy;
extern int OpenURL;
extern int IsIndex;
extern int FindStr;
extern int Authorize;
extern char *FindNextStr;
extern int SaveFile;
extern int sbar_width;
extern int statusHeight;
extern int ToolBarHeight;
extern unsigned long windowColor;
extern unsigned int win_width, win_height, tileWidth, tileHeight;

extern unsigned long textColor, labelColor, windowTopShadow,
                     strikeColor, windowBottomShadow, statusColor, windowColor;

/*
    The current top line is displayed at the top of the window,the pixel
    offset is the number of pixels from the start of the document.
*/

extern char *buffer;            /* the start of the document buffer */
extern long PixelOffset;        /* the pixel offset to top of window */
extern int PixelIndent;
extern Doc *CurrentDoc;
extern XFontStruct *pFontInfo;
extern XFontStruct *Fonts[FONTS];
extern int LineSpacing[FONTS], BaseLine[FONTS], StrikeLine[FONTS];
extern int preformatted;
extern int font;  /* index into Fonts[] array */
extern int above, below;
extern XRectangle displayRect; /* clipping limits for painting html */

Form *forms = NULL;
Field *focus;   /* which field has focus (NULL if none do) */
int cursorpos = 5; /* cursor position in characters */
Bool TextFieldFocus = FALSE;

/*
   The next 3 pointers indicate records to reuse for next form/field/option
   when resizing a document or during 2nd pass thru table data. If NULL the
   corresponding record is created from scratch
*/

Form *next_form = NULL;
Field *next_field = NULL;
Option *next_option = NULL;

void PaintFieldCursorBuffer(GC, unsigned long, EditorBuffer *, Field *);

/* version of strdup that copes with null string pointers */

char *strsav(char *s)
{
    if (s)
        return strdup(s);

    return NULL;
}

/* version of strdup for use with non-terminated strings */

char *strdup2(char *s, int len)
{
    char *str;

    str = (char *)malloc(len + 1);
    memcpy(str, s, len);
    str[len] = '\0';
    return str;
}

Form *FindForm(Form *forms,char *link)
{
    Form *form_link;
    for (form_link = forms; form_link != NULL; form_link = form_link->next)
    {
       if ( form_link->action == link)
	   return(form_link);
    };
    return(NULL); /* failed */
}

Field *FindField(Form *form,int type, char *name, int namelen)
{
    Field *field_link;
    for( field_link = form->fields; field_link != NULL ; field_link = field_link->next)
    {
        if ( field_link->type == type )  /* check type then name... assume it's enough */
	  if ( type == RADIOBUTTON)
	  {
	      if( name == field_link->name)
	          return(field_link);
	  }
	  else
	      if ( strncasecmp( name, field_link->name, namelen) == 0 )
	          return(field_link);
    };
    return(NULL);
}


int FieldCount(Form *form)
{
    int n;
    Field *field;

    n = 0;
    field = form->fields;

    while (field)
    {
        ++n;
        field = field->next;
    }

    return n;
}

void FreeForms(void)
{
    Option *option;
    Form *form;
    Field *fields, *field;

    next_form = NULL;
    next_field = NULL;
    next_option = NULL;

    while (forms != NULL)
    {
        form = forms;
        forms = forms->next;
        fields = form->fields;

        while (fields)
        {
            field = fields;
	    /* Free(field->name); --Spif name is only a copy of pointer 13-Oct-95 */
            Free(field->value);
	    /* --Spif we must clear image */
	    if (field->image != NULL)
	    {
	        Free( field->image->pixels );
		Free( field->image->url );
		Free( field->image );
	    };
            fields = fields->next;
	    
            while (field->options)
            {
                option = field->options;

                if (option->label)
                    Free(option->label);

                field->options = option->next;
                Free(option);
            }

            Free(field);
        }

        if (form->action)
            Free(form->action);

        Free(form);
    }
}

/* scroll field up/down by delta - this is called
   to adjust y offset of field - not to paint fields */
void ScrollForms(long delta)
{
    Form *form;
    Field *field;

    for (form = forms; form; form = form->next)
    {
        for (field = form->fields; field; field = field->next)
            if (field->baseline >= 0 && field->baseline - delta >= 0)
                field->baseline -= delta;
    }
}

/*
   Preserves form contents across window resizing / backtracking.
   It is assumed that FreeForms() is called before loading new
   documents or reloading existing ones.
*/

void ResizeForm(void)
{
    if (forms != NULL)
    {
        next_form = forms;
        next_field = next_form->fields;
        next_option = next_field->options;
    }
}

/* ParseTable() makes 2 passes thru table data, so we need
   to avoid recreating elements on 2nd pass. This procedure
   should be called before pass == 1 and pass == 2 */

void ProcessTableForm(int pass)
{
    static Form *form;
    static Field *field;
    static Option *option;

    if (pass == 1)
    {
        form = next_form;
        field = next_field;
        option = next_option;
    }
    else  /* pass == 2 */
    {
        next_form = form;
        next_field = field;
        next_option = option;
    }
}

Form *GetForm(int method, char *url, int len)
{
    Form *form, *last;

    /* is form already defined */

    if (next_form)
    {
        form = next_form;
        next_form = form->next;
        next_field = form->fields;
        next_option = next_field->options;
        return form;
    }

    form = (Form *)malloc(sizeof(Form));

    form->next = NULL;
    form->fields = NULL;
    form->action = url; /* strdup2(url, len); --Spif */
    form->alen = len;
    form->method = method;
    next_field = NULL;
    next_option = NULL;

    /* insert at end of list */

    if (forms)
    {
        last = forms;

        while (last->next)
            last = last->next;

        last->next = form;
    }
    else
        forms = form;

    return form;
}

Form *DefaultForm(void)
{
    return GetForm(GET, CurrentDoc->url, strlen(CurrentDoc->url));
}

/* font should be passed in lower 4 bits of flags */

Field *GetField(Form *form, int type, int x, char *name, int nlen,
               char *value, int vlen, int rows, int cols , int flags)
{
    int em, font;
    char *s;
    Field *field, *last;

    if (form == NULL)
        return NULL;

    /* if we are resizing the form or backtracking to it then
       we need to resuse the existing data structures to preserve
       any chances that have been made to the default values */
    
    if ((field=FindField(form, type, name, nlen))== NULL)
    {

        if (next_field)
	{
	    field = next_field;
	    next_field = field->next;
	    next_option = field->options;
	    goto set_size;
	}

	field = (Field *)malloc(sizeof(Field));
	next_option = NULL;

	field->next = NULL;
	field->form = form;
	field->type = type;
	field->name = name;
	field->nlen = nlen;
	field->value = www_strndup(value, vlen);
	field->bufsize = vlen+1;
	field->buflen = vlen;    
	field->flags = flags;
	field->x = x;
	field->x_indent = 0;
	field->y_indent = 0;
	field->baseline = -1;
	field->options = 0;

    /* insert at end of list */
	
	if (form->fields)
	{
	    last = form->fields;

	    while (last->next)
	        last = last->next;

	    last->next = field;
	}
	else
	    form->fields = field;

	if (type == TEXTFIELD) {
	    field->text_field = (TextField *)malloc(sizeof(TextField));
	    field->text_field->buffer = NULL;
	    field->text_field->length = 0;
	    field->text_field->used = 0;
	    field->text_field->scratch[0] = '\0';
	}

    set_size:
	field->x = x;
	font = flags & 0x0F;

	em = WIDTH(font, "mite", 4)/4;

	if (type == RADIOBUTTON || type == CHECKBOX)
	{
	    field->width = ASCENT(font) + DESCENT(font) - 2;
	    field->height = field->width;
	    field->above = ASCENT(font) - 1;
	}
	else  /* TEXTFIELD and OPTIONLIST */
	{
	    if (type == SUBMITBUTTON)
	    {
	        if (vlen == 0)
		{
		    s = " Submit Query ";
		    vlen = strlen(s);
		    field->value = strdup2(s, vlen);
		    field->buflen = vlen;
		    field->bufsize = 1 + vlen;
		} 

		field->width = 8 + WIDTH(font, field->value, field->buflen);
	    }
	    else if (type == RESETBUTTON)
	    {
	        if (vlen == 0)
		{
		    s = " Reset ";
		    vlen = strlen(s);
		    field->value = strdup2(s, vlen);
		    field->buflen = vlen;
		    field->bufsize = 1 + vlen;
		} 
		
		field->width = 8 + WIDTH(font, field->value, field->buflen);
	    }
	    else
	        field->width = 8 + cols * em;

	    field->height = 4 + rows * SPACING(Fonts[font]);
	    field->above = ASCENT(font) + 3;
	}
    }
    else
    {
      return(field);
    }
    return field;
}

/* howcome 27/7/95: should AddOption return Option ? */

void AddOption(Field *field, int flags, char *label, int len)
{
    int width, font;
    Option *option, *last;

    /* if we are resizing the form or backtracking to it then
       we need to resuse the existing data structures to preserve
       any chances that have been made to the default values */

    if (next_option)
    {
        option = next_option;
        next_option = option->next;
        return /*option*/;
    }

    option = (Option *)malloc(sizeof(Option));

    option->next = NULL;
    option->flags = flags;
    option->label = strdup(label);
    font = flags & 0xF;

    width = 6 + WIDTH(font, label, len) + field->height;

    if (width > field->width)
	field->width = width;
    
    /* insert at end of list */
    
    if (field->options)
    {
        last = field->options;
	while (last->next)
	{
	    if (strlen(last->label) == len)
		if ( strncmp( last->label, label, len) == 0)
		{
		    if(option) /* avoid any possibility of memory leak --SPif 18/Oct/95 */
		    {
			Free(option->label);
			Free(option);
		    };
		    return /*NULL*/;
		}; /* don't add 2 times the same option --Spif 18-Oct-95 */
            last = last->next;
	};
	if (strlen(last->label) != len)
	    last->next = option;
	else
	    if ( strncmp( last->label, label, len))
		last->next = option;
	    else
		return/* NULL*/;
    }
    else
    {
        option->flags |= CHECKED;
        field->options = option;
    };

    field->y_indent++;
}

Field *CurrentRadioButton(Form *form, char *name, int nlen)
{
    Field *field;

    if (form == NULL)
        return NULL;

    field = form->fields;

    while (field)
    {
        if (strncasecmp(field->name, name, nlen) == 0 && (field->flags & CHECKED))
            return field;

        field = field->next;
    }

    return field;
}

void HideDropDown(GC gc, Field *field)
{
    int width, height, x1, y1;

    height = field->height;
    width = field->width - height + 2;
    x1 = field->frame_indent + field->x - PixelIndent;
    y1 = height + Abs2Win(field->baseline) - ASCENT(font) - 2;
    DisplayHTML(x1, y1, width, 6 + field->y_indent * (height - 4));
    ClipToWindow();
}

/* called when testing mouse down/up (event = BUTTONDOWN or BUTTONUP) */
int ClickedInField(GC gc, int indent, int baseline, Field *field, int x, int y, int event)
{
    int y1;
    Field *field_link;

    if(CurrentDoc->edited_field)   /* can get rid of this (in www.c buttonpress) */
    {
	Free(CurrentDoc->edited_field->value);
	CurrentDoc->edited_field->value = Buffer2Str(CurrentDoc->field_editor);
	CurrentDoc->edited_field = NULL;
	FreeBuffer(CurrentDoc->field_editor);
	CurrentDoc->field_editor = NULL;
    };

    if (field->type == CHECKBOX || field->type == RADIOBUTTON)
        y1 = baseline - ASCENT(font) + 2;
    else if (field->type == OPTIONLIST)
        y1 = baseline - ASCENT(font) - 2;
    else   /* TEXTFIELD */
        y1 = baseline - ASCENT(font) - 2;

    x -= indent; /* fix event bug with form fields *//* howcome: applied 18/11/94 */

    if (field->x <= x && x < field->x + field->width &&
                y1 <= y && y < y1 + field->height)
    {
        if (event == BUTTONUP)
        {
	  TextFieldFocus = FALSE; /* howcome: will be set to true if this is a text field */

            /* remove focus from current field */

            if (focus && focus != field)
            {
                focus->flags &= ~CHECKED;

                if (focus->type == OPTIONLIST)
                    HideDropDown(gc, focus);
                else if (focus->type == TEXTFIELD)
                    PaintField(gc, -1, Abs2Win(focus->baseline), focus);

                focus = NULL;
            }

            if (field->type == RADIOBUTTON)
            {
	        field->flags |= CHECKED;
		PaintField(gc, indent, baseline, field);
		for( field_link=field->form->fields ; field_link!=NULL; field_link = field_link->next)
		{
		    if( field_link!= field) /* set the other buttons with the same name ;) */
		    {
		        if( (field_link->type == RADIOBUTTON) && (strncasecmp(field_link->name,field->name,field->nlen)==0))
			    if(field_link->flags & CHECKED)
			    {
			      field_link->flags ^= CHECKED;
			      if (field_link->baseline >= 0)
				  PaintField(gc, -1, Abs2Win(field_link->baseline), field_link);
			    };
		    };
		};
	    }
            else if (field->type == CHECKBOX) 
            {
	        field->flags ^= CHECKED;
	        PaintField(gc, indent, baseline, field);
	    }
            if (field->type == OPTIONLIST)
            {
                if (field->flags & CHECKED)
                {
                    field->flags &= ~CHECKED;
                    HideDropDown(gc, field);
                }
                else
                {
                    field->flags |= CHECKED;
                    PaintDropDown(gc, field, indent);
                    focus = field;
                }
            }
            else if ((field->type == TEXTFIELD) ||(field->type == PASSWD))
            {
                field->flags |= CHECKED;
                PaintField(gc, indent, baseline, field);
                focus = field;

		CurrentDoc->edited_field = field;
		CurrentDoc->field_editor = Str2Buffer(field->value);
		PaintFieldCursorBuffer(gc, strikeColor, CurrentDoc->field_editor, field);

		/* howcome started playing 19/1/95 */
		TextFieldFocus = TRUE;
		if (OpenURL|Authorize|FindStr) {
		    OpenURL = Authorize = FindStr = 0;
		    DisplayUrl();
		}
            }
        }
        return 1;
    }
    return 0;
}

/* set clip rectangle to intersection with displayRect
   to clip text strings in text fields */

int ClipIntersection(GC gc, int x, int y, unsigned int width, unsigned int height)
{
    int xl, yl, xm, ym;
    XRectangle rect;

    xl = x;
    xm = x + width;
    yl = y;
    ym = y + height;

    if (xl < displayRect.x)
        xl = displayRect.x;

    if (yl < displayRect.y)
        yl = displayRect.y;

    if (xm > displayRect.x + displayRect.width)
        xm = displayRect.x + displayRect.width;

    if (ym > displayRect.y + displayRect.height)
        ym = displayRect.y + displayRect.height;

    if (xm > xl && ym > yl)
    {
        rect.x = xl;
        rect.y = yl;
        rect.width = xm - xl;
        rect.height = ym - yl;
        XSetClipRectangles(display, gc, 0, 0, &rect, 1, Unsorted);
    }

    return 0;
}

int ClickedInDropDown(GC gc, Field *field, int indent, int event, int x, int y)
{
    int font, baseline, bw, bh, dh, x1, y1, n;
    Option *option;

    font = field->flags & 0x0F;
   
    baseline = Abs2Win(field->baseline);
    bw = field->width - field->height + 2;
    bh = 6 + field->y_indent * (field->height - 4);
    dh = field->height;
    x1 = field->frame_indent + field->x - PixelIndent;
    y1 = field->height + baseline - ASCENT(font) - 2;
    n = field->x_indent;

    if (x1 + 2 < x && x < x1 + bw - 2 && y1 + 2 < y && y < y1 + bh)
    {
        if (event == BUTTONDOWN)
            return 1;

        if ( !(field->flags & MULTIPLE) )
        {
            for (option = field->options; option; option = option->next)
	        if((option->flags & CHECKED) == CHECKED)
		  option->flags ^= CHECKED;
	}
        n = ((y - y1) * field->y_indent) / bh;
        field->x_indent = n;
        option = field->options;

        while (n-- > 0){
            option = option->next;
	  };
        if (option->flags & CHECKED)
        {
            option->flags ^= CHECKED;

            for (option = field->options; option; option = option->next)
            {
                if (option->flags & CHECKED)
                    break;
            }
        }
        else
            option->flags |= CHECKED;

        if ( !(field->flags & MULTIPLE) )
	    field->flags &= ~CHECKED;
	    
        Free(field->value);
        field->value = (option ? strdup(option->label) : "");
        ClipToWindow();
        PaintField(gc, -1, Abs2Win(focus->baseline), field);

        if (field->flags & MULTIPLE)
            PaintDropDown(gc, field, indent);
        else
            HideDropDown(gc, field);
        return 1;
    }

    return 0;
}

void PaintDropDown(GC gc, Field *field, int indent)
{
    int font, baseline, width, height, h, x1, y1, y2, n, i;
    Option *option;

    /* nasty kludge !!!! */
    if (indent < 0)
        indent = field->frame_indent;

    baseline = Abs2Win(field->baseline);
    font = field->flags & 0x0F;

    SetFont(gc, font);
    height = field->height;
    width = field->width - height + 2;
    x1 = indent + field->x - PixelIndent;
    y1 = height + baseline - ASCENT(font) - 2;
    n = field->x_indent;
    h = 6 + field->y_indent * (height - 4);
    y2 = baseline + height;
    option = field->options;

    XSetForeground(display, gc, windowColor);
    XFillRectangle(display, win, gc, x1, y1, width, h);
    DrawOutSet(win, gc, x1, y1, width, h);

    y1 += 2;

    for (option = field->options, i = 0; option; option = option->next)
    {
        if (option->flags & CHECKED)
        {
            XSetForeground(display, gc, statusColor);
            XFillRectangle(display, win, gc, x1+2, y1, width-4, height-2);
        }

        XSetForeground(display, gc, textColor);
        XDrawString(display, win, gc, x1+4, y2, option->label, strlen(option->label));
        y2 += height - 4;
        y1 += height - 4;
        ++i;
    }
}

void PaintTickMark(GC gc, int x, int y, unsigned int w, unsigned int h)
{
    int x1, y1, x2, y2, x3, y3;

    x1 = x;
    x2 = x + w/3;
    x3 = x + w-1;
    y1 = y + h - h/3 - 1;
    y2 = y + h - 1;
    y3 = y;

    XSetForeground(display, gc, textColor);
    XDrawLine(display, win, gc, x1, y1, x2, y2);
    XDrawLine(display, win, gc, x2, y2, x3, y3);
}

void PaintCross(GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, strikeColor);
    XDrawLine(display, win, gc, x, y, x+w, y+w);
    XDrawLine(display, win, gc, x, y+w, x+w, y);
    XSetForeground(display, gc, textColor);
}

void PaintField(GC gc, int indent, int baseline, Field *field)
{
    char *s,*pos,*dump;
    int font, active, width, height, x1, y1, y2, r;
    Option *option;

    active = field->flags & CHECKED;
    font = field->flags & 0x0F;

    width = field->width;
    height = field->height;

 /* cache absolute position of baseline */
    field->baseline = Win2Abs(baseline);

    /* kludge for fields in different frames */
    if (indent >= 0)
        field->frame_indent = indent;
    else
        indent = field->frame_indent;

    if (field->type == TEXTFIELD)
    {
        x1 = indent + field->x - PixelIndent - field->x_indent;
        y2 = baseline - ASCENT(font) - 2;
        ClipIntersection(gc, x1, y2, width, height);
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillRectangle(display, win, gc, x1, y2, width, height);
        DrawInSet(win, gc, x1, y2, width, height);
        ClipIntersection(gc, x1+2, y2, width-4, height);

        if (field->buflen > 0)
        {
	    dump = (char *)malloc(strlen(field->value)+1);
	    strncpy(dump,field->value, field->buflen);
	    *(dump+field->buflen)=0;
	    pos = dump;
	    while(*pos)
		if(*pos=='\n')
		    *pos++=0;
		else
		    pos++;
	    pos = dump;
	    while(pos-dump<field->buflen)
	    {
		y1 = y2 + ASCENT(font) + 2;
		SetFont(gc, font);
		XSetForeground(display, gc, textColor);
		XDrawString(display, win, gc, x1+4, y1, pos, strlen(pos));
		y2 += ASCENT(font)+4;
		pos += strlen(pos)+1;
	    };
	    Free(dump);
        }
        XSetForeground(display, gc, textColor);
        XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);
    }
    else if (field->type == PASSWD)
    {
        x1 = indent + field->x - PixelIndent - field->x_indent;
        y2 = baseline - ASCENT(font) - 2;
        ClipIntersection(gc, x1, y2, width, height);
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillRectangle(display, win, gc, x1, y2, width, height);
        DrawInSet(win, gc, x1, y2, width, height);
        ClipIntersection(gc, x1+2, y2, width-4, height);

        if (field->buflen > 0)
        {
	    int loop_i;
	    dump = (char *)malloc(strlen(field->value)+1);
	    for(loop_i=0;loop_i<field->buflen;loop_i++)
		dump[loop_i] = '*';
	    *(dump+field->buflen)=0;
	    pos = dump;
	    while(*pos)
		if(*pos=='\n')
		    *pos++=0;
		else
		    pos++;
	    pos = dump;
	    while(pos-dump<field->buflen)
	    {
		y1 = y2 + ASCENT(font) + 2;
		SetFont(gc, font);
		XSetForeground(display, gc, textColor);
		XDrawString(display, win, gc, x1+4, y1, pos, strlen(pos));
		y2 += ASCENT(font)+4;
		pos += strlen(pos)+1;
	    };
	    Free(dump);
        }
        XSetForeground(display, gc, textColor);
        XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);
    }
    else if (field->type == SUBMITBUTTON || field->type == RESETBUTTON)
    {
	char *the_string = NULL;
	int i, j, k, len, ok;
	
	s = field->value;
	if (field->buflen > 0)
        {
	    s = field->value;
	    the_string = (char *)calloc(strlen(s)+1, sizeof(char));
	    len = strlen(s);
	    for(i=k=0; i<len; i++,k++)
		if(s[i] == '&')
		{
		    for(ok= FALSE,j=i; !ok && j< len; j++)
			ok = ((s[j]==';') || (s[j]==' ')); 
		    the_string[k] = entity(s+i+1, &j);
		    i += j - 1;
		}
		else
		    the_string[k] = s[i];
	    field->width = 8 + WIDTH(font, the_string, strlen(the_string));
	    width = field->width;     
	};
        x1 = indent + field->x - PixelIndent - field->x_indent;
        y2 = baseline - ASCENT(font) - 2;
        ClipIntersection(gc, x1, y2, width, height);
        XSetForeground(display, gc, windowColor);
        XFillRectangle(display, win, gc, x1, y2, width, height);
        DrawOutSet(win, gc, x1, y2, width, height);
        ClipIntersection(gc, x1+2, y2, width-4, height);
        
        if (field->buflen > 0)
        {
	    y1 = y2 + ASCENT(font) + 2;
            SetFont(gc, font);
            XSetForeground(display, gc, textColor);
            XDrawString(display, win, gc, x1+4, y1, the_string, strlen(the_string));
	    Free(the_string);
        }

        XSetForeground(display, gc, textColor);
        XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);
    }
    else if (field->type == OPTIONLIST)
    {
        x1 = indent + field->x - PixelIndent;
        y2 = baseline - ASCENT(font) - 2;
        XSetForeground(display, gc, windowColor);
        XFillRectangle(display, win, gc, x1, y2, width, height);
        DrawOutSet(win, gc, x1, y2, width, height);

        if (field->flags & MULTIPLE)
        {
            DrawOutSet(win, gc, x1+3+width-height, y2 - 2 + height/3, height-7, 6);
            DrawOutSet(win, gc, x1+3+width-height, y2 - 3 + 2*height/3, height-7, 6);
        }
        else /* single choice menu drawn with one bar */
        {
            DrawOutSet(win, gc, x1+3+width-height, y2 - 3 + height/2, height-7, 6);

            if (*(field->value) == '\0' && field->options != NULL)
            {
                field->x_indent = 0;
                option = field->options;
                option->flags |= CHECKED;
                Free(field->value);
                field->value = strdup(option->label);
            }
        }

        if (field->y_indent > 0 && field->value)
        {
            y1 = y2 + ASCENT(font) + 2;
            SetFont(gc, font);
            XSetForeground(display, gc, textColor);
            XDrawString(display, win, gc, x1+4, y1, field->value, strlen(field->value));
        }

        XSetForeground(display, gc, textColor);
    }
    else if (field->type == CHECKBOX)
    {
        x1 = indent + field->x - PixelIndent;
        y2 = baseline-ASCENT(font) + 2;
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillRectangle(display, win, gc, x1, y2, width, width);

        if (active)
        {
            PaintTickMark(gc, x1+3, y2+3, width-6, width-7);
#if 0
            XSetForeground(display, gc, windowBottomShadow);
            XFillRectangle(display, win, gc, x1+3, y2+3, width-6, width-6);
#endif
            DrawInSet(win, gc, x1, y2, width, width);
        }
        else
            DrawOutSet(win, gc, x1, y2, width, width);

        XSetForeground(display, gc, textColor);
    }
    else if (field->type == RADIOBUTTON)
    {
        x1 = indent + field->x - PixelIndent;
        y2 = baseline-ASCENT(font)+2;
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillArc(display, win, gc, x1, y2, width, width, 0, 360<<6);

        if (active)
        {
            r = width/4;
            DrawInSetCircle(gc, x1, y2, width, width);
            XSetForeground(display, gc, windowBottomShadow);
            width -= r+r;
            XFillArc(display, win, gc, x1+r, y2+r, width, width, 0, 360<<6);
        }
        else
            DrawOutSetCircle(gc, x1, y2, width, width);

        XSetForeground(display, gc, textColor);
    }
}


/* use this routine to hide/show cursor by
   using color = windowShadow/textColor respectively */

void PaintPopupField(Window popwin,GC gc, int indent, int baseline, Field *field)
{
    char *pos,*dump;
    int font, active, width, height, x1, y1, y2;
 
    active = field->flags & CHECKED;
    font = field->flags & 0x0F;

    width = field->width;
    height = field->height;

    field->baseline = baseline;

    /* kludge for fields in different frames */
    if (indent >= 0)
        field->frame_indent = indent;
    else
        indent = field->frame_indent;

    if (field->type == TEXTFIELD)
    {
        x1 = indent + field->x - field->x_indent;
        y2 = baseline - ASCENT(font) - 2;
/*        ClipIntersection(gc, x1, y2, width, height); */
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillRectangle(display, popwin, gc, x1, y2, width, height);
        DrawInSet(popwin, gc, x1, y2, width, height);
/*        ClipIntersection(gc, x1+2, y2, width-4, height);*/

        if (field->buflen > 0)
        {
	    dump = (char *)malloc(strlen(field->value)+1);
	    strncpy(dump,field->value, field->buflen);
	    *(dump+field->buflen)=0;
	    pos = dump;
	    while(*pos)
		if(*pos=='\n')
		    *pos++=0;
		else
		    pos++;
	    pos = dump;
	    while(pos-dump<field->buflen)
	    {
		y1 = y2 + ASCENT(font) + 2;
		SetFont(gc, font);
		XSetForeground(display, gc, textColor);
		XDrawString(display, popwin, gc, x1+4, y1, pos, strlen(pos));
		y2 += ASCENT(font)+4;
		pos += strlen(pos)+1;
	    };
	    Free(dump);
        }
        XSetForeground(display, gc, textColor);
	/*  XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);*/
    }
    else if (field->type == PASSWD)
    {
        x1 = indent + field->x - field->x_indent;
        y2 = baseline - ASCENT(font) - 2;
	/*     ClipIntersection(gc, x1, y2, width, height);*/
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillRectangle(display, popwin, gc, x1, y2, width, height);
        DrawInSet(popwin, gc, x1, y2, width, height);
	/*       ClipIntersection(gc, x1+2, y2, width-4, height);*/

        if (field->buflen > 0)
        {
	    int loop_i;
	    dump = (char *)malloc(strlen(field->value)+1);
	    for(loop_i=0;loop_i<field->buflen;loop_i++)
		dump[loop_i] = '*';
	    *(dump+field->buflen)=0;
	    pos = dump;
	    while(*pos)
		if(*pos=='\n')
		    *pos++=0;
		else
		    pos++;
	    pos = dump;
	    while(pos-dump<field->buflen)
	    {
		y1 = y2 + ASCENT(font) + 2;
		SetFont(gc, font);
		XSetForeground(display, gc, textColor);
		XDrawString(display, popwin, gc, x1+4, y1, pos, strlen(pos));
		y2 += ASCENT(font)+4;
		pos += strlen(pos)+1;
	    };
	    Free(dump);
        }
        XSetForeground(display, gc, textColor);
	/*    XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);*/
    }
}

void PaintCursor(Window win, GC gc,  int indent, int baseline ,Field *field, int pos)
{
    int font, active, width, height, x1, y2, r;
    
    active = field->flags & CHECKED;
    font = field->flags & 0x0F;

    width = field->width;
    height = field->height;
    field->baseline = baseline;
    /* kludge for fields in different frames */
    if (indent >= 0)
        field->frame_indent = indent;
    else
        indent = field->frame_indent;

    if (pos > 0)
        r = WIDTH(font, field->value, pos);
    else
        r = 0;

    x1 = indent + field->x - field->x_indent;
    y2 = baseline - ASCENT(font) - 2;

    XSetForeground(display, gc, strikeColor);
    XFillRectangle(display, win, gc, x1 + ((r)?4:2) + r, y2+2, 1,ASCENT(font)+2);
    XSetForeground(display, gc, textColor);
}

void PaintFieldCursor(GC gc, unsigned long color)
{
    int font, width, height, x1, y2, r;

    font = focus->flags & 0x0F;

    width = focus->width;
    height = focus->height;
    x1 = focus->x - PixelIndent - focus->x_indent;
    y2 = focus->baseline - ASCENT(font) - 2;

    if (focus->buflen > 0)
        r = WIDTH(font, focus->value, cursorpos);
    else
        r = 0;

    XSetForeground(display, gc, color);
    XFillRectangle(display, win, gc, x1 + 3 + r, y2+2, 1, height-4);
}

/*
   Repair a given area of a field - called by ScrollField()
   also useful when typing a char into field
*/
void RepairField(GC gc, int start, int width)
{
    int font, height, x1, y1, y2, r;
    XRectangle rect;

    font = focus->flags & 0x0F;

    width = focus->width;
    height = focus->height;
    x1 = focus->x - PixelIndent - focus->x_indent;

    rect.x = start;
    rect.y = focus->baseline - ASCENT(font);
    rect.width = width;
    rect.height = focus->height - 4;
    XSetClipRectangles(display, gc, 0, 0, &rect, 1, Unsorted);

    y2 = focus->baseline - ASCENT(font) - 2;
    XSetForeground(display, gc, statusColor);
    XFillRectangle(display, win, gc, x1, y2, width, height);

    if (focus->buflen > 0)
    {
        y1 = y2 + ASCENT(font) + 2;
        SetFont(gc, font);
        XSetForeground(display, gc, textColor);
        XDrawString(display, win, gc, x1+4, y1, focus->value, focus->buflen);
    }

    if (focus->buflen > 0)
        r = WIDTH(font, focus->value, cursorpos);
    else
	r = 0;

    XSetForeground(display, gc, strikeColor);
    XFillRectangle(display, win, gc, x1 + 3 + r, y2+2, 1, height-4);

    rect.x = focus->x + 2;
    rect.y = focus->baseline - ASCENT(font);
    rect.width = focus->width - 4;
    rect.height = focus->height - 4;
    XSetClipRectangles(display, gc, 0, 0, &rect, 1, Unsorted);
}

/*
   Text field lies in rectangle box, scroll it horizontally
   by delta pixels and repair exposed area. When scrolling to left
   restrict scroll area to right of "start"

   delta is greater than zero for right scrolls
   and less than zero for left scrolls

   Useful for backspace, del char, and cursor motion
   at extreme limits of field
*/

void ScrollField(GC gc, int toleft, int start, int delta)
{
    XRectangle rect;
    int width;

    rect.x  = focus->x + 2;
    rect.y = focus->baseline - ASCENT(font);
    rect.width = focus->width - 4;
    rect.height = focus->height - 4;
    XSetClipRectangles(display, gc, 0, 0, &rect, 1, Unsorted);

    if (delta < 0)  /* scroll left: delta < 0 */
    {
        if (start <= rect.x)
        {
            XCopyArea(display, win, win, gc,
                    rect.x - delta, rect.y,
                    rect.width + delta, rect.y,
                    rect.x, rect.y);

            RepairField(gc, rect.x + rect.width + delta, -delta);
        }
        else if (start < rect.x + rect.width /* width */) /* 12-Mar-96 herman@htbrug.hobby.nl */
        {
            width = rect.width + rect.x - start + delta;

            XCopyArea(display, win, win, gc,
                    start - delta, rect.y,
                    width, rect.y,
                    rect.x, rect.y);

            RepairField(gc, start + width, -delta);
        }
    }
    else  /* scroll right: delta is > 0 */
    {
        if (delta < rect.width)
        {
            XCopyArea(display, win, win, gc,
                    rect.x, rect.y,
                    rect.width - delta, rect.y,
                    rect.x + delta, rect.y);

            RepairField(gc, rect.x, delta);
        }
    }
}


char *WWWEncode(char *raw)
{
    int i,rlen;
    char *encoded,*j;
    
    rlen = strlen(raw);
    encoded=(char *)malloc(3*rlen+1); /* max size = size*3 */
    j=encoded;
    for(i=0;i<rlen;i++)
    {
	if( *(raw+i) ==' ')
	    *j++ = '+';       
	else if ((*(raw+i) == '-')||(*(raw+i) == '_'))
	    *j++=*(raw+i);
	else if ((*(raw+i) < '0' ) || ((*(raw+i)<'A')&&(*(raw+i)>'9'))){
	    *j++ = '%';
	    sprintf(j,"%X",*(raw+i));
	    j+=(*(raw+i)<16)? 1 :2;
	} else if ((*(raw+i) > 'z') || ((*(raw+i)<'a')&&(*(raw+i)>'Z'))){
	    *j++ = '%';
	    sprintf(j,"%X",*(raw+i));
	    j+=2;
	} else 
	    *j++=*(raw+i);
    };
    *j=0;
    j = (char *)malloc(strlen(encoded)+1);
    strcpy(j, encoded); /* we copy the string again to save the extra space allocated */
    Free(encoded);
    return (j);
}


void SubmitForm(Form *form, int method, char *action, int alen, int type,
		 char *name, int nlen, char *value, int vlen, Image *image,int dx, int dy, char *bufpos)
{
    char *query,*encoded,*encode_buffer;
    EditorBuffer *buffer;
    char itoabuff[32];
    Field *field,*field_link;
    Option *option;
    int first,size;

    buffer = CreateBuffer();

    first = (method == GET);

    if (image != NULL)
    {
	if(first)
	    AppendChar(buffer,'?');
	AppendnChar(buffer, name, nlen);
	AppendString(buffer, ".x=");
	sprintf(itoabuff,"%d&", dx);
	AppendString(buffer, itoabuff);
	AppendnChar(buffer, name, nlen);
	AppendString(buffer,".y=");
	sprintf(itoabuff,"%d",dy);
	AppendString(buffer,itoabuff);
	field = FindField(form,type,name,nlen);
	first = 0;
    }
    else
    {
	if(nlen)
	    if(vlen)
	    {
		if(first)
		    AppendChar(buffer,'?');
		AppendnChar(buffer,name,nlen);
		AppendChar(buffer,'=');
		encode_buffer = (char *)calloc( vlen+1, sizeof(char));
		strncpy(encode_buffer,value,vlen);
		*(encode_buffer+vlen)=0;
		encoded = WWWEncode(encode_buffer);
		AppendString(buffer,encoded); /* thanks to Marc Salomon <marc@ckm.ucsf.edu> */
		Free(encode_buffer);
		Free(encoded);
		field = FindField(form,type,name,nlen);
		first = 0;
	    }
	    else
	    {
		if(first)
		    AppendChar(buffer,'?');
		AppendnChar(buffer,name,nlen);
		AppendString(buffer,"=Submit+Query");
		field = FindField(form,type,name,nlen);
		first = 0;
	    }
	else
	{
	    first = (method == GET) ? 1 : 2;
	    field = FindField(form,type,bufpos,7);
	}
    };
    for( field_link=form->fields ; field_link!=NULL; field_link = field_link->next)
    {
	if( field_link!= field)
	{
	    switch (field_link->type)
	    {
	    case SUBMITBUTTON:
	    case RESETBUTTON:
		break;
	    case RADIOBUTTON:
		if(!(field_link->flags & CHECKED))
		    break;
	    case HIDDEN:	
	    case TEXTFIELD:
		if(first)
		{
		    if(first == 1)
			AppendChar(buffer,'?');
		    first=0;
		}
		else
		    AppendChar(buffer,'&');

		encode_buffer = (char *) calloc( field_link->nlen+1, sizeof(char));
		strncpy(encode_buffer, field_link->name, field_link->nlen);
		*(encode_buffer+field_link->nlen)=0;
		encoded = WWWEncode(encode_buffer);
		AppendString(buffer, encoded);
		Free(encoded);
		Free(encode_buffer);
		AppendChar(buffer, '=');
		encode_buffer = (char *) calloc( field_link->buflen+1, sizeof(char));
		strncpy(encode_buffer,field_link->value,field_link->buflen);
		*(encode_buffer+field_link->buflen)=0;
		encoded = WWWEncode(encode_buffer);
		AppendString(buffer, encoded);
		Free(encoded);
		Free(encode_buffer);
		break;
	    case CHECKBOX:
		if(field_link->flags & CHECKED)
		{
		    if(first)
		    {
			if(first == 1)
			    AppendChar(buffer,'?');
			first = 0;
		    }
		    else
			AppendChar(buffer,'&');
		    encode_buffer = (char *) calloc( field_link->nlen+1, sizeof(char));
		    strncpy(encode_buffer, field_link->name, field_link->nlen);
		    *(encode_buffer+field_link->nlen)=0;
		    encoded = WWWEncode(encode_buffer);
		    AppendString(buffer, encoded);
		    Free(encoded);
		    Free(encode_buffer);
		    AppendString(buffer, "=on");
		};
		break; 
	    case OPTIONLIST:
		if(field_link->flags & MULTIPLE)
		{
		    for (option = field_link->options; option; option = option->next)
		    {
			if((option->flags & CHECKED) == CHECKED)
			{
			    if(first)
			    {
				if(first == 1)
				    AppendChar(buffer,'?');
				first = 0;
			    }
			    else
				AppendChar(buffer,'&');
			    
			    encode_buffer = (char *) calloc( field_link->nlen+1, sizeof(char));
			    strncpy(encode_buffer, field_link->name, field_link->nlen);
			    *(encode_buffer+field_link->nlen)=0;
			    encoded = WWWEncode(encode_buffer);
			    AppendString(buffer, encoded);
			    Free(encoded);
			    Free(encode_buffer);
			    AppendChar(buffer, '=');
			    size = strlen(option->label);
			    if(size)
				if(field_link->value[size-1]==' ')
				    field_link->value[size-1]=0;  /* "eat" the last blank */
			    encode_buffer = (char *) calloc( strlen(option->label)+1, sizeof(char));
			    strcpy(encode_buffer,option->label);
			    encoded = WWWEncode(encode_buffer);
			    AppendString(buffer, encoded);
			    Free(encoded);
			    Free(encode_buffer);
			};
		    }
		}
		else
		{
		    if(first)
		    {
			if(first == 1)
			    AppendChar(buffer,'?');
			first = 0;
		    }
		    else
			AppendChar(buffer,'&');
		    
		    encode_buffer = (char *) calloc( field_link->nlen+1, sizeof(char));
		    strncpy(encode_buffer, field_link->name, field_link->nlen);
		    *(encode_buffer+field_link->nlen)=0;
		    encoded = WWWEncode(encode_buffer);
		    AppendString(buffer, encoded);
		    Free(encoded);
		    Free(encode_buffer);
		    AppendChar(buffer, '=');
		    size = strlen(field_link->value);
		    if(size)
			if(field_link->value[size-1]==' ')
			    field_link->value[size-1]=0;  /* "eat" the last blank */
		    encode_buffer = (char *) calloc( strlen(field_link->value)+1, sizeof(char));
		    strcpy(encode_buffer,field_link->value);
		    encoded = WWWEncode(encode_buffer);
		    AppendString(buffer, encoded);
		    Free(encoded);
		    Free(encode_buffer);
		};
		break;
	    };
	};
    }
    switch(method)
    {
    case GET:
	if(action)
	    InsertnChar(buffer,0,action,alen);
	else
	    InsertString(buffer,0,CurrentDoc->href);
	query = Buffer2Str(buffer);
/*	printf("fetching |%s|\n",query); */
	OpenDoc(query);
	break;
    case POST:
	encoded = Buffer2Str(buffer);
	query = (char *) calloc(alen+1, sizeof(char));
	strncpy(query, action, alen);
	*(query+alen)=0;
	PostDoc(query, encoded); 
/*	printf("posting [%s][%s]\n",query,encoded); */
	break;
    };
}

void PaintFieldCursorBuffer(GC gc, unsigned long color, EditorBuffer *buffer, Field *field)
{
    int font, width, height, x1, y2, r;
    int numline;
    int numcol;

    font = field->flags & 0x0F;
    numline = LineNumber(buffer);
    width = field->width;
    height = field->height;
    x1 = field->x + field->frame_indent - PixelIndent - field->x_indent ;
    y2 = Abs2Win(field->baseline) + (numline-1)*(ASCENT(font)+4);
    ClipIntersection(gc, x1+2, Abs2Win(field->baseline) - ASCENT(font) - 4, width-4, height);
    numcol = ColNumber(buffer);
    if (buffer->size > 0)
        r = WIDTH(font, field->value + buffer->pos - numcol, numcol);
    else
        r = 0;
    
    XSetForeground(display, gc, color);
    XFillRectangle(display, win, gc, x1 + 7 + r, y2+4, 1,ASCENT(font)+4);
    XSetForeground(display, gc, textColor);
    XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);
}

#if 0
void PaintFieldEditor(GC gc, int indent, int baseline, Field *field) /* to display editor --Spif 28-Nov-95 */
{
    char *s,*pos,*dump;
    int font, active, width, height, x1, y1, y2, r;
    Option *option;
    
    active = field->flags & CHECKED;
    font = field->flags & 0x0F;
    
    width = field->width;
    height = field->height;
    
    /* cache absolute position of baseline */
    field->baseline = Win2Abs(baseline);

    /* kludge for fields in different frames */
    if (indent >= 0)
        field->frame_indent = indent;
    else
        indent = field->frame_indent;

    if (field->type == TEXTFIELD)
    {
        x1 = indent + field->x - PixelIndent - field->x_indent;
        y2 = baseline - ASCENT(font) - 2;
        ClipIntersection(gc, x1, y2, width, height);
        XSetForeground(display, gc, (active ? statusColor : windowColor));
        XFillRectangle(display, win, gc, x1, y2, width, height);
        DrawInSet(win, gc, x1, y2, width, height);
        ClipIntersection(gc, x1+2, y2, width-4, height);

        if (field->buflen > 0)
        {
	    dump = (char *)malloc(strlen(field->value)+1);
	    strcpy(dump,field->value);
	    pos = dump;
	    while(*pos)
		if(*pos=='\n')
		    *pos++=0;
		else
		    pos++;
	    pos = dump;
	    while(pos-dump<field->buflen)
	    {
		printf("[%s]\n",pos);
		y1 = y2 + ASCENT(font) + 2;
		SetFont(gc, font);
		XSetForeground(display, gc, textColor);
		XDrawString(display, win, gc, x1+4, y1, pos, strlen(pos));
		y2 += ASCENT(font)+4;
		pos += strlen(pos)+1;
	    };
	    Free(dump);
        }

        if (active)
        {
            if (field->buflen > 0)
                r = WIDTH(font, field->value, cursorpos);
            else
                r = 0;

            XSetForeground(display, gc, strikeColor);
            XFillRectangle(display, win, gc, x1 + 3 + r, y2+2, 1, SPACING(Fonts[font]));
        }

        XSetForeground(display, gc, textColor);
        XSetClipRectangles(display, gc, 0, 0, &displayRect, 1, Unsorted);
    }
}
#endif
