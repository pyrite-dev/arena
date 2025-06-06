#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include "www.h"

extern Display *display;
extern Colormap colormap;
extern double lens_factor;
extern int screen;

extern XFontStruct *Fonts[FONTS];  /* array of fonts */
extern char *FontsName[FONTS];
extern int LineSpacing[FONTS];
extern int BaseLine[FONTS];
extern int StrikeLine[FONTS];
extern int LineThickness[FONTS];
extern int debug;

long ParseColor(const char *s)
{
    XColor xc;

    if (XParseColor(display, colormap, s, &xc))
	return (((xc.red >> 8) << 16) | ((xc.green >> 8) << 8) | (xc.blue >> 8));
    return 0; /* i.e. black */
}


int myLoadFont(char *name)
{
    int direction_hint, font_ascent, font_descent;
    XCharStruct overall;
    XFontStruct *font = NULL;
    static int fix = 0;
    /* janet 24/07/95: not used:    int use_fix = -1; */
    int i;
    char *test = "Testing";
    
    for(i=0; i < fix; i++) {
	if (strcmp(name, FontsName[i]) == 0) {
	    if (FONT_TRACE)
		fprintf(stderr,"font: %s found in cache, fix = %d\n",name, i);
    	    return i;
	} 
    }

    /* a font with the same name has not been loaded, so try it.. */

    font = XLoadQueryFont(display, name);
    
    if (font) {
	FontsName[fix] = strdup(name);
	Fonts[fix] = font;

	XTextExtents(font, test, strlen(test),
		     &direction_hint,
		     &font_ascent,
		     &font_descent,
		     &overall);

	LineSpacing[fix] = SPACING(font);
	BaseLine[fix] = BASELINE(font);
	StrikeLine[fix] = STRIKELINE(font);
	LineThickness[fix] = LINETHICKNESS(font);
	fix++;
/*	fprintf(stderr,"FIX %d\n",fix);*/
	if (FONT_TRACE)
	    fprintf(stderr,"font %s: xloadqueryfonts successful\n",name);
	return (fix - 1);
    }
    
    if (FONT_TRACE)
	fprintf(stderr,"font: mloadFont failed %s\n",name);
    return (0); /* default font returned on error */
}


int GetFont(char *family_l, long px, long weight, long style, BOOL small_caps)
{	
    int fix = -1;
    char name[200];
    char *weight_str, *slant_str, *condensed_str, *serif_str, *rgstry;
    char *family;
    /*    int i; */
    HTList *l;

    px = (int)((double) px * lens_factor);
	
    /* if a point size is requested, let's compute the pixel size to
       avoid using the dpi in the request */

    switch (weight) {
      case SV_FONT_WEIGHT_LIGHT:
	weight_str = "light";
	break;
      case SV_FONT_WEIGHT_MEDIUM:
	weight_str = "medium";
	break;
      case SV_FONT_WEIGHT_DEMI_BOLD:
	weight_str = "demibold";
	break;
      case SV_FONT_WEIGHT_BOLD:
	weight_str = "bold";
	break;
      case SV_FONT_WEIGHT_EXTRA_BOLD:
	weight_str = "black";
	break;
      default:
	weight_str = "medium";
	break;	
    }

    switch (style) {
      case SV_FONT_STYLE_ROMAN:
	slant_str = "r";
	break;
      case SV_FONT_STYLE_ITALIC:
      case SV_FONT_STYLE_OBLIQUE:
	slant_str = "i";
	break;
      default:
	slant_str = "r";
	break;	
    }

    serif_str = "*";
    condensed_str = "normal";

/*    l = family_l; */

    while((fix < 0) && (family = (char *)family_l/*HTList_nextObject(l)*/)) {
	if (strcasecmp(family,"symbol") == 0)
	    rgstry = "*-*";
	else
	    rgstry = "iso8859-1";

	sprintf(name,"-*-%s-%s-%s-%s-%s-%ld-*-*-*-*-*-%s", family, weight_str, slant_str, condensed_str, serif_str, px, rgstry);
	fix = myLoadFont(name);

	if ((fix < 0) && ((style == SV_FONT_STYLE_ITALIC) || (style == SV_FONT_STYLE_OBLIQUE))){
	    slant_str = "o";
	    sprintf(name,"-*-%s-%s-%s-%s-%s-%ld-*-*-*-*-*-%s", family, weight_str, slant_str, condensed_str, serif_str, px, rgstry);
	    fix = myLoadFont(name);
	}

    }

    if (FONT_TRACE)
	fprintf(stderr,"font: %s %s\n", name,((fix >= 0) ? "succeded" : "failed, trying again"));

#if 0
    if ((fix < 0) && (style == SV_FONT_STYLE_ITALIC)) {
	slant_str = "o";

	l = family_l;
	while((fix <= 0) && (family = (char *)HTList_nextObject(l))) {
	    if (strcasecmp(family,"symbol") == 0)
		rgstry = "*-*";
	    else
		rgstry = "iso8859-1";

	    sprintf(name,"-*-%s-%s-%s-%s-%s-%ld-*-*-*-*-*-%s", family, weight_str, slant_str, condensed_str, serif_str, px, rgstry);
	    fix = myLoadFont(name);
	}

	if (FONT_TRACE)
	    fprintf(stderr,"font: %s %s\n", name,((fix >= 0) ? "succeded" : "failed, trying again"));
    }
#endif

    rgstry = "iso8859-1";

    if (fix < 0) {
	if (FONT_TRACE)
	    fprintf(stderr,"font: resorting to 9x15\n");
	strcpy(name,"9x15");
	fix = myLoadFont(name);
/*	fprintf(stderr,"font: %s %s\n", name,((fix >= 0) ? "succeded" : "failed, trying again"));*/
    }

    if (fix < 0) {
	if (FONT_TRACE)
	    fprintf(stderr,"font: resorting to 9x15\n");
	strcpy(name,"fixed");
	fix = myLoadFont(name);
/*	fprintf(stderr,"font: %s %s\n", name,((fix >= 0) ? "succeded" : "failed, trying again"));*/
    }

    if (fix > 255) {
	fprintf(stderr,"GetFont: WARNING! more that 255 fonts loaded!\n");
        Exit(0);
    }

    return fix;
}

float dpi(void)
{
    float wmm, w;

    wmm = DisplayWidthMM(display, screen);
    w   = DisplayWidth(display, screen);

    return((float)(w/wmm));
}

/*
   how to get from mm to px:

    1 inch = 25.4 mm = 72pt

    pix / mm ~~ 4 (100dpi)
                3 (75 dpi)

    14 pt / 2.83 (pt/mm)

    14 pt * mm/pt * pix/mm = pix
*/



