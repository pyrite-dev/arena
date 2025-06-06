/*

howcome  27/9/95:

This is the style sheet module (i.e. the "styler") of Arena. It will
parse CSS style sheets into structures that can be queried by the
formatter of e.g an HTML browser.

Typically, the application will create a new style sheet initialized
with the application fallback values. For this, use StyleGetInit

Then, if the user has a personal style sheet -- or if incoming
documents have style sheets -- they can be parsed and added to an
exsiting style sheet by StyleChew

As the HTML/SGML parser encounters tags, FormatElementStart should be
called to have style properties evaluated for the specific element in
this specific context. The set of properties are pushed onto a stack
which corresponds to the document structure.

The formatter can query the style properties of the current element
StyleGet.

FormatElementEnd pops a set of style properties from the stack and
shoul be called when the parser reports the end of an element.


History:

howcome 15/2/94: written from scratch

Dave Raggett:

How about the following notation for style:

        * -> font.name = times

        H1 -> font.name = garamond
        H[2-6] -> font.name = helvetica
        
        P.CLASS = abstract ->
            {
                font.name = helvetica
                font.size = 10 pt
                indent.left = 4 em
                indent.right = 4 em
            }

The left hand side of the "->" operator matches tag names and uses
the dot notation for attribute values. You can use regular expressions
such as [digit-digit] [letter-letter] * and ?. The browser is responsible
for sorting the rules in order of most to least specificity so it doesn't
matter that the first rule matches anything.

The right hand side simply sets property values. The HTML formatter knows
what properties are relevant to which elements. Some properties are
specific to one kind of element, while others are generic. Properties
set by a more specific rule (matching the current element) override values
set by least specific rules or values inherited from nested or peer elements.


howcome 1/4/95: 

  Dave's proposal for syntax has been implemented with some shortcuts.
  The "->" construct was replaced by ":" whish should have the same associations. 

  The "P.CLASS = abstract" construct is more general than required for
  the class attribute. As a shortcut, "P [abstract]" has been implemented.

howcome 5/4/95: 

  pattern searches implemented /H1 P/: color.text = #FFF

  Also, commands can be put on the same line with a ";" separating.

  # this is a comment
  / so is this

howcome 5/95: changes to the syntax:

  class is now always denoted p.foo
  elemets and hints can be grouped:

  H1, H2: font.family = helvetica; font.color = #F00

howcome 8/95:

  style sheet module updated to support the august 10 version of

    http://www.w3.org/hypertext/WWW/Style/css/draft.html

*/


#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "www.h"

extern int debug;
extern Doc *CurrentDoc;
extern Context *context;
extern int PixOffset;
extern int paintlen;
extern int paintStartLine;
extern Byte *paint;
extern unsigned int ui_n;
extern unsigned int win_width, win_height;
extern int fontsize;
extern int prepass;
extern int sbar_width;
extern unsigned int win_width, win_height;
extern int ToolBarHeight;
extern int statusHeight;
extern int damn_table;

double lens_factor = 1.0;

extern int math_normal_sym_font, math_small_sym_font,
    math_normal_text_font, math_small_text_font, math_italic_text_font,
    math_bold_text_font;


HTArray *style_stack = NULL;
StyleFlat *current_flat;
StyleSheet *current_sheet;

/* 
  str2list is a convenience function to make a list out of two strings
*/

HTList *str2list(char *s, char *t)
{
    HTList *l = HTList_new();

    if (t)
	HTList_addObject(l, strdup(t));
    if (s)
	HTList_addObject(l, strdup(s));
    return l;			      
}

/*
  pt2px converts points to pixels

  display_pt2px =  (25.4 / 72.0) * 
    (((double) DisplayWidth(display,screen)) / ((double) DisplayWidthMM(display, screen)));
*/


extern double display_pt2px;

int pt2px(double pt)
{
    return (int) ((display_pt2px * pt) + 0.5);
}


/* 
  The Flag functions are used to remember various settings. E.g., if
  there is a big initial, we want that property to inherit to child
  elements, but we do not want it to be used more than once. Depending
  on your HTML/SGML parser, this problem may have more elegant solution
*/


BOOL StyleGetFlag(int flag)
{
    switch (flag)
    {
    case S_INITIAL_FLAG:
	return current_sheet->initial_flag;
	break;
    case S_LEADING_FLAG:
	return current_sheet->leading_flag;
	break;
    case S_INDENT_FLAG:
	return current_sheet->indent_flag;
	break;
    case S_MARGIN_TOP_FLAG:
	return current_sheet->margin_top_flag;
	break;
    }
    return FALSE;
}

void StyleSetFlag(int flag, BOOL value)
{
    switch (flag)
    {
    case S_INITIAL_FLAG:
	current_sheet->initial_flag = value;
	break;
    case S_LEADING_FLAG:
	current_sheet->leading_flag = value;
	break;
    case S_INDENT_FLAG:
	current_sheet->indent_flag = value;
	break;
    case S_MARGIN_TOP_FLAG:
	current_sheet->margin_top_flag = value;
	break;	
    }
}




/* methods */


StyleRule *NewStyleRule()
{
    return (StyleRule *)calloc(1,sizeof(StyleRule));
}


StyleSelector *NewStyleSelector()
{
    return (StyleSelector *)calloc(1,sizeof(StyleSelector));
}

StyleSimpleSelector *NewStyleSimpleSelector()
{
    return (StyleSimpleSelector *)calloc(1,sizeof(StyleSimpleSelector));
}

StyleSheet *NewStyleSheet()
{
    return (StyleSheet *)calloc(1,sizeof(StyleSheet));
}

/* NewStyleFlat allocates memory AND sets initial values. CHANGE! */

StyleFlat *NewStyleFlat()
{
    StyleFlat *sf;

    sf = (StyleFlat *)calloc(1, sizeof(StyleFlat));

    sf->value[SI_MARGIN_TOP].value = 5;
    sf->value[SI_MARGIN_TOP].type = ST_PX;

    sf->value[SI_MARGIN_RIGHT].value = 20;
    sf->value[SI_MARGIN_RIGHT].type = ST_PX;

    sf->value[SI_MARGIN_BOTTOM].value = 5;
    sf->value[SI_MARGIN_BOTTOM].type = ST_PX;

    sf->value[SI_MARGIN_LEFT].value = 20;
    sf->value[SI_MARGIN_LEFT].type = ST_PX;

    sf->value[SI_FONT].value = -2;

    return sf;
}


StyleValue *NewStyleValue()
{
    return (StyleValue *)calloc(1,sizeof(StyleValue));
}



void FreeStyleSelector(StyleSelector *selector)
{
    StyleSelector *prev_selector;

    prev_selector = selector;

    while((selector = selector->ancestor)) {
	Free(prev_selector->ss.class);
	Free(prev_selector);
	prev_selector = selector;
    }
    if(!prev_selector)
    {
	Free(prev_selector->ss.class);
	Free(prev_selector);
    }
}
	

void FreeStyleRule(StyleRule *r)
{
    if (!r)
	return;

    FreeStyleSelector(r->selector);
    Free(r->warning);
}
    

void FreeStyleSheet(StyleSheet *s)
{
    HTList *l;
    StyleRule *r;
    StyleInternalProperty i;

    if (!s)
	return;

    for (i=SI_UNKNOWN; i < SI_NPROPERTY; i++) {
	if ((l = s->iproperty[i]))
	    while ((r = (StyleRule *)HTList_nextObject(l)))
		FreeStyleRule(r);
    }
    Free(s);
}

/*
void StyleFree(StyleSheet *sheet)
{
    int i;
    
    for (i=SI_UNKNOWN; i < SI_NPROPERTY; i++) {
	HTList_destroy(sheet->element[i]);
    }
    Free(sheet);
}
*/

StyleSelector *CopySelector(StyleSelector *s)
{
    StyleSelector *sc;

    if (!s)
	return NULL;

    sc = NewStyleSelector();
    memcpy(sc, s, sizeof(StyleSelector));
    sc->ancestor = CopySelector (s->ancestor);
    return(sc);
}

StyleSheet *StyleCopy(StyleSheet *s)
{
    StyleSheet *sc = NewStyleSheet();
    StyleInternalProperty i;
    HTList *l, *lc;
    StyleRule *r, *rc;

    for (i=SI_UNKNOWN; i<SI_NPROPERTY; i++) {
	l = s->iproperty[i];
	lc = sc->iproperty[i];
	while ( (r = (StyleRule *) HTList_nextObject(l) )) {	
	    rc = NewStyleRule();
	    rc->iproperty = r->iproperty;
	    rc->value_p = r->value_p;
	    rc->weight = r->weight;
	    if (r->warning)
		rc->warning = strdup(r->warning);

	    rc->selector = CopySelector(r->selector);
	    if (!lc)
		lc = sc->iproperty[i] = HTList_new();
	    HTList_addObject(lc, (void *) rc);
	}
    }
    return sc;
}


/* find, and grade selector matches */

/* 
   SimpleSelectorMatch matches two "selector utits", i.e. an element
   with accompanying class (and, in level 2, attributes in general.

   r = rule, s = stack

   The gradins schems is simple: 

   0 = failure, no match
   1 = element matches, no classes specified in rule
   2 = element matches, classes specified and matches

*/

int SimpleSelectorMatch(StyleSimpleSelector *r, StyleSimpleSelector *s)
{
    if (r->element == s->element) 
    {
	if (r->class) 
	{
	    if ((s->class) && (strcasecmp(r->class, s->class) == 0))
		return 2;
	    return 0;
	}
	return 1;
    }
    return 0;
}		

int SelectorMatch(StyleSelector *s, HTArray *style_stack)
{
    HTArray *l = style_stack;
    StyleSimpleSelector *stack_el;
    int pos = style_stack->size - 1;
    int score = 0;

    return (SimpleSelectorMatch(&s->ss, (StyleSimpleSelector *)style_stack->data[style_stack->size - 1]));

    while (pos && (stack_el = (StyleSimpleSelector *)style_stack->data[pos--])) {
	if (SimpleSelectorMatch(&s->ss, stack_el))
	    s = s->ancestor;
	if (!s)
	    return 0;
    }
    return 1;
}

/*
  StyleDeductValue is used to convert all units into pixels. for percentages, we must
  keep the previous values (if inherited) except for margins...
*/

void StyleDeductValue(StyleValue *rule_value, StyleValue *prev_value, StyleValue *value)
{

    value->type = rule_value->type;
    value->value = rule_value->value;
    if(rule_value->iproperty == SI_COLOR)
    {
	if(rule_value->type == ST_RGB)
	    value->value = rgb2ix( (int)(rule_value->value >> 24), (int)((rule_value->value >> 16) & 0xFF), 
				   (int)((rule_value->value >> 8) & 0xFF), (int)(rule_value->value & 0xFF), False);
	else
	    value->value = rgb2ix( (int)(rule_value->value >> 24), (int)((rule_value->value >> 16) & 0xFF), 
				   (int)((rule_value->value >> 8) & 0xFF), (int)(rule_value->value & 0xFF), False);
    }
    else
    {
	if(rule_value->type == ST_PERCENT)
	{
	    if((rule_value->iproperty != SI_MARGIN_LEFT) &&
	       (rule_value->iproperty != SI_MARGIN_RIGHT) &&
	       (rule_value->iproperty != SI_MARGIN_TOP) &&
	       (rule_value->iproperty != SI_MARGIN_BOTTOM))
	    {
		value->value = (int) (((double)prev_value->value) * ((double)rule_value->value) / 100.0);
		value->type = ST_PX;
	    }
	}
	else
	    if(rule_value->type == ST_PT)
	    {
		if(rule_value->iproperty != SI_FONT_SIZE)
		{
		    value->value = (long)(pt2px((double)rule_value->value));
		    value->type = ST_PX;
		}
	    }
    }
}
/* 
   StyleEval flattens the whole list of properties. One could argue
   that properties should only be evaluated when needed. That would
   complicate things a bit, and may not be worth it..
*/

StyleFlat *StyleEval(StyleSheet *sheet, HTArray *style_stack)
{
    StyleSimpleSelector *stack_el;
    StyleFlat *last_flat = NULL, *flat = NewStyleFlat();
    StyleRule *rule;
    StyleInternalProperty i;
    HTList *l;
    BOOL recompute_font = FALSE, recompute_alt_font = FALSE;
    long value;
    int selector_score;

    if (REGISTER_TRACE)
	fprintf(stderr,"StyleEval:\n");

    if (style_stack->size <= 0)
	return NULL;
    if (style_stack->size > 1) {
	StyleSimpleSelector *last_stack_el = (StyleSimpleSelector *) style_stack->data[style_stack->size - 2];
	last_flat = last_stack_el->flat;
    }
    
    
    stack_el = (StyleSimpleSelector *) style_stack->data[style_stack->size - 1];
    
	
	/* copy the inherited properties from ancestor */
	
    for (i=SI_UNKNOWN; i < SI_NPROPERTY; i++) {
	if (last_flat) {

	    switch (i) {

		/* these properies are not inherited */
		
	    case SI_BACKGROUND:
	    case SI_BACKGROUND_COLOR:
	    case SI_BACKGROUND_IMAGE:
	    case SI_TEXT_DECORATION:

	    case SI_PADDING_TOP:
	    case SI_PADDING_RIGHT:
	    case SI_PADDING_BOTTOM:
	    case SI_PADDING_LEFT:

	    case SI_MARGIN_TOP:
	    case SI_MARGIN_RIGHT:
	    case SI_MARGIN_BOTTOM:
	    case SI_MARGIN_LEFT:

	    case SI_DISPLAY:
	    case SI_WIDTH:
	    case SI_HEIGHT:
	    case SI_FLOAT:
	    case SI_CLEAR:
	    case SI_PACK:

	    case SI_BORDER_STYLE_TOP:
	    case SI_BORDER_STYLE_RIGHT:
	    case SI_BORDER_STYLE_BOTTOM:
	    case SI_BORDER_STYLE_LEFT:

	    case SI_BORDER_WIDTH_TOP:
	    case SI_BORDER_WIDTH_RIGHT:
	    case SI_BORDER_WIDTH_BOTTOM:
	    case SI_BORDER_WIDTH_LEFT:

	    case SI_BORDER_COLOR_TOP:
	    case SI_BORDER_COLOR_RIGHT:
	    case SI_BORDER_COLOR_BOTTOM:
 	    case SI_BORDER_COLOR_LEFT:

		break;
		
	    default:
		flat->value[i] = last_flat->value[i];
		flat->weight[i] = 0;
		flat->origin[i] = 0;
		flat->specificity[i] = 0;
	    }
	}
	
	recompute_font += (flat->value[SI_FONT].value==-2);

	/* go through list of rules associated with this property */

	switch (i) {
	    
	case SI_FONT:
	    if(recompute_font)
		flat->value[SI_FONT].value =  GetFont((char *) flat->value[SI_FONT_FAMILY].value,
						      (int)flat->value[SI_FONT_SIZE].value,
						      (int)flat->value[SI_FONT_WEIGHT].value,
						      (int)flat->value[SI_FONT_STYLE_SLANT].value, False);
	    break;
	default:
	    l = sheet->iproperty[i];
	    while ( (rule = (StyleRule *) HTList_nextObject(l) )) 
	    {
		int current_selector_match = 1;
		int tmp_match;

		tmp_match = SelectorMatch(rule->selector, style_stack);
		
		if(tmp_match>=current_selector_match)
		{
		    current_selector_match = tmp_match;
		    if ((rule->weight >= flat->weight[i]) && 
			(rule->origin >= flat->origin[i])
#if 0                      
			&&
			(rule->specificity >= flat->specificity[i]) &&
			(SelectorMatch(rule->selector, style_stack))
#endif
			)
		    {
			/* ok, the rule applies  */
			
			if ((i == SI_FONT_SIZE) ||
			    (i == SI_FONT_FAMILY) ||
			    (i == SI_FONT_WEIGHT) ||
			    (i == SI_FONT_STYLE_SLANT))
			    if(flat->value[i].value != rule->value_p->value)
				recompute_font++;

			flat->weight[i] = rule->weight;
			flat->origin[i] = rule->origin;
			flat->specificity[i] = rule->specificity;
			
			
		    /* Here the magic will appear ! */
#if 1
			if(last_flat)
			    StyleDeductValue(rule->value_p, &(last_flat->value[i]), &(flat->value[i]));
			else
			    StyleDeductValue(rule->value_p, NULL, &(flat->value[i]));
#else

		    /*	    StyleDeductValue(rule->value_p, &(flat->value[i])); */

		    flat->value[i].type = rule->value_p->type;
		    if( i != SI_COLOR)
			flat->value[i].value = rule->value_p->value;
		    else
			flat->value[i].value = rgb2ix( (int)(rule->value_p->value >> 24), (int)((rule->value_p->value >> 16) & 0xFF), 
						       (int)((rule->value_p->value >> 8) & 0xFF), (int)(rule->value_p->value & 0xFF), False);
#endif
		    }
		}
	    }
	}
    }
    return flat;
}



/* 
  StyleGet is used by the UA to pull values out of a flattened stylesheet
*/


long StyleGet(StyleInternalProperty iproperty)
{
    if((iproperty != SI_MARGIN_LEFT) &&
       (iproperty != SI_MARGIN_RIGHT) &&
       (iproperty != SI_MARGIN_TOP) &&
       (iproperty != SI_MARGIN_BOTTOM))
	return (current_flat->value[iproperty].value);
    else
    {
	if(current_flat->value[iproperty].type == ST_PERCENT)
	{
	    Frame *parent_frame, *curr_frame;
#if 0
	    curr_frame = (Frame *)Pop();
	    if(curr_frame)
	    {
#endif
		parent_frame = (Frame *)Pop();
		if(parent_frame)
		{
		    Push(parent_frame);
#if 0
		    Push(curr_frame);  /* restore the stack */
#endif
		    if((iproperty != SI_MARGIN_LEFT) &&
		       (iproperty != SI_MARGIN_RIGHT))
			return ((int)(((float)parent_frame->height)*((float)current_flat->value[iproperty].value)/100.0));
		    else
			return ((int)(((float)(parent_frame->width - parent_frame->leftmargin - parent_frame->rightmargin))*((float)current_flat->value[iproperty].value)/100.0));
		}
#if 0
		else 
		{
		    Push(curr_frame);
		    return 0; /* error, no parent frame */
		}
	    }
#endif
	    else
		return 0;  /* error, no frames!!! */
	}
	else
	    return (current_flat->value[iproperty].value);
    }
}

/*
  Store and manage style rules
*/


/*
  StyleSane is a debugging functions which tries to print out the
  content of a style sheet
*/

void StyleSane(StyleSheet *s)
{
    StyleInternalProperty i;
    StyleRule *r;

    if (STYLE_TRACE)
	fprintf(stderr,"\nstylesane:\n");

    if (!s)
	return;

    for (i=SI_UNKNOWN; i < SI_NPROPERTY; i++) {
	HTList *l = s->iproperty[i];
	StyleSelector *sel;

	if (STYLE_TRACE)
	    fprintf(stderr,"  iproperty %d\n", i);
	while ( (r = (StyleRule *)HTList_nextObject(l)) ) {
	    if (STYLE_TRACE)
		fprintf(stderr,"    selector (");
	    sel = r->selector;
	    if (STYLE_TRACE)
	    {
		do {
		    if (sel->ss.class)
			fprintf(stderr,"%d.%s ",sel->ss.element, sel->ss.class);
		    else
			fprintf(stderr,"%d ",sel->ss.element);
		} while ((sel = sel->ancestor));
		fprintf(stderr,") ");

	    fprintf(stderr,"    property %d \tweight %d \tvalue ", r->iproperty, r->weight);
	    
	    switch (r->value_p->type) {
	    case ST_UNKNOWN:
	    case ST_FLAG:
	    case ST_FLOAT:
	    case ST_PX:
	    case ST_PERCENT:
	    case ST_EM:
		fprintf(stderr,"%ld ", (long)r->value_p->value);
		break;
	    case ST_RGB:
		fprintf(stderr,"#%x,%x,%x ",
			(int)(((int)r->value_p->value >> 16) & 0xFF),
			(int)(((int)r->value_p->value >> 8) & 0xFF),
			(int)(((int)r->value_p->value >> 0) & 0xFF));
		break;
	    case ST_STR: {
		char *s = (char *)r->value_p->value;

		fprintf(stderr,"%s+",s);
		fprintf(stderr," ");
		break;
	    }
	    }
	    }
	}
    }
    if (STYLE_TRACE)
	fprintf(stderr,"stylesane end\n");
}


/* StyleAddRule adds a rule to the pool of style rules */

void StyleAddRule(StyleSheet *sheet, StyleSelector *selector, HTList *values, Byte origin, Byte weight)
{
    HTList *l = values;
    StyleValue *sv_p;

    while ((sv_p = (StyleValue *)HTList_nextObject(l))) {
	StyleInternalProperty iproperty = sv_p->iproperty;
	StyleRule *r = NewStyleRule();

	r->selector = selector;
	r->iproperty = iproperty;
	r->value_p = sv_p;
	r->weight = weight;
	r->origin = origin;
	r->specificity = 0;

	if (!sheet->iproperty[iproperty])
	    sheet->iproperty[iproperty] = HTList_new();
	HTList_addObject(sheet->iproperty[iproperty], (void *) r);
    }
}


void StyleAddSimpleRule(StyleSheet *sheet, int element, StyleInternalProperty iproperty, Byte type, int origin, long value)
{
    StyleSelector *s = NewStyleSelector();
    StyleValue *sv_p = NewStyleValue();
    StyleRule *r = NewStyleRule();

    s->ss.element = element;
    sv_p->value = value;
    sv_p->type = type;
    sv_p->iproperty = iproperty;

    r->selector = s;
    r->iproperty = iproperty;
    r->value_p = sv_p;
    r->weight = SW_NORMAL;
    r->origin = origin;
    r->specificity = 0;

    if (!sheet->iproperty[iproperty])
	sheet->iproperty[iproperty] = HTList_new();
    HTList_addObject(sheet->iproperty[iproperty], (void *) r);
}



/*
 Parse section
*/


StyleExternalProperty str2eproperty(char *s)
{
    int l = strlen(s);

    if (!strncasecmp(s,"font", 4)) {
	if (l == 4)
	    return SE_FONT;
	if ((l == 9) && (strcasecmp(&s[5], "size") == 0))
	    return SE_FONT_SIZE;
	if ((l == 11) && (strcasecmp(&s[5], "family") == 0))
	    return SE_FONT_FAMILY;
	if ((l == 11) && (strcasecmp(&s[5], "weight") == 0))
	    return SE_FONT_WEIGHT;
	if ((l == 10) && (strcasecmp(&s[5], "style") == 0))
	    return SE_FONT_STYLE;

    } else if (strcasecmp(s, "color") == 0) {
      return SE_COLOR;
    } else if(!strcasecmp(s,"background")) {
	return SE_BACKGROUND;
    } else if (strncasecmp(s, "bg", 2) == 0) {
	if ((l == 18) && (strcasecmp(&s[3], "blend-direction") == 0))
	    return SE_BG_BLEND_DIRECTION;
	if ((l == 8) && (strcasecmp(&s[3], "style") == 0))
	    return SE_BG_STYLE;
	if ((l == 10) && (strcasecmp(&s[3], "postion") == 0))
	    return SE_BG_POSITION;
    }
    else if (!strncasecmp(s,"text", 4)) {
	if ((l == 9) && (strcasecmp(&s[5], "decoration") == 0))
	    return SE_TEXT_DECORATION;
	if ((l == 14) && (strcasecmp(&s[5], "transform") == 0))
	    return SE_TEXT_TRANSFORM;
	if ((l == 10) && (!strcasecmp(s+5, "align")))
	    return SE_TEXT_ALIGN;
	if ((l == 11) && (!strcasecmp(s+5, "indent")))
	    return SE_TEXT_INDENT;
    } else if (!strncasecmp(s,"padding",7)) {
	return SE_PADDING;

    } else if(!strncasecmp(s,"bg", 2)) {
	if ((l == 8 )&&(!strcasecmp(s+3,"style")))
	    return SE_BG_STYLE;
	if ((l == 11 ) &&(!strcasecmp(s+3,"position")))
	    return SE_BG_POSITION;
	if ((l == 18 ) && (!strcasecmp(s+3,"blend-direction")))
	    return SE_BG_BLEND_DIRECTION;

    } else if (!strncasecmp(s,"letter", 6)) {
	if ((l == 14) && (strcasecmp(&s[7], "spacing") == 0))
	    return SE_LETTER_SPACING;
    } else if (!strncasecmp(s,"word", 4)) {
	if ((l == 12) && (strcasecmp(&s[5], "spacing") == 0))
	    return SE_WORD_SPACING;

    } else if (!strncasecmp(s,"margin", 6)) {
	if (l == 6)
	    return SE_MARGIN;
	if ((l == 10) && (strcasecmp(&s[7], "top") == 0))
	    return SE_MARGIN_TOP;
	if ((l == 12) && (strcasecmp(&s[7], "right") == 0))
	    return SE_MARGIN_RIGHT;
	if ((l == 13) && (strcasecmp(&s[7], "bottom") == 0))
	    return SE_MARGIN_BOTTOM;
	if ((l == 11) && (strcasecmp(&s[7], "left") == 0))
	    return SE_MARGIN_LEFT;

    } else if (!strcasecmp(s,"line-height")) {
	return SE_LINE_HEIGHT;
    } else if (!strcasecmp(s,"vertical-align")) {
	return SE_VERTICAL_ALIGN;
    } else if (!strcasecmp(s,"display")) {
	return SE_DISPLAY;
    } else if (!strcasecmp(s,"width")) {
	return SE_WIDTH;
    } else if (!strcasecmp(s,"height")) {
	return SE_HEIGHT;
    } else if (!strcasecmp(s,"float")) {
	return SE_FLOAT;
    } else if (!strcasecmp(s,"clear")) {
	return SE_CLEAR;
    } else if (!strcasecmp(s,"pack")) {
	return SE_PACK;
    } else {
	if (STYLE_TRACE)
	    fprintf(stderr,"Unknown style property: %s\n", s);
	return SE_UNKNOWN;
    }
}


int element_enum(char *s, int *token_class_p)
{
    int c, len;

    if (!s)
	return UNKNOWN;

    len = strlen(s);
    c = TOLOWER(*s);

    if (isalpha(c))
    {
        if (c == 'a')
        {
            if (len == 1 && strncasecmp(s, "a", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_ANCHOR;
            }

            if (len == 3 && strncasecmp(s, "alt", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_ALT;
            }

            if (len == 5 && strncasecmp(s, "added", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_ADDED;
            }

            if (len == 7 && strncasecmp(s, "address", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_ADDRESS;
            }

            if (len == 8 && strncasecmp(s, "abstract", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_ABSTRACT;
            }
        }
        else if (c == 'b')
        {
            if (len == 1)
            {
                *token_class_p = EN_TEXT;
                return TAG_BOLD;
            }

            if (len == 2 && strncasecmp(s, "br", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_BR;
            }

            if (len == 4 && strncasecmp(s, "body", len) == 0)
            {
                *token_class_p = EN_MAIN;
                return TAG_BODY;
            }

            if (len == 10 && strncasecmp(s, "blockquote", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_QUOTE;
            }

            if (len == 4 && strncasecmp(s, "base", len) == 0)
            {
                *token_class_p = EN_SETUP;
                return TAG_BASE;
            }
        }
        else if (c == 'c')
        {
            if (len == 4)
            {
                if (strncasecmp(s, "code", len) == 0)
                {
                    *token_class_p = EN_TEXT;
                    return TAG_CODE;
                }

                if (strncasecmp(s, "cite", len) == 0)
                {
                    *token_class_p = EN_TEXT;
                    return TAG_CITE;
                }
            }
            else if (len == 7 && (strncasecmp(s, "caption", len) == 0))/* howcome 3/2/95: = -> == after hint from P.M.Hounslow@reading.ac.uk */
            {
                *token_class_p = EN_BLOCK;
                return TAG_CAPTION;
            }
        }
        else if (c == 'd')
        {
            if (len == 3 && strncasecmp(s, "dfn", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_DFN;
            }

	    /* howcome 11/8/95: added upport for DIR */

            if (len == 3 && strncasecmp(s, "dir", len) == 0)
            {
                *token_class_p = EN_LIST;
                return TAG_UL;
            }


            if (len != 2)
                return 0;

            if (strncasecmp(s, "dl", len) == 0)
            {
                *token_class_p = EN_LIST;
                return TAG_DL;
            }

            if (strncasecmp(s, "dt", len) == 0)
            {
                *token_class_p = EL_DEFLIST;
                return TAG_DT;
            }

            if (strncasecmp(s, "dd", len) == 0)
            {
                *token_class_p = EL_DEFLIST;
                return TAG_DD;
            }
        }
        else if (c == 'e')
        {
            if (len == 2 && strncasecmp(s, "em", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_EM;
            }
        }
        else if (c == 'f')
        {
            if (len == 3 && strncasecmp(s, "fig", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_FIG;
            }
        }
        else if (c == 'h')
        {
            if (len == 4) {
		if (strncasecmp(s, "head", len) == 0)
		{
		    *token_class_p = EN_SETUP;
		    return TAG_HEAD;
		} 

		/* added by howcome 27/8/95 */

		else if (strncasecmp(s, "html", len) == 0)
		{
		    *token_class_p = EN_SETUP;
		    return TAG_HTML;
		}
	    }

            if (len != 2)
                return 0;

            *token_class_p = EN_HEADER;
            c = TOLOWER(s[1]);

            switch (c)
            {
                case '1':
                    return TAG_H1;
                case '2':
                    return TAG_H2;
                case '3':
                    return TAG_H3;
                case '4':
                    return TAG_H4;
                case '5':
                    return TAG_H5;
                case '6':
                    return TAG_H6;
                case 'r':
                    *token_class_p = EN_BLOCK;
                    return TAG_HR;
            }
        }
        else if (c == 'i')
        {
            if (len == 1)
            {
                *token_class_p = EN_TEXT;
                return TAG_ITALIC;
            }

            if (len == 3 && strncasecmp(s, "img", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_IMG;
            }

            if (len == 5 && strncasecmp(s, "input", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_INPUT;
            }

            if (len == 7 && strncasecmp(s, "isindex", len) == 0)
            {
                *token_class_p = EN_SETUP;
                return TAG_ISINDEX;
            }
        }
        else if (c == 'k')
        {
            if (len == 3 && strncasecmp(s, "kbd", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_KBD;
            }
        }
        else if (c == 'l')
        {
            if (len == 2 && strncasecmp(s, "li", len) == 0)
            {
                *token_class_p = EN_LIST;
                return TAG_LI;
            }

            if (len == 4 && strncasecmp(s, "link", len) == 0)
            {
                *token_class_p = EN_SETUP;
                return TAG_LINK;
            }

        }
        else if (c == 'm')
        {
            if (len == 4 && strncasecmp(s, "math", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_MATH;
            }

            if (len == 6 && strncasecmp(s, "margin", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_MARGIN;
            }

	    /* howcome 11/8/95: added MENU to be compatible with HTML2 */
            if (len == 4 && strncasecmp(s, "menu", len) == 0)
            {
                *token_class_p = EN_LIST;
                return TAG_UL;
            }


        }
        else if (c == 'n')
        {
            if (len == 4 && strncasecmp(s, "note", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_NOTE;
            }
        }
        else if (c == 'o')
        {
            if (len == 2 && strncasecmp(s, "ol", len) == 0)
            {
                *token_class_p = EN_LIST;
                return TAG_OL;
            }

            if (len == 6 && strncasecmp(s, "option", len) == 0)
            {
                *token_class_p = EN_TEXT;  /* kludge for error recovery */
                return TAG_OPTION;
            }
        }
        else if (c == 'p')
        {
            if (len == 1)
            {
                *token_class_p = EN_BLOCK;
                return TAG_P;
            }

            if (len == 3 && strncasecmp(s, "pre", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_PRE;
            }
        }
        else if (c == 'q')
        {
            if (len == 1)
            {
                *token_class_p = EN_TEXT;
                return TAG_Q;
            }

            if (len == 5 && strncasecmp(s, "quote", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_QUOTE;
            }
        }
        else if (c == 'r')
        {
            if (len == 7 && strncasecmp(s, "removed", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_REMOVED;
            }
        }
        else if (c == 's')
        {
            if (len == 1)
            {
                *token_class_p = EN_TEXT;
                return TAG_STRIKE;
            }

            if (len == 3 && strncasecmp(s, "sup", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_SUP;
            }

            if (len == 3 && strncasecmp(s, "sub", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_SUB;
            }

            if (len == 4 && strncasecmp(s, "samp", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_SAMP;
            }

            if (len == 5 && strncasecmp(s, "small", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_SMALL;
            }

            if (len == 6 && strncasecmp(s, "strong", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_STRONG;
            }

            if (len == 6 && strncasecmp(s, "select", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_SELECT;
            }

            if (len == 6 && strncasecmp(s, "strike", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_STRIKE;
            }

	    /* howcome 26/2/95 */

            if (len == 5 && strncasecmp(s, "style", len) == 0)
            {
                *token_class_p = EN_SETUP;
                return TAG_STYLE;
            }

        }
        else if (c == 't')
        {
            if (len == 5 && strncasecmp(s, "title", len) == 0)
            {
                *token_class_p = EN_SETUP;
                return TAG_TITLE;
            }

            if (len == 2 && strncasecmp(s, "tt", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_TT;
            }

            if (len == 2 && strncasecmp(s, "tr", len) == 0)
            {
                *token_class_p = EN_TABLE;
                return TAG_TR;
            }

            if (len == 2 && strncasecmp(s, "th", len) == 0)
            {
                *token_class_p = EN_TABLE;
                return TAG_TH;
            }

            if (len == 2 && strncasecmp(s, "td", len) == 0)
            {
                *token_class_p = EN_TABLE;
                return TAG_TD;
            }

            if (len == 5 && strncasecmp(s, "table", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_TABLE;
            }

            if (len == 8 && strncasecmp(s, "textarea", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_TEXTAREA;
            }
        }
        else if (c == 'u')
        {
            if (len == 1)
            {
                *token_class_p = EN_TEXT;
                return TAG_UNDERLINE;
            }

            if (len == 2 && strncasecmp(s, "ul", len) == 0)
            {
                *token_class_p = EN_LIST;
                return TAG_UL;
            }
        }
        else if (c == 'v')
        {
            if (len == 3 && strncasecmp(s, "var", len) == 0)
            {
                *token_class_p = EN_TEXT;
                return TAG_VAR;
            }
        }
        else if (c == 'x')
        {
            if (len == 3 && strncasecmp(s, "xmp", len) == 0)
            {
                *token_class_p = EN_BLOCK;
                return TAG_PRE;
            }
        }
    }

    *token_class_p = EN_UNKNOWN;
    return UNKNOWN; /* unknown tag */
}


BOOL ParseColorName(const char *s, StyleType *type_p, long *value_p)
{
    char *sp;

    switch (TOLOWER(*s)) {

    case 'a':
	if (!strcasecmp("aliceblue", s)) { 
	    *type_p = ST_RGB; *value_p = (240 << 16) | (248 << 8) | 255; return True;
	} else
	if (!strcasecmp("antiquewhite", s)) { 
	    *type_p = ST_RGB; *value_p = (250 << 16) | (235 << 8) | 215; return True;
	} else
	if (!strcasecmp("aqua", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (255 << 8) | 255; return True;
	} else
	if (!strcasecmp("aquamarine", s)) { 
	    *type_p = ST_RGB; *value_p = (127 << 16) | (255 << 8) | 212; return True;
	} else
	if (!strcasecmp("azure", s)) { 
	    *type_p = ST_RGB; *value_p = (240 << 16) | (255 << 8) | 255; return True;
	}
	break;
    case 'b':
	if (!strcasecmp("beige", s)) { 
	    *type_p = ST_RGB; *value_p = (245 << 16) | (245 << 8) | 220; return True;
	} else
	if (!strcasecmp("bisque", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (228 << 8) | 196; return True;
	} else
	if (!strcasecmp("black", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (0 << 8) | 0; return True;
	} else
	if (!strcasecmp("blanchedalmond", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (235 << 8) | 205; return True;
	} else
	if (!strcasecmp("blue", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (0 << 8) | 255; return True;
	} else
	if (!strcasecmp("blueviolet", s)) { 
	    *type_p = ST_RGB; *value_p = (138 << 16) | (43 << 8) | 226; return True;
	} else
	if (!strcasecmp("brown", s)) { 
	    *type_p = ST_RGB; *value_p = (165 << 16) | (42 << 8) | 42; return True;
	} else
	if (!strcasecmp("burlywood", s)) { 
	    *type_p = ST_RGB; *value_p = (222 << 16) | (184 << 8) | 135; return True;
	}
	break;
    case 'c':
	if (!strcasecmp("cadetblue", s)) { 
	    *type_p = ST_RGB; *value_p = (95 << 16) | (158 << 8) | 160; return True;
	} else
	if (!strcasecmp("chartreuse", s)) { 
	    *type_p = ST_RGB; *value_p = (127 << 16) | (255 << 8) | 0; return True;
	} else
	if (!strcasecmp("chocolate", s)) { 
	    *type_p = ST_RGB; *value_p = (210 << 16) | (105 << 8) | 30; return True;
	} else
	if (!strcasecmp("coral", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (127 << 8) | 80; return True;
	} else
	if (!strcasecmp("cornflowerblue", s)) { 
	    *type_p = ST_RGB; *value_p = (100 << 16) | (149 << 8) | 237; return True;
	} else
	if (!strcasecmp("cornsilk", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (248 << 8) | 220; return True;
	} else
	if (!strcasecmp("crimson", s)) { 
	    *type_p = ST_RGB; *value_p = (220 << 16) | (20 << 8) | 60; return True;
	} else
	if (!strcasecmp("cyan", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (255 << 8) | 255; return True;
	}
	break;
    case 'd':
	if (!strcasecmp("darkblue", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (0 << 8) | 139; return True;
	} else
	if (!strcasecmp("darkcyan", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (139 << 8) | 139; return True;
	} else
	if (!strcasecmp("darkgoldenrod", s)) { 
	    *type_p = ST_RGB; *value_p = (184 << 16) | (134 << 8) | 11; return True;
	} else
	if (!strcasecmp("darkgray", s)) { 
	    *type_p = ST_RGB; *value_p = (169 << 16) | (169 << 8) | 169; return True;
	} else
	if (!strcasecmp("darkgreen", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (100 << 8) | 0; return True;
	} else
	if (!strcasecmp("darkkhaki", s)) { 
	    *type_p = ST_RGB; *value_p = (189 << 16) | (183 << 8) | 107; return True;
	} else
	if (!strcasecmp("darkmagenta", s)) { 
	    *type_p = ST_RGB; *value_p = (139 << 16) | (0 << 8) | 139; return True;
	} else
	if (!strcasecmp("darkolivegreen", s)) { 
	    *type_p = ST_RGB; *value_p = (85 << 16) | (107 << 8) | 47; return True;
	} else
	if (!strcasecmp("darkorange", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (140 << 8) | 0; return True;
	} else
	if (!strcasecmp("darkorchid", s)) { 
	    *type_p = ST_RGB; *value_p = (153 << 16) | (50 << 8) | 204; return True;
	} else
	if (!strcasecmp("darkred", s)) { 
	    *type_p = ST_RGB; *value_p = (139 << 16) | (0 << 8) | 0; return True;
	} else
	if (!strcasecmp("darksalmon", s)) { 
	    *type_p = ST_RGB; *value_p = (233 << 16) | (150 << 8) | 122; return True;
	} else
	if (!strcasecmp("darkseagreen", s)) { 
	    *type_p = ST_RGB; *value_p = (143 << 16) | (188 << 8) | 143; return True;
	} else
	if (!strcasecmp("darkslateblue", s)) { 
	    *type_p = ST_RGB; *value_p = (72 << 16) | (61 << 8) | 139; return True;
	} else
	if (!strcasecmp("darkslategray", s)) { 
	    *type_p = ST_RGB; *value_p = (47 << 16) | (79 << 8) | 79; return True;
	} else
	if (!strcasecmp("darkturquoise", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (206 << 8) | 209; return True;
	} else
	if (!strcasecmp("darkviolet", s)) { 
	    *type_p = ST_RGB; *value_p = (148 << 16) | (0 << 8) | 211; return True;
	} else
	if (!strcasecmp("deeppink", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (20 << 8) | 147; return True;
	} else
	if (!strcasecmp("deepskyblue", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (191 << 8) | 255; return True;
	} else
	if (!strcasecmp("dimgray", s)) { 
	    *type_p = ST_RGB; *value_p = (105 << 16) | (105 << 8) | 105; return True;
	} else
	if (!strcasecmp("dodgerblue", s)) { 
	    *type_p = ST_RGB; *value_p = (30 << 16) | (144 << 8) | 255; return True;
	}
	break;
    case 'f':
	if (!strcasecmp("firebrick", s)) { 
	    *type_p = ST_RGB; *value_p = (178 << 16) | (34 << 8) | 34; return True;
	} else
	if (!strcasecmp("floralwhite", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (250 << 8) | 240; return True;
	} else
	if (!strcasecmp("forestgreen", s)) { 
	    *type_p = ST_RGB; *value_p = (34 << 16) | (139 << 8) | 34; return True;
	} else
	if (!strcasecmp("fuchsia", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (0 << 8) | 255; return True;
	}
	break;
    case 'g':
	if (!strcasecmp("gainsboro", s)) { 
	    *type_p = ST_RGB; *value_p = (220 << 16) | (220 << 8) | 220; return True;
	} else
	if (!strcasecmp("ghostwhite", s)) { 
	    *type_p = ST_RGB; *value_p = (248 << 16) | (248 << 8) | 255; return True;
	} else
	if (!strcasecmp("gold", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (215 << 8) | 0; return True;
	} else
	if (!strcasecmp("goldenrod", s)) { 
	    *type_p = ST_RGB; *value_p = (218 << 16) | (165 << 8) | 32; return True;
	} else
	if (!strcasecmp("gray", s)) { 
	    *type_p = ST_RGB; *value_p = (128 << 16) | (128 << 8) | 128; return True;
	} else
	if (!strcasecmp("green", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (128 << 8) | 0; return True;
	} else
	if (!strcasecmp("greenyellow", s)) { 
	    *type_p = ST_RGB; *value_p = (173 << 16) | (255 << 8) | 47; return True;
	}
	break;
    case 'h':
	if (!strcasecmp("honeydew", s)) { 
	    *type_p = ST_RGB; *value_p = (240 << 16) | (255 << 8) | 240; return True;
	} else
	if (!strcasecmp("hotpink", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (105 << 8) | 180; return True;
	}
	break;
    case 'i':
	if (!strcasecmp("indianred", s)) { 
	    *type_p = ST_RGB; *value_p = (205 << 16) | (92 << 8) | 92; return True;
	} else
	if (!strcasecmp("indigo", s)) { 
	    *type_p = ST_RGB; *value_p = (75 << 16) | (0 << 8) | 130; return True;
	} else
	if (!strcasecmp("ivory", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (255 << 8) | 240; return True;
	}
	break;
    case 'k':
	if (!strcasecmp("khaki", s)) { 
	    *type_p = ST_RGB; *value_p = (240 << 16) | (230 << 8) | 140; return True;
	}
	break;
    case 'l':
	if (!strcasecmp("lavender", s)) { 
	    *type_p = ST_RGB; *value_p = (230 << 16) | (230 << 8) | 250; return True;
	} else
	if (!strcasecmp("lavenderblush", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (240 << 8) | 245; return True;
	} else
	if (!strcasecmp("lawngreen", s)) { 
	    *type_p = ST_RGB; *value_p = (124 << 16) | (252 << 8) | 0; return True;
	} else
	if (!strcasecmp("lemonchiffon", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (250 << 8) | 205; return True;
	} else
	if (!strcasecmp("lightblue", s)) { 
	    *type_p = ST_RGB; *value_p = (173 << 16) | (216 << 8) | 230; return True;
	} else
	if (!strcasecmp("lightcoral", s)) { 
	    *type_p = ST_RGB; *value_p = (240 << 16) | (128 << 8) | 128; return True;
	} else
	if (!strcasecmp("lightcyan", s)) { 
	    *type_p = ST_RGB; *value_p = (224 << 16) | (255 << 8) | 255; return True;
	} else
	if (!strcasecmp("lightgoldenrodyellow", s)) { 
	    *type_p = ST_RGB; *value_p = (250 << 16) | (250 << 8) | 210; return True;
	} else
	if (!strcasecmp("lightgreen", s)) { 
	    *type_p = ST_RGB; *value_p = (144 << 16) | (238 << 8) | 144; return True;
	} else
	if (!strcasecmp("lightgrey", s)) { 
	    *type_p = ST_RGB; *value_p = (211 << 16) | (211 << 8) | 211; return True;
	} else
	if (!strcasecmp("lightpink", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (182 << 8) | 193; return True;
	} else
	if (!strcasecmp("lightsalmon", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (160 << 8) | 122; return True;
	} else
	if (!strcasecmp("lightseagreen", s)) { 
	    *type_p = ST_RGB; *value_p = (32 << 16) | (178 << 8) | 170; return True;
	} else
	if (!strcasecmp("lightskyblue", s)) { 
	    *type_p = ST_RGB; *value_p = (135 << 16) | (206 << 8) | 250; return True;
	} else
	if (!strcasecmp("lightslategray", s)) { 
	    *type_p = ST_RGB; *value_p = (119 << 16) | (136 << 8) | 153; return True;
	} else
	if (!strcasecmp("lightsteelblue", s)) { 
	    *type_p = ST_RGB; *value_p = (176 << 16) | (196 << 8) | 222; return True;
	} else
	if (!strcasecmp("lightyellow", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (255 << 8) | 224; return True;
	} else
	if (!strcasecmp("lime", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (255 << 8) | 0; return True;
	} else
	if (!strcasecmp("limegreen", s)) { 
	    *type_p = ST_RGB; *value_p = (50 << 16) | (205 << 8) | 50; return True;
	} else
	if (!strcasecmp("linen", s)) { 
	    *type_p = ST_RGB; *value_p = (250 << 16) | (240 << 8) | 230; return True;
	}
	break;
    case 'm':
	if (!strcasecmp("magenta", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (0 << 8) | 255; return True;
	} else
	if (!strcasecmp("maroon", s)) { 
	    *type_p = ST_RGB; *value_p = (128 << 16) | (0 << 8) | 0; return True;
	} else
	if (!strcasecmp("mediumaquamarine", s)) { 
	    *type_p = ST_RGB; *value_p = (102 << 16) | (205 << 8) | 170; return True;
	} else
	if (!strcasecmp("mediumblue", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (0 << 8) | 205; return True;
	} else
	if (!strcasecmp("mediumorchid", s)) { 
	    *type_p = ST_RGB; *value_p = (186 << 16) | (85 << 8) | 211; return True;
	} else
	if (!strcasecmp("mediumpurple", s)) { 
	    *type_p = ST_RGB; *value_p = (147 << 16) | (112 << 8) | 219; return True;
	} else
	if (!strcasecmp("mediumseagreen", s)) { 
	    *type_p = ST_RGB; *value_p = (60 << 16) | (179 << 8) | 113; return True;
	} else
	if (!strcasecmp("mediumslateblue", s)) { 
	    *type_p = ST_RGB; *value_p = (123 << 16) | (104 << 8) | 238; return True;
	} else
	if (!strcasecmp("mediumspringgreen", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (250 << 8) | 154; return True;
	} else
	if (!strcasecmp("mediumturquoise", s)) { 
	    *type_p = ST_RGB; *value_p = (72 << 16) | (209 << 8) | 204; return True;
	} else
	if (!strcasecmp("mediumvioletred", s)) { 
	    *type_p = ST_RGB; *value_p = (199 << 16) | (21 << 8) | 133; return True;
	} else
	if (!strcasecmp("midnightblue", s)) { 
	    *type_p = ST_RGB; *value_p = (25 << 16) | (25 << 8) | 112; return True;
	} else
	if (!strcasecmp("mintcream", s)) { 
	    *type_p = ST_RGB; *value_p = (245 << 16) | (255 << 8) | 250; return True;
	} else
	if (!strcasecmp("mistyrose", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (228 << 8) | 225; return True;
	} else
	if (!strcasecmp("moccasin", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (228 << 8) | 181; return True;
	}
	break;
    case 'n':
	if (!strcasecmp("navajowhite", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (222 << 8) | 173; return True;
	} else
	if (!strcasecmp("navy", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (0 << 8) | 128; return True;
	}
	break;
    case 'o':
	if (!strcasecmp("oldlace", s)) { 
	    *type_p = ST_RGB; *value_p = (253 << 16) | (245 << 8) | 230; return True;
	} else
	if (!strcasecmp("olive", s)) { 
	    *type_p = ST_RGB; *value_p = (128 << 16) | (128 << 8) | 0; return True;
	} else
	if (!strcasecmp("olivedrab", s)) { 
	    *type_p = ST_RGB; *value_p = (107 << 16) | (142 << 8) | 35; return True;
	} else
	if (!strcasecmp("orange", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (165 << 8) | 0; return True;
	} else
	if (!strcasecmp("orangered", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (69 << 8) | 0; return True;
	} else
	if (!strcasecmp("orchid", s)) { 
	    *type_p = ST_RGB; *value_p = (218 << 16) | (112 << 8) | 214; return True;
	}
	break;
    case 'p':
	if (!strcasecmp("palegoldenrod", s)) { 
	    *type_p = ST_RGB; *value_p = (238 << 16) | (232 << 8) | 170; return True;
	} else
	if (!strcasecmp("palegreen", s)) { 
	    *type_p = ST_RGB; *value_p = (152 << 16) | (251 << 8) | 152; return True;
	} else
	if (!strcasecmp("paleturquoise", s)) { 
	    *type_p = ST_RGB; *value_p = (175 << 16) | (238 << 8) | 238; return True;
	} else
	if (!strcasecmp("palevioletred", s)) { 
	    *type_p = ST_RGB; *value_p = (219 << 16) | (112 << 8) | 147; return True;
	} else
	if (!strcasecmp("papayawhip", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (239 << 8) | 213; return True;
	} else
	if (!strcasecmp("peachpuff", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (218 << 8) | 185; return True;
	} else
	if (!strcasecmp("peru", s)) { 
	    *type_p = ST_RGB; *value_p = (205 << 16) | (133 << 8) | 63; return True;
	} else
	if (!strcasecmp("pink", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (192 << 8) | 203; return True;
	} else
	if (!strcasecmp("plum", s)) { 
	    *type_p = ST_RGB; *value_p = (221 << 16) | (160 << 8) | 221; return True;
	} else
	if (!strcasecmp("powderblue", s)) { 
	    *type_p = ST_RGB; *value_p = (176 << 16) | (224 << 8) | 230; return True;
	} else
	if (!strcasecmp("purple", s)) { 
	    *type_p = ST_RGB; *value_p = (128 << 16) | (0 << 8) | 128; return True;
	}
	break;
    case 'r':
	if (!strcasecmp("red", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (0 << 8) | 0; return True;
	} else
	if (!strcasecmp("rosybrown", s)) { 
	    *type_p = ST_RGB; *value_p = (188 << 16) | (143 << 8) | 143; return True;
	} else
	if (!strcasecmp("royalblue", s)) { 
	    *type_p = ST_RGB; *value_p = (65 << 16) | (105 << 8) | 225; return True;
	} else
	if (!strncasecmp("rgb(", s,4)) {
	    sp = s+strlen(s);
	    *sp=' ';
	    *type_p = ST_RGB;
	    sp = strchr(s,'(')+1;
	    while (*sp == ' ') sp++;
	    *value_p = (long)atoi(sp) & 255;
	    while (*sp != ' ') sp++;
	    while (*sp == ' ') sp++;
	    *value_p = *value_p << 8 | ((long)atoi(sp) & 255);
	    while (*sp != ' ') sp++;
	    while (*sp == ' ') sp++;
	    *value_p = *value_p << 8 | ((long)atoi(sp) & 255);
	    return True;
	}
	break;
    case 's':
	if (!strcasecmp("saddlebrown", s)) { 
	    *type_p = ST_RGB; *value_p = (139 << 16) | (69 << 8) | 19; return True;
	} else
	if (!strcasecmp("salmon", s)) { 
	    *type_p = ST_RGB; *value_p = (250 << 16) | (128 << 8) | 114; return True;
	} else
	if (!strcasecmp("sandybrown", s)) { 
	    *type_p = ST_RGB; *value_p = (244 << 16) | (164 << 8) | 96; return True;
	} else
	if (!strcasecmp("seagreen", s)) { 
	    *type_p = ST_RGB; *value_p = (46 << 16) | (139 << 8) | 87; return True;
	} else
	if (!strcasecmp("seashell", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (245 << 8) | 238; return True;
	} else
	if (!strcasecmp("sienna", s)) { 
	    *type_p = ST_RGB; *value_p = (160 << 16) | (82 << 8) | 45; return True;
	} else
	if (!strcasecmp("silver", s)) { 
	    *type_p = ST_RGB; *value_p = (192 << 16) | (192 << 8) | 192; return True;
	} else
	if (!strcasecmp("skyblue", s)) { 
	    *type_p = ST_RGB; *value_p = (135 << 16) | (206 << 8) | 235; return True;
	} else
	if (!strcasecmp("slateblue", s)) { 
	    *type_p = ST_RGB; *value_p = (106 << 16) | (90 << 8) | 205; return True;
	} else
	if (!strcasecmp("slategray", s)) { 
	    *type_p = ST_RGB; *value_p = (112 << 16) | (128 << 8) | 144; return True;
	} else
	if (!strcasecmp("snow", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (250 << 8) | 250; return True;
	} else
	if (!strcasecmp("springgreen", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (255 << 8) | 127; return True;
	} else
	if (!strcasecmp("steelblue", s)) { 
	    *type_p = ST_RGB; *value_p = (70 << 16) | (130 << 8) | 180; return True;
	}
	break;
    case 't':
	if (!strcasecmp("tan", s)) { 
	    *type_p = ST_RGB; *value_p = (210 << 16) | (180 << 8) | 140; return True;
	} else
	if (!strcasecmp("teal", s)) { 
	    *type_p = ST_RGB; *value_p = (0 << 16) | (128 << 8) | 128; return True;
	} else
	if (!strcasecmp("thistle", s)) { 
	    *type_p = ST_RGB; *value_p = (216 << 16) | (191 << 8) | 216; return True;
	} else
	if (!strcasecmp("tomato", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (99 << 8) | 71; return True;
	} else
	if (!strcasecmp("turquoise", s)) { 
	    *type_p = ST_RGB; *value_p = (64 << 16) | (224 << 8) | 208; return True;
	}
	break;
    case 'v':
	if (!strcasecmp("violet", s)) { 
	    *type_p = ST_RGB; *value_p = (238 << 16) | (130 << 8) | 238; return True;
	}
	break;
    case 'w':
	if (!strcasecmp("wheat", s)) { 
	    *type_p = ST_RGB; *value_p = (245 << 16) | (222 << 8) | 179; return True;
	} else
	if (!strcasecmp("white", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (255 << 8) | 255; return True;
	} else
	if (!strcasecmp("whitesmoke", s)) { 
	    *type_p = ST_RGB; *value_p = (245 << 16) | (245 << 8) | 245; return True;
	}
	break;
    case 'y':
	if (!strcasecmp("yellow", s)) { 
	    *type_p = ST_RGB; *value_p = (255 << 16) | (255 << 8) | 0; return True;
	} else
	if (!strcasecmp("yellowgreen", s)) { 
	    *type_p = ST_RGB; *value_p = (154 << 16) | (205 << 8) | 50; return True;
	}
	break;
    default:
	break;
    }
    return False;
    
}


BOOL StyleParseLength(char *s, BOOL allow_percent, StyleType *type_p, long *value_p)
{
    if (strstr(s,"px")) {
	*type_p = ST_PX;
	*value_p = (long)atoi(s);
	return True;
    } else if (strstr(s,"pt")) {
	*type_p = ST_PT;
	*value_p = (long)(atoi(s));
	return True;
    } else if (strstr(s,"em")) {
	*type_p = ST_EM;
	*value_p = (long)atoi(s);
	return True;
    } else if (allow_percent && (strstr(s,"%"))) {
	*type_p = ST_PERCENT;
	*value_p = (long)atoi(s);
	return True;
    }	
    return False;
    
}

BOOL StyleParseNumber(char *s, StyleType *type_p, long *value_p)
{
    double d = atof(s);

    *type_p = ST_FLOAT;
    *value_p = (long)(d*1000.0);
    return True;
}



StyleSelector *ParseSelector(char *str, HTList *selectors)
{
    char *p;
    char *elem_marker = NULL, *subelem_marker = NULL;
    StyleSelector *selector = NULL, *prev_selector = NULL;
    int token_class;
    int i;
    int element;
    char *class;
    
    p = str_tok(str," \t",&elem_marker);
    if(!p)
	return NULL;
    do {
	if(*p != '.')
	{
	    p = str_tok(p, ".",&subelem_marker);
	    element = element_enum(p, &token_class);
	    
	    if (element == UNKNOWN) {
		return NULL;
	    }
	    
	    class = str_tok(NULL," \t",&subelem_marker);
	    
	    selector = NewStyleSelector();
	    selector->ss.element = element;
	    if (class && *class)
		selector->ss.class = strdup(class);
	    selector->ancestor = prev_selector;
	    prev_selector = selector;
	}
	else
	{
	    subelem_marker = p+1;
	    class = str_tok(NULL," \t",&subelem_marker);
	    for(i=1; i<= TAG_FORM; i++)
	    {
		selector = NewStyleSelector();
		selector->ss.element = i;
		if (class && *class)
		    selector->ss.class = strdup(class);
		selector->ancestor = prev_selector;
		prev_selector = selector;
		HTList_addObject(selectors, (void *) selector);
	    }
	    return NULL;
	}
    } while ( (p = str_tok(NULL, "() \t",&elem_marker)) );
    
    return selector;
}


BG_Style *StyleGetBackground(void *value_p, char *str)
{
    BG_Style *bg_style;
    Image *image;
    char *tmp;
    int is_numeric;

    if (value_p) {
	bg_style = (BG_Style *) value_p;
    } else  { 
	bg_style = (BG_Style *) calloc(1, sizeof(BG_Style));
	bg_style->flag = S_BACKGROUND_X_REPEAT | S_BACKGROUND_Y_REPEAT;
	bg_style->x_pos = 50;
	bg_style->y_pos = 50;
    } 
    if(!strcmp(str,":"))
	return (bg_style);
    
    if (str[0] == '#') {
	if (strlen(str) == 4) {
	    bg_style->r = hex2byte(str[1]) * 17;
	    bg_style->g = hex2byte(str[2]) * 17;
	    bg_style->b = hex2byte(str[3]) * 17;
	    bg_style->flag |= S_BACKGROUND_COLOR;
	    /*		    fprintf(stderr,"%x %x %x -> %d\n", r, g, b, *value_p);*/
	} else if (strlen(str) == 7) {
	    bg_style->r = hex2byte(str[1]) * 16 + hex2byte(str[2]);
	    bg_style->g = hex2byte(str[3]) * 16 + hex2byte(str[4]);
	    bg_style->b = hex2byte(str[5]) * 16 + hex2byte(str[6]);
	    bg_style->flag |= S_BACKGROUND_COLOR;
	    /*		    fprintf(stderr,"%x %x %x -> %d\n", r, g, b, *value_p);*/
	}
    } 
    else 
	if (str[0] == '"')
	{
	    if ((image = GetImage(str+1, strlen(str)-2, CurrentDoc->pending_reload))) 
	    {
		bg_style->image = image;
		bg_style->flag |= S_BACKGROUND_IMAGE;
	    }
	}
	else
	    if(!strncmp(str, "url(", 4))
	    {
		if ((image = GetImage(str+4, strlen(str)-5, CurrentDoc->pending_reload))) 
		{
		    bg_style->image = image;
		    bg_style->flag |= S_BACKGROUND_IMAGE;
		}
	    }	
	    else 
	    {		/* try looking up the name */
		if (!strcasecmp(str,"fixed"))
		    bg_style->flag |= S_BACKGROUND_FIXED;
		else 
		{
		    if (!strcasecmp(str,"repeat-x"))
			bg_style->flag &= ~S_BACKGROUND_Y_REPEAT;
		    else 
		    {
			if (!strcasecmp(str,"repeat-y"))
			    bg_style->flag &= ~S_BACKGROUND_X_REPEAT;
			else 
			{
			    if (!strcasecmp(str,"no-repeat"))
				bg_style->flag &= ~(S_BACKGROUND_X_REPEAT|S_BACKGROUND_Y_REPEAT);
			    else
			    {
				is_numeric = TRUE;
				tmp = str;
				while(*tmp&&is_numeric)
				{
				    is_numeric = ((*tmp>='0') && (*tmp <= '9')) || (*tmp=='%');
				    tmp++;
				};
				if(is_numeric)
				{
				    if(bg_style->flag & S_BACKGROUND_ORIGIN)
					bg_style->y_pos = atoi(str);
				    else
				    {
					bg_style->flag |= S_BACKGROUND_ORIGIN;
					bg_style->x_pos = atoi(str);
					bg_style->y_pos = bg_style->x_pos;
				    }
				}
				else
				{
				    StyleType type;
				    long color;
				
				    ParseColorName(str, &type, &color);
				    bg_style->r = (color >> 16) & 0xFF;
				    bg_style->g = (color >>  8) & 0xFF;
				    bg_style->b = color & 0xFF;
				    bg_style->flag |= S_BACKGROUND_COLOR;
				}
			    }
			}
		    }
		}
	    }
    return (bg_style);
}



void StyleAddValue(HTList *values, StyleType type, long value, StyleInternalProperty iproperty) {
    StyleValue *sv_p = NewStyleValue();

    sv_p->type = type;
    sv_p->value = value;
    sv_p->iproperty = iproperty;
    HTList_addObject(values, (void *) sv_p);
}



/* ?? */

HTList * ParseAssignment(char *s, int *weight_p)
{
    StyleExternalProperty eproperty;
    char *str;
    char *elem_str, *value_str;
    char first_str_tok[8], second_str_tok[8];
    HTList *values = NULL;
    StyleType type;
    long value, valuesave;

    if (!s)
	return False;

    valuesave = 0;

    str = str_tok(s," \t:",&elem_str);
    if (!str)
	return False;
    
    eproperty = str2eproperty(str);

    if (eproperty == SE_UNKNOWN)
	return False;
    
    if (STYLE_TRACE & VERBOSE_TRACE)
	fprintf(stderr,"StyleChew: eproperty = %d\n",eproperty);
    

    /* get everything up to '!' */
    
    str = str_tok(NULL,"!",&elem_str);
    if (!str)
	return False;

    str = str_tok(str,",\t ",&value_str);
    
    values = HTList_new();
    while (str) {
	switch (eproperty) {
	case SE_FONT_SIZE:
	    if (strcasecmp(str,"xx-large") == 0) {
		StyleAddValue(values, ST_PX, (long) 21, SI_FONT_SIZE);
	    } else if (strcasecmp(str,"x-large") == 0) {
		StyleAddValue(values, ST_PX, (long) 17, SI_FONT_SIZE);
	    } else if (strcasecmp(str,"large") == 0) {
		StyleAddValue(values, ST_PX, (long)14, SI_FONT_SIZE);
	    } else if (strcasecmp(str,"medium") == 0) {
		StyleAddValue(values, ST_PX, (long)12, SI_FONT_SIZE);
	    } else if (strcasecmp(str,"small") == 0) {
		StyleAddValue(values, ST_PX, (long)10, SI_FONT_SIZE);
	    } else if (strcasecmp(str,"x-small") == 0) {
		StyleAddValue(values, ST_PX, (long)9, SI_FONT_SIZE);
	    } else if (strcasecmp(str,"xx-small") == 0) {
		StyleAddValue(values, ST_PX, (long)8, SI_FONT_SIZE);
	    } else {
		if (StyleParseLength(str, True, &type, &value))
		    StyleAddValue(values, type, value, SI_FONT_SIZE);
	    }
	    break;

	case SE_FONT_FAMILY:
	    StyleAddValue(values, ST_STR, (long)strdup(str), SI_FONT_FAMILY);
	    break;

	case SE_FONT_WEIGHT:

	    if (strcasecmp(str,"extra-light") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_WEIGHT_EXTRA_LIGHT, SI_FONT_WEIGHT);
	    }
	    else if (strcasecmp(str,"light") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_WEIGHT_LIGHT, SI_FONT_WEIGHT);
	    }
	    else if (strcasecmp(str,"medium") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_WEIGHT_MEDIUM, SI_FONT_WEIGHT);
	    }
	    else if (strcasecmp(str,"demi-bold") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_WEIGHT_DEMI_BOLD, SI_FONT_WEIGHT);
	    }
	    else if (strcasecmp(str,"bold") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_WEIGHT_BOLD, SI_FONT_WEIGHT);
            }
	    else if (strcasecmp(str,"extra-bold") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_WEIGHT_EXTRA_BOLD, SI_FONT_WEIGHT);
	    } else
		if (StyleParseNumber(str, &type, &value))
		    StyleAddValue(values, type, value, SI_FONT_WEIGHT);
	    break;

	case SE_FONT_STYLE:
	    
	    if (strncasecmp(str,"italic", 6) == 0) { /* italics */
		StyleAddValue(values, ST_FLAG, SV_FONT_STYLE_ITALIC, SI_FONT_STYLE_SLANT);
	    } else if (strcasecmp(str,"roman") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_STYLE_ROMAN, SI_FONT_STYLE_SLANT);
	    } else if (strcasecmp(str,"small-caps") == 0) {
		StyleAddValue(values, ST_FLAG, SV_FONT_STYLE_SMALL_CAPS, SI_FONT_STYLE_SMALL_CAPS);
	    }
	    break;

	case SE_LINE_HEIGHT:
	    if (StyleParseLength(str, True, &type, &value))
		StyleAddValue(values, type, value, SI_LINE_HEIGHT);
	    break;

	case SE_FONT:
	    break;

	case SE_COLOR:
	    if (str[0] == '#') {
		Byte r, g, b;

		if (strlen(str) == 4) {
		    r = hex2byte(str[1]) * 17;
		    g = hex2byte(str[2]) * 17;
		    b = hex2byte(str[3]) * 17;
		    StyleAddValue(values, ST_RGB, (long)((r<<16) | (g<<8) | b), SI_COLOR);
/*		    fprbytef(stderr,"%x %x %x -> %d\n", r, g, b, *value_p);*/
		} else if (strlen(str) == 7) {
		    r = hex2byte(str[1]) * 16 + hex2byte(str[2]);
		    g = hex2byte(str[3]) * 16 + hex2byte(str[4]);
		    b = hex2byte(str[5]) * 16 + hex2byte(str[6]);
		    StyleAddValue(values, ST_RGB, (long)((r<<16) | (g<<8) | b), SI_COLOR);
/*		    fprintf(stderr,"%x %x %x -> %d\n", r, g, b, *value_p);*/
		}
	    } else {		/* try looking up the name */
		if (ParseColorName(str, &type, &value))
		    StyleAddValue(values, type, value, SI_COLOR);
	    }
	    break;	    

	case SE_BACKGROUND:
	    value = (long) StyleGetBackground((void *)valuesave, str);
	    valuesave = value;
	    break;

	case SE_BG_BLEND_DIRECTION:
	    break;
	case SE_BG_STYLE:
	    break;
	case SE_BG_POSITION:
	    break;

	case SE_WORD_SPACING:
	case SE_LETTER_SPACING:
	    if (!strcasecmp("normal", str)) {
		StyleAddValue(values, ST_FLAG, S_NORMAL, SI_LETTER_SPACING);
	    } else 
		if (StyleParseLength(str, False, &type, &value))
		    StyleAddValue(values, type, value, SI_LETTER_SPACING);
	    break;

	case SE_TEXT_DECORATION:

	    if (! strcasecmp(str,"underline")) {
		StyleAddValue(values, ST_FLAG, SV_TEXT_DECORATION_UNDERLINE, SI_TEXT_DECORATION);
	    } else if (! strcasecmp(str,"overline")) {
		StyleAddValue(values, ST_FLAG, SV_TEXT_DECORATION_OVERLINE, SI_TEXT_DECORATION);
	    } else if (! strcasecmp(str,"line-through")) {
		StyleAddValue(values, ST_FLAG, SV_TEXT_DECORATION_LINE_THROUGH, SI_TEXT_DECORATION);
	    } else if (! strcasecmp(str,"blink")) {
		StyleAddValue(values, ST_FLAG, SV_TEXT_DECORATION_BLINK, SI_TEXT_DECORATION);
	    }
	    break;

	case SE_VERTICAL_ALIGN:
	    break;
	case SE_TEXT_TRANSFORM:
	    break;
	case SE_TEXT_ALIGN:
	    if (strcasecmp(str,"left") == 0) {	
		StyleAddValue(values, ST_FLAG, ALIGN_LEFT, SI_TEXT_ALIGN);
	    } else if (strcasecmp(str,"center") == 0) {	
		StyleAddValue(values, ST_FLAG, ALIGN_CENTER, SI_TEXT_ALIGN);
	    } else if (strcasecmp(str,"right") == 0) {	
		StyleAddValue(values, ST_FLAG, ALIGN_RIGHT, SI_TEXT_ALIGN);
	    } else if (strcasecmp(str,"justify") == 0) {
		StyleAddValue(values, ST_FLAG, ALIGN_JUSTIFY, SI_TEXT_ALIGN);
	    }
	    break;
	case SE_TEXT_INDENT:
	    if (StyleParseLength(str, True, &type, &value))
		StyleAddValue(values, type, value, SI_TEXT_INDENT);
	    break;

	case SE_PADDING:
	    if (StyleParseLength(str, True, &type, &value)) {
		StyleAddValue(values, type, value, SI_PADDING_TOP);
		StyleAddValue(values, type, value, SI_PADDING_RIGHT);
		StyleAddValue(values, type, value, SI_PADDING_BOTTOM);
		StyleAddValue(values, type, value, SI_PADDING_LEFT);
	    }
	    break;

	case SE_MARGIN_TOP:
	    if (! strcasecmp(str,"auto")) {	
		StyleAddValue(values, ST_FLAG, SV_MARGIN_AUTO, SI_MARGIN_TOP);
	    } else
		if (StyleParseLength(str, True, &type, &value))
		    StyleAddValue(values, type, value, SI_MARGIN_TOP);
	    break;
	case SE_MARGIN_RIGHT:
	    if (! strcasecmp(str,"auto")) {	
		StyleAddValue(values, ST_FLAG, SV_MARGIN_AUTO, SI_MARGIN_RIGHT);
	    } else
		if (StyleParseLength(str, True, &type, &value))
		    StyleAddValue(values, type, value, SI_MARGIN_RIGHT);
	    break;
	case SE_MARGIN_BOTTOM:
	    if (! strcasecmp(str,"auto")) {	
		StyleAddValue(values, ST_FLAG, SV_MARGIN_AUTO, SI_MARGIN_BOTTOM);
	    } else
		if (StyleParseLength(str, True, &type, &value))
		    StyleAddValue(values, type, value, SI_MARGIN_BOTTOM);
	    break;
	case SE_MARGIN_LEFT:
	    if (! strcasecmp(str,"auto")) {	
		StyleAddValue(values, ST_FLAG, SV_MARGIN_AUTO, SI_MARGIN_LEFT);
	    } else
		if (StyleParseLength(str, True, &type, &value))
		    StyleAddValue(values, type, value, SI_MARGIN_LEFT);
	    break;

	case SE_MARGIN:
	    break;
	case SE_WIDTH:
	    break;
	case SE_HEIGHT:
	    break;
	case SE_FLOAT:
	    break;
	case SE_CLEAR:
	    break;
	case SE_PACK:
	    break;

	    /* border style */

	case SE_LIST_STYLE:
	    break;
	case SE_MAGNIFICATION:
	    break;
	case SE_WHITE_SPACE:
	    break;
 
	default:
	    if (STYLE_TRACE)
		fprintf(stderr,"Uncnown property enum: %d\n", eproperty);
	    break;
	}


	str = str_tok(NULL,",\t ",&value_str);
    }
    /*  level_declaration: */

    if(eproperty == SE_BACKGROUND)
	StyleAddValue(values, ST_BGSTYLE, value, SI_BACKGROUND);
    
    *weight_p = SW_NORMAL;

    str = str_tok(NULL,"! \t;",&elem_str);
    if (str) {
	if (strcasecmp(str,"important") == 0) {
	    *weight_p = SW_IMPORTANT;
	}
	else if (strcasecmp(str,"legal") == 0) {
	    *weight_p = SW_LEGAL;
	}
    }

    return values;
}


/*
  StyleChew is the main entry point to the CSS parser. 

  *sheet is a pointer to a StyleSheet structure into which the
  assignments will be added

  *str is a pointer to a string containing CSS syntax. Th pointer wil
  not be freed,a dn the content will remain unchanged.

  base_weight identifies the source of the style sheet, author or
  reader

*/




void StyleChew(StyleSheet *sheet, char *str, Byte origin)
{
    char *sheet_marker, *address_marker, *assignment_marker, *rule_marker;
    char *p, *q, *r, *s;
    StyleExternalProperty eproperty;
    StyleSelector *sel = NULL, *sc = NULL;
    int level;
    int weight;
    long value;
    void *bg_save;
    int bg_weight=0;
    HTList *l, *selectors, *values;
    StyleSelector *selector;
    char begin_char,end_char,set_char,*tmp_char; /* used for backward compatibility --Spif 23-Nov-95 */
    char delim_char;
    char str_tok_begin[2], str_tok_set[2], str_tok_end[2];

    address_marker = NULL;
    sheet_marker = NULL;
    assignment_marker = NULL;
    rule_marker = NULL;

    if (STYLE_TRACE & VERBOSE_TRACE)
	fprintf(stderr,"\nStyleChew\n");

    /* h1, p: font-family = helvetica times, font-size = 12pt; */

    if (!str)
	return;


    begin_char = '{';
    set_char   = ':';
    delim_char = ';';
    end_char   = '}';

    *str_tok_end = end_char;
    *str_tok_begin = begin_char;
    *str_tok_set = set_char;
    str_tok_begin[1] = 0;
    str_tok_set[1] = 0;
    str_tok_end[1] = 0;

    s = strdup(str);  /* we will modify the string so we make a copy */
    
    for(tmp_char = s; *tmp_char ;tmp_char++) {
	if(*tmp_char=='\n')
	    *tmp_char = ' ';
    }

    p = str_tok(s,"}",&sheet_marker);
    if (!p)
	return;

    do {
	p = str_tok(p, "{", &rule_marker);

	if (STYLE_TRACE)
	    fprintf(stderr,"(address) %s -> ", p);
	
	selectors = HTList_new();

	p = str_tok(p, ",", &address_marker);
	do {
	    if ((sel = ParseSelector(p, selectors))) {
		if (STYLE_TRACE) {
		    if(sc)
			do {
			    fprintf(stderr,"%d-",sc->ss.element);
			} while((sc = sc->ancestor));
		}

		HTList_addObject(selectors, (void *) sel);
	    }
		
	} while ((p = str_tok(NULL,",",&address_marker)));

	/* now we know what elements we are addressing */

	p = str_tok(NULL, ";}", &rule_marker);
	bg_save = NULL;
	do {
	    if (STYLE_TRACE)
		if(p)
		    fprintf(stderr,"(assignment) %s ", p);
	    value = (long)bg_save;  /* this hack is only for keeping bg_style during the parsing of one element --Spif 23-Nov-95 */

	    if (values = ParseAssignment(p, &weight)) {

#if 0
		if(property == S_BACKGROUND)
		{
		    bg_save = (void *)value;
		    bg_weight = weight;
		}
		else
#endif
		{
		    l = selectors;
		    while ((selector = (StyleSelector *)HTList_nextObject(l)) ) 
		    {
			if (STYLE_TRACE) 
			{
			    StyleSelector *sc = selector;
			    if(sc)
			    {
				fprintf(stderr,"\nnext selector ");
				do 
				{
				    fprintf(stderr,"%d-",sc->ss.element);
				} while((sc = sc->ancestor));
				fprintf(stderr,"\n");
			    }
			}
			StyleAddRule(sheet, selector, values, origin, weight);
		    }
		}
	    }
	} while ( (p = str_tok(NULL,";}",&rule_marker)) ); 

#if 0

	if(bg_save)
	{
	    l = selectors;
	    while ((selector = (StyleSelector *)HTList_nextObject(l)) ) 
	    {
		if (STYLE_TRACE) 
		{
		    StyleSelector *sc = selector;
		    fprintf(stderr,"\nnext selector ");
		    do 
		    {
			fprintf(stderr,"%d-",sc->ss.element);
		    } while((sc = sc->ancestor));
		    fprintf(stderr,"\n");
		}
		StyleAddRule(sheet, selector, S_BACKGROUND,(long)bg_save, bg_weight);
	    }
	}
#endif

	if (STYLE_TRACE)
	    fprintf(stderr,"\n");

/*	
	l = selectors;
	while ( (selector = (StyleSelector *)HTList_nextObject(l)) ) {
	    if (STYLE_TRACE && VERBOSE_TRACE)
		fprintf(stderr,"freeing selector\n");
	    FreeStyleSelector(selector);
	}
	HTList_delete(selectors);
*/
    } while ( (p = str_tok(NULL, "}", &sheet_marker)) ); 

    Free(s);
    StyleSane(sheet);
}






/*
  StyleGetInit returns an initialized style sheet, typically the application's default 
*/

StyleSheet *StyleGetInit()
{
    StyleSheet *sheet = NewStyleSheet();

    /* set toplevel fallback values */
    StyleAddSimpleRule(sheet, TAG_HTML, SI_FONT_FAMILY,	 ST_STR,    S_FALLBACK, (long)"helvetica");
    StyleAddSimpleRule(sheet, TAG_HTML, SI_FONT_SIZE, 	 ST_PX,     S_FALLBACK, (long) 12);
    StyleAddSimpleRule(sheet, TAG_HTML, SI_FONT_WEIGHT,  ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_MEDIUM);
    StyleAddSimpleRule(sheet, TAG_HTML, SI_FONT_STYLE_SLANT, ST_FLAG,   S_FALLBACK, SV_FONT_STYLE_NORMAL);
    StyleAddSimpleRule(sheet, TAG_HTML, SI_FONT_STYLE_SMALL_CAPS, ST_FLAG,   S_FALLBACK, SV_FONT_STYLE_NORMAL);

    StyleAddSimpleRule(sheet, TAG_HTML, SI_COLOR,        ST_RGB,    S_FALLBACK, (long) (100 << 0));  /* i.e. a dark blue */
    
    StyleAddSimpleRule(sheet, TAG_HTML, SI_MARGIN_LEFT,  ST_PX,     S_FALLBACK, 4);
    StyleAddSimpleRule(sheet, TAG_HTML, SI_MARGIN_RIGHT, ST_PX,     S_FALLBACK, 4);
    StyleAddSimpleRule(sheet, TAG_HTML, SI_MARGIN_TOP,   ST_PX,     S_FALLBACK, 4);

    StyleAddSimpleRule(sheet, TAG_HTML, SI_TEXT_ALIGN,   ST_FLAG,   S_FALLBACK, ALIGN_LEFT);

    /* set per-tag fallback values -- exception to the normal fallback values */

    StyleAddSimpleRule(sheet, TAG_H1, SI_FONT_FAMILY,    ST_STR,    S_FALLBACK, (long)"charter"); 
    StyleAddSimpleRule(sheet, TAG_H1, SI_FONT_SIZE,      ST_PX,	    S_FALLBACK, 24);
    StyleAddSimpleRule(sheet, TAG_H1, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_H1, SI_MARGIN_LEFT,    ST_PX,     S_FALLBACK, 5);

    StyleAddSimpleRule(sheet, TAG_H2, SI_FONT_FAMILY,    ST_STR,    S_FALLBACK, (long)"charter"); 
    StyleAddSimpleRule(sheet, TAG_H2, SI_FONT_SIZE,      ST_PX,     S_FALLBACK, 18);
    StyleAddSimpleRule(sheet, TAG_H2, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_H2, SI_MARGIN_LEFT,    ST_PX,     S_FALLBACK, 5);
    
    StyleAddSimpleRule(sheet, TAG_H3, SI_FONT_FAMILY,    ST_STR,    S_FALLBACK, (long) "times"); 
    StyleAddSimpleRule(sheet, TAG_H3, SI_FONT_SIZE,      ST_PX,	    S_FALLBACK, 14);
    StyleAddSimpleRule(sheet, TAG_H2, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK,	SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_H3, SI_MARGIN_LEFT,    ST_PX,     S_FALLBACK, 5);

    StyleAddSimpleRule(sheet, TAG_H4, SI_FONT_FAMILY,    ST_STR,    S_FALLBACK, (long)"charter");
    StyleAddSimpleRule(sheet, TAG_H4, SI_FONT_SIZE,      ST_FLAG,   S_FALLBACK, 12);
    StyleAddSimpleRule(sheet, TAG_H4, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_H4, SI_MARGIN_LEFT,    ST_PX,	    S_FALLBACK, 5);

    StyleAddSimpleRule(sheet, TAG_H5, SI_FONT_FAMILY,    ST_STR,    S_FALLBACK, (long) "charter");
    StyleAddSimpleRule(sheet, TAG_H5, SI_FONT_SIZE,      ST_PX,     S_FALLBACK, 12);
    StyleAddSimpleRule(sheet, TAG_H5, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_H5, SI_MARGIN_LEFT,    ST_PX,     S_FALLBACK, 5);

    StyleAddSimpleRule(sheet, TAG_H6, SI_FONT_FAMILY,    ST_STR,    S_FALLBACK, (long)"charter");
    StyleAddSimpleRule(sheet, TAG_H6, SI_FONT_SIZE,      ST_PX,     S_FALLBACK, 12);
    StyleAddSimpleRule(sheet, TAG_H6, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_H6, SI_MARGIN_LEFT,    ST_FLAG,   S_FALLBACK, 5);

    /*    StyleAddSimpleRule(sheet, TAG_LI, S_MARGIN_LEFT, 	15, 			S_FALLBACK);*/

    StyleAddSimpleRule(sheet, TAG_DL, SI_MARGIN_LEFT,    ST_PX,     S_FALLBACK, 15);
    StyleAddSimpleRule(sheet, TAG_UL, SI_MARGIN_LEFT,    ST_PX,     S_FALLBACK, 15);
    StyleAddSimpleRule(sheet, TAG_OL, SI_MARGIN_LEFT,    ST_PX,	    S_FALLBACK, 15);

    StyleAddSimpleRule(sheet, TAG_DT, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_DD, SI_FONT_WEIGHT,    ST_FLAG,   S_FALLBACK, SV_FONT_WEIGHT_MEDIUM);
 
    StyleAddSimpleRule(sheet, TAG_ADDRESS, SI_TEXT_ALIGN, ST_FLAG,  S_FALLBACK, ALIGN_RIGHT);

    StyleAddSimpleRule(sheet, TAG_MATH, SI_FONT_SIZE,   ST_PX,      S_FALLBACK, 14);
    StyleAddSimpleRule(sheet, TAG_SMALL, SI_FONT_SIZE,  ST_PX,      S_FALLBACK, 8);
    StyleAddSimpleRule(sheet, TAG_SUB, 	SI_FONT_SIZE,   ST_PX,      S_FALLBACK, 8);
    StyleAddSimpleRule(sheet, TAG_SUP, 	SI_FONT_SIZE,   ST_PX,      S_FALLBACK, 8);

    StyleAddSimpleRule(sheet, TAG_STRONG, SI_FONT_WEIGHT,      ST_FLAG,  S_FALLBACK, SV_FONT_WEIGHT_BOLD);
    StyleAddSimpleRule(sheet, TAG_BOLD, SI_FONT_WEIGHT,        ST_FLAG,  S_FALLBACK, SV_FONT_WEIGHT_BOLD);

    StyleAddSimpleRule(sheet, TAG_EM, SI_FONT_STYLE_SLANT,     ST_FLAG,    S_FALLBACK, SV_FONT_STYLE_ITALIC);
    StyleAddSimpleRule(sheet, TAG_ITALIC, SI_FONT_STYLE_SLANT, ST_FLAG,    S_FALLBACK, SV_FONT_STYLE_ITALIC);

    StyleAddSimpleRule(sheet, TAG_PRE, SI_FONT_FAMILY,  ST_STR,    S_FALLBACK, (long)"courier");
    StyleAddSimpleRule(sheet, TAG_TT, SI_FONT_FAMILY,   ST_STR,    S_FALLBACK, (long)"courier");
    StyleAddSimpleRule(sheet, TAG_CODE, SI_FONT_FAMILY,	ST_STR,    S_FALLBACK, (long)"courier");

    /* set default style for TAG_ABSTRACT */

    StyleAddSimpleRule(sheet, TAG_ABSTRACT, SI_FONT_STYLE_SLANT, ST_FLAG,  S_FALLBACK, SV_FONT_STYLE_ITALIC);
    StyleAddSimpleRule(sheet, TAG_ABSTRACT, SI_FONT_WEIGHT, ST_FLAG, S_FALLBACK, SV_FONT_WEIGHT_BOLD);

    StyleAddSimpleRule(sheet, TAG_ABSTRACT, SI_MARGIN_RIGHT, ST_PX,  S_FALLBACK, 40);
    StyleAddSimpleRule(sheet, TAG_ABSTRACT, SI_MARGIN_LEFT, ST_PX,   S_FALLBACK, 40);

    /* set default style for TAG_QUOTE */

    StyleAddSimpleRule(sheet, TAG_QUOTE, SI_FONT_STYLE_SLANT, ST_FLAG,     S_FALLBACK, SV_FONT_STYLE_ITALIC);
    StyleAddSimpleRule(sheet, TAG_QUOTE, SI_MARGIN_RIGHT, ST_PX,     S_FALLBACK, 30);
    StyleAddSimpleRule(sheet, TAG_QUOTE, SI_MARGIN_LEFT, ST_PX,      S_FALLBACK, 50);

    /* set style for tables */

    StyleAddSimpleRule(sheet, TAG_TH, 	SI_FONT_WEIGHT, ST_FLAG,     S_FALLBACK, SV_FONT_WEIGHT_BOLD);
/*
    StyleAddSimpleRule(sheet, TAG_TH, 	SI_TEXT_ALIGN,  ST_FLAG,     S_FALLBACK, ALIGN_CENTER);
    StyleAddSimpleRule(sheet, TAG_TD, 	SI_TEXT_ALIGN,  ST_FLAG,     S_FALLBACK, ALIGN_CENTER);
*/
    /* set style for HR */

    StyleAddSimpleRule(sheet, TAG_HR, 	SI_COLOR, ST_RGB,            S_FALLBACK, (float) (255 << 24));

    return sheet;
}

void StyleParse()
{
    if (STYLE_TRACE)
	fprintf(stderr,"StyleParse\n");

    if (!CurrentDoc->head_style && !CurrentDoc->link_style) {
	CurrentDoc->style = NULL; /* i.e. style->default */
	return;
    }

    if (CurrentDoc->user_style) {

	/* this document contains the user's style sheet if it exists */

	Announce("applying personal style sheet.. ");

/*	rgbClear();*/

	if (context->style) {
	    FreeStyleSheet(context->style);
	    context->style = StyleGetInit();
	}

	StyleChew(context->style, CurrentDoc->head_style, S_READER);
	StyleChew(context->style, CurrentDoc->link_style, S_READER);
	CurrentDoc->style = context->style;
	if (STYLE_TRACE) {
	    fprintf(stderr,"Stylesane: context->style\n");
	    StyleSane(context->style);
	}
	return;
    }

    /* this is a normal incoming document with style sheet attached */

    Announce("applying document's style sheet.. ");

    rgbClear();
    CurrentDoc->style = StyleCopy(context->style);
    StyleChew(CurrentDoc->style, CurrentDoc->head_style, S_READER);
    StyleChew(CurrentDoc->style, CurrentDoc->link_style, S_READER);
    if (STYLE_TRACE) {
	fprintf(stderr,"Stylesane: CurrentDoc->style\n");
	StyleSane(context->style);
    }

    Announce("applying document's style sheet.. done");
}


/*
void StyleWindowChange()
{
    fprintf(stderr,"StyleWindowChange w %d h %d\n", win_width, win_height);
}
*/

/* StyleZoom is a bit too Arena-specific */

void StyleZoomChange(double f)
{
    Announce("loading fonts..");
    ShowBusy();

    lens_factor *= f;
    DisplaySizeChanged(1);
    Redraw(0,0,WinWidth,WinHeight + WinTop); 
    Announce("%s",CurrentDoc->url);
    HideBusy();
}


char *StyleLoad(char *href, int hreflen, BOOL reload)
{
    Doc *doc = NULL;

    /* check for null name */

    doc = GetInline(href, hreflen, reload);
    if (doc) {
/*	fprintf(stderr,"StyleLoad succedded ??\n");*/
	return(doc->content_buffer);
    }
    return NULL;
}


void StyleClearDoc()
{
    HTArray_clear(style_stack);
    style_stack = NULL;
    current_flat = NULL;
}



/* formatting functions, here starts what should become format.c */

#if 0
StyleStackElement *NewStackElement()
{
    return (StyleStackElement *)calloc(1,sizeof(StyleStackElement));
}
#endif

/* 
  FormatElementStart is called when a new element (implied or not) is
  started. It will call StyleEval to flatten a set of properties (called
  a stack element) to be returned by StyleGet. The stack element is then 
  pushed onto the stack.
*/

/**/
void FormatElementStart(int element, char *class, int class_len)
{
    Frame *new_frame, *old_frame;
    
    StyleSimpleSelector *stack_el = NewStyleSimpleSelector();
    StyleSheet *sheet = (CurrentDoc->style ? CurrentDoc->style : context->style);

    if ((element < 0) || (element >= TAG_LAST)) {
      fprintf(stderr,"FormatElementStart: element = %d\n",element);
      element = TAG_P; /* ugly */
    }

    current_sheet = sheet;

    if (REGISTER_TRACE)
	fprintf(stderr,"\nFormatElementStart: %d\n",element);

    if (!style_stack)
	style_stack = HTArray_new(20);

    stack_el->element = element;
    if (class)
	stack_el->class = www_strndup(class, class_len);

    HTArray_addObject(style_stack, (void *)stack_el); 

    stack_el->flat = StyleEval(sheet, style_stack);

#if 0
    {
	void **data = NULL;
	StyleSimpleSelector *ss = (StyleSimpleSelector *) HTArray_firstObject(style_stack, data);

	fprintf(stderr, "-> FormatElementStart %d\n",element);
	while (ss) {
	    fprintf(stderr, "--> FormatElementStart %d\n",ss->element);
	    ss = HTArray_nextObject(style_stack, data);
	}
    }
#endif
		
    current_flat = stack_el->flat;
    StyleSetFlag(S_MARGIN_TOP_FLAG, (int)StyleGet(SI_MARGIN_TOP));
    if(element == TAG_P)
    { 
	StyleSetFlag(S_INDENT_FLAG, (int)StyleGet(SI_TEXT_INDENT));
	StyleSetFlag(S_LEADING_FLAG, TRUE);
    };

    if(element == TAG_HTML || element == TAG_HTML_SOURCE || 
	(/*element != TAG_TABLE && */
	 element != TAG_ABSTRACT &&
	 element != TAG_BLOCKQUOTE && element != TAG_CAPTION &&
	 element != TAG_NOTE && element != TAG_PRE &&
	 element != TAG_QUOTE))  
	return;
    if(prepass||damn_table)
	return;
    old_frame = (Frame *)Pop();
    Push(old_frame);
    if(paintStartLine>0)
	Push((Frame *)1);
    else
    {
	new_frame = (Frame *)calloc(1, sizeof(Frame));
	PixOffset += StyleGet(SI_PADDING_TOP)+ StyleGet(SI_MARGIN_TOP); /* must be S_PADDING_TOP */
	new_frame->offset = PixOffset;
	new_frame->leftmargin = StyleGet(SI_PADDING_LEFT);
	new_frame->rightmargin = StyleGet(SI_PADDING_RIGHT);
	new_frame->indent = old_frame->indent + StyleGet(SI_MARGIN_LEFT);
	new_frame->width = old_frame->width - StyleGet(SI_MARGIN_LEFT) - StyleGet(SI_MARGIN_RIGHT);
	new_frame->style = 0;
	new_frame->border = 0;
#ifdef STYLE_COLOR_BORDER
	new_frame->cb_ix = 0;
#else
	new_frame->cb_ix = 0;
#endif
	new_frame->flow = old_frame->flow; /* StyleGet(S_ALIGN) */
	new_frame->next = new_frame->child = NULL;
	new_frame->box_list = NULL;
	PrintBeginFrame(new_frame);
	Push(new_frame);
    }
#if 0
    new_frame->offset = PixOffset;
    new_frame->indent = (old_frame ?  old_frame->leftmargin + StyleGet(S_MARGIN_LEFT) : StyleGet(S_MARGIN_LEFT));
    new_frame->width = (old_frame ?  old_frame->width - StyleGet(S_MARGIN_LEFT) - StyleGet(S_MARGIN_RIGHT) : 12);
    new_frame->style = 0;
    new_frame->border = 0;
#ifdef STYLE_COLOR_BORDER
    new_frame->cb_ix = 0;
#else
    new_frame->cb_ix = 0;
#endif
    new_frame->flow = ALIGN_CENTER;
    new_frame->next = new_frame->child = NULL;
    new_frame->box_list = NULL;
    new_frame->leftcount = old_frame->leftcount;
    new_frame->pushcount = old_frame->pushcount;
    new_frame->oldmargin = old_frame->oldmargin;
    PrintBeginFrame(new_frame);
    Push(new_frame);
#endif
}

/*
  FormatElementEnd pops a set of flattened style properties from the
  stack. Note that calls to FormatElementStart and FormatElementEnd must
  always match each other. I.e. your SGML/HTML parser must add any
  implied/omitted tags.
*/

void FormatElementEnd()
{
    Frame *old_frame;
    int element;

    StyleSimpleSelector *stack_el;

#if 0
    {
	void **data = NULL;
	StyleSimpleSelector *ss = (StyleSimpleSelector *) HTArray_firstObject(style_stack, data);

	fprintf(stderr, "-> FormatElementEnd \n");
	while (ss) {
	    fprintf(stderr, "--> FormatElementEnd %d\n",ss->element);
	    ss = HTArray_nextObject(style_stack, data);
	}
    }
#endif

    if (style_stack->size > 0) {
	stack_el = (StyleSimpleSelector *) style_stack->data[style_stack->size - 1];
	element = stack_el->element;
    
	Free(stack_el->flat);
	Free(stack_el->class);
	Free(stack_el);
	style_stack->data[style_stack->size - 1] = NULL;
	--style_stack->size;
    }

    if (style_stack->size > 0) {
	stack_el = (StyleSimpleSelector *) style_stack->data[style_stack->size - 1];
	current_flat = stack_el->flat;
    } else {
	if (REGISTER_TRACE)
	    fprintf(stderr,"stack empty!! \n");
	current_flat = NULL;
    }

    if(element == TAG_HTML || element == TAG_HTML_SOURCE || 
	(/*element != TAG_TABLE && */
        element != TAG_ABSTRACT &&
	element != TAG_BLOCKQUOTE && element != TAG_CAPTION &&
	element != TAG_NOTE && element != TAG_PRE &&
	element != TAG_QUOTE))
	return;
    if(prepass||damn_table) 
	return;
    old_frame = (Frame *)Pop();
    if(old_frame)
    {
	Frame *parent_frame;
	char *p;
#define PushValue(p, value) ui_n = (unsigned int)value; *p++ = ui_n & 0xFF; *p++ = (ui_n >> 8) & 0xFF

	if(old_frame != (Frame *)1)
	{
	    parent_frame = (Frame *)Pop();
	    if(parent_frame)
		Push(parent_frame);
	    old_frame->length = paintlen - old_frame->info - FRAMESTLEN;
	    old_frame->height = PixOffset - old_frame->offset + 2*StyleGet(SI_PADDING_TOP); /* ???? */
	    old_frame->offset -= StyleGet(SI_PADDING_TOP);  /* ????? */
	    PixOffset += StyleGet(SI_PADDING_TOP);               /* ????? */
	    p = paint + old_frame->info + 9;
	    PushValue(p, old_frame->height & 0xFFFF);
	    PushValue(p, (old_frame->height >> 16) & 0xFFFF);
	    PrintFrameLength(old_frame);
	    if(parent_frame == (Frame *)1)  
		PrintEndFrame(NULL, old_frame);
	    else
		PrintEndFrame(parent_frame, old_frame);
	    FreeFrames(old_frame); 
	};
    }; 
}






















#if 0

    /* what are the compound properties? i.e. properties that depend
       on other properties? is font the only one? */ 

    if (recompute_font) {
	flat->value[S_FONT] = (void *)GetFont((HTList *)flat->value[S_FONT_FAMILY], (int)flat->value[S_FONT_SIZE], 
				      (int)flat->value[S_FONT_WEIGHT], (int)flat->value[S_FONT_STYLE], False);
	flat->status[S_FONT] = S_INFERRED;
    }

    if (recompute_alt_font) {
	flat->value[S_ALT_FONT] = (void *)
	    GetFont(
		    (HTList *)((flat->status[S_ALT_FONT_FAMILY] == S_INFERRED) ? 
			       (int)flat->value[S_ALT_FONT_FAMILY] : (int)flat->value[S_FONT_FAMILY]),
		    ((flat->status[S_ALT_FONT_SIZE] == S_INFERRED) ? 
		     (int)flat->value[S_ALT_FONT_SIZE] : (int)flat->value[S_FONT_SIZE]),
		    ((flat->status[S_ALT_FONT_WEIGHT] == S_INFERRED) ? 
		     (int)flat->value[S_ALT_FONT_WEIGHT] : (int)flat->value[S_FONT_WEIGHT]),
		    ((flat->status[S_ALT_FONT_STYLE] == S_INFERRED) ? 
		     (int)flat->value[S_ALT_FONT_STYLE] : (int)flat->value[S_FONT_STYLE]),
		    False);
	flat->status[S_ALT_FONT] = S_INFERRED;
    }


    /* we cannot compute the colormap index until we know if the color is blinking or not */

    if (flat->status[S_COLOR] == S_INFERRED) {

	if (flat->status[S_ALT_COLOR] != S_INFERRED) {
	    flat->status[S_ALT_COLOR] = S_INFERRED;
	    flat->value[S_ALT_COLOR] = flat->value[S_COLOR];
	    flat->weight[S_ALT_COLOR] = flat->weight[S_COLOR];
	}

	value = (int)flat->value[S_COLOR];
	flat->value[S_COLOR] = (void *)rgb2ix( (int)(value >> 24), (int)((value >> 16) & 0xFF), 
					    (int)((value >> 8) & 0xFF), (int)(value & 0xFF), False);
	if (REGISTER_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"--> TEXT_COLOR = %ld\n",(int)flat->value[S_COLOR]);
    }

    if (flat->status[S_BACKGROUND] == S_INFERRED) {

	if (flat->status[S_ALT_BACKGROUND] != S_INFERRED) {
	    flat->status[S_ALT_BACKGROUND] = S_INFERRED;
	    flat->value[S_ALT_BACKGROUND] = flat->value[S_BACKGROUND];
	    flat->weight[S_ALT_BACKGROUND] = flat->weight[S_BACKGROUND];
	}
/*
	value = flat->value[S_BACKGROUND];
	flat->value[S_BACKGROUND] = rgb2ix( (int)(value >> 24), (int)((value >> 16) & 0xFF), 
						 (int)((value >> 8) & 0xFF), (int)(value & 0xFF), False);
	if (REGISTER_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"-> S_BACKGROUND = %ld\n",flat->value[S_BACKGROUND]);
*/
    }


    if (flat->status[S_ALT_COLOR] == S_INFERRED) {
	value = (int)flat->value[S_ALT_COLOR];
	flat->value[S_ALT_COLOR] = (void *)rgb2ix( (int)(value >> 24), (int)((value >> 16) & 0xFF), 
						(int)((value >> 8) & 0xFF), (int)(value & 0xFF), False);
	if (REGISTER_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"--> TEXT_COLOR = %ld\n",(int)flat->value[S_ALT_COLOR]);
    }

    if (flat->status[S_ALT_BACKGROUND] == S_INFERRED) {
	value = (int)flat->value[S_ALT_BACKGROUND];
/*
	flat->value[S_ALT_BACKGROUND] = rgb2ix( (int)(value >> 24), (int)((value >> 16) & 0xFF), 
						     (int)((value >> 8) & 0xFF), (int)(value & 0xFF), False);
	if (REGISTER_TRACE && VERBOSE_TRACE)
	    fprintf(stderr,"-> TEXT_BACKGROUND = %ld\n", flat->value[S_ALT_BACKGROUND]);
*/
    }

    /* math is a bit awkward */

    if (unit_p->element == TAG_MATH) {
	
	long sym_size = (long)flat->value[S_FONT_SIZE];
	HTList *l = str2list("symbol", "symbol");
	    
	math_normal_sym_font = GetFont(l, sym_size, FONT_WEIGHT_MEDIUM, FONT_STYLE_NORMAL, False);
	math_small_sym_font = GetFont(l, sym_size/2,FONT_WEIGHT_MEDIUM, FONT_STYLE_NORMAL, True);
	    
	math_normal_text_font = 0;
	math_small_text_font = 0;
	    
	math_bold_text_font = GetFont(l, sym_size/2, FONT_WEIGHT_BOLD, FONT_STYLE_NORMAL, True);
	math_italic_text_font = GetFont(l, sym_size/2, FONT_WEIGHT_NORMAL, FONT_STYLE_ITALIC, True);
    }

    HTList_destroy(l); /* DJB 17-jan-96 */

    return flat;
}

#endif
