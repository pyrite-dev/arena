/* image.c - creates textured background and other pixmap stuff */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>
#include <math.h>

#include "png.h" 
#include "www.h"
#include "www.bm"

#include "arena_png.h"

extern Display *display;
extern int screen;
extern Window win;
extern Visual *visual;

extern unsigned long labelColor, textColor, statusColor, strikeColor,
            transparent, windowColor, windowBottomShadow, windowShadow;

extern int RPixelShift, GPixelShift, BPixelShift;
extern int RPixelMask, GPixelMask, BPixelMask;
extern int depth;
extern int IsIndex;
extern Doc *CurrentDoc;
extern Context *context;
extern BOOL OwnColorMap; /* howcome 10/8/95 */

extern int tileWidth, tileHeight;
extern unsigned char *tileData;

extern Pixmap default_pixmap;
extern int default_pixmap_width, default_pixmap_height;
extern Colormap colormap;
extern int Magic256[256];
extern int Magic16[256];
extern int Magic32[256];
extern int Magic64[256];

extern GC disp_gc;
extern unsigned int win_width, win_height;
extern int debug;

extern double Gamma;

extern HTAtom *text_atom;
extern HTAtom *html_atom;
extern HTAtom *html3_atom;
extern HTAtom *html_level3_atom;
extern HTAtom *gif_atom;
extern HTAtom *jpeg_atom;
extern HTAtom *png_atom;
extern HTAtom *png_exp_atom;
extern HTAtom *xpm_atom;
extern HTAtom *xbm_atom;


Pixmap smile, frown, note_pixmap, caution_pixmap, warning_pixmap;
int imaging; /* set to COLOR888, COLOR232, GREY4 or MONO */
Image *images;  /* linked list of images */
Image *note_image, *caution_image, *warning_image;  /* standard icons */

unsigned long stdcmap[128];  /* 2/3/2 color maps for gifs etc */
unsigned long greymap[16];  /* for mixing with unsaturated colors */
int v2b_table[256];

extern unsigned long mycmap[256]; 

Image *default_image = NULL;


#define smile_xbm_width 15
#define smile_xbm_height 15

/* janet 21/07/95: declared and defined but not used 
static char smile_xbm_bits[] = {
   0x1f, 0x7c, 0xe7, 0x73, 0xfb, 0x6f, 0xfd, 0x5f, 0xfd, 0x5f, 0xce, 0x39,
   0xce, 0x39, 0xfe, 0x3f, 0xfe, 0x3f, 0xee, 0x3b, 0xdd, 0x5d, 0x3d, 0x5e,
   0xfb, 0x6f, 0xe7, 0x73, 0x1f, 0x7c};
   */

#define frown_xbm_width 15
#define frown_xbm_height 15

/* janet 21/07/95: defined but not used
static char frown_xbm_bits[] = {
   0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f,
   0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f,
   0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f};
   */

#define note_width 26
#define note_height 39
static char note_bits[] = {
   0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x38, 0xfc,
   0x00, 0x00, 0x7c, 0xfc, 0x00, 0x00, 0x7c, 0xfc, 0x00, 0x00, 0x7c, 0xfc,
   0x00, 0x00, 0x7c, 0xfc, 0x00, 0x00, 0x7c, 0xfc, 0x00, 0x00, 0x7c, 0xfc,
   0x00, 0x00, 0x7c, 0xfc, 0x00, 0x00, 0x7c, 0xfc, 0x00, 0x00, 0x7c, 0xfc,
   0x30, 0xc6, 0x7c, 0xfc, 0x78, 0xef, 0x7d, 0xfc, 0x78, 0xef, 0x7f, 0xfc,
   0x78, 0xef, 0x7f, 0xfc, 0x7c, 0x2f, 0x00, 0xfc, 0x7c, 0xcf, 0xff, 0xfc,
   0x7c, 0xcf, 0xff, 0xfc, 0x7c, 0xcf, 0xff, 0xfc, 0x7c, 0x2f, 0xff, 0xfc,
   0x7c, 0xef, 0xf8, 0xfc, 0x7c, 0xef, 0xfd, 0xfc, 0x38, 0xef, 0xfd, 0xfc,
   0xc0, 0xf9, 0xff, 0xfc, 0xe0, 0xff, 0xff, 0xfc, 0xe0, 0xff, 0xff, 0xfc,
   0xe0, 0xff, 0xff, 0xfc, 0xe0, 0xff, 0xff, 0xfc, 0xc0, 0xff, 0xff, 0xfc,
   0x80, 0xff, 0x3f, 0xfc, 0x00, 0xff, 0x1f, 0xfc, 0x00, 0xff, 0x0f, 0xfc,
   0x00, 0xff, 0x0f, 0xfc, 0x00, 0xff, 0x0f, 0xfc, 0x00, 0xff, 0x0f, 0xfc,
   0x00, 0xff, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc};

#define caution_width 26
#define caution_height 38
static char caution_bits[] = {
   0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x00, 0xc0, 0x00, 0xfc,
   0x00, 0xe0, 0x01, 0xfc, 0x00, 0xee, 0x19, 0xfc, 0x00, 0xef, 0x3d, 0xfc,
   0x00, 0xef, 0x3d, 0xfc, 0x00, 0xef, 0x3d, 0xfc, 0x00, 0xef, 0x3d, 0xfc,
   0x70, 0xef, 0x3d, 0xfc, 0x78, 0xef, 0x3d, 0xfc, 0x78, 0xef, 0x3d, 0xfc,
   0x78, 0xef, 0x3d, 0xfc, 0x78, 0xef, 0x3d, 0xfc, 0x78, 0xef, 0xdd, 0xfc,
   0x78, 0xef, 0xed, 0xfc, 0x78, 0xff, 0xef, 0xfc, 0xf8, 0xff, 0xf7, 0xfc,
   0xf8, 0xff, 0xf7, 0xfc, 0xf8, 0xff, 0xf7, 0xfc, 0xf8, 0xff, 0xf7, 0xfc,
   0xf8, 0xff, 0xf9, 0xfc, 0xf8, 0x7f, 0xfe, 0xfc, 0xf8, 0xbf, 0xff, 0xfc,
   0xf8, 0xbf, 0xff, 0xfc, 0xf8, 0xdf, 0xff, 0xfc, 0xf8, 0xdf, 0xff, 0xfc,
   0xf8, 0xdf, 0xff, 0xfc, 0xf0, 0xff, 0x7f, 0xfc, 0xe0, 0xff, 0x3f, 0xfc,
   0x80, 0xff, 0x0f, 0xfc, 0x80, 0xff, 0x07, 0xfc, 0x80, 0xff, 0x07, 0xfc,
   0x80, 0xff, 0x07, 0xfc, 0x80, 0xff, 0x07, 0xfc, 0x80, 0xff, 0x07, 0xfc,
   0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc};

#define warning_width 26
#define warning_height 38
static char warning_bits[] = {
 0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0xfc,0x00,0xc0,0x00,0xfc,0x00,0xe0,0x01,
 0xfc,0x00,0xee,0x19,0xfc,0x00,0xef,0x3d,0xfc,0x00,0xef,0x3d,0xfc,0x00,0xef,
 0x3d,0xfc,0x00,0xef,0x3d,0xfc,0x70,0xef,0x3d,0xfc,0x78,0xef,0x3d,0xfc,0x78,
 0x2f,0x30,0xfc,0x78,0x0f,0x38,0xfc,0x78,0x0f,0x3c,0xfc,0x78,0x07,0xde,0xfc,
 0x78,0x07,0xef,0xfc,0xf8,0x03,0xef,0xfc,0xf8,0x01,0xf6,0xfc,0xf8,0x0f,0xf7,
 0xfc,0xf8,0x8f,0xf7,0xfc,0xf8,0xe7,0xf7,0xfc,0xf8,0xf3,0xf9,0xfc,0xf8,0x79,
 0xfe,0xfc,0xf8,0xbe,0xff,0xfc,0xf8,0xbf,0xff,0xfc,0xf8,0xdf,0xff,0xfc,0xf8,
 0xdf,0xff,0xfc,0xf8,0xdf,0xff,0xfc,0xf0,0xff,0x7f,0xfc,0xe0,0xff,0x3f,0xfc,
 0x80,0xff,0x0f,0xfc,0x80,0xff,0x07,0xfc,0x80,0xff,0x07,0xfc,0x80,0xff,0x07,
 0xfc,0x80,0xff,0x07,0xfc,0x80,0xff,0x07,0xfc,0x00,0x00,0x00,0xfc,0x00,0x00,
 0x00,0xfc};

Image *DefaultImage()
{
    if (default_image)
	return default_image;

    default_pixmap = XCreatePixmapFromBitmapData(display, win, www_bits,
                      www_width, www_height, textColor, transparent, depth);

    default_image = (Image *)malloc(sizeof(Image));
    default_image->pixmap =  XCreatePixmapFromBitmapData(display, win, www_bits,
			      www_width, www_height, textColor, transparent, depth);
    default_image->width = www_width;
    default_image->height = www_height;

    return default_image;
}

int Brightness2Voltage(int brightness)
{
    double voltage;
    static double log_a = 0; /* howcome added double */

    if (brightness == 0)
        return 0;

    if (log_a == 0)
        log_a = (Gamma - 1.0) * log((double)65535);      /* cast added by howcome 21/9/94 */

    voltage = (log_a + log((double)brightness))/Gamma;   /* cast added by howcome 21/9/94 */

    return (int)(0.5 + exp(voltage));
}

int V2B(int voltage)
{
    double brightness;
    static double log_a;

    if (voltage == 0)
        return 0;

    if (log_a == 0)
        log_a = (Gamma - 1.0) * log((double)255);   /* cast added by howcome 21/9/94 */

    brightness = Gamma * log((double)voltage) - log_a;    /* cast added by howcome 21/9/94 */

    return (int)(0.5 + exp(brightness));
}

void build_v2b_table()
{
    int i;
    
    for(i=0;i<256;i++)
	v2b_table[i] = V2B(i);
}

int Voltage2Brightness(voltage)
    int voltage;
{
    return v2b_table[voltage];
}

/* don't add this to images list to avoid trouble when freeing document images */

Image *MakeIcon(unsigned int depth, char *name, int width, int height, char *bits)
{
    GC drawGC;
    Pixmap pixmap;
    XImage *ximage;
    Image *image;
    unsigned char *data, *p, *s;
    int size, i, j, k;
    unsigned int byte = 0;

    size = width * height;

    if (size == 0)
        return NULL;

    image = (Image *)malloc(sizeof(Image));
    image->url = name;
    image->npixels = 0;
    image->pixels = 0;
    image->next = NULL;
    image->width = width;
    image->height = height;

    s = (unsigned char*)bits;

    if (depth == 8)
    {
        p = data = (unsigned char *)malloc(size);

        for (i = 0; i < height; ++i)
            for (j = 0, k= 8; j < width; ++j)
            {
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    byte = *s++;
                    k = 1;
                }

                if (byte & 0x01)
                    *p++ = textColor;
                else
                    p = Transparent(p, j, i);

                byte = byte >> 1;
            }
    }
    else  if (depth == 16)
    {
        p = data = (unsigned char *)malloc(size * 2);

        for (i = 0; i < height; ++i)
            for (j = 0, k= 8; j < width; ++j)
            {
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    byte = *s++;
                    k = 1;
                }

                if (byte & 1)
                {
		    *p++ = (textColor >>  8) & 0xFF;
		    *p++ = textColor & 0xFF;
		}
                else
                    p = Transparent(p, j, i);

                byte = byte >> 1;
            }
    }
    else  if (depth == 24)
    {
        p = data = (unsigned char *)malloc(size * 4);

        for (i = 0; i < height; ++i)
            for (j = 0, k= 8; j < width; ++j)
            {
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    byte = *s++;
                    k = 1;
                }

                if (byte & 1)
                {
                    *p++ = '\0';
                    *p++ = (textColor >> 16) & 0xFF;
                    *p++ = (textColor >>  8) & 0xFF;
                    *p++ = textColor & 0xFF;
                }
                else
                    p = Transparent(p, j, i);

                byte = byte >> 1;
            }
    }
    else if (depth == 4 || depth == 2 || depth == 1)
    {
	int             ppb, bpl, shift = 0;
	int		newbyte;
	
	ppb = 8/depth; /* pixels per byte */
	bpl = width/ppb + (width%ppb ? 1 : 0); /* bytes per line */

        p = data = (unsigned char *)malloc(bpl * height);
	newbyte = 1;
	
        for (i = 0; i < height; ++i) {
            for (j = 0, k= 8; j < width; ++j)
            {
	        if (newbyte) {
		    *p = 0;
	            newbyte = 0;
	        }

                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    byte = *s++;
                    k = 1;
                }

                if (byte & 0x01) {
		    shift = (((7 - (j % 8)) % ppb) * depth);
		    *p |= textColor << shift;
		    if (shift == 0) {
			p++;
			newbyte = 1;
		    }
		}
                else {
		    shift = (((7 - (j % 8)) % ppb) * depth);
		    *p |= transparent << shift;
		    if (shift == 0) {
			p++;
			newbyte = 1;
		    }
		}
                byte = byte >> 1;
            }
	    
	    if (shift) {
		p++;   /* make sure we start on a new byte for the next line */
		newbyte = 1;
	    }
	}
    }
    else
    {
	fprintf(stderr,"Icons for display depth %d unsupported\n", depth);
        return NULL;
    }

    if ((ximage = XCreateImage(display, DefaultVisual(display, screen),
             depth, ZPixmap, 0, (char *)data,
             width, height, (depth == 24 ? 32 :(depth == 16 ? 16 : 8)), 0)) == 0)
    {
        Warn("Failed to create XImage: %s", image->url);
        Free(data);
	Free(image);
        return DefaultImage();
    }

    /* howcome 22/2/95: do we need to set these?? */

    ximage->byte_order = MSBFirst;
                       
    ximage->bitmap_bit_order = BitmapBitOrder(display);

    if ((pixmap = XCreatePixmap(display, win, width, height, depth)) == 0)
    {
        Warn("Failed to create Pixmap: %s", image->url);
        XDestroyImage(ximage); /* also free's image data */
	Free(image);
        return DefaultImage();
    }

    drawGC = XCreateGC(display, pixmap, 0, 0);
    XSetFunction(display, drawGC, GXcopy);
    XPutImage(display, pixmap, drawGC, ximage, 0, 0, 0, 0, width, height);
    XFreeGC(display, drawGC);
    XDestroyImage(ximage);  /* also free's image data */

    /* janet 27/07/95: image not always freed! */
    Free(image); 
      
    image = (Image *)malloc(sizeof(Image));
    image->url = name;
    image->npixels = 0;
    image->pixels = 0;
    image->next = NULL;
    image->width = width;
    image->height = height;
    image->pixmap = pixmap;
    return image;    
}

void MakeIcons(unsigned int depth)
{
    note_image = MakeIcon(depth, "note", note_width, note_height, note_bits);
    caution_image = MakeIcon(depth, "caution", caution_width, caution_height, caution_bits);
    warning_image = MakeIcon(depth, "warning", warning_width, warning_height, warning_bits);
}

unsigned char *gamma_table=NULL;


static unsigned int
range (unsigned int val, unsigned int vmax)
{
  return (val > vmax) ? vmax : val;
}

void
build_gamma_table()
{
  if (!gamma_table) {
    int i;
    double file_gamma = 1.0/2.2;
    double g = 1.0 / (file_gamma * Gamma);
  
    gamma_table = (unsigned char *) malloc(256 * sizeof(char));
    for (i = 0; i < 256; i++)
	gamma_table[i] = (unsigned char)range((pow((double)i / 255.0, g) * 255.0), 255);
  }
}


void
free_gamma_table()
{
  if(gamma_table){
    Free(gamma_table);
  }
}

int AllocGreyScale(void)
{
    unsigned long g;
    XColor color;

    greymap[0] = BlackPixel(display, screen);
    greymap[15] = WhitePixel(display, screen);

/*    mycmap[128 & 0] = BlackPixel(display, screen);
    mycmap[128 & 15] = WhitePixel(display, screen);
    */
    for (g = !OwnColorMap; g < 16; ++g)
    {
        color.red = color.green = color.blue = (g * 65535)/15;

     /* map brightness values into voltages for Gamma correction */

        color.red = Brightness2Voltage(color.red);
        color.green = Brightness2Voltage(color.green);
        color.blue = Brightness2Voltage(color.blue);

        if (XAllocColor(display, colormap, &color) == 0)
        {
            fprintf(stderr, "Can't allocate standard grey palette\n");

            while (g > 1)
                XFreeColors(display, colormap, &(greymap[--g]), 1, 0);

            return 0;
        }
        greymap[g] = color.pixel;
/*	mycmap[128 & g] = color.pixel;*/
    }

    return 1;
}


int AllocStandardColors(void)
{
    unsigned long i;
    XColor colors[128];  /* howcome 5/10/94 */
    int status[128];  /* howcome 5/10/94 */
    XColor color;
    int color_ok=1;

    stdcmap[0] = BlackPixel(display, screen);
    stdcmap[127] = WhitePixel(display, screen);

    mycmap[0] = BlackPixel(display, screen);
    mycmap[127] = WhitePixel(display, screen);

    for (i = 0; i < 128; ++i)
    {
#if 0
	double gammaC = 0.7;
        color.red = /*(i & 0x3) * 65535/3 ; */ pow((((double)(i & 0x3))/3.0), gammaC)*65535;
        color.green = /*((i >> 2) & 0x7) * 65535/7 ;*/ pow((((double)((i >> 2) & 0x7))/7.0), gammaC)*65535;	   
        color.blue = /*((i >> 5) & 0x3) * 65535/3 ;*/ pow((((double)((i >> 5) & 0x3))/3.0), gammaC)*65535;
#endif
	color.red = (i & 0x3) * 65535/3;
        color.green = ((i >> 2) & 0x7) * 65535/7;
        color.blue = ((i >> 5) & 0x3) * 65535/3;

	if (COLOR_TRACE)
	    fprintf(stderr,"AllocStandardColors exact %d %d %d\n", (color.red >> 8), (color.green >> 8), (color.blue >> 8));

	/* howcome 5/10/94: added support for XAllocColors which will
           speed things up. */

	/* map brightness values into voltages for Gamma correction */

	colors[i].red = Brightness2Voltage(color.red);
        colors[i].green = Brightness2Voltage(color.green);
        colors[i].blue = Brightness2Voltage(color.blue);
#if 0
	colors[i].red = color.red;
        colors[i].green = color.green;
        colors[i].blue = color.blue; 
#endif
	if (COLOR_TRACE)
	    fprintf(stderr,"AllocStandardColors %d %d %d\n", (colors[i].red >> 8), (colors[i].green >> 8), (colors[i].blue >> 8));
    }

    /* howcome 5/10/94: here comes the one and only call to XAllocColors */

    if (!OwnColorMap)
    {
	if (XAllocColors(display, colormap, colors, i, status)) {
	    for (i = 1; i < 127; i++ ) {
		stdcmap[i] = colors[i].pixel;
		mycmap[i] = colors[i].pixel;
	    }
	} else {
	    int j;
	    for(j = 1; j < i ; j++)
		if (status[j])
		    XFreeColors( display, colormap, &colors[j].pixel, 1, 0L );
	    OwnColorMap = 1; /* --Spif 25-Oct-95 -cm flag now works */
	    color_ok=0;
	}
    };
  
    /* howcome 10/8/05: added support for own colormap */
	
    if (OwnColorMap) {
	int j;
	if(!color_ok) {
	    fprintf(stderr,"Can't alloc colors, creating new colormap..\n");
	    for(i=1; i < 15 ; i++)
		XFreeColors(display, colormap, &(greymap[i]), 1, 0);
	};
	colormap = XCreateColormap(display, RootWindow(display, screen),
				   visual, AllocNone);
	AllocGreyScale();
	for(j=0;j<128;j++)
	{
	    if (XAllocColor(display, colormap, &colors[j]) == 0)
	    {
		int g;
		fprintf(stderr, "FATAL ERROR:Can't allocate my own palette!!\n");
		g=j;
		while (g > 1)
		    XFreeColors(display, colormap, &(stdcmap[--g]), 1, 0);
		return 0;
	    };
	    stdcmap[j] = colors[j].pixel;
	    mycmap[j] = colors[j].pixel;
	}
	
	return 1; /* assume success, what could go wrong? */
	
    }

    return 1;
}

int SupportTrueColor(void)
{
    long visual_info_mask;
    int number_visuals, i, flag;
    XVisualInfo *visual_array, visual_info_template;

    visual_info_template.screen = DefaultScreen(display);
    visual_info_mask = VisualClassMask | VisualScreenMask;

    visual_info_template.class = TrueColor;
    visual_array = XGetVisualInfo(display, visual_info_mask,
                        &visual_info_template,
                        &number_visuals);

    for (i = flag = 0; i < number_visuals; ++i)
    {
	if (visual_array[i].depth == 16)
	{
	    long int maskval;
	    int j=0, k;

	    maskval = (*visual).blue_mask;
	    while ((maskval & 1) == 0) { j++; maskval = maskval >> 1; }
	    maskval = (*visual).blue_mask;
	    BPixelMask = 0;
	    for(k=0; k<16; k++)
	    {
		if(maskval & 1)
		{
		    if(!BPixelMask)
			BPixelMask = 1;
		    else
			BPixelMask += BPixelMask + 1;
		};
		maskval = maskval >> 1;
	    }
	    j=0;
	    maskval = (*visual).green_mask;
	    while ((maskval & 1) == 0) { j++; maskval = maskval >> 1; }
	    GPixelShift = j;
	    GPixelMask = 0;
	    for(k=0; k<16; k++)
	    {
		if(maskval & 1)
		{
		    if(!GPixelMask)
			GPixelMask = 1;
		    else
			GPixelMask += GPixelMask + 1;
		};
		maskval = maskval >> 1;
	    }
	    j=0;
	    maskval = (*visual).red_mask;
	    while ((maskval & 1) == 0) { j++; maskval = maskval >> 1; }
	    RPixelShift = j;
	    RPixelMask = 0;
	    for(k=0; k<16; k++)
	    {
		if(maskval & 1)
		{
		    if(!RPixelMask)
			RPixelMask = 1;
		    else
			RPixelMask += RPixelMask + 1;
		};
		maskval = maskval >> 1;
	    };
            flag = 1;
	}
	if (visual_array[i].depth == 24)
        {
	    long int maskval;
	    int j=0;
	    
	    maskval = (*visual).blue_mask;
	    while ((maskval & 1) == 0) { j++; maskval = maskval >> 1; }
	    BPixelShift = j;
	    j=0;
	    maskval = (*visual).green_mask;
	    while ((maskval & 1) == 0) { j++; maskval = maskval >> 1; }
	    GPixelShift = j;
	    j=0;
	    maskval = (*visual).red_mask;
	    while ((maskval & 1) == 0) { j++; maskval = maskval >> 1; }
	    RPixelShift = j;
            flag = 1;
            break;
	}
    }

    XFree((void *)visual_array);
    return flag;
}

int InitImaging(int ColorStyle)
{
    imaging = MONO;
    
    build_gamma_table();
    build_v2b_table();

    greymap[0] = BlackPixel(display, screen);
    greymap[15] = WhitePixel(display, screen);

    if (ColorStyle == MONO)
    {
        imaging = ColorStyle;
        return imaging;
    }

    if (ColorStyle == COLOR888 && SupportTrueColor())
    {
        imaging = ColorStyle;
        return imaging;
    }

    if(OwnColorMap)
	if (AllocStandardColors())
	{
	    imaging = COLOR232;
	    return imaging;
	};
    
    if (AllocGreyScale())
    {
        imaging = GREY4;
	
        if (ColorStyle == GREY4)
            return imaging;
	
        if (AllocStandardColors())
            imaging = COLOR232;
    } else 
	if(OwnColorMap)
	    if (AllocStandardColors())
		imaging = COLOR232;
    return imaging;
}

void ReportVisuals(void)
{
    long visual_info_mask;
    int number_visuals, i;
    XVisualInfo *visual_array, visual_info_template;

    visual_info_template.screen = DefaultScreen(display);

    visual_info_mask = VisualClassMask | VisualScreenMask;

    printf("TrueColor:\n");

    visual_info_template.class = TrueColor;
    visual_array = XGetVisualInfo(display, visual_info_mask,
                        &visual_info_template,
                        &number_visuals);

    for (i = 0; i < number_visuals; ++i)
    {
        printf("  visual Id 0x%x\n", visual_array[i].visualid);
        printf("  depth = %d, bits per rgb = %d, size = %d\n", visual_array[i].depth,
                    visual_array[i].bits_per_rgb, visual_array[i].colormap_size);
        printf("   rgb masks %lx, %lx, %lx\n", visual_array[i].red_mask,
                    visual_array[i].green_mask, visual_array[i].blue_mask);
    }

    XFree((void *)visual_array);

    printf("DirectColor:\n");

    visual_info_template.class = DirectColor;
    visual_array = XGetVisualInfo(display, visual_info_mask,
                        &visual_info_template,
                        &number_visuals);

    for (i = 0; i < number_visuals; ++i)
    {
        printf("  visual Id 0x%x\n", visual_array[i].visualid);
        printf("  depth = %d, bits per rgb = %d, size = %d\n", visual_array[i].depth,
                    visual_array[i].bits_per_rgb, visual_array[i].colormap_size);
        printf("   rgb masks %lx, %lx, %lx\n", visual_array[i].red_mask,
                    visual_array[i].green_mask, visual_array[i].blue_mask);
    }

    XFree((void *)visual_array);

    printf("PseudoColor:\n");

    visual_info_template.class = PseudoColor;
    visual_array = XGetVisualInfo(display, visual_info_mask,
                        &visual_info_template,
                        &number_visuals);

    for (i = 0; i < number_visuals; ++i)
    {
        printf("  visual Id 0x%x\n", visual_array[i].visualid);
        printf("  depth = %d, bits per rgb = %d, size = %d\n", visual_array[i].depth,
                    visual_array[i].bits_per_rgb, visual_array[i].colormap_size);
        printf("   rgb masks %lx, %lx, %lx\n", visual_array[i].red_mask,
                    visual_array[i].green_mask, visual_array[i].blue_mask);
    }

    XFree((void *)visual_array);
}

void ReportStandardColorMaps(Atom which_map)
{
    XStandardColormap *std_colormaps;
    int i, number_colormaps;
    char *atom_name;

    if (XGetRGBColormaps(display, RootWindow(display, screen),
            &std_colormaps, &number_colormaps, which_map) != 0)
    {
        atom_name = XGetAtomName(display, which_map);
        printf("\nPrinting %d standard colormaps for %s\n",
                number_colormaps, atom_name);
        XFree(atom_name);

        for  (i = 0; i < number_colormaps; ++i)
        {
            printf("\tColormap: 0x%x\n", std_colormaps[i].colormap);
            printf("\tMax cells (rgb): %d, %d, %d\n",
                std_colormaps[i].red_max,
                std_colormaps[i].green_max,
                std_colormaps[i].blue_max);
            printf("\tMultipliers: %d, %d, %d\n",
                std_colormaps[i].red_mult,
                std_colormaps[i].green_mult,
                std_colormaps[i].blue_mult);
            printf("\tBase pixel: %d\n", std_colormaps[i].base_pixel);
            printf("\tVisual Id 0x%x, Kill Id 0x%x\n",
                std_colormaps[i].visualid,
                std_colormaps[i].killid);
        }

        XFree((void *)std_colormaps);
    }
}

XColor paperrgb[3];

/* Init paperrgb */
void InitPaperRGB ()
{
  int i;
  
  for (i=0; i<3; i++) {
    int r=0, g=0, b=0;
    switch(i) {
      case 0:
	r=230; g=218; b=194;
	break;
      case 1:
	r=220; g=209; b=186;
	break;
      case 2:
	r=210; g=199; b=177;
	break;
    }
	  
    paperrgb[i].red=/*gamma_table[r]*/ r;
    paperrgb[i].green=/*gamma_table[g]*/ g;
    paperrgb[i].blue=/*gamma_table[b]*/ b;
  }
}	  


/* Barely used colormap */
XColor papercols[256];


/* create a textured background as paper */
unsigned char *CreateBackground(unsigned int width, unsigned int height, unsigned int depth)
{
    unsigned char *data, *p;
    int size, i, j;
    unsigned long cs[3]; /* howcome 21/9/94 */
    unsigned long int ulp;
    
    if (depth == 8)
    {
      /* howcome 4/10/94: changed last arg to GetColor */
      
      int i;
      for (i=0; i<3; i++) {
	int r, g, b;
	r=paperrgb[i].red;
	g=paperrgb[i].green;
	b=paperrgb[i].blue;
	if (!GetColor(r, g, b, &cs[i]))
	  return NULL;
	paperrgb[i].pixel=cs[i];
	papercols[cs[i]].red=r;
	papercols[cs[i]].green=g;
	papercols[cs[i]].blue=b;
      }

      size = width * height;
    }
    else if (depth == 24)
        size = width * height * 4;
    else if (depth == 16)
	size = width * height *2;
    else
        return NULL;

    p = data = (unsigned char *)malloc(size);

    if (data == NULL)
        return NULL;

    srand(0x6000);

    if (depth == 8)
    {
        for (i = 0; i < height; ++i)
            for (j = 0; j < width; ++j)
            {
                /* howcome 21/9/94: rand returns different ranges on different platforms, therefore: */
            
	      *p++ = paperrgb[rand() % 3].pixel;
            }
    }
    else if (depth == 24)
    {
        for (i = 0; i < height; ++i)
            for (j = 0; j < width; ++j)
	    {
		int col,r,g,b;
                
		col=rand() % 3;

		r=paperrgb[col].red;
		g=paperrgb[col].green;
		b=paperrgb[col].blue;
		
		GetColor(r, g, b, &ulp);
                
		*p++ = '\0';
		*p++ = (ulp >> 16) & 0xff; 
		*p++ = (ulp >> 8) & 0xff; 
		*p++ = ulp & 0xff; 		
#if 0
		*p++ = '\0';
		*p++ = paperrgb[col].red;
		*p++ = paperrgb[col].green;
		*p++ = paperrgb[col].blue;
#endif
            }
    } else if (depth == 16) 
    {
	 for (i = 0; i < height; ++i)
            for (j = 0; j < width; ++j)
	    {
		int col,r,g,b;
                
		col=rand() % 3;
		r=paperrgb[col].red;
		g=paperrgb[col].green;
		b=paperrgb[col].blue;
		
		GetColor(r, g, b, &ulp);
                
		*p++ = ((char*)&ulp)[1]; 
		*p++ = ((char*)&ulp)[0]; 
	    }
    }
    return data;
}

#if 0  /* used to allow for nested comments */
/* XPM */
/********************************************************/
/**   (c) Copyright Hewlett-Packard Company, 1992.     **/
/********************************************************/
static char ** arizona.l.px  = {
/* width height ncolors cpp [x_hot y_hot] */
"28 38 13 1",
/* colors */
"   s iconColor2    m white c white",
".  s iconGray2     m white c #c8c8c8c8c8c8",
"X  s iconColor1    m black c black",
"o  s iconGray6     m black c #646464646464",
"O  s iconGray3     m white c #afafafafafaf",
"+  s iconColor3    m black c red",
"@  s iconColor8    m white c magenta",
"#  s iconGray4     m white c #969696969696",
"$  s iconGray5     m black c #7d7d7d7d7d7d",
"%  s iconColor6    m white c yellow",
"&  s iconGray1     m white c #e1e1e1e1e1e1",
"*  s iconColor4    m black c green",
"=  s bottomShadowColor     m black c #646464646464",
/* pixels */
"                            ",
" ..........................X",
" ..............oo..........X",
" .........OOOoo+@@oooOOOOOOX",
" .....OOOoooo@####@+ooooo..X",
" ..ooo#oo+@###$$$$#####OOO.X",
" ..OOOOOO###$$....$$#@+ooo.X",
" .......+@#$.%%%%%%.$###OOOX",
" ..o.ooo..$.%%%%%%%%%$#@+ooX",
and so on, ending with:
" XXXXXXXXXXXXXXXXXXXXXXXXXXX"};
#endif

#define NEXTCHAR(s) (*s ? *s++ : '\0')

static char *FindCh(char *s, char ch)
{
    char c;

    for (;;)
    {
        c = NEXTCHAR(s);

        if (c == ch || c == '\0')
            return s;
    }
}

/* *c to first char and return last word */

static char *ReadColor(char *s, char **name, int *ch)
{
    char *p;
    int c;
    static char line[256];

    s = FindCh(s, '"');
    *ch = NEXTCHAR(s);
    p = line;

    while ((c = NEXTCHAR(s)) != '"' && c != '\0' && p < line+255)
        *p++ = c;

    *p = '\0';
    p = strrchr(line, ' ');

    if (p)
        ++p;

    *name = p;
    return s;
}

unsigned char *Transparent(unsigned char *p, int x, int y)
{
    unsigned int i;
    unsigned char *s;

    if (tileData)
    {
        x = x % tileWidth;
        y = y % tileHeight;
        i = y * tileWidth + x;
	
	if (depth == 24)
        {
            s = tileData + 4 * i;
	    
            *p++ = *s++;
            *p++ = *s++;
            *p++ = *s++;
            *p++ = *s++;
	    
            return p;
        }
	
	if (depth == 16)
	{
	    s = tileData + 2 * i;
	    
	    *p++ = *s++;
	    *p++ = *s++;
	    
	    return p;
	};
	
	*p++ = gamma_table[tileData[i]];
	return p;
    }

    if (depth == 24)
    {
        *p++ = '\0';
        *p++ = (transparent  >> 16) & 0xFF;
        *p++ = (transparent  >> 8) & 0xFF;
	*p++ = transparent & 0xFF;
        return p;
    }

/* must be GPixelShift ...??? */

    if (depth == 16) 
    {
	*p++ = (transparent  >> 8) & 0xFF;
	*p++ = transparent & 0xFF;
        return p;
    };

    *p++ = transparent;
    return p;
}

/* load data from an XPM file and allocate colors */

char *LoadXpmImage(Image *image, Block *bp, unsigned int depth)
{
    int c, i, j, cr, cg, cb, r, g, b, ncolors, size, map[256];
    unsigned int width, height;
    /*    unsigned long pixel, *pixdata; */ /* janet 21/07/95: not used */
    unsigned char *data = NULL, *p;
    unsigned long ccolor;
    char *name, *s;
    Color *colors, color;
    XColor xcolor;

    s = bp->buffer + bp->next;

    s = FindCh(s, '"');
    sscanf(s, "%d %d %d", &width, &height, &ncolors);
    s = FindCh(s, '\n');

    size = width * height;
    image->width = width;
    image->height = height;

    if (size == 0 || ncolors == 0)
        return NULL;


    if (depth != 8 && depth != 24 && depth != 4 && depth != 2 && depth != 1)
    {
        printf("Display depth %d unsupported\n", depth);
        return NULL;
    }

    image->npixels = 0;
    image->pixels = 0; /*(unsigned long *)malloc(ncolors * sizeof(unsigned long)); */

    colors = (Color *)malloc(ncolors * sizeof(Color));

    for (i = 0; i < 256; ++i)
        map[i] = -1;

    for (i = 0; i < ncolors; ++i)
    {
        s = ReadColor(s, &name, &c);

        if (XParseColor(display, colormap, name, &xcolor) == 0)
        {
            map[c] = -1;
            continue;
        }

        map[c] = i;
        r = xcolor.red >> 8;
        g = xcolor.green >> 8;
        b = xcolor.blue >> 8;

     /* apply Gamma correction to map voltages to brightness values */

        if (imaging != COLOR888)
        {
	    /*
	    r = gamma_table[r];
            g = gamma_table[g];
            b = gamma_table[b];*/
	    r = Voltage2Brightness(r);
	    g = Voltage2Brightness(g);
	    b = Voltage2Brightness(b);
        }

        colors[i].red = r;
        colors[i].green = g;
        colors[i].blue = b;
        colors[i].grey = (3*r + 6*g + b)/10;
    }


    if (depth == 8)
    {
        p = data = malloc(size);

        for (i = 0; i < height; ++i)
        {
            s = FindCh(s, '"');

            for (j = 0; j < width; ++j)
            {
                c = *s++;
                c = map[c];

                if (c < 0)
                {
                    p = Transparent(p, j, i);
                    continue;
                }

                color = colors[c];
                c = ((i % 16) << 4) + (j % 16);

                if (imaging == COLOR232)
                {
                    cr = color.red;
                    cg = color.green;
                    cb = color.blue;

                    if (cr == cg  && cg == cb)
                    {
                        cg  = color.grey;
                        g = cg & 0xF0;

                        if (cg - g > Magic16[c])
                            g += 16;

                        g = min(g, 0xF0);
                        *p++ = greymap[g >> 4];
                    }
                    else
                    {
                        r = cr & 0xC0;
                        g = cg & 0xE0;
                        b = cb & 0xC0;

                        if (cr - r > Magic64[c])
                            r += 64;

                        if (cg - g > Magic32[c])
                            g += 32;

                        if (cb - b > Magic64[c])
                            b += 64;

                        r = min(r, 255) & 0xC0;
                        g = min(g, 255) & 0xE0;
                        b = min(b, 255) & 0xC0;

                        *p++ = stdcmap[(r >> 6) | (g >> 3) | (b >> 1)];
                    }
                }
                else if (imaging == GREY4)
                {
                    cg  = color.grey;
                    g = cg & 0xF0;

                    if (cg - g > Magic16[c])
                        g += 16;

                    g = min(g, 0xF0);
                    *p++ = greymap[g >> 4];
                }
                else /* MONO */
                {
                    if (color.grey < Magic256[c])
                        *p++ = greymap[0];
                    else
                        *p++ = greymap[15];
                }
            }

            s = FindCh(s, '\n');
        }
    }
    else if (depth == 24)
    {
        p = data = malloc(size * 4);

        for (i = 0; i < height; ++i)
        {
            s = FindCh(s, '"');

            for (j = 0; j < width; ++j)
            {
                if ((c = map[(unsigned char)*s++]) < 0)
                {
                    p = Transparent(p, j, i);
                    continue;
                };

                color = colors[c];
                *p++ = '\0';
                *p++ = color.red;
                *p++ = color.green;
                *p++ = color.blue;
            }

            s = FindCh(s, '\n');
        }
    } 
    else if (depth == 16)
    {
	p = data = malloc(size * 2);
	
        for (i = 0; i < height; ++i)
        {
            s = FindCh(s, '"');
	    
            for (j = 0; j < width; ++j)
            {
                if ((c = map[(unsigned char)*s++]) < 0)
                {
                    p = Transparent(p, j, i);
                    continue;
                };
		
                color = colors[c];
		GetColor(color.red, color.green, color.blue, &ccolor);
                *p++ = (ccolor >> 8 )& 0xff;
                *p++ = (ccolor & 0xff);
	    }
	    s = FindCh(s, '\n');
        }	
    }
    else if (depth == 1 || depth == 2 || depth == 4)  /* howcome added support for these */
    {
	int             ppb, bpl, shift = 0;
	int		newbyte;
	
	ppb = 8/depth; /* pixels per byte */
	bpl = width/ppb + (width%ppb ? 1 : 0); /* bytes per line */

        p = data = (unsigned char *)malloc(bpl * height);
	newbyte = 1;

        for (i = 0; i < height; ++i)
	{
            s = FindCh(s, '"');

            for (j = 0; j < width; ++j)
            {
		if (newbyte) {
		    *p = 0;
		    newbyte = 0;
		}
		
		c = *s++;
                c = map[c];

                if (c < 0)
                {
		    shift = (((7 - (j % 8)) % ppb) * depth);
		    *p |= transparent << shift;
		    if (shift == 0) {
			p++;
			*p = 0;
		    }	
                    continue;
                }

                color = colors[c];
                c = ((i % 16) << 4) + (j % 16);

                if (imaging == COLOR232)
                {
                    cr = color.red;
                    cg = color.green;
                    cb = color.blue;

                    if (cr == cg  && cg == cb)
                    {
                        cg  = color.grey;
                        g = cg & 0xF0;

                        if (cg - g > Magic16[c])
                            g += 16;

                        g = min(g, 0xF0);

			shift = (((7 - (j % 8)) % ppb) * depth);
			*p |= greymap[g >> 4] << shift;
			if (shift == 0) {
			    p++;
			    newbyte = 1;
			}

                    }
                    else
		    {
                        r = cr & 0xC0;
                        g = cg & 0xE0;
                        b = cb & 0xC0;

                        if (cr - r > Magic64[c])
                            r += 64;

                        if (cg - g > Magic32[c])
                            g += 32;

                        if (cb - b > Magic64[c])
                            b += 64;

                        r = min(r, 255) & 0xC0;
                        g = min(g, 255) & 0xE0;
                        b = min(b, 255) & 0xC0;

			shift = (((7 - (j % 8)) % ppb) * depth);
			*p |= stdcmap[(r >> 6) | (g >> 3) | (b >> 1)] << shift;
			if (shift == 0) {
			    p++;
			    newbyte = 1;
			}
                    }
                }
                else if (imaging == GREY4)
                {
                    cg  = color.grey;
                    g = cg & 0xF0;

                    if (cg - g > Magic16[c])
                        g += 16;

                    g = min(g, 0xF0);

		    shift = (((7 - (j % 8)) % ppb) * depth);
		    *p |= greymap[g >> 4] << shift;
		    if (shift == 0) {
			p++;
			newbyte = 1;
		    }
                }
                else /* MONO */
                {
                    if (color.grey < Magic256[c]) {
			shift = (((7 - (j % 8)) % ppb) * depth);
			*p |= greymap[0] << shift;
			if (shift == 0) {
			    p++;
			    newbyte = 1;
			}
		    }
                    else {
			shift = (((7 - (j % 8)) % ppb) * depth);
			*p |= greymap[15] << shift;
			if (shift == 0) {
			    p++;
			    newbyte = 1;
			}
		    }
		}
	    }

	    s = FindCh(s, '\n');
	    
	    if (shift) {
	        p++;   /* make sure we start on a new byte for the next line */
	        newbyte = 1;
	    }	
	}
    }

    Free(colors);
    return (char *)data;
}

/* 
    Load data from an XBM file

#define back_width 20
#define back_height 23
static char back_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0f,
   0x00, 0x80, 0x0f, 0x00, 0x80, 0x0f, 0x00, 0x80, 0x0f, 0x00, 0x80, 0x0f,
   0x00, 0x80, 0x0f, 0x60, 0x80, 0x0f, 0x70, 0x80, 0x0f, 0x78, 0x00, 0x00,
   0xfc, 0xff, 0x0f, 0xfe, 0xff, 0x07, 0xff, 0xff, 0x03, 0xfe, 0xff, 0x01,
   0xfc, 0xff, 0x00, 0x78, 0x00, 0x00, 0x70, 0x00, 0x00, 0x60, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

So find line start with #define and read last number as width.
Repeat to get height. Should really look at names to avoid problems
if order is swapped, but probably won't be necessary.

Number of bytes of data is:     ((height * width) + 7)/8

Then skip to '{' char and then start reading data:

    whitespace or ',' followed by hex number
*/

char *LoadXbmImage(Image *image, Block *bp, unsigned int depth)
{
    char *s;
    int c, i, j, k, size;
    unsigned int width, height, byte;
    unsigned char *data, *p;

    s = (char *)bp->buffer + bp->next;

    s = FindCh(s, '#');         /* find #define */
    s = FindCh(s, ' ');         /* find 1st space char */
    s = FindCh(s, ' ');         /* find 2nd space char */
    sscanf(s, "%d", &width);    /* and read width */
    s = FindCh(s, '#');         /* find next #define */
    s = FindCh(s, ' ');         /* find 1st space char */
    s = FindCh(s, ' ');         /* find 2nd space char */
    sscanf(s, "%d", &height);   /* and read width */
    s = FindCh(s, '{');         /* find opening brace for data */

    size = width * height;
    image->width = width;
    image->height = height;

    if (size == 0)
        return NULL;

    image->npixels = 0;
    image->pixels = 0;

    if (depth == 8)
    {
        p = data = (unsigned char *)malloc(size);

        for (i = 0; i < height; ++i)
            for (j = 0, k= 8; j < width; ++j)
            {
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    s = FindCh(s, '0');
                    sscanf(s+1, "%x", &byte); /* howcome 5/11/94: '-1' changed to '+1' since some don't like 0x */
                    while ((c = NEXTCHAR(s)) != ',' && c != '}');
                    k = 1;
                }

                if (byte & 0x01)
                    *p++ = textColor;
                else
                    p = Transparent(p, j, i);

                byte = byte >> 1;
            }
    }
    else if (depth == 24)
    {
        p = data = (unsigned char *)malloc(size * 4);
        

        for (i = 0; i < height; ++i)
            for (j = 0, k= 8; j < width; ++j)
            {
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    s = FindCh(s, '0');
                    sscanf(s-1, "%x", &byte);
                    while ((c = NEXTCHAR(s)) != ',' && c != '}');
                    k = 1;
                }

                if (byte & 1)
                {
                    *p++ = '\0';
                    *p++ = (textColor >> 16) & 0xFF;
                    *p++ = (textColor >>  8) & 0xFF;
                    *p++ = textColor & 0xFF;
                }
                else
                    p = Transparent(p, j, i);

                byte = byte >> 1;
            }
    }
    else if (depth == 16)
    {
        p = data = (unsigned char *)malloc(size * 2);
        

        for (i = 0; i < height; ++i)
            for (j = 0, k= 8; j < width; ++j)
            {
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    s = FindCh(s, '0');
                    sscanf(s-1, "%x", &byte);
                    while ((c = NEXTCHAR(s)) != ',' && c != '}');
                    k = 1;
                }

                if (byte & 1)
                {
		    *p++ = (textColor >>  8) & 0xFF;
                    *p++ = textColor & 0xFF;
                }
                else
                    p = Transparent(p, j, i);

                byte = byte >> 1;
            }
    }
    else if (depth == 1 || depth == 2 || depth == 4)  /* howcome added support for these */
    {
	int             ppb, bpl, shift;
	int		newbyte;

	ppb = 8/depth; /* pixels per byte */
	bpl = width/ppb + (width%ppb ? 1 : 0); /* bytes per line */

        p = data = (unsigned char *)malloc(bpl * height);
	newbyte = 1;

        for (i = 0; i < height; ++i) {
            for (j = 0, k= 8; j < width; ++j)
            {
		if (newbyte) {
 		    *p = 0;
 	            newbyte = 0;
 	        }
		
                if (++k > 8)  /* need to read next 8 pixel values */
                {
                    s = FindCh(s, '0');
                    sscanf(s+1, "%x", &byte); /* howcome 5/11/94: '-1' changed to '+1' since some don't like 0x */
                    while ((c = NEXTCHAR(s)) != ',' && c != '}');
                    k = 1;
                }

                if (byte & 0x01) {
		    shift = (((7 - (j % 8)) % ppb) * depth);
		    *p |= textColor << shift;
		}
                else {
		    shift = (((7 - (j % 8)) % ppb) * depth);
		    *p |= transparent << shift;
		}

		if (shift == 0) {
		    p++;
		    newbyte = 1;
		}	

                byte = byte >> 1;
            }

	    if (shift) {
		p++;
		newbyte = 1;
	    }
	}
    }
    else
    {
        Warn("image/x-xbitmap unsupported for depth %d", depth);
        return NULL;
    }

    return (char *)data;
}


Image *ProcessLoadedImage(Doc *doc)
{
    char *data;
    Image *image;
    Block block;
    unsigned int width, height;
    XImage *ximage;
    Pixmap pixmap;
    GC drawGC;
    HTAtom *a = NULL;

    if (doc && doc->anchor)
	a = HTAnchor_format(doc->anchor->parent);

    image = (Image *)malloc(sizeof(Image));
    image->npixels = 0;

    block.next = 0;  /*NewDoc.hdrlen; */
    block.size = doc->loaded_length; /* NewDoc.length; */
    block.buffer = doc->content_buffer;
    
    if (!doc->content_buffer) {	/* probably externally viewed image */
        doc->state = DOC_EXTERNAL;
	return NULL;
/*	return DefaultImage(image); */
    }
#ifdef PROGPNG
    Announce("Processing image %s...", doc->url); 
#endif
    if (a == gif_atom)
	{
	    if ((data = (char *)LoadGifImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load GIF image: %s", doc->url);
		    Free(block.buffer);
/*		    return DefaultImage(image); */
		    doc->state = DOC_REJECTED;
		    Free(image);
		    return NULL;
		}
	}

    else if (a == xpm_atom)
	{
	    if ((data = LoadXpmImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load XPM image: %s", doc->url);
		    Free(block.buffer);
		    doc->state = DOC_REJECTED;
/*		    return DefaultImage(image); */
		    Free(image);
		    return NULL;
		}
	}

    else if (a == xbm_atom)
	{
	    if ((data = LoadXbmImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load XBM image: %s", doc->url);
		    Free(block.buffer);
		    doc->state = DOC_REJECTED;
/*		    return DefaultImage(image); */
		    Free(image);
		    return NULL;
		}
	}

#ifdef JPEG
      else if (a == jpeg_atom)
	{
	    if ((data = LoadJPEGImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load JPEG image: %s", doc->url);
		    Free(block.buffer);
		    doc->state = DOC_REJECTED;
		    Free(image);
		    return NULL;
		}
	}
#endif /* JPEG */


#ifdef PNG
      else if (a == png_atom || a == png_exp_atom)
	{
	    if ((data = LoadPNGImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load PNG image: %s", doc->url);
		    /* Free(block.buffer); */
		    doc->state = DOC_REJECTED;
		    Free(image);
		    return NULL;
		}
	}
#endif /* PNG */

      else
	{
	    Warn("Failed to load unknown image format: %s", doc->url);
	    Free(block.buffer);
	    doc->state = DOC_REJECTED;
/*	    return DefaultImage(image); */
	    Free(image);
	    return NULL;
	}
    
    Free(block.buffer);
    doc->state = DOC_PROCESSED;
    doc->content_buffer = NULL; /* howcome 4/12/94: no need to keep the gif source around */
    width = image->width;
    height = image->height;
    
    if ((ximage = XCreateImage(display, DefaultVisual(display, screen),
			       depth, ZPixmap, 0, data, width, height,
			       (depth == 24 ? 32 :(depth == 16 ? 16 : 8)), 0)) == 0)
	{	
	    Warn("Failed to create XImage: %s", doc->url);
	    doc->state = DOC_REJECTED;
	    Free(data);	
/* 	    return DefaultImage(image);*/
	    return NULL;
	}

    /* howcome 22/2/95: do we need to set these?? */

    ximage->byte_order = MSBFirst;
    ximage->bitmap_bit_order = BitmapBitOrder(display);
       
    if ((pixmap = XCreatePixmap(display, win, width, height, depth)) == 0)
	{
	    Warn("Failed to create Pixmap: %s", doc->url);
	    XDestroyImage(ximage); /* also free's image data */
	    doc->state = DOC_REJECTED;
/*	    return DefaultImage(image); */
	    return NULL;
	}
    
    drawGC = XCreateGC(display, pixmap, 0, 0);
    XSetFunction(display, drawGC, GXcopy);
    XPutImage(display, pixmap, drawGC, ximage, 0, 0, 0, 0, width, height);
    XFreeGC(display, drawGC);
    XDestroyImage(ximage);  /* also free's image data */
  
    image->pixmap = pixmap;
    image->width = width;
    image->height = height;
    doc->image = image;
    return doc->image;
}


Image *ProcessProgressiveImage(Doc *doc)
{
    char *data;
    Image *image;
    Block block;
    unsigned int width, height;
    XImage *ximage;
    Pixmap pixmap;
    GC drawGC;
    HTAtom *a = NULL;

    if (doc && doc->anchor)
	a = HTAnchor_format(doc->anchor->parent);

    image = (Image *)malloc(sizeof(Image));
    image->npixels = 0;

    block.next = 0;  /*NewDoc.hdrlen; */
    block.size = doc->loaded_length; /* NewDoc.length; */
    block.buffer = doc->content_buffer;
    
    if (!doc->content_buffer) {	/* probably externally viewed image */
        doc->state = DOC_EXTERNAL;
	return NULL;
/*	return DefaultImage(image); */
    }
#ifdef PROGPNG
    Announce("Processing image %s...", doc->url); 
#endif
#if 0
    if (a == gif_atom)
	{
	    if ((data = (char *)LoadGifImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load GIF image: %s", doc->url);
		    Free(block.buffer);
/*		    return DefaultImage(image); */
		    doc->state = DOC_REJECTED;
		    Free(image);
		    return NULL;
		}
	}

    else if (a == xpm_atom)
	{
	    if ((data = LoadXpmImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load XPM image: %s", doc->url);
		    Free(block.buffer);
		    doc->state = DOC_REJECTED;
/*		    return DefaultImage(image); */
		    Free(image);
		    return NULL;
		}
	}

    else if (a == xbm_atom)
	{
	    if ((data = LoadXbmImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load XBM image: %s", doc->url);
		    Free(block.buffer);
		    doc->state = DOC_REJECTED;
/*		    return DefaultImage(image); */
		    Free(image);
		    return NULL;
		}
	}

#ifdef JPEG
      else if (a == jpeg_atom)
	{
	    if ((data = LoadJPEGImage(image, &block, depth)) == NULL)
		{
		    Warn("Failed to load JPEG image: %s", doc->url);
		    Free(block.buffer);
		    doc->state = DOC_REJECTED;
		    Free(image);
		    return NULL;
		}
	}
#endif /* JPEG */

#endif

#ifdef PNG
      else if (a == png_atom || a == png_exp_atom)
	{
	    if ((data = LoadProgressivePNGImage(image, &block, depth, doc->loaded_length)) == NULL)
		{
		    Warn("Failed to load PNG image: %s", doc->url);
		    /* Free(block.buffer); */
		    doc->state = DOC_REJECTED;
		    Free(image);
		    return NULL;
		}
	}
#endif /* PNG */

      else
	{
	    Warn("Failed to load unknown image format: %s", doc->url);
	    Free(block.buffer);
	    doc->state = DOC_REJECTED;
/*	    return DefaultImage(image); */
	    Free(image);
	    return NULL;
	}
#if 0
    Free(block.buffer);
#endif
    doc->state = DOC_PROCESSED;
#if 0
    doc->content_buffer = NULL; /* howcome 4/12/94: no need to keep the gif source around */
#endif
    width = image->width;
    height = image->height;
    
    if ((ximage = XCreateImage(display, DefaultVisual(display, screen),
			       depth, ZPixmap, 0, data, width, height,
			       (depth == 24 ? 32 :(depth == 16 ? 16 : 8)), 0)) == 0)
	{	
	    Warn("Failed to create XImage: %s", doc->url);
	    doc->state = DOC_REJECTED;
	    Free(data);	
/* 	    return DefaultImage(image);*/
	    return NULL;
	}

    /* howcome 22/2/95: do we need to set these?? */

    ximage->byte_order = MSBFirst;
    ximage->bitmap_bit_order = BitmapBitOrder(display);
       
    if ((pixmap = XCreatePixmap(display, win, width, height, depth)) == 0)
	{
	    Warn("Failed to create Pixmap: %s", doc->url);
	    XDestroyImage(ximage); /* also free's image data */
	    doc->state = DOC_REJECTED;
/*	    return DefaultImage(image); */
	    return NULL;
	}
    
    drawGC = XCreateGC(display, pixmap, 0, 0);
    XSetFunction(display, drawGC, GXcopy);
    XPutImage(display, pixmap, drawGC, ximage, 0, 0, 0, 0, width, height);
    XFreeGC(display, drawGC);
    XDestroyImage(ximage);  /* also free's image data */
  
    image->pixmap = pixmap;
    image->width = width;
    image->height = height;
    doc->image = image;
    return doc->image;
}

Image *GetImage(char *href, int hreflen, BOOL reload)
{
    Doc *doc = NULL;

    /* check for null name */

    if ((doc = GetInline(href, hreflen, reload))) {
	if (doc->image)
	    return (doc->image);
	else
	    return (ProcessLoadedImage(doc));
    }
    
    return NULL;
}




void FreeImages(int cloned)
{
    Image *im;

    while (images)
    {
        /* deallocate colors */

        if (!cloned && images->npixels > 0)
            XFreeColors(display, colormap, images->pixels, images->npixels, 0);

        /* free pixmap and image structure */

        if (!cloned && images->pixmap != default_pixmap)
            XFreePixmap(display, images->pixmap);

        im = images;
        images = im->next;
        Free(im->url);

        if (im->npixels > 0)
            Free(im->pixels);

        Free(im);
    }
}
