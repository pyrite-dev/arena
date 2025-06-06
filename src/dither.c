/*    dither.c 
	    - Dave Raggett, 16th December 1994
            - revised 9th Feb 1994 to speed decoding & add gif89 support
	    - separated from gif.c by Phill Hallam-Baker

   The colors are preallocated by image.c with 4:8:4 R:G:B colors and
   16 grey scales. These are rendered using an ordered dither based on
   16x16 dither matrices for monochrome, greyscale and color images.

   This approach ensures that we won't run out of colors regardless of
   how many images are on screen or how many independent copies of www
   are running at any time. The scheme is compatible with HP's MPOWER
   tools. The code will be extended to take advantage of 24 bit direct
   color or secondary true color hardware colormaps as soon as practical.
*/


#include <X11/Intrinsic.h>
#include <stdio.h>
#include <stdlib.h>

#include "www.h"
#include "image.h"

#define MAXCOLORMAPSIZE 256

#define TRUE    1
#define FALSE   0

#define CM_RED          0
#define CM_GREEN        1
#define CM_BLUE         2

#define INTERLACE       0x40
#define LOCALCOLORMAP   0x80


#define BitSet(byte, bit) (((byte) & (bit)) == (bit))
#define LM_to_uint(a,b)   (((b)<<8)|(a))

extern Display *display;
extern int screen;
extern int RPixelShift;
extern int depth;
extern Colormap colormap;
extern int imaging;  /* set to COLOR888, COLOR232, GREY4 or MONO */
extern int tileWidth, tileHeight;
extern unsigned char *tileData;
extern unsigned long windowColor;
extern unsigned long greymap[16];
extern debug; /* howcome 16/10/94 */
extern unsigned long transparent; /* howcome 30/10/94 */
extern BOOL Quiet; /* howcome 11/8/95 */
extern unsigned char *gamma_table;

extern unsigned long stdcmap[128];  /* 2/3/2 color maps for gifs etc */
unsigned long mycmap[256];


/* mycmap is an array containing X11 pixel values. The layout of the array is as follows:

    mycmap[0] = BlackPixel(display, screen);
    mycmap[127] = WhitePixel(display, screen);
    mycmap[1-126]: magic colors

    mycymap[128] = BlackPixel(display, screen);
    mycmap[143] = WhitePixel(display, screen);
    mycmap[129-142]: magic gray

    150-159: read only colors for application
    160-199: read only colors for current document

    200-209: read/write colors for application
    210-249: read/write colors for current document
    
*/

#define RO_COL_DOC_START 160
#define RO_COL_DOC_END   199

#define RW_COL_DOC_START 210
#define RW_COL_DOC_END   249


int Magic256[256] =    /* for halftoning */
{
    0, 223, 48, 207, 14, 237, 62, 221, 3, 226, 51, 210, 13, 236, 61, 220,
    175, 80, 128, 96, 189, 94, 141, 110, 178, 83, 130, 99, 188, 93, 140, 109,
    191, 32, 239, 16, 205, 46, 253, 30, 194, 35, 242, 19, 204, 45, 252, 29,
    112, 143, 64, 159, 126, 157, 78, 173, 115, 146, 67, 162, 125, 156, 77, 172,
    11, 234, 59, 218, 5, 228, 53, 212, 8, 231, 56, 215, 6, 229, 54, 213,
    186, 91, 138, 107, 180, 85, 132, 101, 183, 88, 135, 104, 181, 86, 133, 102,
    202, 43, 250, 27, 196, 37, 244, 21, 199, 40, 247, 24, 197, 38, 245, 22,
    123, 154, 75, 170, 117, 148, 69, 164, 120, 151, 72, 167, 118, 149, 70, 165,
    12, 235, 60, 219, 2, 225, 50, 209, 15, 238, 63, 222, 1, 224, 49, 208,
    187, 92, 139, 108, 177, 82, 129, 98, 190, 95, 142, 111, 176, 81, 128, 97,
    203, 44, 251, 28, 193, 34, 241, 18, 206, 47, 254, 31, 192, 33, 240, 17,
    124, 155, 76, 171, 114, 145, 66, 161, 127, 158, 79, 174, 113, 144, 65, 160,
    7, 230, 55, 214, 9, 232, 57, 216, 4, 227, 52, 211, 10, 233, 58, 217,
    182, 87, 134, 103, 184, 89, 136, 105, 179, 84, 131, 100, 185, 90, 137, 106,
    198, 39, 246, 23, 200, 41, 248, 25, 195, 36, 243, 20, 201, 42, 249, 26,
    119, 150, 71, 166, 121, 152, 73, 168, 116, 147, 68, 163, 122, 153, 74, 169

};

int Magic16[256] =    /* for 16 levels of gray */
{
    0, 13, 3, 12, 1, 14, 4, 13, 0, 13, 3, 12, 1, 14, 4, 13,
    10, 5, 8, 6, 11, 6, 8, 6, 10, 5, 8, 6, 11, 5, 8, 6,
    11, 2, 14, 1, 12, 3, 15, 2, 11, 2, 14, 1, 12, 3, 15, 2,
    7, 8, 4, 9, 7, 9, 5, 10, 7, 9, 4, 10, 7, 9, 5, 10,
    1, 14, 3, 13, 0, 13, 3, 12, 0, 14, 3, 13, 0, 13, 3, 13,
    11, 5, 8, 6, 11, 5, 8, 6, 11, 5, 8, 6, 11, 5, 8, 6,
    12, 3, 15, 2, 12, 2, 14, 1, 12, 2, 15, 1, 12, 2, 14, 1,
    7, 9, 4, 10, 7, 9, 4, 10, 7, 9, 4, 10, 7, 9, 4, 10,
    1, 14, 4, 13, 0, 13, 3, 12, 1, 14, 4, 13, 0, 13, 3, 12,
    11, 5, 8, 6, 10, 5, 8, 6, 11, 6, 8, 7, 10, 5, 8, 6,
    12, 3, 15, 2, 11, 2, 14, 1, 12, 3, 15, 2, 11, 2, 14, 1,
    7, 9, 4, 10, 7, 9, 4, 9, 7, 9, 5, 10, 7, 8, 4, 9,
    0, 14, 3, 13, 1, 14, 3, 13, 0, 13, 3, 12, 1, 14, 3, 13,
    11, 5, 8, 6, 11, 5, 8, 6, 11, 5, 8, 6, 11, 5, 8, 6,
    12, 2, 14, 1, 12, 2, 15, 1, 11, 2, 14, 1, 12, 2, 15, 2,
    7, 9, 4, 10, 7, 9, 4, 10, 7, 9, 4, 10, 7, 9, 4, 10

};

int Magic32[256] =    /* for 8 levels of green */
{
    0, 27, 6, 25, 2, 29, 8, 27, 0, 27, 6, 26, 2, 29, 7, 27,
    21, 10, 16, 12, 23, 11, 17, 13, 22, 10, 16, 12, 23, 11, 17, 13,
    23, 4, 29, 2, 25, 6, 31, 4, 24, 4, 29, 2, 25, 5, 31, 4,
    14, 17, 8, 19, 15, 19, 9, 21, 14, 18, 8, 20, 15, 19, 9, 21,
    1, 28, 7, 27, 1, 28, 6, 26, 1, 28, 7, 26, 1, 28, 7, 26,
    23, 11, 17, 13, 22, 10, 16, 12, 22, 11, 16, 13, 22, 10, 16, 12,
    25, 5, 30, 3, 24, 4, 30, 3, 24, 5, 30, 3, 24, 5, 30, 3,
    15, 19, 9, 21, 14, 18, 8, 20, 15, 18, 9, 20, 14, 18, 8, 20,
    1, 29, 7, 27, 0, 27, 6, 25, 2, 29, 8, 27, 0, 27, 6, 25,
    23, 11, 17, 13, 22, 10, 16, 12, 23, 12, 17, 13, 21, 10, 16, 12,
    25, 5, 31, 3, 23, 4, 29, 2, 25, 6, 31, 4, 23, 4, 29, 2,
    15, 19, 9, 21, 14, 18, 8, 20, 15, 19, 10, 21, 14, 18, 8, 19,
    1, 28, 7, 26, 1, 28, 7, 26, 0, 28, 6, 26, 1, 28, 7, 26,
    22, 11, 16, 12, 22, 11, 17, 13, 22, 10, 16, 12, 23, 11, 17, 13,
    24, 5, 30, 3, 24, 5, 30, 3, 24, 4, 30, 2, 24, 5, 30, 3,
    14, 18, 9, 20, 15, 19, 9, 20, 14, 18, 8, 20, 15, 19, 9, 21

};

int Magic64[256] =    /* for 4 levels of red and blue */
{
    0, 55, 12, 51, 3, 59, 15, 55, 1, 56, 13, 52, 3, 58, 15, 54,
    43, 20, 32, 24, 47, 23, 35, 27, 44, 20, 32, 24, 47, 23, 35, 27,
    47, 8, 59, 4, 51, 11, 63, 7, 48, 9, 60, 5, 50, 11, 62, 7,
    28, 35, 16, 39, 31, 39, 19, 43, 28, 36, 16, 40, 31, 39, 19, 43,
    3, 58, 15, 54, 1, 56, 13, 52, 2, 57, 14, 53, 1, 57, 13, 53,
    46, 22, 34, 26, 45, 21, 33, 25, 45, 22, 33, 26, 45, 21, 33, 25,
    50, 11, 62, 7, 48, 9, 60, 5, 49, 10, 61, 6, 49, 9, 61, 5,
    30, 38, 18, 42, 29, 37, 17, 41, 30, 37, 18, 41, 29, 37, 17, 41,
    3, 58, 15, 54, 0, 56, 12, 52, 4, 59, 16, 55, 0, 55, 12, 51,
    46, 23, 34, 27, 44, 20, 32, 24, 47, 23, 35, 27, 44, 20, 32, 24,
    50, 11, 62, 7, 48, 8, 60, 4, 51, 12, 63, 8, 47, 8, 59, 4,
    31, 38, 19, 42, 28, 36, 16, 40, 31, 39, 19, 43, 28, 36, 16, 40,
    2, 57, 14, 53, 2, 57, 14, 53, 1, 56, 13, 52, 2, 58, 14, 54,
    45, 21, 33, 25, 46, 22, 34, 26, 44, 21, 32, 25, 46, 22, 34, 26,
    49, 10, 61, 6, 49, 10, 61, 6, 48, 9, 60, 5, 50, 10, 62, 6,
    29, 37, 17, 41, 30, 38, 18, 42, 29, 36, 17, 40, 30, 38, 18, 42

};


Byte color_ix_start = 128, color_ix_end = 128;


unsigned long magic2color(Byte ix)
{
    int r, g, b;
    unsigned long color;
    

    r = (ix << 6) & 0xC0;
    g = (ix << 3) & 0xE0;
    b = (ix << 1) & 0xC0;

    GetColor(r, g, b, &color);
/*    fprintf(stderr,"color %d %d %d ->  %ld\n",r,g,b,color);*/
    return color;
}

Byte rgb2magic(int r, int g, int b)
{
    /* janet 21/07/95: not used:    int cr, cg, cb; */


    if (r == g && g == b) {
/*
	cr = Voltage2Brightness(r);
	return(128 | (cr >> 4));
*/
	return(128 | r >> 4);
    }
/*
    cr = Voltage2Brightness(r);
    cg = Voltage2Brightness(g);
    cb = Voltage2Brightness(b);
    
    r = cr & 0xC0;
    g = cg & 0xE0;
    b = cb & 0xC0;
*/
    return ((r >> 6) | (g >> 3) | (b >> 1));
}


long ix2color(int ix)
{
    return mycmap[ix];
}


Byte rgb2ix(int status, int r, int g, int b, Bool blink)
{
    XColor xc;
    /* janet 21/07/95: not used:    int cr, cg, cb; */
    unsigned long pixels[1];
    int i;
    unsigned long px;

    if (status)
	return (255);

    xc.red = r << 8;
    xc.green = g << 8;
    xc.blue = b << 8;
    xc.flags = DoRed | DoGreen | DoBlue;

    if (blink) {
	if (XAllocColorCells(display, colormap, False, NULL, 0, pixels, 1)) {
	    xc.pixel = pixels[0];
	    XStoreColor(display, colormap, &xc);

	    for(i = RW_COL_DOC_START; (i <= RW_COL_DOC_END) && (mycmap[i] != 0); i++)
		;
	    mycmap[i] = xc.pixel;
	    return(i);
	}
    }

    if (!XAllocColor(display, colormap, &xc)) {
        /* janet 21/07/95: not used:	Byte ix; */
	/* janet 21/07/95: not used:	unsigned long color; */
    
	if (VERBOSE_TRACE)
	    fprintf(stderr,"Can't allocate exact color for #%2.2x%2.2x%2.2x, ", r, g, b);

	r = min(r, 255) & 0xC0;
	g = min(g, 255) & 0xE0;
	b = min(b, 255) & 0xC0;

	if (VERBOSE_TRACE)
	    fprintf(stderr,"using substitute #%2.2x%2.2x%2.2x\n", r, g, b);

/*	fprintf(stderr,"rgb2ix: %d\n", ((r >> 6) | (g >> 3) | (b >> 1)));*/
	return ((r >> 6) | (g >> 3) | (b >> 1));
    }

    px = xc.pixel;

    for (i = 0; i < 144; i++) {
	if (mycmap[i] == px)
	    return(i);
    }

    for (i = RO_COL_DOC_START; (i <= RO_COL_DOC_END) && (mycmap[i] != 0); i++) {
	if (mycmap[i] == px) {
	    if (COLOR_TRACE)
		fprintf(stderr,"rgb2ix old: (%d %d %d) %d, %ld\n",r,g,b,i,px);
	    return(i);
	}
    }

    if (i <= RO_COL_DOC_END) {
	mycmap[i] = px;
	if (COLOR_TRACE)
	    fprintf(stderr,"rgb2ix new: (%d %d %d) %d, %ld\n",r, g, b, i,px);
	return (i);
    }

    fprintf(stderr,"rgb2ix ran out of colors.. \n");

    /* janet 21/07/95 function does not return value! */
    /* return ???; */ 
}

long rgb2color(int status, int r, int g, int b, Bool rw)
{
    return(ix2color(rgb2ix(status, r, g, b, rw)));
}



void rgbClear()
{
    int i;
    /* janet 21/07/95: not used:    XColor xca; */

    if (COLOR_TRACE)
	fprintf(stderr,"rgbClear\n");

    for (i = RO_COL_DOC_START; (i <= RO_COL_DOC_END) && (mycmap[i] != 0); i++) {
	if (COLOR_TRACE)
	    fprintf(stderr,"rgbClear %d %ld\n", i, mycmap[i]);
	XFreeColors( display, colormap, &mycmap[i], 1, 0L );
	mycmap[i]=0;
    }

}

static int ImageOutputScanlines_1_2_4 (output_image_data *output, 
			image_data *idata) {
  /*extern int ImageOutputScanlines_1_2_4 (output_image_data *output, 
			image_data *idata) { */

    /*janet 21/07/95: not used:    unsigned char   c;      */
    int             v;
    int             xpos = 0, ypos = 0, yindex; 	 /* janet 21/07/95: not used: pass = 0 */
    unsigned char   *dp, *scan_buffer; 			/* janet 21/07/95: not used: *dy */
    int             cr=0, cg=0, cb=0, r, g, b, row, col; /* 12-Mar-96 <herman@htbrug.hobby.nl> */
    /*  janet 21/07/95: not used:   Color           color; */
    int             ppb, bpl, shift=0;
    int             newbyte;

/* howcome added support for 1,2,4 depths */

    if (depth != 1 && depth !=2 && depth !=4) {
	fprintf(stderr,"sorry, images are not supported for depth %d\n",depth);
	return FALSE;
    }

    ppb = 8/depth; /* pixels per byte */
    bpl = output->bytes_per_row;
    newbyte = 1;
    
    if (TRUE) {
        dp = output->data + output->row * output->bytes_per_row;
        for (yindex = 0; yindex < idata->rows; yindex++) {
	    ypos = yindex + output->row;
	    scan_buffer = idata->buffer + (yindex * output->width * 3);

            col = ypos & 15;

            for (xpos = 0; xpos < idata->width; ++xpos) {
                row = xpos & 15;
		
		if (newbyte) {
		  *dp = 0;
		  newbyte = 0;
		}

                if (imaging == COLOR232) {

		    /* do we have to do grayscale like in ImageOutputScanlines_8 ?? */

		    switch (idata->components) {
		      case 1:
			cr = cg = cb = Voltage2Brightness(scan_buffer[xpos]);
/*			cr = cg = cb = gamma_table[scan_buffer[xpos]]; */
			break;
		      case 3:
			cr = Voltage2Brightness(scan_buffer [0+(xpos * 3)]);
			cg = Voltage2Brightness(scan_buffer [1+(xpos * 3)]);
			cb = Voltage2Brightness(scan_buffer [2+(xpos * 3)]);
#if 0
			cr = gamma_table[scan_buffer [0+(xpos * 3)]];
			cg = gamma_table[scan_buffer [1+(xpos * 3)]];
			cb = gamma_table[scan_buffer [2+(xpos * 3)]];
#endif
			break;
		      default:
			fprintf(stderr,"ImageOutputScanlines_1_2_4, components = %d\n",idata->components);
		    }

                    r = cr & 0xC0;
                    g = cg & 0xE0;
                    b = cb & 0xC0;

                    v = (row << 4) + col;

                    if (cr - r > Magic64[v])
                        r += 64;

                    if (cg - g > Magic32[v])
                        g += 32;

                    if (cb - b > Magic64[v])
                        b += 64;

                 /* clamp error to keep color in range 0 to 255 */

                    r = min(r, 255) & 0xC0;
                    g = min(g, 255) & 0xE0;
                    b = min(b, 255) & 0xC0;


		    /* howcome added support for <8 bits 25/1/95 */

		    shift = (((7 - (xpos % 8)) % ppb) * depth);
		    *dp |= (stdcmap[(r >> 6) | (g >> 3) | (b >> 1)]) << shift;
		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }
                }

                else if (imaging == MONO) {

		    switch (idata->components) {
		      case 1:
			cg = Voltage2Brightness(scan_buffer[xpos]);
/*			cg = gamma_table[scan_buffer[xpos]]; */
			break;
		      case 3:
			cg = Voltage2Brightness(
						(0.59*scan_buffer [0+(xpos * 3)]) +
						(0.20*scan_buffer [1+(xpos * 3)]) +
						(0.11*scan_buffer [2+(xpos * 3)]));
#if 0			
			cg = gamma_table[(int)
						((0.59*scan_buffer [0+(xpos * 3)]) +
						(0.20*scan_buffer [1+(xpos * 3)]) +
						(0.11*scan_buffer [2+(xpos * 3)]))];
#endif
			break;
		      default:
			fprintf(stderr,"ImageOutputScanlines_8, components = %d\n",idata->components);
		    }

		    shift = (((7 - (xpos % 8)) % ppb) * depth);

                    if (cg < Magic256[(row << 4) + col])
                        *dp |= greymap[0] << shift;   /* was *dp++ 17-jan-96 */
                    else
                        *dp |= greymap[15] << shift;  /* idem */

		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }
                }

		else {

		    cg = Voltage2Brightness(
			(0.59*scan_buffer [0+(xpos * 3)]) +
			(0.20*scan_buffer [1+(xpos * 3)]) +
			(0.11*scan_buffer [2+(xpos * 3)]));
#if 0
		   cg = gamma_table[(int)
			((0.59*scan_buffer [0+(xpos * 3)]) +
			(0.20*scan_buffer [1+(xpos * 3)]) +
			(0.11*scan_buffer [2+(xpos * 3)]))];
#endif
                    g = cg & 0xF0;

                    if (cg - g > Magic16[(row << 4) + col]) {
                        g += 16;
			}
                    g = min(g, 0xF0);

		    shift = (((7 - (xpos % 8)) % ppb) * depth);

                    *dp++ |= greymap[g >> 4] << shift;

		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }

		}
	    }

	    if (shift) {
		dp++;   /* make sure we start on a new byte for the next line */
		newbyte = 1;
	    }

        }
    }
    return TRUE;
}

static int ImageOutputScanlines_8 (output_image_data *output, 
			image_data *idata) {

  /*extern int ImageOutputScanlines_8 (output_image_data *output, 
			image_data *idata) { */

    /* janet 21/07/95: not used:  unsigned char   c;   */   
    int             v;
    int             xpos = 0, ypos = 0,  yindex; /* janet 21/07/95: not used: pass = 0 */
    unsigned char   *dp, *scan_buffer; /* janet 21/07/95: not used: *dy */
    int             cr=0, cg=0, cb=0, r, g, b, row, col;
    /* janet 21/07/95: not used:     Color           color; */

    if (TRUE) {
        dp = output->data + output->row * output->bytes_per_row;

	if (IMAGE_TRACE)
	    fprintf(stderr,"ImageOutputScanlines_8 %d %d\n", output->row, output->bytes_per_row);

        for (yindex = 0; yindex < idata->rows; yindex++) {
	    ypos = yindex + output->row;
	    scan_buffer = idata->buffer + (yindex * output->width * idata->components); /* howcome 19/2/95: 3 -> idata->components */

	    if (IMAGE_TRACE)
		fprintf(stderr,"ImageOutputScanlines: y %d scanbuffer %lx\n",yindex, scan_buffer);

            col = ypos & 15;

            for (xpos = 0; xpos < idata->width; ++xpos) {
                row = xpos & 15;

                if (imaging == COLOR232) {

		    switch (idata->components) {

			/* do grayscale */
			
		      case 1:
			cr = cg = cb = Voltage2Brightness(scan_buffer[xpos]);
/*			cr = cg = cb = gamma_table[scan_buffer[xpos]]; */
			g = cg & 0xF0;
			if (cg - g > Magic16[(row << 4) + col])
			    g += 16;
			g = min(g, 0xF0);
			*dp++ = greymap[g >> 4];
			break;

		      case 3:
			cr = Voltage2Brightness(scan_buffer [0+(xpos * 3)]);
			cg = Voltage2Brightness(scan_buffer [1+(xpos * 3)]);
			cb = Voltage2Brightness(scan_buffer [2+(xpos * 3)]);
#if 0
			cr = gamma_table[scan_buffer [0+(xpos * 3)]];
			cg = gamma_table[scan_buffer [1+(xpos * 3)]];
			cb = gamma_table[scan_buffer [2+(xpos * 3)]];
#endif
			r = cr & 0xC0;
			g = cg & 0xE0;
			b = cb & 0xC0;

			v = (row << 4) + col;

			if (cr - r > Magic64[v])
			    r += 64;

			if (cg - g > Magic32[v])
			    g += 32;

			if (cb - b > Magic64[v])
			    b += 64;

			/* clamp error to keep color in range 0 to 255 */

			r = min(r, 255) & 0xC0;
			g = min(g, 255) & 0xE0;
			b = min(b, 255) & 0xC0;

			*dp++ = stdcmap[(r >> 6) | (g >> 3) | (b >> 1)];

			break;
		      default:
			fprintf(stderr,"ImageOutputScanlines_8, components = %d\n",idata->components);
		    }
                }

                else if (imaging == MONO) {

		    switch (idata->components) {
		      case 1:
			cg = Voltage2Brightness(scan_buffer[xpos]);
			cg = gamma_table[scan_buffer[xpos]];
			break;
		      case 3:
			cg = Voltage2Brightness(
						(0.59*scan_buffer [0+(xpos * 3)]) +
						(0.20*scan_buffer [1+(xpos * 3)]) +
						(0.11*scan_buffer [2+(xpos * 3)]));
#if 0			
			cg = gamma_table[(int)
						((0.59*scan_buffer [0+(xpos * 3)]) +
						(0.20*scan_buffer [1+(xpos * 3)]) +
						(0.11*scan_buffer [2+(xpos * 3)]))];
#endif	
			break;
		      default:
			fprintf(stderr,"ImageOutputScanlines_8, components = %d\n",idata->components);
		    }

                    if (cg < Magic256[(row << 4) + col])
                        *dp++ = greymap[0];
                    else
                        *dp++ = greymap[15];
                }

		else {
		    switch (idata->components) {
		      case 1:
			cg = Voltage2Brightness(scan_buffer[xpos]);
/*			cg = gamma_table[scan_buffer[xpos]]; */
			break;
		      case 3:
			cg = Voltage2Brightness(
						(0.59*scan_buffer [0+(xpos * 3)]) +
						(0.20*scan_buffer [1+(xpos * 3)]) +
						(0.11*scan_buffer [2+(xpos * 3)]));
#if 0
			cg = gamma_table[(int)
						((0.59*scan_buffer [0+(xpos * 3)]) +
						(0.20*scan_buffer [1+(xpos * 3)]) +
						(0.11*scan_buffer [2+(xpos * 3)]))];
#endif
			break;
		      default:
			fprintf(stderr,"ImageOutputScanlines_8, components = %d\n",idata->components);
		    }

                    g = cg & 0xF0;

                    if (cg - g > Magic16[(row << 4) + col]) {
                        g += 16;
			}
                    g = min(g, 0xF0);
                    *dp++ = greymap[g >> 4];
		}
            }
        }
    }
    return TRUE; 	/* janet: added. should it return other value? */
}

static int ImageOutputScanlines_24 (output_image_data *output, 
			image_data *idata) {
/*
extern int ImageOutputScanlines_24 (output_image_data *output, 
			image_data *idata) {		*/

  /* janet 24/07/95: not used:    unsigned char   c; */
  /* janet 24/07/95: not used:    int             v; */
    int             xpos = 0, ypos = 0, yindex; /* janet 21/07/95: not used: pass = 0 */
    unsigned char   *dp, *scan_buffer; /* janet 21/07/95: not used: *dy */
    int             row, col; /* janet 21/07/95: not used: cr, cg, cb, r, g, b */
    /* janet 21/07/95: not used:    Color           color; */

    if (TRUE) {	
        dp = output->data + output->row * output->bytes_per_row;

	if (IMAGE_TRACE)
	    fprintf(stderr,"ImageOutputScanlines_24 %d %d\n", output->row, output->bytes_per_row);

        for (yindex = 0; yindex < idata->rows; yindex++) {
	    ypos = yindex + output->row;
	    scan_buffer = idata->buffer + (yindex * output->width * 3);

	    if (IMAGE_TRACE)
		fprintf(stderr,"ImageOutputScanlines_24: y %d scanbuffer %lx\n",yindex, scan_buffer);

            col = ypos & 15;

            for (xpos = 0; xpos < idata->width; ++xpos) {
                row = xpos & 15;

/*
                cr = scan_buffer [0+(xpos * 3)];
                cg = scan_buffer [1+(xpos * 3)];
                cb = scan_buffer [2+(xpos * 3)];
*/

        	*dp++ = '\0';
		switch (idata->components) {
		  case 1:
		    *dp++ = scan_buffer [xpos];
		    *dp++ = scan_buffer [xpos];
		    *dp++ = scan_buffer [xpos];
		    break;
		  case 3:
		    *dp++ = scan_buffer [((RPixelShift) ? 0 : 2)+(xpos*3)];
		    *dp++ = scan_buffer [1+(xpos*3)];
		    *dp++ = scan_buffer [((RPixelShift) ? 2 : 0)+(xpos*3)];
		    break;
		  default:
		    fprintf(stderr,"ImageOutputScanlines_8, components = %d\n",idata->components);
		}
            }
        }
    }
    return TRUE;	/* janet 1/8/95: added. should it return other value? */
} 

static int ImageOutputScanlines_16 (output_image_data *output, 
			image_data *idata) {

    int             xpos = 0, ypos = 0, yindex;
    unsigned char   *dp, *scan_buffer;
    int             row, col, cr, cg, cb; 
    long            ulp;
  
    if (TRUE) {	
        dp = output->data + output->row * output->bytes_per_row;

	if (IMAGE_TRACE)
	    fprintf(stderr,"ImageOutputScanlines_16 %d %d\n", output->row, output->bytes_per_row);

        for (yindex = 0; yindex < idata->rows; yindex++) {
	    ypos = yindex + output->row;
	    scan_buffer = idata->buffer + (yindex * output->width * 3);

	    if (IMAGE_TRACE)
		fprintf(stderr,"ImageOutputScanlines_24: y %d scanbuffer %lx\n",yindex, scan_buffer);

            col = ypos & 15;

            for (xpos = 0; xpos < idata->width; ++xpos) {
                row = xpos & 15;

/*
                cr = scan_buffer [0+(xpos * 3)];
                cg = scan_buffer [1+(xpos * 3)];
                cb = scan_buffer [2+(xpos * 3)];
		*/

		switch (idata->components) {
		  case 1:
		      cr = cg = cb = scan_buffer [xpos];
		      GetColor(cr, cg, cb, &ulp);
		      *dp++ = ((char*)&ulp)[1]; 
		      *dp++ = ((char*)&ulp)[0];
		      break;
		  case 3:
		      cr = scan_buffer [0+(xpos * 3)];
		      cg = scan_buffer [1+(xpos * 3)];
		      cb = scan_buffer [2+(xpos * 3)];
		      GetColor(cr, cg, cb, &ulp);
		      *dp++ = ((char*)&ulp)[1]; 
		      *dp++ = ((char*)&ulp)[0];
		      break; 
		default:
		    fprintf(stderr,"ImageOutputScanlines_16, components = %d\n",idata->components);
		}
            }
        }
    }
    return TRUE;	/* janet 1/8/95: added. should it return other value? */
} 

#ifdef NEVER
static int ReadColorMap(Block *bp, Image *image, int ncolors, Color *colors, int *grey)
{
    int i, flag, npixels, drb, dgb, dbr;
    unsigned char rgb[3];
    unsigned long pixel, *pixels;

    flag = 1;
    image->npixels = npixels = 0;

    if (imaging == GREY4 || imaging == MONO)
        *grey = flag = 1;

    for (i = 0; i < ncolors; ++i)
    {
        if (! ReadOK(bp, rgb, sizeof(rgb)))
        {
            fprintf(stderr, "bad colormap\n");
            return 0;
        }

        colors->red = rgb[0];
        colors->green = rgb[1];
        colors->blue = rgb[2];

     /* apply gamma correction to map voltages to brightness values */

        if (imaging != COLOR888)
        {
            colors->red = Voltage2Brightness(colors->red);
            colors->green = Voltage2Brightness(colors->green);
            colors->blue = Voltage2Brightness(colors->blue);
        }

     /* set grey value iff color is very close to grey (or GREY/MONO imaging) */

        drb = abs(rgb[1] - rgb[0]);
        dgb = abs(rgb[2] - rgb[1]);
        dbr = abs(rgb[0] - rgb[2]);

        if (*grey || (drb < 20 && dgb < 20 && dbr < 20))
        {   
            flag &= 1;
            colors->grey = (3*colors->red + 6*colors->green + colors->blue)/10;
        }
        else
        {
            if (!(*grey))
                flag = 0;

            colors->grey = 0;
        }

        ++colors;
    }

    *grey = flag;
    return 1;
}
#endif


extern int ImageOutputInit (void *output_in, image_data *data) { 
  /* janet 24/07/95: not used: extern Image *image; */

    output_image_data *output = (output_image_data *) output_in;
    int		blocksize;
    /* janet 24/07/95: not used:     int		i; */

    output->height = data->height;
    output->width = data->width;
    output->row = 0;

    if ((output->depth == 1) || (output->depth == 2) || (output->depth == 4)) {
	int 	ppb;

        ppb  = 8/output->depth;
        output->bytes_per_row = data->width/ppb + (data->width%ppb ? 1 : 0); 

/*
	fprintf(stderr,"sorry, jpeg images are not supported for depth %d\n",depth);
	output->data = NULL;
	return NULL;
*/
	}
    else if (output->depth == 8) {
	output->bytes_per_row = 1 * data->width;
    }
    else if (output->depth == 16) {
        output->bytes_per_row = 2 * data->width;
    }  
    else if (output->depth == 24) {
        output->bytes_per_row = 4 * data->width;  /* 4 ???? */
    }    
    else {
	fprintf(stderr,"sorry, images are not supported for depth %d (%d)\n",depth, output->depth);
	output->data = NULL;
	return FALSE;
    }

    blocksize = output->bytes_per_row * data->height;
    output->data = malloc (blocksize);

    return TRUE; /* janet 1/8/95: added. */
} 


extern int ImageOutputScanlines (void *output_in, image_data *data) { 
    output_image_data *output = (output_image_data *) output_in;

    if (output->data == NULL) {
	return FALSE;
	}

    else if ((output->depth == 1) || (output->depth == 2) || (output->depth == 4)) {
        ImageOutputScanlines_1_2_4 (output, data);
	output->row = output->row + data->rows;
	}
    else if (output->depth == 8) {
        ImageOutputScanlines_8 (output, data);
	output->row = output->row + data->rows;
	}    
    else if (output->depth == 24) {
        ImageOutputScanlines_24 (output, data);
	output->row = output->row + data->rows;	/* howcome 9/2/94: phill forgot this one */
	} 
    else if (output->depth == 16) {
        ImageOutputScanlines_16 (output, data);
	output->row = output->row + data->rows;
	} 
    else {
	fprintf(stderr,"sorry, images are not supported for depth %d\n",depth);
	return FALSE;
	}
    return TRUE; 	/* janet 1/8/95: added. */
}













