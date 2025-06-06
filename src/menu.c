#include <stdio.h>
#include "www.h"

extern Display *display;
extern int screen;
extern int depth;
extern Visual *visual;
extern unsigned long labelColor, windowColor, windowBottomShadow, windowTopShadow;
extern Colormap colormap;
extern int statusHeight, ToolBarHeight, sbar_width, buf_height, buf_width;
extern unsigned int win_height, win_width, display_width, display_height;
extern int button_x, button_y, PixelIndent;
extern Doc *CurrentDoc;
extern Context *context;
extern char *anchor_start, *anchor_end;
extern Byte *WhichObject(int event, int x, int y,
                         int *type, char **start, char **end, int *dx, int *dy);

extern int debug;
extern int ColorStyle;

Cursor menucursor;
Window pop;
GC pop_gc;
XFontStruct *popf;


MenuRoot *mr;

int itemheight, r, l;

MenuRoot *NewMenuRoot()
{
  MenuRoot *tmp;  
  tmp = (MenuRoot *)malloc(sizeof(MenuRoot));
  tmp->name = 0;
  tmp->first = NULL;
  tmp->last = NULL;
  tmp->items = 0;
  tmp->w = 0;
  tmp->h = 0;
  tmp->y_offset = 0;
  tmp->x = 0;
  tmp->y = 0;
  tmp->x_root = 0;
  tmp->y_root = 0;
  return (tmp);
}

MenuItem *NewMenuItem()
{
    MenuItem *tmp;

    tmp = (MenuItem *)malloc(sizeof(MenuItem));
    tmp->next = NULL;
    tmp->label = (char *)malloc(MAX_ITEMLENGTH +1);
    tmp->url = 0;
    tmp->action = 0;
    tmp->item_pos = 0; 
    tmp->y = 0;
    tmp->state = 0;
    tmp->strlen = 0;
    return (tmp);
}

static void destroyItem(MenuItem *mi)
{
    if (mi)
    {
        destroyItem(mi->next);
        mi->label = 0;
    }
    
    mi = NULL;
}
#if 0
static void destroyMenu(MenuRoot *mr)
{
    if (mr->first)
        destroyItem(mr->first);
    mr = NewMenuRoot();
}
#endif
void printItems(MenuItem *mi)
{
    MenuItem *tmp;
    tmp = mi;
    
    if (tmp)
    {
	fprintf(stderr, "item %d: %s\n", tmp->item_pos, tmp->label);
        printItems(mi->next);
    }
    
}


/*initial stuff to do at startup time */
void initPopup(XFontStruct *pf)
{
    popf = pf;
        /* define height of items */
    mr = NewMenuRoot ();
    itemheight = 12 + pf->max_bounds.ascent + pf->max_bounds.descent;
        /* define position for popup-title string */
    r = popf->max_bounds.ascent + 3;
   
        /* define position for separator line below title */
    l = r + popf->max_bounds.descent + 2;
    mr->y_offset = l + 4 + 1;
        /* create cursor */
    menucursor = XCreateFontCursor(display, XC_draft_large);
}
#if 0
static char *findTitle(char *buf)
{
    int i = 0, j = 0;
    char c;
    char *s;
    char *tmp;
    tmp = (char *)malloc(MAX_ITEMLENGTH + 1);
    if (!buf)
        return 0;
    
    if ((s = strstr(buf, "<TITLE>")) || (s = strstr(buf, "<title>"))
        || (s = strstr(buf, "<Title>")))    
    {
        s += 7;
        c = *s;
    
        while (c != '<' && j != MAX_ITEMLENGTH - 1)
        {
            switch (c)
            {
                case '\n':
                case '\t':
                        break;
                default:
                    tmp[j++] = c;
                    
                    
            }
            s++;
            c = *s;
        }
        tmp[j] = '\0';
    }
    
    return (tmp);
}
#endif
                
            
/* is the pointer on an anchor?*/
static Bool onAnchor(int x, int y, char *label)
{
    Byte *object;
    char *start, *end, *href, *name;
    int tag, hreflen, namelen, align, ismap, dx, dy, width, height;
 
    /* the window should only be clickable for certain content_types and modes */
 
    if (!DocHTML(CurrentDoc))
        return FALSE;
 
    if (CurrentDoc->show_raw)
        return FALSE;
 
    
    tag = UNKNOWN;
    object = WhichObject(BUTTONUP, x, y, &tag, &start, &end, &dx, &dy);
 
    if ((tag == TAG_ANCHOR || tag == TAG_IMG) &&
                start == anchor_start && end == anchor_end)
    {
        if (tag == TAG_IMG)
        {
            ParseImageAttrs(&href, &hreflen, &align, &ismap, &width, &height);
        }
        else /* tag == TAG_ANCHOR */
        {
	    int class_len; 
	    char *class = NULL;

            ParseAnchorAttrs(&href, &hreflen, &name, &namelen, &class, &class_len);
        }
        
        label = href;
        label[hreflen] = '\0';
        return TRUE;
        
    }

    return FALSE;
}

/* make menuitem for anchor popup */
static void makeAnchorItem(int x, int y, char *label)
{
    MenuItem *mi;
    
    mi = NewMenuItem();
    mi->label = label;
    mi->x = mr->x + 4;
    mi->y = mr->y_offset;
    mi->strlen = strlen(mi->label);
        /* rest has default value */
    mr->first = mi;
}


/*  make menu-items for history-popup, recursively*/
static void makeHistoryItem(MenuItem *mi, int pos)
{
    MenuItem *tmp;
    
    History *h;
    Doc *d;
    char *title, *s;
    int this_one = pos;
    int i, current = context->history_pos;
    
    if (MENU_TRACE)
	HistoryList();
    
    tmp = mi;
    
    title = 0;
    
        /* if there are still items in the list  or the list is not too long, make a new item */
    if ( (this_one < HTList_count(context->history)) && (this_one < MAX_ITEMS) )
    {

        
        h = (History *)HTList_objectAt(context->history, this_one);
        
        if (h->state != HISTORY_VERIFIED || this_one == current )
            mi->action = NO_ACTION;
/*
        d = (Doc *)h->anchor->parent->document;
        if (!d)
        {
            s = "??";
            mi->url = "";
        }
        else
        {
        s = findTitle(d->content_buffer);
            mi->url = d->url;
            
            if (!s)
            {
            s = d->url;
            
            }
*/
        if (h->title)
            s = strdup(h->title);
        else
        {
            s = strdup(h->anchor->parent->physical);
            i = strlen(s);
            if (i > MAX_ITEMLENGTH+1)
            {
                s = s + (i - MAX_ITEMLENGTH);
                s[0] = '.';
                s[1] = '.';
                s[2] = '.';
            }
            
        }
        
        if (strlen(s) > MAX_ITEMLENGTH )
        {
            title = s;
            title[MAX_ITEMLENGTH + 1] = '\0';
            
            for (i = 0; i < 3; i++) title[MAX_ITEMLENGTH - i] = '.';
        }
        else
            title = s;
        
        mi->label = title;
        mi->x = 4;
        mi->y = ( mr->items != 0 ? (mr->items * itemheight) + mr->y_offset :
                  mr->y_offset );
/*         mi->url = d->url; */
        mi->action = VISIT_HISTORY;
        mi->item_pos = this_one;
        mi->strlen = strlen(mi->label);
/*         mr->last = mi; */
        mr->items++;
        mr->last = mi;

        this_one++;
        
        if ( (this_one < HTList_count(context->history)) && (this_one < MAX_ITEMS) )
        {
            mi->next = NewMenuItem();
            makeHistoryItem(mi->next, this_one);
        }
        
    }
    
    if (mi)
    {
        mr->first = mi;    
    }
    
}

/*  create the menu-items and add them to menuroot*/
static void makeMenu(int x, int y, int status, char *label)
{
    int charwidth;
    
    mr->x = x;
    mr->y = y;
    mr->items = 0;
        /* forget about previous menu */
/*     if (mr->first) */
/*         destroyItem(mr->first); */
    
    if (status == POPUP_ANCHOR)
    {
        mr->name = ANCHOR_POPUP_TITLE;
        mr->w = XTextWidth(popf, label, strlen(label)) + 6;
        makeAnchorItem(mr->x, mr->y, label);
        
    }
    else
    {
        mr->name = HISTORY_POPUP_TITLE;
        charwidth = XTextWidth(popf, "ABCabc", 6)/6;
        mr->w = (MAX_ITEMLENGTH * charwidth) + 6;
        if (mr->first)
            destroyItem(mr->first);
        
        mr->first = NewMenuItem();
        
        makeHistoryItem(mr->first, 0);

    }
    mr->h = itemheight * (mr->items) + l;
    
}


/*  creation of window, GC,  setting attrs, and mapping*/
static void createPopup(int x, int y, unsigned int w, unsigned int h)
{
    XSetWindowAttributes setwinattr;
    unsigned long valuemask;

        /* decide whether x and y are ok or need to be modified */
    x = (x + w > display_width ? display_width - w - 1 : x);
    y = (y + h > display_height ? display_height - h - 1 : y);

    mr->x = x;
    mr->y = y;

        /* create window */
    pop = XCreateSimpleWindow(display, RootWindow(display, screen), x, y, w, h, 0,
                        BlackPixel(display,screen), WhitePixel(display, screen));

        /* define a cursor  */
    menucursor = XCreateFontCursor(display, XC_draft_large);

        /* set winattrs */
    XDefineCursor(display, pop, menucursor);
    
    valuemask =  CWOverrideRedirect | CWEventMask ;
    if(ColorStyle != COLOR888)
	valuemask |= CWColormap;
    setwinattr.override_redirect = True;
    if(ColorStyle != COLOR888)
	setwinattr.colormap = colormap;
    setwinattr.event_mask =
        ExposureMask | ButtonReleaseMask | Button3MotionMask | OwnerGrabButtonMask;

    
    XChangeWindowAttributes(display, pop, valuemask, &setwinattr);

        /* create GC for pop */
    pop_gc = XCreateGC(display, pop, 0, NULL);
    XSetFont(display, pop_gc, popf->fid);


        /* map the popup window */
    XMapWindow(display, pop);

}
     

/*  draw menu-items in the window*/
static void drawEntry(MenuItem *mi)
{
    int y;
    if (mi)
    {
        
        y = mr->y_offset + r + (mi->item_pos * itemheight);
    
        XDrawString(display, pop, pop_gc, 3, y, mi->label, strlen(mi->label));

        drawEntry(mi->next);
    }
}




static void drawMenu(int x, int y, unsigned int w, unsigned int h)
{
    XRectangle rect;
    rect.x = 0;
    rect.y = 0;
    rect.width = w;
    rect.height = h;
    

        /* fill the window */
    XSetForeground(display, pop_gc, windowColor);
    XSetClipRectangles(display, pop_gc, 0, 0, &rect, 1, Unsorted);
    XFillRectangle(display, pop, pop_gc, 0, 0, w, h);
        /* create shadow borders */
    DrawOutSet(pop, pop_gc, 0, 0, w, h);
    
        /* create separator lines */

    XDrawLine(display, pop, pop_gc, 2, l + 2, w - 2, l + 2);
    XSetForeground(display, pop_gc, windowBottomShadow);
    XDrawLine(display, pop, pop_gc, 2, l, w - 2, l);
    XDrawLine(display, pop, pop_gc, 2, l + 3, w - 2, l + 3);
    XSetForeground(display, pop_gc, windowTopShadow);
    XDrawLine(display, pop, pop_gc, 2, l + 1, w - 2, l + 1);
    XDrawLine(display, pop, pop_gc, 2, l + 4, w - 2, l + 4);
        /* draw the title of the menu  (mr->name) */
    XSetForeground(display, pop_gc, labelColor);
    XDrawString(display, pop, pop_gc, 6, r, mr->name, strlen(mr->name));

    drawEntry(mr->first);
}

void RemoveOutset(Window aWin, GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, windowColor);

    XFillRectangle(display, aWin, gc, x, y, w, 1);
    XFillRectangle(display, aWin, gc, x, y+1, w-1, 1);
    XFillRectangle(display, aWin, gc, x, y, 1, h);
    XFillRectangle(display, aWin, gc, x+1, y+1, 1, h-1);

    XFillRectangle(display, aWin, gc, x, y+h-1, w, 1);
    XFillRectangle(display, aWin, gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, aWin, gc, x+w-1, y, 1, h);
    XFillRectangle(display, aWin, gc, x+w-2, y+1, 1, h-1);
}

/*  destroy popup */
static void unPopup()
{
    XUndefineCursor(display, pop);
    XUnmapWindow(display, pop);
    XDestroyWindow(display, pop);
}

static void executeAction(MenuItem *mi)
{
    if (mi->action == VISIT_HISTORY)
    {
        GotoDoc(mi->item_pos);
    }
}

static void warpPointer()
{
    MenuItem *mi = mr->first;
    
    if (mi->item_pos == context->history_pos)
        XWarpPointer(display, pop, pop, mr->x, mr->y, 0, 0, mi->x, mi->y);
    else
        while ((mi = mi->next))
            if (mi->item_pos == context->history_pos)
                XWarpPointer(display, pop, pop, mr->x, mr->y, 0, 0, mi->x, mi->y);
}



                     
/*  events while popup active*/
static void popupEventLoop()
{
    XEvent e;
    MenuItem *mi = NULL, *latest = NULL;
    int i, y;
    
    if (MENU_TRACE)
	printItems(mr->first);
    
    while (1)
    {
	if (MENU_TRACE)
	    fprintf(stderr, "in enless loop\n");
        XNextEvent(display, &e);
        
        if (e.type == Expose)
        {
	    if (MENU_TRACE)
		fprintf(stderr, "in enless loop: expose\n");
/*             while (XCheckTypedEvent(display, Expose, &e));     */
             drawMenu(mr->x, mr->y, mr->w, mr->h);
        }
        else if (e.type == GraphicsExpose)
        {
	    if (MENU_TRACE)
		fprintf(stderr, "in enless loop: graphics expose\n");
            drawMenu(mr->x, mr->y, mr->w, mr->h);
        }
        else if (e.type == MotionNotify)
        {
	    if (MENU_TRACE)
		fprintf(stderr, "got motion\n");
            
            if (e.xmotion.x > mr->x && e.xmotion.x < mr->x + mr->w &&
                e.xmotion.y > mr->y && e.xmotion.y < mr->y + mr->h)
                for (i = 0, mi = mr->first; i < mr->items + 1 && mi != NULL; i++, mi = mi->next)
                    if (e.xmotion.y > mi->y && e.xmotion.y < mi->y + itemheight)
                        if (mi->action != NO_ACTION && mi != latest)
                        {
                      
                            if (latest)
                                RemoveOutset(pop, pop_gc, latest->x, latest->y,
                                             mr->w, itemheight);

                            DrawOutSet(pop, pop_gc, mi->x, mi->y, mr->w, itemheight);

                            if (!latest)
                                latest = NewMenuItem();
                            
                            latest = mi;
                            break;
                        }
                        else
                            break;
                    
            else
            {
                if (latest)
                {
                    RemoveOutset(pop, pop_gc, latest->x, latest->y, mr->w, itemheight);
                    latest = NULL;
                }
                
            }
        }
        
        else if (e.type == ButtonRelease)
        {
	    if (MENU_TRACE)
		fprintf(stderr, "in enless loop: buttonrelease\nbutton.x=%d\nbutton.y=%d\n"
                    "x=%d, y=%d\n",
                    e.xbutton.x_root, e.xbutton.y_root, mr->x, mr->y);
            unPopup();
            if (e.xbutton.x_root > mr->x && e.xbutton.x_root < mr->w + mr->x &&
                e.xbutton.y_root > mr->y && e.xbutton.y_root < mr->h + mr->y)
            {

		if (MENU_TRACE)
		    fprintf(stderr, "got pos\n");
                for (i = 0, mi = mr->first; i < mr->items + 1 && mi != NULL; i++, mi = mi->next)
                {

		    if (MENU_TRACE)
			fprintf(stderr, "item: y = %d\n", mi->y);
                    y = mr->y + mi->y;
                    if (e.xbutton.y_root > y && e.xbutton.y_root < y + itemheight)
                    {
			if (MENU_TRACE)
			    fprintf(stderr, "got item!\n");
                        
                        if (mi->action != NO_ACTION)
                            executeAction(mi);
                    }
                }
                
            }
            
            break;
        }
        
    }
}

/*  function called from main program to pop up the menu*/
void Popup(int x, int y)
{
    int status;
    char *label=NULL;
    if (onAnchor(x, y, label))
        status = POPUP_ANCHOR;
    else
        status = POPUP_HISTORY;
    
    makeMenu(x, y, status, label);
    createPopup(mr->x, mr->y, mr->w, mr->h);
    drawMenu(mr->x, mr->y, mr->w, mr->h); 
    warpPointer();
    popupEventLoop();    
}




/* test popup --------------------------------------------------- */

#ifdef TEST_POPUP
extern int depth;
extern Visual *visual;
extern unsigned long textColor, labelColor, windowColor, windowBottomShadow,
    windowTopShadow;
extern Colormap colormap;

void Popup(int x_root, int y_root, int x, int y, GC aGC, XFontStruct *pf)
{
    Window pop;
    XFontStruct *popf;
    XEvent e;
    GC pop_gc;
    XRectangle rect;
    XSetWindowAttributes setwinattr;
    unsigned int width = 100;
    unsigned int height = 100;
    unsigned long valuemask;/*, valmask;*/
    
    int r, l;
    char *s;
       
    popf = pf;
    
    s = "History";

    rect.x = 0;
    rect.y = 0;
    rect.width = width;
    rect.height = height;
   
    
    menucursor = XCreateFontCursor(display, XC_draft_large);
    
    valuemask =  CWOverrideRedirect | CWEventMask/* | (ColorStyle != COLOR888)? CWColormap: 0;*/

    setwinattr.override_redirect = True;
    /*  if(ColorStyle != COLOR888)
	setwinattr.colormap = colormap;*/
    
/*     janet: setwinattr.save_under = True; */
    
    setwinattr.event_mask =
        ExposureMask | ButtonReleaseMask;

/*     janet: pop = XCreateWindow(display, RootWindow(display), x, y, width, height, 0, */
/*                         janet: depth, InputOnly, visual,  */
/*                         janet: valuemask, &setwinattr); */

    pop = XCreateSimpleWindow(display, RootWindow(display, screen),x_root, y_root,
                              width, height, 0, BlackPixel(display, screen),
                              WhitePixel(display, screen));
    
    
    XDefineCursor(display, pop, menucursor);
    XChangeWindowAttributes(display, pop, valuemask, &setwinattr); 
        
    pop_gc = XCreateGC(display, pop, 0, NULL);
    XSetFont(display, pop_gc, popf->fid);
    r = popf->max_bounds.ascent + 2;
    l = r + popf->descent + 1;

    XMapWindow(display, pop);
    XSetForeground(display, pop_gc, windowColor);
    XSetClipRectangles(display, pop_gc, 0, 0, &rect, 1, Unsorted);
    XFillRectangle(display, pop, pop_gc, 0, 0, width, height);
    XDrawLine(display, pop, pop_gc, 2, l + 2, width-2, l + 2);
    DrawMenuOutSet(pop, pop_gc, 0, 0, width, height);
    
    XSetForeground(display, pop_gc, labelColor);
    XDrawString(display, pop, pop_gc, 3, r, s, strlen(s));
        /* shadow separator */
    XSetForeground(display, pop_gc, windowBottomShadow);
    XDrawLine(display, pop, pop_gc, 2, l, width-2, l);
    XDrawLine(display, pop, pop_gc, 2, l + 3, width-2, l + 3);
    XSetForeground(display, pop_gc, windowTopShadow);
    XDrawLine(display, pop, pop_gc, 2, l + 1, width-2, l + 1);
    XDrawLine(display, pop, pop_gc, 2, l + 4, width-2, l + 4);

    while (1)
    {
        XNextEvent(display, &e);
        if (e.type == Expose)
        {
            while (XCheckTypedEvent(display, Expose, &e));
          
        }
        else if (e.type == ButtonRelease)
            break; 
    }
    
    XUndefineCursor(display, pop);
    XUnmapWindow(display, pop);
    XDestroyWindow(display, pop);
 
    
}

void DrawMenuOutSet(Window aWin, GC gc, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(display, gc, windowTopShadow);
 
    XFillRectangle(display, aWin, gc, x, y, w, 1);
    XFillRectangle(display, aWin, gc, x, y+1, w-1, 1);
    XFillRectangle(display, aWin, gc, x, y, 1, h);
    XFillRectangle(display, aWin, gc, x+1, y+1, 1, h-1);
 
    XSetForeground(display, gc, windowBottomShadow);
 
    XFillRectangle(display, aWin, gc, x, y+h-1, w, 1);
    XFillRectangle(display, aWin, gc, x+1, y+h-2, w-1, 1);
    XFillRectangle(display, aWin, gc, x+w-1, y, 1, h);
    XFillRectangle(display, aWin, gc, x+w-2, y+1, 1, h-1);
}
#endif
                
