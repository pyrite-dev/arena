#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/utsname.h>

#if 0
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef __hpux
#include <X11/HPkeysym.h>
#endif

#endif

#ifndef NFDBITS
#define NFDBITS 0
#endif

#include "www.h"
#include "popup.h"
#include "style.h"
#include "neweditor.h"

#ifndef XK_DeleteChar
#define XK_DeleteChar XK_Delete
#endif

#define BITMAPDEPTH 1

/* values for window_size in main, is window big enough to be useful */

#define SMALL 1
#define OK 0

extern int statusHeight;
extern int sbar_width;
extern int ToolBarHeight;
extern long PixelOffset;
extern long buf_height;
extern int PixelIndent;
extern int buf_width;
extern char *buffer;
extern int hdrlen;
extern int Authorize;
extern int OpenURL;
extern int IsIndex;
extern int SaveFile;
extern int FindStr;
extern char *FindStrVal, *FindNextStr;
extern int font;
extern Field *focus;
extern Frame background;
extern Byte *paint;
extern Bool TextFieldFocus;
extern double lens_factor;
extern GC disp_gc, gc_fill;
extern unsigned char *gamma_table;

int debug = 0;        /* used to control reporting of errors */
int initialised = 0;  /* avoid X output until this is true! */
int busy = 0;         /* blocks hypertext links etc while receiving data */
int UseHTTP2 = 1;     /* use new HTRQ/HTTP protocol */
int OpenSubnet = 0;   /* if true host is on OpenSubnet */
int UsePaper = 1; 
int fontsize = 0;
int NoTerminal = 0; /* howcome 5/11/94: can we ask for passwd? */
char *CacheRoot = "/tmp"; /* howcome 5/10/94: command line option to turn caching on */
char *Printer = NULL; /* howcome 5/10/94: command line option enables printing */
char *Editor = NULL; /* howcome 5/10/94: command line option turns the editor on */
int  Badflags = 5; /* howcome 5/10/94: how many badflags do we want in the output */
char *Icon = NULL; /* howcome 5/10/94: command line option turns the use of icon */
double Gamma = 2.2; /* howcome 7/11/94: made into command line option */
int NoMailcap = 0; /* howcome 7/11/94 */
BOOL NoStyle = FALSE; /* howcome 2/3/95 */
BOOL Quiet = FALSE; /* howcome 10/8/95 */
BOOL OwnColorMap = FALSE; /* howcome 10/8/95 */
Atom ARENA_LOCATION; 
Atom ARENA_COMMAND;

long main_stack[256];
long *stack;

int library_trace = 0; /* howcome 20/11/94 */

Doc *CurrentDoc;
Context *context;

extern float dpi();

/* Display and screen are used as arguments to nearly every Xlib
 * routine, so it simplifies routine calls to declare them global.
 * If there were additional source files, these variables would be
 * declared `extern' in them.
 */

Display *display;
Visual *visual;
Colormap colormap;
int screen;
int depth;
Window win;
int ExposeCount;   /* used to monitor GraphicsExpose events during scrolling */
char *prog;        /* name of invoking program */
int default_pixmap_width, default_pixmap_height;
Pixmap background_pixmap, default_pixmap;
Pixmap icon_pixmap;

Cursor hourglass;
int shape;

unsigned long textColor, labelColor, windowColor, strikeColor, statusColor,
       transparent, windowTopShadow, windowBottomShadow, windowShadow;

#ifdef SELECTION
unsigned long statusSelectColor; /* howcome */
static char *selectStr=NULL;		/* howcome 2/8/94 */
#endif

int charWidth, charHeight, charAscent, lineHeight;
unsigned int win_width, win_height, tileWidth, tileHeight;
unsigned char *tileData;

/*int document;*/
int gadget;
int gatewayport = 3000; /* GATEWAYPORT; */
char *gateway;      /* gateway if can't access server directly */
char *help;
/* char *printer; */ /* howcome 16/10/94 */
char *startwith;    /* default initial document */
char *user;

int RepeatButtonDown = 0;

XFontStruct *fixed_font;
XFontStruct *Fonts[FONTS];  /* array of fonts */
char *FontsName[FONTS];
int LineSpacing[FONTS];
int BaseLine[FONTS];
int StrikeLine[FONTS];
int LineThickness[FONTS];

int ListIndent1, ListIndent2;

GC gc_fill;
static GC gc_scrollbar, gc_status, gc_text;
static char *display_name = NULL;
static int window_size = 0;    /* OK or SMALL to display contents */
static int button_x, button_y;
static XSizeHints size_hints;
int ColorStyle;

unsigned int display_width, display_height;
double display_pt2px;

struct itimerval itimer, old_itimer;


void ShowBusy()
{
    XDefineCursor(display, win, hourglass);
    XFlush(display);
    busy = 1;
}

void HideBusy()
{
    XUndefineCursor(display, win);
    XFlush(display);
    busy = 0;
}



/* From Scott Nelson <snelson@canopus.llnl.gov> 24 bit color  1/12/94 */
/* these are here for speed reasons */
int RPixelShift=16, GPixelShift=8, BPixelShift=0;
int RPixelMask=0xff, GPixelMask=0xff, BPixelMask=0xff;


/* pause for delay milliseconds */
/* howcome moved this from tcp.c 20/9/94 */

void Pause(int delay)
{
    struct timeval timer;

    timer.tv_sec = 0;
    timer.tv_usec = delay * 1000;

    select(NFDBITS, 0, 0, 0, &timer);
}


void CatchCore()
{
    fprintf(stderr,"CATCH_CORE %d%c\n",(int)getpid(),7);
    Exit(1);
}

void CatchQuit()
{
    fprintf(stderr,"CATCH_QUIT\n");
    Exit(1);
}

void CatchX(Display * err_dpy_p, XErrorEvent *err_event)
{
    fprintf(stderr,"catchX pid %d\n", (int)getpid());
}

/* howcome 4/10/94: need to clean the library cache.. */
int Exit(int i)
{
    if (VERBOSE_TRACE)
        printf("bridge.c: Exit\n");
    libExit(0);
    printf("\nThanks for using %s %s. The authors -- Dave, Hakon, Henrik \nand Yves -- can be reached at arena@w3.org. \nPlease check %s \nbefore reporting bugs.\n", BANNER, VERSION, HELP_URL);
    exit(i);
}



/* find best visual for specified visual class */
Visual *BestVisual(int class, int *depth)
{
    long visual_info_mask;
    int number_visuals, i, best_depth;
    XVisualInfo *visual_array, visual_info_template;
    Visual *best_visual;

    visual_info_template.class = class;
    visual_info_template.screen = DefaultScreen(display);
    visual_info_mask = VisualClassMask | VisualScreenMask;

    visual_array = XGetVisualInfo(display, visual_info_mask,
                        &visual_info_template,
                        &number_visuals);

    *depth = 0;
    best_depth = 0; /* *depth */
    best_visual = 0;
	 
    for (i = 0; i < number_visuals; ++i)
    {
        if (visual_array[i].depth > *depth)
        {
            best_visual = visual_array[i].visual;
            *depth = visual_array[i].depth;
        }

        if (*depth == best_depth)
            break;
    }

    XFree((void *)visual_array);
    return best_visual;
}


#ifdef SELECTION

void SetSelection(char *s) /* howcome 2/8/94 */
{
  Free(selectStr);
  selectStr = strdup(s);
  XSetSelectionOwner(display, XA_PRIMARY, win, CurrentTime);
  XStoreBytes(display, selectStr, strlen(selectStr));
}

#endif


/* send myself an expose event to force redrawing of give rectangle */

void Redraw(int x, int y, int w, int h)
{
    XExposeEvent event;

    event.type = Expose;
    event.serial = 0;
    event.send_event = 1;
    event.display = display;
    event.window = win;
    event.x = x;
    event.y = y;
    event.width = w;
    event.height = h;
    event.count = 0;

    if (X_TRACE)
	fprintf(stderr,"Redraw\n");
    XSendEvent(display, win, 0, ExposureMask, (XEvent *)&event);
}

void SetBanner(char *title)
{
    XTextProperty textprop;

     /* Support multiple <TITLE>s in a document */
    Free(CurrentDoc->title);

    CurrentDoc->title = strdup(title);
    XStoreName(display, win, title);
    if(CurrentDoc->url)
    {
	char *v;
	int v_size;
	v_size = strlen(title)+strlen(CurrentDoc->url)+13; /* 13: 'URL='(string)\0'TITLE='(string)\0 \0 */ 
	v = (char *)calloc(v_size, sizeof(char));
	sprintf(v, "URL=%s%cTITLE=%s",CurrentDoc->url,0,title);
	XChangeProperty(display, win, ARENA_LOCATION, XA_STRING, 8, PropModeReplace,
			v, v_size);
	Free(v);
    }
/*
    if (strlen(title) > 8 && (p = strrchr(title, '/')))
    {
            if (p[1] != '\0')
                ++p;

            title = p;
    }
*/
    textprop.value = (unsigned char *)title;
    textprop.encoding = XA_STRING;
    textprop.format = 8;
    textprop.nitems = strlen(title);

    XSetWMIconName(display, win, &textprop);
}

void LoadFont(int idx, char *font_name, char *fall_back)
{
    int direction_hint, font_ascent, font_descent;
    XFontStruct *font;
    XCharStruct overall;
    char *test = "Testing";

    if ((Fonts[idx] = XLoadQueryFont(display, font_name)) == NULL)
    {
        (void) fprintf(stderr, "Cannot open %s font\n", font_name);

        if ((Fonts[idx] = XLoadQueryFont(display, fall_back)) == NULL)
        {
            (void) fprintf(stderr, "Cannot open alternate font: %s\n", fall_back);
            Exit(1);
        }

        fprintf(stderr, "Using alternate font: %s\n", fall_back);
    }

    font = Fonts[idx];

    XTextExtents(font, test, strlen(test),
                    &direction_hint,
                    &font_ascent,
                    &font_descent,
                    &overall);

    charWidth = overall.width/strlen(test);
    charHeight = overall.ascent + overall.descent;
    charAscent = overall.ascent;
    lineHeight = charHeight + overall.ascent/4 + 2;

    LineSpacing[idx] = SPACING(font);
    BaseLine[idx] = BASELINE(font);
    StrikeLine[idx] = STRIKELINE(font);
}


#if 0
janet 21/07/95: commented, function not used at all

static int hex(char *s)
{
    int n, c;

    n = toupper((unsigned char *)*s++) - '0';
    n = toupper((int)*s++) - '0'; /* howcome changing cast 22/8/94  */

    if (n > 9)
	n += '0' - 'A' + 10;
    
    c = toupper((unsigned char *)*s) - '0';
    c = toupper((int)*s) - '0'; /* howcome changing cast 22/8/94 */

    if (c > 9)
	c += '0' - 'A' + 10;

    return c + (n << 4);
}
#endif


/* From Scott Nelson <snelson@canopus.llnl.gov> 24bit color  1/12/94  Dec 1 1994 */
/*
   For efficiency, these should be computed after the visual is
	changed and then stored and reused, or stored in each routine
	as a static variable, or as a global variable.
*/
/*
int RPixelShift(){
   int i = 0;
   long int maskval;
	
	maskval = (*visual).red_mask;
	
	while ((maskval & 1) == 0) { i++; maskval = maskval >> 1; }
	
	return i;
}
int GPixelShift(){
   int i = 0;
   long int maskval;
	
	maskval = (*visual).green_mask;
	
	while ((maskval & 1) == 0) { i++; maskval = maskval >> 1; }
	
	return i;
}
int BPixelShift(){
   int i = 0;
   long int maskval;
	
	maskval = (*visual).blue_mask;
	
	while ((maskval & 1) == 0) { i++; maskval = maskval >> 1; }
	
	return i;
}
*/

void CalcPixelShift(){
	/* this needs to be called after the visual has changed */
	
   int i = 0;
   long int maskval;
	
	maskval = (*visual).red_mask;
	
	while ((maskval & 1) == 0) { i++; maskval = maskval >> 1; }
	
	RPixelShift = i;

	
   i = 0;
	
	maskval = (*visual).green_mask;
	
	while ((maskval & 1) == 0) { i++; maskval = maskval >> 1; }
	
	GPixelShift = i;


	i = 0;
	
	maskval = (*visual).blue_mask;
	
	while ((maskval & 1) == 0) { i++; maskval = maskval >> 1; }
	
	BPixelShift = i;
}


int Pixel4Color(XColor *color, unsigned long *pixel)
{
    unsigned long r, g, b;

    r = gamma_table[color->red >> 8];
    g = gamma_table[color->green >> 8];
    b = gamma_table[color->blue >> 8];

/*    *pixel = (b | g << 8 | r << 16); */ /* From Scott Nelson <snelson@canopus.llnl.gov> 24bit color  1/12/94  Dec 1 1994 */
/*	   *pixel = (r | g << 8 | b << 16);*/  /* some systems use RGB masks, others use BGR masks */
    *pixel = (b << BPixelShift | g << GPixelShift | r << RPixelShift ); 
	 
    return 1;
}

int Pixel4PngColor(XColor *color, unsigned long *pixel)
{
    unsigned long r, g, b;

    r = color->red >> 8;
    g = color->green >> 8;
    b = color->blue >> 8;

/*    *pixel = (b | g << 8 | r << 16); */ /* From Scott Nelson <snelson@canopus.llnl.gov> 24bit color  1/12/94  Dec 1 1994 */
/*	   *pixel = (r | g << 8 | b << 16);*/  /* some systems use RGB masks, others use BGR masks */
    *pixel = (b << BPixelShift | g << GPixelShift | r << RPixelShift ); 
	 
    return 1;
}

int Pixel2Color(XColor *color, unsigned long *pixel)
{
    unsigned long r, g, b;

    r = (color->red * (long)(RPixelMask+1)) >> 16;
    g = (color->green *(long)(GPixelMask+1)) >> 16;
    b = (color->blue *(long)(BPixelMask+1)) >> 16;

    *pixel = (b << BPixelShift | g << GPixelShift | r << RPixelShift );
    return 1;
}

int Pixel2PngColor(XColor *color, unsigned long *pixel)
{
    unsigned long r, g, b;

    r = (color->red * (long)(RPixelMask+1)) >> 16;
    g = (color->green *(long)(GPixelMask+1)) >> 16;
    b = (color->blue *(long)(BPixelMask+1)) >> 16;

    *pixel = (b << BPixelShift | g << GPixelShift | r << RPixelShift );
    return 1;
}

int GetNamedColor(char *name, unsigned long *pix)
{
    XColor color;

    if (depth == 24)
    {
        if (XParseColor(display, colormap, name, &color) == 0)
        {
            if (X_TRACE)
                fprintf(stderr, "www: can't allocate named color `%s'\n", name);

            return 0;
        }

        return Pixel4Color(&color, pix);
    }
    if (depth == 16)
    {
        if (XParseColor(display, colormap, name, &color) == 0)
        {
            if (X_TRACE)
                fprintf(stderr, "www: can't allocate named color `%s'\n", name);

            return 0;
        }
        return Pixel2Color(&color, pix);
    }
    else if (XParseColor(display, colormap, name, &color) == 0 ||
                     XAllocColor(display, colormap, &color) == 0)
    {
        if (X_TRACE)
            fprintf(stderr, "www: can't allocate named color `%s'\n", name);

        return 0;
    }

    *pix = color.pixel;
    return 1;
}

int GetColor(int red, int green, int blue, unsigned long *pix)
{
    XColor color;
/*
    if (red == 255 && green == 255 && blue == 255)
    {
        *pix = WhitePixel(display, screen);
        return 1;
    }

    if (red == 0 && green == 0 && blue == 0)
    {
        *pix = BlackPixel(display, screen);
        return 1;
    }
*/
    color.red = red << 8;
    color.green = green << 8;
    color.blue = blue  << 8;

    if (depth == 24)
        return Pixel4Color(&color, pix);
    else if (depth == 16)
	return Pixel2Color(&color, pix);
    else if (XAllocColor(display, colormap, &color) == 0)
    {
/*        if (X_TRACE)*/
            fprintf(stderr, "www: can't allocate color %d:%d:%d\n", red, green, blue);
	*pix = BlackPixel(display, screen);
        return 0;
    }

    *pix = color.pixel;
    return 1;
}

int GetPngColor(int red, int green, int blue, unsigned long *pix)
{
    XColor color;
/*
    if (red == 255 && green == 255 && blue == 255)
    {
        *pix = WhitePixel(display, screen);
        return 1;
    }

    if (red == 0 && green == 0 && blue == 0)
    {
        *pix = BlackPixel(display, screen);
        return 1;
    }
*/
    color.red = red << 8;
    color.green = green << 8;
    color.blue = blue  << 8;

    if (depth == 24)
        return Pixel4PngColor(&color, pix);
    else if (depth == 16)
	return Pixel2PngColor(&color, pix);
    else if (XAllocColor(display, colormap, &color) == 0)
    {
/*        if (X_TRACE)*/
            fprintf(stderr, "www: can't allocate color %d:%d:%d\n", red, green, blue);
	*pix = BlackPixel(display, screen);
        return 0;
    }

    *pix = color.pixel;
    return 1;
}

char *DetermineValue(char *prog, char *param, char *def)
{
    char *value;

    if ((value = XGetDefault(display, prog, param)) == NULL)
        value = def;

    return value;
}

int DetermineColor(char *prog, char *param,
        int r, int g, int b,
        unsigned long *pix)
{
    char *colorname, *p1, *p2;

    colorname = XGetDefault(display, prog, param);

    if (colorname)  /* parse and GetColor() or GetNamedColor() */
    {
        /* "red:green:blue" e.g."205:184:157" or named color */

        p1 = strchr(colorname, ':');
        p2 = strrchr(colorname, ':');

        if (p1 && p2 && p1 != p2)
        {
            sscanf(colorname, "%d:", &r);
            sscanf(p1+1, "%d:", &g);
            sscanf(p2+1, "%d", &b);

            return GetColor(r, g, b, pix);
        }

        return GetNamedColor(colorname, pix);
    }

    return GetColor(r, g, b, pix);
}

void ButtonDown(unsigned int button, unsigned int state, int x, int y, int x_root, int y_root)
{
    if (X_TRACE)
        fprintf(stderr, "ButtonDown %d %d\n",x,y);


    if (y < WinTop){
        if (y < StatusTop)
            gadget = ToolBarButtonDown(x, y);
        else 
            gadget = StatusButtonDown(button, x, y);
    }
    else if (x < WinRight && y < WinBottom)
#ifdef POPUP
        if (button == Button3)
        {            
            Popup(x_root, y_root);   
        }
        else
#endif
	    if (button == Button2)   /* spif -> button2 shows URL */
	    {
		ShowLink(x, y);
	    }
	    else
		gadget = WindowButtonDown(x, y);
    else if ( (x >= WinRight && y < WinBottom) ||
              (x <= WinRight && y >= WinBottom) )
    {
        button_x = x;
        button_y = y;
        gadget = ScrollButtonDown(x, y);
    }
    else
        gadget = VOID;
}

void ButtonUp(unsigned int button, unsigned int state, int x, int y)
{
    int shifted;

    RepeatButtonDown = 0;
    shifted = ShiftMask & state;

    if (gadget == WINDOW)
        WindowButtonUp(shifted, x, y);
    else if (gadget == SCROLLBAR)
        ScrollButtonUp(x, y);
    else if (gadget == TOOLBAR)
        ToolBarButtonUp(x, y);
    else if (gadget == STATUS)
        StatusButtonUp(x, y);
}


int GuiEvents(int s, HTRequest * rq, int f)
{
    char keybuf[20];
    int keybufsize = 20;
    int count;
    KeySym key;
    XEvent event;
    XComposeStatus cs;
    int block = 0;
    int w, h;
    
    while (block || XEventsQueued(display, QueuedAfterReading) != 0)
    {
/*
	if (RepeatButtonDown) {
	    XButtonEvent e;

	    e.type = ButtonPress;
	    XSendEvent(event.xselectionrequest.display, event.xselectionrequest.requestor, False, 0, (XEvent *) &selEv);
*/

/*
        if (RepeatButtonDown && !ExposeCount &&
                XEventsQueued(display, QueuedAfterReading) == 0)
        {
            XFlush(display);
            ButtonDown(event.xbutton.button, event.xbutton.state, button_x, button_y);
            Pause(10);
            continue;
        }
*/

        XNextEvent(display, &event);

        switch(event.type)
        {
            case Expose:
                    /* the following commented by janet: redrawing won't work, you get a trail  */
                    /* get rid of all other Expose events on the queue */
                    /*                 while (XCheckTypedEvent(display, Expose, &event)); */

                if (event.xexpose.y < WinTop) /* janet: added check for whether to draw or not */
                {
                    DisplayToolBar();
                    DisplayStatusBar();
                    DisplayIcon();
                }
                
                w = event.xexpose.x + event.xexpose.width; /* the real coordinate */
                h = event.xexpose.y + event.xexpose.height; /* the real coordinate  */

/*                 DisplayIcon(); */
/*                 DisplayDoc(WinLeft, WinTop, WinWidth, WinHeight); */
                if ((h > WinTop && event.xexpose.x < WinWidth &&   /* checking whether to draw.. */
                     event.xexpose.y < (WinHeight+WinTop)))
                {
                    
                        /* only redraw pieces that were exposed */
                    DisplayDoc(event.xexpose.x, event.xexpose.y, 
                               event.xexpose.width, event.xexpose.height);
                }
                    /* we do the scrollbar last, to make sure DisplayDoc is not drawing on top of it */
                if ((w > WinWidth || h > WinHeight + WinTop))
                    DisplayScrollBar();
                
                break;

            case GraphicsExpose:
		/*	if (event.xexpose.y < WinTop)
		{*/
		    SetToolBarWin(win);
		    SetToolBarGC(gc_status);
		    DisplayToolBar();
		    
		    SetStatusWin(win);
		    SetStatusGC(gc_status);
		    DisplayStatusBar();
		    
		    SetIconWin(win);
		    SetIconGC(gc_status);
		    DisplayIcon();
		    /*	};*/
		
		SetScrollBarWin(win);
		SetScrollBarGC(gc_scrollbar);
		DisplayScrollBar();
		
		SetDisplayGC(gc_text);
		DisplayDoc(event.xgraphicsexpose.x,
			   event.xgraphicsexpose.y,
			   event.xgraphicsexpose.width,
			   event.xgraphicsexpose.height);
		
		ExposeCount = event.xgraphicsexpose.count;
		break;
		
            case NoExpose:
                ExposeCount = 0;
                break;

            case ConfigureNotify:
                if (win_width == event.xconfigure.width &&
                        win_height == event.xconfigure.height)
                    break;

                win_width = event.xconfigure.width;
                win_height = event.xconfigure.height;

                if ((win_width < size_hints.min_width) ||
                        (win_height < size_hints.min_height))
                    window_size = SMALL;
                else
                    window_size = OK;

                if (focus && focus->flags & CHECKED)
                {
                    focus->flags &= ~CHECKED;
                    focus = NULL;
                }

                DisplaySizeChanged(1);

#ifdef RESIZE_BUG
		/* 
		   howcome 26/7/94: need to expose after redraw 
		   this appears to be the case on suu4-sol2, and decstations??
		 */
                Redraw(0,0,WinWidth,WinHeight + WinTop); /* janet: changed height argument */
#endif
                break;

            case KeyPress:
                if (X_TRACE)
                    fprintf(stderr, "KeyPress\n");

                count = XLookupString((XKeyEvent*)&event, keybuf, keybufsize, &key, &cs);
                keybuf[count] = 0;

                if (key == XK_F8)
                {
                    XCloseDisplay(display);
                    Exit(1);
                }
                else if (key == XK_F2)
                    count = 1;
/*                else if (key == XK_F4)
                    ShowPaint(background.top-paint, 14); */
                else if (key == XK_F6)
                    ReportVisuals();
                else if (key == XK_F7)
                {
                    ReportStandardColorMaps(XA_RGB_DEFAULT_MAP);
                    ReportStandardColorMaps(XA_RGB_BEST_MAP);
                }
/*
		else if (TextFieldFocus && key !=XK_Begin && key !=XK_End && key != XK_Prior && key != XK_Next) {
		    if (IsEditChar(*keybuf))
			FormEditChar(*keybuf);
		    else
			FormMoveCursor(key);
		}
*/
		else if (key == XK_v && (event.xkey.state & ControlMask))
		    MoveDownPage();
		else if (key == XK_v && (event.xkey.state & Mod1Mask))
		    MoveUpPage();
/* 28-Nov-95 starting to play with the editor --Spif */
		else if ((CurrentDoc->field_editor) && (key == XK_Left))
		{
		    if(CurrentDoc->field_editor->pos)
		    {
			CurrentDoc->field_editor->pos--;
			PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field );
			PaintFieldCursorBuffer(disp_gc, strikeColor, CurrentDoc->field_editor, CurrentDoc->edited_field);
		    };
                      /* afficher le curseur */
		}
		else if ((CurrentDoc->field_editor) && (key == XK_Right))
		{
		    if(CurrentDoc->field_editor->pos < CurrentDoc->field_editor->size)
		    {
			CurrentDoc->field_editor->pos++;
			PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field );
			PaintFieldCursorBuffer(disp_gc, strikeColor, CurrentDoc->field_editor, CurrentDoc->edited_field);
		    };
                        /* afficher le curseur */
		}
		else if ((CurrentDoc->field_editor) && (key == XK_Delete || key == XK_DeleteChar))
		{
		    DeleteChar(CurrentDoc->field_editor, CurrentDoc->field_editor->pos);
		    Free(CurrentDoc->edited_field->value);
		    CurrentDoc->edited_field->value = Buffer2Str(CurrentDoc->field_editor);
		    CurrentDoc->edited_field->buflen = CurrentDoc->field_editor->size;
		    PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field );
		    PaintFieldCursorBuffer(disp_gc, strikeColor, CurrentDoc->field_editor, CurrentDoc->edited_field);
		    /* Afficher tout */
		}
		else if ((CurrentDoc->field_editor) && ((key < 255)||(key == XK_Return))) /* normal keypress... adding */
		{
		    /* printf("adding [%c] [%d]\n",key, key); */
		    if(key == XK_Return)
			InsertChar(CurrentDoc->field_editor, CurrentDoc->field_editor->pos , '\n');
		    else
			InsertChar(CurrentDoc->field_editor, CurrentDoc->field_editor->pos , key);
		    CurrentDoc->field_editor->pos++;
		    Free(CurrentDoc->edited_field->value);
		    CurrentDoc->edited_field->value = Buffer2Str(CurrentDoc->field_editor);
		    CurrentDoc->edited_field->buflen = CurrentDoc->field_editor->size;
		    PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field );
		    PaintFieldCursorBuffer(disp_gc, strikeColor, CurrentDoc->field_editor, CurrentDoc->edited_field);
                    /* Afficher tout */
		}
		else if ((CurrentDoc->field_editor) && (key == XK_Up))
		{
		    PrevLine(CurrentDoc->field_editor);
		    PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field );
		    PaintFieldCursorBuffer(disp_gc, strikeColor, CurrentDoc->field_editor, CurrentDoc->edited_field);
		}
		else if ((CurrentDoc->field_editor) && (key == XK_Down))
		{
		    NextLine(CurrentDoc->field_editor);
		    PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field );
		    PaintFieldCursorBuffer(disp_gc, strikeColor, CurrentDoc->field_editor, CurrentDoc->edited_field);
		}
                else if ((Authorize|OpenURL|IsIndex|SaveFile|FindStr) && IsEditChar(*keybuf))
                    EditChar(*keybuf);
                else if ((Authorize|OpenURL|IsIndex|SaveFile|FindStr) && (key == XK_Left || key == XK_Right))
                    MoveStatusCursor(key);
                else if ((Authorize|OpenURL|IsIndex|SaveFile|FindStr) && (key == XK_Delete || key == XK_DeleteChar))
                    EditChar(127);
#ifdef _HPUX_SOURCE
                else if ((Authorize|OpenURL|IsIndex|SaveFile|FindStr) && (key == XK_ClearLine || key == XK_DeleteLine))
                {
                    ClearStatus();
                    SetStatusString(0); /* force refresh */
                }
#endif
                else if (key == XK_Up && !ExposeCount)
                {
		    if (event.xkey.state & ControlMask)
			MoveUpPage();
		    else
			MoveUpLine();
                    XFlush(display);   /* to avoid scrollbar flashing */
                }
                else if (key == XK_Down && !ExposeCount)
                {
		    if (event.xkey.state & ControlMask)
			MoveDownPage();
		    else
			MoveDownLine();
                    XFlush(display);   /* to avoid scrollbar flashing */
                }
                else if (key == XK_Left && !ExposeCount)
                {
		    if (event.xkey.state & (Mod4Mask | ControlMask))
			BackDoc();
		    else
			MoveLeftLine();
                    XFlush(display);   /* to avoid scrollbar flashing */
                }
                else if (key == XK_Right && !ExposeCount)
                {
		    if (event.xkey.state & (Mod4Mask | ControlMask))
			ForwardDoc();
		    else
			MoveRightLine();
                    XFlush(display);   /* to avoid scrollbar flashing */
                }
                else if (key == XK_Prior && !ExposeCount)
                    MoveUpPage();
                else if ((key == XK_space || key == XK_Next) && !ExposeCount)
                    MoveDownPage();
                else if ((key == XK_Begin || key == XK_Home) && !ExposeCount)
                {
                    if (event.xkey.state & ShiftMask)
                        MoveToEnd();
                    else
                        MoveToStart();
                }
                else if (key == XK_End && !ExposeCount)
                    MoveToEnd();
                else if (key == XK_Escape)
                {
                    if (Authorize)
                        HideAuthorizeWidget();
                    else if (busy)
                        Beep();
                    else if (OpenURL)
                    {
                        OpenURL = 0;
                        SetStatusString("");
                    }
                    else if (SaveFile)
                    {
                        SaveFile = 0;
                        SetStatusString("");
                    }
                    else if (FindStr)
                    {
                        FindStr = 0;
                        SetStatusString("");
                    }
                }
                else if (busy)
                    Beep();
                else if (key == XK_F2)
                    BackDoc();
                else if (key == XK_F3 && FindStrVal)
                    FindString(FindStrVal, &FindNextStr);
                break;

            case KeyRelease:
                count = XLookupString((XKeyEvent*)&event, keybuf, keybufsize, &key, &cs);
                keybuf[count] = 0;
                break;

            case ButtonRelease:
                ButtonUp(event.xbutton.button, event.xbutton.state, event.xbutton.x, event.xbutton.y);
                break;

            case ButtonPress:
                if (X_TRACE)
                    fprintf(stderr, "ButtonPress %d %d\n",event.xbutton.x, event.xbutton.y);
		if(CurrentDoc->edited_field)
		{
		    Free(CurrentDoc->edited_field->value);
		    CurrentDoc->edited_field->value = Buffer2Str(CurrentDoc->field_editor);
		    CurrentDoc->edited_field->flags &= ~CHECKED;
		    TextFieldFocus = FALSE;
		    PaintField(disp_gc, -1, Abs2Win(CurrentDoc->edited_field->baseline),CurrentDoc->edited_field);
		    CurrentDoc->edited_field = NULL;
		    FreeBuffer(CurrentDoc->field_editor);
		    CurrentDoc->field_editor = NULL;
		};
                ButtonDown(event.xbutton.button, event.xbutton.state, event.xbutton.x, event.xbutton.y, event.xbutton.x_root, event.xbutton.y_root);
                break;

            case MotionNotify:  /* only sent when Button1 is down! */
              /* ignore event if still repairing holes from earlier copy operation*/
                if (ExposeCount)
                    break;

                /* get rid of all other MotionNotify events on the queue */
                while (XCheckTypedEvent(display, MotionNotify, &event));

#ifdef SELECTION
		SelectStatus(event.xbutton.x, event.xbutton.y);
#endif
		ScrollButtonDrag(event.xbutton.x, event.xbutton.y);
                break;

           /* the following is a failed attempt to exit tidily when the user
                forces an exit through the system menu - I need to do better ! */

            case DestroyNotify:
		if (event.xdestroywindow.window == win) {
		    if (X_TRACE)
                fprintf(stderr, "DestroyNotify event received\n");
		    /* XCloseDisplay(display); */
		    Exit(1);
		}

            case MapNotify:
                initialised = 1;   /* allow status display now its safe */
                break;

            case UnmapNotify:
                if (X_TRACE)	/* howcome 31/10/94 */
                    fprintf(stderr, "UnmapNotify\n");
                break;

            case ReparentNotify:
                break;

	
	    case PropertyNotify:
	    {

		/* thanks to Colas Nahaboo */

		Atom            actual_type;
		int             actual_format;
		unsigned long   nitems;
		unsigned long   bytes_after;
		unsigned char  *buffer = 0;
		
		if (Success == XGetWindowProperty(display, win, ARENA_COMMAND, 0, 8000, TRUE,
						  AnyPropertyType, &actual_type,
						  &actual_format, &nitems, &bytes_after,
						  &buffer))
		{
		    if(buffer)
		    {
			buffer[nitems] = '\0';
			if (!(actual_type == XA_STRING && ((signed long)nitems >= 0))) 
			{
			    XFree(buffer);
			    buffer = 0;
			} /* else ok */
		    } else {
			if (buffer) XFree(buffer);
			buffer = 0;
		    }
		}
		
		if(buffer)
		{
		    ParseCommandLine(buffer, nitems);
		}
	    }
	    break;

#ifdef SELECTION

		/* selection event handling added by howcome 2/8/94 */

	    case SelectionClear:
		ClearStatusSelection();
		DisplayStatusBar();
		break;
	    case SelectionNotify:
		fprintf(stderr,"notify\n");
		break;
	    case SelectionRequest:
		{
		  XSelectionEvent selEv; 

		  if (event.xselectionrequest.target != XA_STRING) {
		    selEv.property = None;
		  }
		  else {
		    selEv.property = event.xselectionrequest.property;
		    XChangeProperty(event.xselectionrequest.display,
				  event.xselectionrequest.requestor,
				  event.xselectionrequest.property,
				  event.xselectionrequest.target,
				  8, PropModeReplace, selectStr,
				  strlen(selectStr));
		  }

		  selEv.type = SelectionNotify;
		  selEv.display = event.xselectionrequest.display;
		  selEv.requestor = event.xselectionrequest.requestor;
		  selEv.selection = event.xselectionrequest.selection;
		  selEv.target = event.xselectionrequest.target;
		  selEv.time = event.xselectionrequest.time;


		  XSendEvent(event.xselectionrequest.display, event.xselectionrequest.requestor, False, 0, (XEvent *) &selEv);
		}
		break;

#endif /* SELECTION */


            default:  /* handle all other events */
                if (X_TRACE) 
		    fprintf(stderr, "Unexpected Event: %d\n", event.type);
                break;
        }
	XFlush(display);
    }

    busy = 0;
    return HT_WOULD_BLOCK; /* howcome 24/7/95: must do this to not insult library */
}




/* get font/colour resources */

void GetResources(void)
{
    if (fontsize == 0)
    {
        LoadFont(IDX_H1FONT, DetermineValue(prog, "h1font", H1FONT), "variable");
        LoadFont(IDX_H2FONT, DetermineValue(prog, "h2font", H2FONT), "variable");
        LoadFont(IDX_H3FONT, DetermineValue(prog, "h3font", H3FONT), "variable");
        LoadFont(IDX_H4FONT, DetermineValue(prog, "h4font", H4FONT), "variable");
        LoadFont(IDX_LABELFONT, DetermineValue(prog, "labelfont", LABELFONT), "fixed");
        LoadFont(IDX_NORMALFONT, DetermineValue(prog, "normalfont", NORMALFONT), "fixed");
        LoadFont(IDX_INORMALFONT, DetermineValue(prog, "italicfont", ITALICFONT), "fixed");
        LoadFont(IDX_BNORMALFONT, DetermineValue(prog, "boldfont", BOLDFONT), "fixed");
        LoadFont(IDX_BINORMALFONT, DetermineValue(prog, "bolditalicfont", BINORMFONT), "fixed");
        LoadFont(IDX_FIXEDFONT, DetermineValue(prog, "fixedfont", RFIXEDFONT), "fixed");
        LoadFont(IDX_IFIXEDFONT, DetermineValue(prog, "ifixedfont", IFIXEDFONT), "fixed");
        LoadFont(IDX_BFIXEDFONT, DetermineValue(prog, "bfixedfont", BFIXEDFONT), "fixed");
        LoadFont(IDX_BIFIXEDFONT, DetermineValue(prog, "bifixedfont", BIFIXEDFONT), "fixed");
        LoadFont(IDX_SYMBOLFONT, DetermineValue(prog, "symbolfont", SYMFONT), "grk-s30");
        LoadFont(IDX_SUBSYMFONT, DetermineValue(prog, "subsymbolfont", SUBSYMFONT), "grk-s25");
        LoadFont(IDX_SMALLFONT, DetermineValue(prog, "smallfont", SUBSCRFONT), "micro");
    }
    else if (fontsize == 1)
    {
        LoadFont(IDX_H1FONT, DetermineValue(prog, "h1font", H1FONTL), "vg-20");
        LoadFont(IDX_H2FONT, DetermineValue(prog, "h2font", H2FONTL), "fg-16");
        LoadFont(IDX_H3FONT, DetermineValue(prog, "h3font", H3FONTL), "variable");
        LoadFont(IDX_H4FONT, DetermineValue(prog, "h4font", H4FONTL), "variable");
        LoadFont(IDX_LABELFONT, DetermineValue(prog, "labelfont", LABELFONT), "fixed");
        LoadFont(IDX_NORMALFONT, DetermineValue(prog, "normalfont", NORMALFONTL), "fixed");
        LoadFont(IDX_INORMALFONT, DetermineValue(prog, "italicfont", ITALICFONTL), "fixed");
        LoadFont(IDX_BNORMALFONT, DetermineValue(prog, "boldfont", BOLDFONTL), "fixed");
        LoadFont(IDX_BINORMALFONT, DetermineValue(prog, "bolditalicfont", BINORMFONTL), "fixed");
        LoadFont(IDX_FIXEDFONT, DetermineValue(prog, "fixedfont", RFIXEDFONTL), "fixed");
        LoadFont(IDX_IFIXEDFONT, DetermineValue(prog, "ifixedfont", IFIXEDFONTL), "fixed");
        LoadFont(IDX_BFIXEDFONT, DetermineValue(prog, "bfixedfont", BFIXEDFONTL), "fixed");
        LoadFont(IDX_BIFIXEDFONT, DetermineValue(prog, "bifixedfont", BIFIXEDFONTL), "fixed");
        LoadFont(IDX_SYMBOLFONT, DetermineValue(prog, "symbolfont", SYMFONTL), "grk-s30");
        LoadFont(IDX_SUBSYMFONT, DetermineValue(prog, "subsymbolfont", SUBSYMFONTL), "grk-s25");
        LoadFont(IDX_SMALLFONT, DetermineValue(prog, "smallfont", SUBSCRFONTL), "micro");
    }
    else if (fontsize >= 2)
    {
        LoadFont(IDX_H1FONT, DetermineValue(prog, "h1font", H1FONTG), "vg-20");
        LoadFont(IDX_H2FONT, DetermineValue(prog, "h2font", H2FONTG), "fg-16");
        LoadFont(IDX_H3FONT, DetermineValue(prog, "h3font", H3FONTG), "variable");
        LoadFont(IDX_H4FONT, DetermineValue(prog, "h4font", H4FONTG), "variable");
        LoadFont(IDX_LABELFONT, DetermineValue(prog, "labelfont", LABELFONT), "fixed");
        LoadFont(IDX_NORMALFONT, DetermineValue(prog, "normalfont", NORMALFONTG), "fixed");
        LoadFont(IDX_INORMALFONT, DetermineValue(prog, "italicfont", ITALICFONTG), "fixed");
        LoadFont(IDX_BNORMALFONT, DetermineValue(prog, "boldfont", BOLDFONTG), "fixed");
        LoadFont(IDX_BINORMALFONT, DetermineValue(prog, "bolditalicfont", BINORMFONTG), "fixed");
        LoadFont(IDX_FIXEDFONT, DetermineValue(prog, "fixedfont", RFIXEDFONTG), "fixed");
        LoadFont(IDX_IFIXEDFONT, DetermineValue(prog, "ifixedfont", IFIXEDFONTG), "fixed");
        LoadFont(IDX_BFIXEDFONT, DetermineValue(prog, "bfixedfont", BFIXEDFONTG), "fixed");
        LoadFont(IDX_BIFIXEDFONT, DetermineValue(prog, "bifixedfont", BIFIXEDFONTG), "fixed");
        LoadFont(IDX_SYMBOLFONT, DetermineValue(prog, "symbolfont", SYMFONTG), "grk30s25");
        LoadFont(IDX_SUBSYMFONT, DetermineValue(prog, "subsymbolfont", SUBSYMFONTG), "grk-s25");
        LoadFont(IDX_SMALLFONT, DetermineValue(prog, "smallfont", SUBSCRFONTG), "micro");
    }

    fixed_font = Fonts[IDX_FIXEDFONT];

    /* list indents for ordered/unordered lists */

    ListIndent1 = XTextWidth(Fonts[IDX_NORMALFONT], "ABCabc", 6)/6;
    ListIndent2 = ListIndent1;
    ListIndent1 = 2 * ListIndent1;

    /* check for monchrome displays */

    if (DefaultDepth(display, screen) == 1)
    {
        strikeColor = labelColor = textColor = BlackPixel(display, screen);
        transparent = statusColor = windowColor = WhitePixel(display, screen);
        windowShadow = windowTopShadow = windowBottomShadow = textColor;
    }
    else   /* try for color but degrade as sensibly as possible */
    {
        if (!DetermineColor(prog, "windowColor", 220, 209, 186, &windowColor))
        {
            if (GetNamedColor("gray", &windowColor))
            {
                windowBottomShadow = BlackPixel(display, screen);

                if (!GetNamedColor("dim gray", &windowShadow))
                    windowShadow = windowColor;

                if (!GetNamedColor("light gray", &windowTopShadow))
                    windowTopShadow = WhitePixel(display, screen);

                strikeColor = textColor = labelColor = WhitePixel(display, screen);
            }
            else
            {
                labelColor = strikeColor = textColor = BlackPixel(display, screen);
                windowShadow = windowColor = WhitePixel(display, screen);
                windowShadow = windowTopShadow = windowBottomShadow = textColor;
            }
        }
        else
        {
            DetermineColor(prog, "textColor", 0, 0, 100, &textColor);
            DetermineColor(prog, "labelColor", 0, 100, 100, &labelColor);
            DetermineColor(prog, "strikeColor", 170, 0, 0, &strikeColor);

            DetermineColor(prog, "windowShadow", 200, 188, 169, &windowShadow);
            DetermineColor(prog, "windowTopShadow", 255, 242, 216, &windowTopShadow);
            DetermineColor(prog, "windowBottomShadow", 180, 170, 152, &windowBottomShadow);

            if (windowTopShadow == windowColor)
                windowTopShadow = WhitePixel(display, screen);

            if (windowBottomShadow == windowColor)
                windowBottomShadow = BlackPixel(display, screen);

            if (windowColor == textColor)
            {
                windowShadow = windowColor = BlackPixel(display, screen);
                strikeColor = windowTopShadow = windowBottomShadow = textColor;
            }
        }

        transparent = windowColor;
        statusColor = windowShadow;
    }
}

void MakePaper()
{
    XGCValues values;
    unsigned int valuemask;
    Pixmap pixmap;
    XImage *image;
    GC drawGC;

    gc_fill = XCreateGC(display, win, 0, 0);
    XSetFunction(display, gc_fill, GXcopy);

    InitPaperRGB ();

    if (! (UsePaper && (depth == 8 || depth == 16 || depth == 24)))
    {
        UsePaper = 0;
        XSetForeground(display, gc_fill, windowColor);
    }
    else
    {
        tileWidth = tileHeight = 64;
	
	/* howcome 15/3/95: commented out XQueryTile since it on some
           platforms set tile* to 0. On other servers it returns a low
           number which dosn't look good */

/*        XQueryBestTile(display, win, tileWidth, tileHeight, &tileWidth, &tileHeight); */

        tileData = CreateBackground(tileWidth, tileHeight, depth);

        if (!tileData)
        {
            UsePaper = 0;
            XSetForeground(display, gc_fill, windowColor);
            return;
        }

        if ((pixmap = XCreatePixmap(display, win, tileWidth, tileHeight, depth)) == 0)
        {
            fprintf(stderr, "Failed to create Pixmap for background!\n");
            Exit(1);
        }

        if ((image = XCreateImage(display, DefaultVisual(display, screen),
				  depth, ZPixmap, 0, (char *)tileData,
				  tileWidth, tileHeight, (depth == 24 ? 32 : (depth == 16 ? 16 : 8)), 0)) == 0)
        {
            /* Free(tileData); keep for transparent backgrounds */
            XFreePixmap(display, pixmap);
            fprintf(stderr, "Failed to create X Image for background!\n");
            Exit(1);
        }

	/* howcome 22/2/95: do we need to set these?? */

	image->byte_order = MSBFirst;
	image->bitmap_bit_order = BitmapBitOrder(display); /* was MSBFirst*/ ;

        drawGC = XCreateGC(display, pixmap, 0, 0);
        XSetFunction(display, drawGC, GXcopy);
        XPutImage(display, pixmap, drawGC, image, 0, 0, 0, 0, tileWidth, tileHeight);
        XFreeGC(display, drawGC);
        image->data = NULL;   /* we don't want to free image data */
        XDestroyImage(image); /* normally frees image data */

        valuemask = GCTile|GCFillStyle;
        values.tile = pixmap;
        values.fill_style = FillTiled;
        XChangeGC(display, gc_fill, valuemask, &values);
    }
}

/* create clone of self by forking and duplicating resources

    creates duplicate of history file and closes
    all open files, then duplicates stdio ...

    returns 0 for parent, and 1 for clone
*/

int CloneSelf(void)
{

#if 0

    int childpid, tty;
    int x = 0, y = 0;               /* window position */
    unsigned int border_width = 4;  /* border four pixels wide */
/*    unsigned int display_width, display_height;*/
    char *window_name = BANNER;
    char *icon_name = BANNER;
    int i, fh, depth;
    unsigned int class;
    Visual *visual;
    unsigned long valuemask, where;
    XSetWindowAttributes attributes;
    Pixmap icon_pixmap;

    if ((tty = open("/dev/tty", 2)) == -1 && (tty = open("/dev/null", 2)) == -1)
    {
        Warn("Can't open /dev/tty");
        return 0;
    }

    /* ensure that children won't become zombies */

#ifdef SIGCHLD
    signal(SIGCHLD, SIG_IGN);
#else
    signal(SIGCLD, SIG_IGN);
#endif

    if ((childpid = fork()) < 0)
    {
        close(tty);
        Warn("Can't fork new process");
        return 0;
    }

    if (childpid != 0)
    {
        close(tty);
        return 0;
    }

    /* ok child process - so dup stdio  - this leaves other files open! */

    fprintf(stderr,"CloneSelf: child\n");

    close(0); dup(tty);
    close(1); dup(tty);
    close(2); dup(tty);
    close(tty);

    close(ConnectionNumber(display));  /* close TCP connection to X server */

    /* now clone history file and close original */

    initialised = 0;  /* avoid X output until this is true! */

/*    XCloseDisplay(display); */ /* close connection for parent display */

    /* connect to X server */

    if ( (display = XOpenDisplay(display_name)) == NULL )
    {
        (void) fprintf(stderr, "www: cannot connect to X server %s\n",
                        XDisplayName(display_name));
        exit(-1);
    }

    /* free memory but not pixmaps which we no longer own! */
    FreeImages(1);
    FreeForms();   /* is this right? */
                     
  /* try to allocate 128 fixed colors + 16 grey scales */
    if (InitImaging(ColorStyle) == MONO)
        UsePaper = 0;

/*    GetResources();  *//* load font and colour resources */

    if (ColorStyle == MONO)
        statusColor = windowColor;

    /* get screen size from display structure macro */

    screen = DefaultScreen(display);

    depth = DisplayPlanes(display, screen);
    class = InputOutput;    /* window class*/
    visual = DefaultVisual(display, screen);
    valuemask = CWColormap | CWBorderPixel | CWBitGravity |
             CWBackingStore | CWBackingPlanes;
    attributes.colormap = colormap;
    attributes.bit_gravity = ForgetGravity;
    attributes.backing_planes = 0;
    attributes.backing_store = NotUseful;

    win = XCreateWindow(display, RootWindow(display, screen),
            x,y, win_width, win_height, border_width,
            depth, class, visual, valuemask, &attributes);

    XSetWindowBackground(display, win, windowColor);

    /* Create pixmap of depth 1 (bitmap) for icon */

    icon_pixmap = XCreateBitmapFromData(display, win, www_bits,
                                    www_width, www_height);

    /* Create default pixmap for use when we can't load images */

    default_pixmap = XCreatePixmapFromBitmapData(display, win, www_bits,
                      www_width, www_height, textColor, transparent, depth);

    default_pixmap_width = www_width;
    default_pixmap_height = www_height;

    /* initialize size hint property for window manager */

    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = win_width;
    size_hints.height = win_height;
    size_hints.min_width = 440;
    size_hints.min_height = 250;

    /* set properties for window manager (always before mapping) */

    XSetStandardProperties(display, win, window_name, icon_name,
        icon_pixmap, (char **)0, 0, &size_hints);

    /* select events wanted */

    XSelectInput(display, win, ExposureMask | KeyPressMask | KeyReleaseMask |
       Button1MotionMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask |PropertyChangeMask);

    /* create hourglass cursor */

    hourglass = XCreateFontCursor(display, XC_watch);

    /* create GCs for text and drawing: gc_scrollbar, gc_status, gc_text */

    gc_scrollbar = XCreateGC(display, win, 0, 0);
    gc_status = XCreateGC(display, win, 0, 0);
    gc_text = XCreateGC(display, win, 0, 0);
    MakePaper();
    font = -1;

    if (DocHTML(CurrentDoc))
        SetBanner(CurrentDoc->url);

    /* refresh paint buffer to ensure that images resources are created */

    where = PixelOffset;
    PixelOffset = 0;
    ParseHTML(&i);

    if (where > 0)
        DeltaHTMLPosition(where);

    /* Map Display Window */

    XMapWindow(display, win);

    return 1;

#endif
    return 1;

}

Context *NewContext()
{
    return ((Context *) calloc (sizeof(Context), 1));
}


void myAlarm()
{
    fprintf(stderr,"myAlarm\n");

}

void Push(void *item)
{
    *stack = (long)item;
    stack += sizeof(long);
}

void *Pop()
{
    if(stack == main_stack)
	return NULL;
    
    stack -= sizeof(long);
    return((void *)*stack);
}

void main(int argc, char **argv)
{
    int x = 0, y = 0;               /* window position */
    unsigned int border_width = 2;  /* border four pixels wide */
    char *window_name = BANNER;
    char *icon_name = BANNER;
    int best_depth;		   
    /*    unsigned long *pixels;  janet 21/07/95:  not used */
    unsigned int class;
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    XClassHint class_hints;
    mode_t cmask;
	 

#ifdef SIGWAITING_IGN
    signal(SIGWAITING, SIG_IGN);           /* howcome: solaris binaries receive this signal, but why? */
#endif
    signal(SIGABRT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
#if !defined __QNX__ 
    signal(SIGINT, Exit);   /* howcome 4/10/94: trap C-C so that we can delete cache */
#endif
    signal(SIGIOT, CatchCore);
/*    signal(SIGSEGV, CatchCore); */
    cmask = umask(0000);

    stack = main_stack;

    UseHTTP2 = 1;
    UsePaper = 1;

    ColorStyle = COLOR888;

    Editor = getenv("EDITOR"); /* howcome 16/10/94: has to do it before command line arg processing */

    prog = argv[0];

    while (argc > 1)
    {
	if (!strcasecmp(argv[1], "-help")||!strcasecmp(argv[1], "-h"))
	{
	    printf("Usage: arena [-large][-giant][-mono][-grey][-color][-color8]\n");
	    printf("             [-plain][-nocache][-noterminal][-nomailcap][-nostyle]\n");
	    printf("             [-cm][-quiet][-display display][-editor editor]\n");
	    printf("             [-cache dir][-geometry geom] [URL]\n");
	    Exit(1);
	}

	if (strcmp(argv[1], "-large") == 0)
        {
            fontsize = 1;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-giant") == 0)
        {
            fontsize = 2;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-mono") == 0)
        {
            ColorStyle = MONO;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-grey") == 0 ||
                     strcmp(argv[1], "-gray") == 0)
        {
            ColorStyle = GREY4;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-color") == 0)
        {
            ColorStyle = COLOR888;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-color8") == 0)
        {
            ColorStyle = COLOR232;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-plain") == 0)
        {
            UsePaper = 0;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-nocache") == 0)
        {
            CacheRoot = NULL;
            --argc;
            ++argv;
        }
        else if (strcmp(argv[1], "-noterminal") == 0)
        {
            NoTerminal = 1;
            --argc;
            ++argv;
        }

        else if (strcmp(argv[1], "-nomailcap") == 0)
        {
            NoMailcap = 1;
            --argc;
            ++argv;
        }

        else if (strcmp(argv[1], "-nostyle") == 0)
        {
            NoStyle = 1;
            --argc;
            ++argv;
        }

        else if (strcmp(argv[1], "-cm") == 0)
        {
            OwnColorMap = 1;
            --argc;
            ++argv;
        }

        else if (strcmp(argv[1], "-quiet") == 0)
        {
            Quiet = 1;
            --argc;
            ++argv;
        }

        else if (strcmp(argv[1], "-display") == 0)
        {
            --argc;
            ++argv;
	    if (argc > 1) {
                --argc;
                ++argv;
                display_name = *argv;
                if (OPTION_TRACE)
		    printf("cache dir = %s\n", CacheRoot);
            }
        }

        /* howcome 5/10/94: added cache command line option */
        else if (strcmp(argv[1], "-cache") == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                CacheRoot = *argv;
                if (OPTION_TRACE)
		    printf("cache dir = %s\n", CacheRoot);
            }
        }

        /* howcome 10/10/94: added editor command line option */
        else if (strncmp(argv[1], "-editor",5) == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                Editor = *argv;
                if (OPTION_TRACE) 
		    printf("Editor = %s\n", Editor);
            }
        }

        /* howcome 10/10/94: added badflags command line option */
        else if (strcmp(argv[1], "-badflags") == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                Badflags = atoi(*argv);
                if (OPTION_TRACE) 
		    printf("Editor = %s\n", Editor);
            }
        }

        /* howcome 13/10/94: added icon command line option */
        else if (strcmp(argv[1], "-icon") == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                Icon = *argv;
                if (OPTION_TRACE) 
		    printf("Icon = %s\n", Icon);
            }
        }

        /* howcome 10/10/94: added command line option */
        else if (strncmp(argv[1], "-printer",6) == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                Printer = *argv;
                if (OPTION_TRACE) 
		    printf("Printer = %s\n", Printer);
            }
        }

        /* howcome 7/11/94: added command line option */
        else if (strncmp(argv[1], "-gamma",6) == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                Gamma = (double) atof (*argv);
                if (OPTION_TRACE) 
		    printf("Gamma = %f\n", Gamma);
            }
        }

        else if (strcmp(argv[1], "-debug") == 0)
        {
            debug = 255;
	    library_trace = 255;
            --argc;
            ++argv;
        }

        /* howcome 7/11/94: added command line option */
        else if (strncmp(argv[1], "-ct",3) == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                debug = (int) atoi(*argv);
/*                if (debug) */
		    printf("Client trace = %d\n", debug);
            }
        }

        /* howcome 9/5/95: added command line option */
        else if (strncmp(argv[1], "-lens",5) == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;
                lens_factor = (int) atof(*argv);
		fontsize = (int) (lens_factor + 0.5);
/*                if (debug) */
		    printf("Client trace = %d\n", debug);
            }
        }


        /* howcome 7/11/94: added command line option */
        else if (strncmp(argv[1], "-lt",3) == 0)
        {
            --argc;
            ++argv;
            if (argc > 1) {
                --argc;
                ++argv;

		{
		    char *p = *argv;
		    library_trace = 0;
		    for(; *p; p++) {
			switch (*p) {
 			  case 'a': library_trace |= SHOW_ANCHOR_TRACE; break;
 			  case 'b': library_trace |= SHOW_BIND_TRACE; break;
 			  case 'c': library_trace |= SHOW_CACHE_TRACE; break;
 			  case 'g':	library_trace |= SHOW_SGML_TRACE; break;
 			  case 'p':	library_trace |= SHOW_PROTOCOL_TRACE; break;
 			  case 's':	library_trace |= SHOW_STREAM_TRACE; break;
 			  case 't':	library_trace |= SHOW_THREAD_TRACE; break;
 			  case 'u': library_trace |= SHOW_URI_TRACE; break;
  			  case 'v': library_trace = SHOW_ALL_TRACE; break;
			}
		    }
		}

/*                library_trace = (int) atoi(*argv);*/
/*                if (debug) */
		    printf("Library trace = %d\n", library_trace);
            }
        }
       /* pdd 31/10/95: added crippled window position option */
        else if (strcmp(argv[1], "-geometry") == 0)
        {
           --argc;
           ++argv;
           if (argc > 1) {
              --argc;
              ++argv;
	      sscanf(*argv,"+%d+%d", &x, &y);
              /* this part may not be necessary... but can't hurt */
	      if (x < 0 || x >= 2048 || y < 0 || y >= 1536) {
		fprintf(stderr,"-geometry: only supports +X+Y\n");
		x = 0;
		y = 0;
	      }
            }
         }

        /* pdd 31/10/95: added option to force end of option parsing */
        else if (strcmp(argv[1],"--") == 0)
        {
          --argc;
          ++argv;
          break;
        }

        /* pdd 31/10/95: silently bypass invalid arguments */
        else if (strncmp(argv[1],"-",1) == 0)
        {
          --argc;
          ++argv;
        }


        /* pdd 31/10/95: treat first argument not starting with a "-"
           as a new starting URL */
        else if (strncmp(argv[1],"-",1) != 0)
        {
          break;
        }
      } /* end of command line option parsing */


    /* initialise ISO character entity definitions */

    InitEntities();

    /* connect to X server */

    if ( (display=XOpenDisplay(display_name)) == NULL )
    {
        (void) fprintf(stderr, "www: cannot connect to X server %s\n",
                        XDisplayName(display_name));
        Exit(-1);
    }

    {
	char *dpy_name, *host_name, *tmp1, *tmp2;
	int ok = 1;
	struct utsname os_info;

	dpy_name = DisplayString(display);
	uname(&os_info);
	
	tmp1 = (char *)malloc(strlen(dpy_name)+1);
	strcpy(tmp1, dpy_name);
	for(tmp2 = tmp1; *tmp2; tmp2++)
	    if(*tmp2 == '.') *tmp2 = 0;

	if(!strcasecmp(tmp1, os_info.nodename) || (*tmp1 == ':')) /* it is local */
	{
	    if(!strcasecmp(os_info.sysname, "IRIX"))
	    {
		FILE *gammafile;

		gammafile = fopen("/etc/config/system.glGammaVal","r");

		if(gammafile)
		{
		    float local_gamma;
		    
		    fscanf(gammafile, "%f", &local_gamma);
		    
		    Gamma = Gamma / local_gamma;
		    if(X_TRACE)
			printf("DEBUG: GAMMA = %f (local was %f)\n", Gamma, local_gamma);
		}
	    }
	}
	free(tmp1);
    }


/*    XSetErrorHandler(CatchX);*/


/* for debuging  - disable buffering of output queue */

/*    XSynchronize(display, 0); */ /* 0 enable, 1 disable */

/*    gateway = DetermineValue(prog, "gateway", GATEWAY_HOST); */
    help = DetermineValue(prog, "help", HELP_URL);

/*    printer = DetermineValue(prog, "printer", PRINTER);*/ /* howcome 16/10/94: best not to set this */

    startwith = getenv("WWW_HOME");
    if (!startwith)
	startwith = DetermineValue(prog, "startwith", DEFAULT_URL);

    /* get screen size from display structure macro */

    screen = DefaultScreen(display);
    depth = DisplayPlanes(display, screen);

    if (depth == 1)
      ColorStyle = MONO;
    
    visual = BestVisual(TrueColor, &best_depth);
    if (best_depth == 24 || best_depth == 16){

	/* From Scott Nelson <snelson@canopus.llnl.gov> 24bit color: 12/5/94 */
	/* visual = BestVisual(DirectColor, &best_depth); */ 
	visual=BestVisual(TrueColor, &best_depth); 
	CalcPixelShift();
    } else {
	visual = BestVisual(DirectColor, &best_depth);
    }

    if (ColorStyle != COLOR888 || best_depth <= depth)
    {
        colormap = DefaultColormap(display, screen);
        visual = DefaultVisual(display, screen);

        if ((ColorStyle = InitImaging(ColorStyle)) == MONO)
            UsePaper = 0;
    }
    else
    {
        InitImaging(ColorStyle);
        depth = best_depth;
        colormap = XCreateColormap(display, RootWindow(display, screen),
                                                    visual, AllocNone);
    }

    display_width = DisplayWidth(display, screen);
    display_height = DisplayHeight(display, screen);
    display_pt2px =  (25.4 / 72.0) * 
	(((double) DisplayWidth(display,screen)) / ((double) DisplayWidthMM(display, screen)));

    GetResources();  /* load font and colour resources */

    if (ColorStyle == MONO)
        statusColor = windowColor;

    /* size widow with enough room for text */

    charWidth = XTextWidth(Fonts[IDX_NORMALFONT], "ABCabc", 6)/6; 
    charHeight = SPACING(Fonts[IDX_NORMALFONT]);

    win_width = min ( (92 * charWidth), (display_width * 0.9) );
    win_height = min ( (35 * charHeight + 7), (display_height) );

    class = InputOutput;    /* window class*/
    valuemask = CWColormap | CWBorderPixel | CWBitGravity |
             CWBackingStore | CWBackingPlanes;
    attributes.colormap = colormap;
    attributes.bit_gravity = ForgetGravity;
    attributes.backing_planes = 0;
    attributes.backing_store = NotUseful;

    /* create opaque window */

    win = XCreateWindow(display, RootWindow(display, screen),
            x,y, win_width, win_height, border_width,
            depth, class, visual, valuemask, &attributes);

    class_hints.res_name = (char *)calloc(strlen("Browser")+1, sizeof(char));
    strcpy(class_hints.res_name, "Browser");
    class_hints.res_class = (char *)calloc(strlen(BANNER)+1, sizeof(char));
    strcpy(class_hints.res_class, BANNER);
    XSetClassHint(display, win, &class_hints);

    ARENA_LOCATION = XInternAtom(display, "ARENA_LOCATION", False); /* inter client comm */
    ARENA_COMMAND  = XInternAtom(display, "ARENA_COMMAND" , False);

    /* Create pixmap of depth 1 (bitmap) for icon */
/*
    icon_pixmap = XCreateBitmapFromData(display, win, www_bits,
                                    www_width, www_height);
*/

#if 0
    /* Create default pixmap for use when we can't load images */

    default_pixmap = XCreatePixmapFromBitmapData(display, win, www_bits,
                      www_width, www_height, textColor, transparent, depth);

    default_pixmap_width = www_width;
    default_pixmap_height = www_height;

    default_image->pixmap = default_pixmap;
    default_image->width = default_pixmap_width;
    default_image->height = default_pixmap_height;
#endif

    /* initialize size hint property for window manager */

    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = win_width;
    size_hints.height = win_height;

    size_hints.min_width = 100;
    size_hints.min_height = 150;
/*
    size_hints.min_width = 440;
    size_hints.min_height = 350;
*/
    /* before we can call XSetStandardProperties we need the
       icon. Befor we can get the icon, we need to initialize the
       library */

    context = NewContext();
    context->registered_anchors = HTList_new();
/*    context->pending_docs = HTList_new();
    context->pending_anchors = HTList_new(); */
    context->style = StyleGetInit();
    context->history = HTList_new();
    context->conversions = HTList_new();
    context->memory_leaks = HTList_new();


    libEntry(); /* howcome: library init */
    LoadIcon(); /* howcome 14/10/94 */

    /* set properties for window manager (always before mapping) */

    XSetStandardProperties(display, win, window_name, icon_name,
        icon_pixmap, argv, argc, &size_hints);

    /* select events wanted */

    XSelectInput(display, win, ExposureMask | KeyPressMask | KeyReleaseMask |
       Button1MotionMask | ButtonPressMask | ButtonReleaseMask |
       StructureNotifyMask | SubstructureNotifyMask| PropertyChangeMask);

    /* create hourglass cursor */

    hourglass = XCreateFontCursor(display, XC_watch);

    /* create GCs for text and drawing: gc_scrollbar, gc_status, gc_text */

    gc_scrollbar = XCreateGC(display, win, 0, 0);
    gc_status = XCreateGC(display, win, 0, 0);
    gc_text = XCreateGC(display, win, 0, 0);

    /* find user's name */

     user = getenv("USER");     /* shell variable for user name */


/*    InitCurrent(CurrentDirectory()); */  /* set current host to self etc. */
    SetToolBarFont(Fonts[IDX_LABELFONT]);
    SetStatusFont(Fonts[IDX_LABELFONT]);

    SetToolBarWin(win);
    SetToolBarGC(gc_status);
    SetStatusWin(win);
    SetStatusGC(gc_status);
    SetScrollBarWin(win);
    SetScrollBarGC(gc_scrollbar);
    SetIconWin(win);
    SetIconGC(gc_status);
    SetToolBarGC(gc_status);
    SetDisplayGC(gc_text);
#ifdef POPUP
    initPopup(Fonts[IDX_LABELFONT]);
#endif    
    MakePaper();  /* create gc_fill for textured paper */
    MakeIcons(depth);     /* create images for icons */

    /* create pixmap for testing initial <img> implementation */

    /* Map Display Window */

    XMapWindow(display, win);

    /* get buffer with named file in it */

    hdrlen = 0;
    buffer = NULL;


    if (argc != 1)
        startwith = argv[1];

#if 0
    signal(SIGALRM, myAlarm);

    itimer.it_value.tv_sec     = 10;
    itimer.it_value.tv_usec    = 0;
    itimer.it_interval.tv_sec  = 0;
    itimer.it_interval.tv_usec = 500000;

    setitimer (ITIMER_REAL, &itimer, &old_itimer);
#endif

    libEventLoop(ConnectionNumber(display), startwith);
}
