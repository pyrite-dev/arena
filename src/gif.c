/*    gif.c - Dave Raggett, 16th December 1994
            - revised 9th Feb 1994 to speed decoding & add gif89 support

   Derived from the pbmplus package, so we include David's copyright.

   The colors are preallocated by image.c with 4:8:4 R:G:B colors and
   16 grey scales. These are rendered using an ordered dither based on
   16x16 dither matrices for monochrome, greyscale and color images.

   This approach ensures that we won't run out of colors regardless of
   how many images are on screen or how many independent copies of www
   are running at any time. The scheme is compatible with HP's MPOWER
   tools. The code will be extended to take advantage of 24 bit direct
   color or secondary true color hardware colormaps as soon as practical.
*/

/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, David Koblas.                                     | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */


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

#define MAX_LWZ_BITS    12

#define INTERLACE       0x40
#define LOCALCOLORMAP   0x80

#if 0 /* replaced by version reading from memory buffer */
#define ReadOK(file,buffer,len) (fread(buffer, len, 1, file) != 0)
#endif

#define BitSet(byte, bit) (((byte) & (bit)) == (bit))
#define LM_to_uint(a,b)   (((b)<<8)|(a))

extern Display *display;
extern int screen;
extern int depth;
extern Colormap colormap;
extern int imaging;  /* set to COLOR888, COLOR232, GREY4 or MONO */
extern int tileWidth, tileHeight;
extern unsigned char *tileData;
extern unsigned long windowColor;
extern unsigned long stdcmap[128];  /* 2/3/2 color maps for gifs etc */
extern unsigned long greymap[16];
extern debug; /* howcome 16/10/94 */
extern unsigned long transparent; /* howcome 30/10/94 */

static int GetDataBlock(Block *bp, unsigned char *buf);

/* defined in module dither */
extern int Magic256[256];
extern int Magic16[256];
extern int Magic32[256];
extern int Magic64[256];
extern unsigned char* gamma_table;

struct
    {
        unsigned int    Width;
        unsigned int    Height;
        Color           colors[MAXCOLORMAPSIZE];
        unsigned int    BitPixel;
        unsigned int    ColorResolution;
        unsigned int    Background;
        unsigned int    AspectRatio;
        int             xGreyScale;
    } GifScreen;

struct
    {
        int     transparent;
        int     delayTime;
        int     inputFlag;
        int     disposal;
    } Gif89 = { -1, -1, -1, 0 };

int verbose = FALSE;
int showComment = FALSE;
int ZeroDataBlock = FALSE;

size_t ReadOK(Block *bp, unsigned char *buffer, int len)
{
    if (bp->size > bp->next)
    {
        if (bp->next + len > bp->size)
            len = bp->size - bp->next;

        memcpy(buffer, bp->buffer + bp->next, len);
        bp->next += len;
        return len;
    }

    return 0;
}

/*
**  Pulled out of nextCode
*/
static  int             curbit, lastbit, get_done, last_byte;
static  int             return_clear;
/*
**  Out of nextLWZ
*/
static int      stack[(1<<(MAX_LWZ_BITS))*2], *sp;
static int      code_size, set_code_size;
static int      max_code, max_code_size;
static int      clear_code, end_code;

static void initLWZ(int input_code_size)
{
  /* janet 21/07/95: not used:        static int      inited = FALSE; */

        set_code_size = input_code_size;
        code_size     = set_code_size + 1;
        clear_code    = 1 << set_code_size ;
        end_code      = clear_code + 1;
        max_code_size = 2 * clear_code;
        max_code      = clear_code + 2;

        curbit = lastbit = 0;
        last_byte = 2;
        get_done = FALSE;

        return_clear = TRUE;

        sp = stack;
}

static int nextCode(Block *bp, int code_size)
{
        static unsigned char    buf[280];
        static int maskTbl[16] = {
                0x0000, 0x0001, 0x0003, 0x0007,
                0x000f, 0x001f, 0x003f, 0x007f,
                0x00ff, 0x01ff, 0x03ff, 0x07ff,
                0x0fff, 0x1fff, 0x3fff, 0x7fff,
        };
        int                     i, j, ret, end;

        if (return_clear) {
                return_clear = FALSE;
                return clear_code;
        }

        end = curbit + code_size;

        if (end >= lastbit) {
                int     count;

                if (get_done) {
                        if (curbit >= lastbit)
                        {
#if 0
                                ERROR("ran off the end of my bits" );
#endif
                        }
                        return -1;
                }
                buf[0] = buf[last_byte-2];
                buf[1] = buf[last_byte-1];

                if ((count = GetDataBlock(bp, &buf[2])) == 0)
                        get_done = TRUE;

                last_byte = 2 + count;
                curbit = (curbit - lastbit) + 16;
                lastbit = (2+count)*8 ;

                end = curbit + code_size;
        }

        j = end >>3 ;
        i = curbit >> 3;

        if (i == j)
                ret = (int)buf[i];
        else if (i + 1 == j)
                ret = (int)buf[i] | ((int)buf[i+1] << 8);
        else
                ret = (int)buf[i] | ((int)buf[i+1] << 8) | ((int)buf[i+2] << 16);

        ret = (ret >> (curbit & 7)) & maskTbl[code_size];

        curbit += code_size;

        return ret;
}

#define readLWZ(bp) ((sp > stack) ? *--sp : nextLWZ(bp))

static int nextLWZ(Block *bp)
{
        static int       table[2][(1<< MAX_LWZ_BITS)];
        static int       firstcode, oldcode;
        int              code, incode;
        register int     i;

        while ((code = nextCode(bp, code_size)) >= 0) {
               if (code == clear_code) {

                        /* corrupt GIFs can make this happen */
                        if (clear_code >= (1<<MAX_LWZ_BITS))
                        {
                                return -2;
                        }

                       for (i = 0; i < clear_code; ++i) {
                               table[0][i] = 0;
                               table[1][i] = i;
                       }
                       for (; i < (1<<MAX_LWZ_BITS); ++i)
                               table[0][i] = table[1][i] = 0;
                       code_size = set_code_size+1;
                       max_code_size = 2*clear_code;
                       max_code = clear_code+2;
                       sp = stack;
                        do {
                               firstcode = oldcode = nextCode(bp, code_size);
                        } while (firstcode == clear_code);

                        return firstcode;
               }
               if (code == end_code) {
                       int             count;
                       unsigned char   buf[260];

                       if (ZeroDataBlock)
                               return -2;

                       while ((count = GetDataBlock(bp, buf)) > 0)
                               ;

                       if (count != 0)
                        {
#if 0
                               INFO_MSG(("missing EOD in data stream (common occurence)"));
#endif
                        }
                       return -2;
               }

               incode = code;

               if (code >= max_code) {
                       *sp++ = firstcode;
                       code = oldcode;
               }

               while (code >= clear_code) {
                       *sp++ = table[1][code];
                       if (code == table[0][code])
                        {
#if 0
                               ERROR("circular table entry BIG ERROR");
                               return(code);
#endif
                        }
                       code = table[0][code];
               }

               *sp++ = firstcode = table[1][code];

               if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
                       table[0][code] = oldcode;
                       table[1][code] = firstcode;
                       ++max_code;
                       if ((max_code >= max_code_size) &&
                               (max_code_size < (1<<MAX_LWZ_BITS))) {
                               max_code_size *= 2;
                               ++code_size;
                       }
               }

               oldcode = incode;

               if (sp > stack)
                       return *--sp;
        }
        return code;
}


static int GetDataBlock(Block *bp, unsigned char *buf)
{
    unsigned char count;

    count = 0;

    if (! ReadOK(bp,&count,1))
    {
        fprintf(stderr, "error in getting DataBlock size\n");
        return -1;
    }

    ZeroDataBlock = count == 0;

    if ((count != 0) && (! ReadOK(bp, buf, count)))
    {
	if (VERBOSE_TRACE)
	    fprintf(stderr, "error in reading DataBlock\n");
        return -1;
    }

    return (int)(count);
}

static unsigned char *ReadImage_1_2_4(Block *bp, int len, int height,
     Color colors[MAXCOLORMAPSIZE], int grey, int interlace, int ignore, int depth)
{
    unsigned char   c;      
    int             v;
    int             xpos = 0, ypos = 0;		/* janet 21/07/95: not used: pass = 0 */
    unsigned char   *data, *dp;	/* janet 21/07/95: not used: *dy */
    int             cr, cg, cb, r, g, b, row, col;
    Color           color;
    int             ppb, bpl, shift =0, mask;

/* howcome added support for 1,2,4 depths */

    if (depth != 1 && depth !=2 && depth !=4) {
	fprintf(stderr,"sorry, images are not supported for depth %d\n",depth);
	return NULL;
    }

    ppb = 8/depth; /* pixels per byte */
    mask = ppb - 1;   /* 12-Mar-96 herman@htbrug.hobby.nl */
    bpl = len/ppb + (len&mask ? 1 : 0); /* bytes per line */
#if 0
    bpl = len/ppb + (len%ppb ? 1 : 0); /* bytes per line */
#endif


  /*
   *  Initialize the Compression routines
   */

    if (! ReadOK(bp,&c,1))
    {
	if (VERBOSE_TRACE)
	    fprintf(stderr, "EOF / read error on image data\n");
        return(NULL);
    }

    initLWZ(c);

   /*
    *  If this is an "uninteresting picture" ignore it.
    */

    if (ignore)
    {
        if (verbose)
            fprintf(stderr, "skipping image...\n" );

        while (readLWZ(bp) >= 0);

        return NULL;
    }

    data = (unsigned char *)malloc(bpl * height);

    if (data == NULL)
    {
        fprintf(stderr, "Cannot allocate space for image data\n");
        return(NULL);
    }

    if (IMAGE_TRACE)
        fprintf(stderr, "reading %d by %d%s GIF image\n",
                 len, height, interlace ? " interlaced" : "" );

    if (interlace)
    {
        int i;
        int pass = 0, step = 8;
        for (i = 0; i < height; ++i)
        {
	    int newbyte = 1;
            dp = &data[bpl*ypos]; /* howcome 7/2/95: changed len to bpl */
            col = ypos & 15;

            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;
		
		if(newbyte) {
		  *dp = 0;
		  newbyte = 0;
		}
		
                if (v == Gif89.transparent)
                {
		    shift = (((7 - (xpos & 7)) % ppb) * depth);
		    *dp |= transparent << shift;
		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }
                    continue;
                }

                color = colors[v];
                row = xpos & 15;

                if (!grey && imaging == COLOR232)
                {
                    if (color.grey > 0)
                        goto grey_color1;

                    cr = color.red;
                    cg = color.green;
                    cb = color.blue;

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


/*

Some quick notes to remeber how shift is calculated:

           (%8 = &7) -- 12-Mar-96 herman@htbrug.hobby.nl

0 1 2 3 4 5 6 7 8 9 	xpos
0 1 2 3 4 5 6 7 0 1     xpos %8
7 6 5 4 3 2 1 0 7 6     7 - (xpos % 8)  ( %8 = &7)

1 0 1 0 1 0 1 0 1 0     (7 - (xpos % 8)) % ppb               pixels perl byte = 2
4 0 4 0 4 0 4 0 4 0     ((7 - (xpos % 8)) % ppb) * depth     depth = 4

3 2 1 0 3 2 1                                                pixels per byte = 4
6 4 2 0 6 4 2                                                depth = 2

*/

		    shift = (((7 - (xpos & 7)) % ppb) * depth);
		    *dp |= (stdcmap[(r >> 6) | (g >> 3) | (b >> 1)]) << shift;
		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }

                    continue;
                }

                if (imaging == MONO)
                {

		    shift = (((7 - (xpos & 7)) % ppb) * depth);

                    if (color.grey < Magic256[(row << 4) + col])
                        *dp |= greymap[0] << shift;
                    else
                        *dp |= greymap[15] << shift;

		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }

                    continue;
                }

           grey_color1:

                cg  = color.grey;
                g = cg & 0xF0;

                if (cg - g > Magic16[(row << 4) + col])
                    g += 16;

                g = min(g, 0xF0);

		shift = (((7 - (xpos & 7)) % ppb) * depth);

                *dp |= greymap[g >> 4] << shift;

		if (shift == 0) {
		    dp++;
		    newbyte = 1;
		}
            }

            if ((ypos += step) >= height)
            {
                if (pass++ > 0)
                    step >>= 1;

                ypos = step >> 1;
            }
        }
    }
    else
    {
        dp = data;
        for (ypos = 0; ypos < height; ++ypos)
        {
	    int newbyte = 1;
            col = ypos & 15;

            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

		if(newbyte) {
		  *dp = 0;
		  newbyte = 0;
		}
		
                if (v == Gif89.transparent)
                {
		    shift = (((7 - (xpos & 7)) % ppb) * depth);
		    *dp |= transparent << shift;
		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }
                    continue;
                }

                color = colors[v];
                row = xpos & 15;

                if (!grey && imaging == COLOR232)
                {
                    if (color.grey > 0)
                        goto grey_color2;

                    cr = color.red;
                    cg = color.green;
                    cb = color.blue;

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

		    shift = (((7 - (xpos & 7)) % ppb) * depth);

                    *dp |= stdcmap[(r >> 6) | (g >> 3) | (b >> 1)] << shift;

		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }

                    continue;
                }

                if (imaging == MONO)
                {
		    shift = (((7 - (xpos & 7)) % ppb) * depth);

                    if (color.grey < Magic256[(row << 4) + col])
                        *dp |= greymap[0] << shift;
                    else
                        *dp |= greymap[15] << shift;

		    if (shift == 0) {
			dp++;
			newbyte = 1;
		    }

                    continue;
                }

            grey_color2:

                cg  = color.grey;
                g = cg & 0xF0;

                if (cg - g > Magic16[(row << 4) + col])
                    g += 16;

                g = min(g, 0xF0);

		shift = (((7 - (xpos & 7)) % ppb) * depth);

                *dp |= greymap[g >> 4] << shift;

		if (shift == 0) {
		    dp++;
		    newbyte = 1;
		}
            }
	    if (shift) {
		dp++;   /* make sure we start on a new byte for the next line */
		newbyte = 1;
	    }
        }
    }

 fini:

    if (readLWZ(bp) >= 0)
	if (debug) /* howcome 16/10/94 */
	    fprintf(stderr, "too much input data, ignoring extra...\n");

    return data;
}


static unsigned char *ReadImage(Block *bp, int len, int height,
     Color colors[MAXCOLORMAPSIZE], int grey, int interlace, int ignore)
{
    unsigned char   c;      
    int             v;
    int             xpos = 0, ypos = 0; /* janet 21/07/95: not used: pass = 0 */
    unsigned char   *data, *dp; /* janet 21/07/95: not used: *dy */
    int             cr, cg, cb, r, g, b, row, col;
    Color           color;

   /*
   *  Initialize the Compression routines
   */

    if (! ReadOK(bp,&c,1))
    {
	if (VERBOSE_TRACE)
	    fprintf(stderr, "EOF / read error on image data\n");
        return(NULL);
    }

    initLWZ(c);

   /*
    *  If this is an "uninteresting picture" ignore it.
    */

    if (ignore)
    {
        if (verbose)
            fprintf(stderr, "skipping image...\n" );

        while (readLWZ(bp) >= 0);

        return NULL;
    }

    data = (unsigned char *)malloc(len * height);

    if (data == NULL)
    {
        fprintf(stderr, "Cannot allocate space for image data\n");
        return(NULL);
    }

    if (IMAGE_TRACE)
        fprintf(stderr, "reading %d by %d%s GIF image\n",
                 len, height, interlace ? " interlaced" : "" );

    if (interlace)
    {
        int i;
        int pass = 0, step = 8;

        for (i = 0; i < height; ++i)
        {
            dp = &data[len*ypos];
            col = ypos & 15;

            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

                if (v == Gif89.transparent)
                {
                    dp = Transparent(dp, xpos, ypos);
                    continue;
                }

                color = colors[v];
                row = xpos & 15;

                if (!grey && imaging == COLOR232)
                {
                    if (color.grey > 0)
                        goto grey_color1;

                    cr = color.red;
                    cg = color.green;
                    cb = color.blue;

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
                    continue;
                }

                if (imaging == MONO)
                {
                    if (color.grey < Magic256[(row << 4) + col])
                        *dp++ = greymap[0];
                    else
                        *dp++ = greymap[15];
                    continue;
                }

           grey_color1:

                cg  = color.grey;
                g = cg & 0xF0;

                if (cg - g > Magic16[(row << 4) + col])
                    g += 16;

                g = min(g, 0xF0);
                *dp++ = greymap[g >> 4];
            }

            if ((ypos += step) >= height)
            {
                if (pass++ > 0)
                    step >>= 1;

                ypos = step >> 1;
            }
        }
    }
    else
    {
        dp = data;

        for (ypos = 0; ypos < height; ++ypos)
        {
            col = ypos & 15;

            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

                if (v == Gif89.transparent)
                {
                    dp = Transparent(dp, xpos, ypos);
                    continue;
                }

                color = colors[v];
                row = xpos & 15;

                if (!grey && imaging == COLOR232)
                {
                    if (color.grey > 0)
                        goto grey_color2;

                    cr = color.red;
                    cg = color.green;
                    cb = color.blue;

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
                    continue;
                }

                if (imaging == MONO)
                {
                    if (color.grey < Magic256[(row << 4) + col])
                        *dp++ = greymap[0];
                    else
                        *dp++ = greymap[15];
                    continue;
                }

            grey_color2:

                cg  = color.grey;
                g = cg & 0xF0;

                if (cg - g > Magic16[(row << 4) + col])
                    g += 16;

                g = min(g, 0xF0);
                *dp++ = greymap[g >> 4];
            }
        }
    }

 fini:

    if (readLWZ(bp) >= 0)
	if (VERBOSE_TRACE)
	    fprintf(stderr, "too much input data, ignoring extra...\n");

    return data;
}



static unsigned char *ReadImage24(Block *bp, int len, int height,
     Color colors[MAXCOLORMAPSIZE], int nColors, int interlace, int ignore)
{
    unsigned char   c;      
    int             v, trans;
    int             xpos = 0, ypos = 0; /* janet 21/07/95: not used: pass = 0 */
    unsigned char   *dp;
    unsigned char pixels[4*MAXCOLORMAPSIZE], *pp, *data24;
    unsigned long int ulp;

   /* setup pixel table for faster rendering */

    dp = (unsigned char *)&(pixels[0]);
    trans = Gif89.transparent;

    for (v = 0; v < nColors; ++v)
    {

        if (v == trans)
        {
            *dp++ = '\0';
            *dp++ = (windowColor >> 16) & 0xFF;
            *dp++ = (windowColor >>  8) & 0xFF;
            *dp++ = windowColor & 0xFF;
            continue;
        }

        /* From Scott Nelson <snelson@canopus.llnl.gov> 24bit color  1/12/94  Dec 1 1994 */

        GetColor(colors[v].red, colors[v].green, colors[v].blue, &ulp);
	
	*dp++ = '\0';
	*dp++ = (ulp >> 16) & 0xff; 
	*dp++ = (ulp >> 8) & 0xff; 
	*dp++ = ulp & 0xff; 
#if 0
        *dp++ = '\0';
        *dp++ = colors[v].red;
        *dp++ = colors[v].green;
        *dp++ = colors[v].blue;
#endif
    }

    if (!tileData)   /* default to window background */
        trans = -1;

  /*
   *  Initialize the Compression routines
   */

    if (! ReadOK(bp,&c,1))
    {
	if (VERBOSE_TRACE)
	    fprintf(stderr, "EOF / read error on image data\n");
        return(NULL);
    }

    initLWZ(c);

   /*
    *  If this is an "uninteresting picture" ignore it.
    */

    if (ignore)
    {
        if (VERBOSE_TRACE)
            fprintf(stderr, "skipping image...\n" );

        while (readLWZ(bp) >= 0);

        return NULL;
    }

    data24 = (unsigned char *)malloc(len * height * 4);

    if (data24 == NULL)
    {
        fprintf(stderr, "Cannot allocate space for image data\n");
        return(NULL);
    }

    if (verbose)
        fprintf(stderr, "reading %d by %d%s GIF image\n",
                 len, height, interlace ? " interlaced" : "" );

    if (interlace)
    {
        int i;
        int pass = 0, step = 8;

        for (i = 0; i < height; ++i)
        {
            pp = &data24[len*ypos*4];

            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

                if (v == trans)
                {
                    Transparent((unsigned char *)pp, xpos, ypos);
                    pp+=4;
                    continue;
                }

                *pp++ = pixels[4*v];
		*pp++ = pixels[4*v+1];
		*pp++ = pixels[4*v+2];
		*pp++ = pixels[4*v+3];
            }

            if ((ypos += step) >= height)
            {
                if (pass++ > 0)
                    step >>= 1;

                ypos = step >> 1;
            }
        }
    }
    else
    {
        pp = data24;

        for (ypos = 0; ypos < height; ++ypos)
        {
            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

                if (v == trans)
                {
                    Transparent((unsigned char *)pp, xpos, ypos);
                    pp+=4;
                    continue;
                }

                *pp++ = pixels[4*v];
		*pp++ = pixels[4*v+1];
		*pp++ = pixels[4*v+2];
		*pp++ = pixels[4*v+3];
            }
        }
    }

 fini:

    if (readLWZ(bp) >= 0)
        fprintf(stderr, "too much input data, ignoring extra...\n");

    return (unsigned char *)data24;
}

static unsigned char *ReadImage16(Block *bp, int len, int height,
     Color colors[MAXCOLORMAPSIZE], int nColors, int interlace, int ignore)
{
    unsigned char   c;      
    int             v, trans;
    int             xpos = 0, ypos = 0; /* janet 21/07/95: not used: pass = 0 */
    unsigned char   *dp;
    unsigned char pixels[2*MAXCOLORMAPSIZE];
    unsigned char *data16, *pp;
    unsigned long int ulp;

   /* setup pixel table for faster rendering */

    dp = (unsigned char *)&(pixels[0]);
    trans = Gif89.transparent;

    for (v = 0; v < nColors; ++v)
    {

        if (v == trans)
        {
            *dp++ = (windowColor >> 8) & 0xFF;
            *dp++ = (windowColor) & 0xFF;
            continue;
        }

         GetColor(colors[v].red, colors[v].green, colors[v].blue, &ulp);
        *dp++ = ((char*)&ulp)[1]; 
        *dp++ = ((char*)&ulp)[0];
    }

    if (!tileData)   /* default to window background */
        trans = -1;

  /*
   *  Initialize the Compression routines
   */

    if (! ReadOK(bp,&c,1))
    {
	if (VERBOSE_TRACE)
	    fprintf(stderr, "EOF / read error on image data\n");
        return(NULL);
    }

    initLWZ(c);

   /*
    *  If this is an "uninteresting picture" ignore it.
    */

    if (ignore)
    {
        if (VERBOSE_TRACE)
            fprintf(stderr, "skipping image...\n" );

        while (readLWZ(bp) >= 0);

        return NULL;
    }

    data16 = (unsigned char *)malloc(len * height * 2);

    if (data16 == NULL)
    {
        fprintf(stderr, "Cannot allocate space for image data\n");
        return(NULL);
    }

    if (verbose)
        fprintf(stderr, "reading %d by %d%s GIF image\n",
                 len, height, interlace ? " interlaced" : "" );

    if (interlace)
    {
        int i;
        int pass = 0, step = 8;

        for (i = 0; i < height; ++i)
        {
            pp = &data16[len*ypos*2];

            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

                if (v == trans)
                {
                    Transparent((unsigned char *)pp, xpos, ypos);
                    pp+=2;
                    continue;
                }

                *pp++ = pixels[2*v];
		*pp++ = pixels[2*v+1];
            }

            if ((ypos += step) >= height)
            {
                if (pass++ > 0)
                    step >>= 1;

                ypos = step >> 1;
            }
        }
    }
    else
    {
        pp = data16;

        for (ypos = 0; ypos < height; ++ypos)
        {
            for (xpos = 0; xpos < len; ++xpos)
            {
                if ((v = readLWZ(bp)) < 0)
                    goto fini;

                if (v == trans)
                {
                    Transparent((unsigned char *)pp, xpos, ypos);
                    pp +=2;
                    continue;
                }

                *pp++ = pixels[2*v];
		*pp++ = pixels[2*v+1];
            }
        }
    }

 fini:

    if (readLWZ(bp) >= 0)
        fprintf(stderr, "too much input data, ignoring extra...\n");

    return (unsigned char *)data16;
}

static int DoExtension(Block *bp, int label)
{
    static char buf[256];
    char *str;

    switch (label)
    {
        case 0x01:              /* Plain Text Extension */
            str = "Plain Text Extension";
            break;

        case 0xff:              /* Application Extension */
            str = "Application Extension";
            break;

        case 0xfe:              /* Comment Extension */
            str = "Comment Extension";
            while (GetDataBlock(bp, (unsigned char*) buf) != 0)
            {
                if (showComment)
                    fprintf(stderr, "gif comment: %s\n", buf);
            }
            return FALSE;

        case 0xf9:              /* Graphic Control Extension */
            str = "Graphic Control Extension";
            (void) GetDataBlock(bp, (unsigned char*) buf);
            Gif89.disposal    = (buf[0] >> 2) & 0x7;
            Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
            Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);

            if ((buf[0] & 0x1) != 0)
                Gif89.transparent = (int)((unsigned char)buf[3]);

            while (GetDataBlock(bp, (unsigned char*) buf) != 0)
                   ;
            return FALSE;

        default:
            str = buf;
            sprintf(buf, "UNKNOWN (0x%02x)", label);
            break;
    }

    /* fprintf(stderr, "got a '%s' extension\n", str); */

    while (GetDataBlock(bp, (unsigned char*) buf) != 0)
           ;

    return FALSE;
}

static int ReadColorMap(Block *bp, Image *image, int ncolors, Color *colors, int *grey)
{
    int i, flag, npixels, drb, dgb, dbr;
    unsigned char rgb[3];
    /* janet 21/07/95: not used:    unsigned long pixel, *pixels; */

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
#if 0
	/* Always gamma correct palette (if needed) */
/*	if(gamma_table) {*/
	colors->red = gamma_table[colors->red];
	colors->green = gamma_table[colors->green];
	colors->blue = gamma_table[colors->blue];
/*	}*/
#endif
     /* set grey value iff color is very close to grey (or GREY/MONO imaging) */

        drb = abs(rgb[1] - rgb[0]);
        dgb = abs(rgb[2] - rgb[1]);
        dbr = abs(rgb[0] - rgb[2]);

        if (*grey || (drb < 20 && dgb < 20 && dbr < 20))
        {   
            flag &= 1;
            colors->grey = (30*colors->red + 59*colors->green + 11*colors->blue)/100;
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

unsigned char *LoadGifImage(Image *image, Block *bp, unsigned int depth)
{
    unsigned char   buf[16];
    unsigned char   *data = NULL;
    unsigned char   c;
    Color           *cmap, colors[MAXCOLORMAPSIZE];
    int             useGlobalColormap;
    int             bitPixel;
    int             imageCount = 0;
    char            version[4];
    int             imageNumber = 1;
    /* janet 21/07/95: not used:    int             i; */
    int             greyScale = 0;
    /* janet 21/07/95: not used:    unsigned long pixel; */
    /* janet 21/07/95: not used:    unsigned int w, h; */

    verbose = FALSE;
    showComment = FALSE;

 /* initialize GIF89 extensions */

    Gif89.transparent = -1;
    Gif89.delayTime = -1;
    Gif89.inputFlag = -1;
    Gif89.disposal = 0;

    if (! ReadOK(bp,buf,6))
    {
        fprintf(stderr, "error reading magic number\n");
        return(NULL);
    }

    if (strncmp((char *)buf,"GIF",3) != 0)
    {
        if (verbose)
            fprintf(stderr, "not a GIF file\n");

        return(NULL);
    }

    strncpy(version, (char *)buf + 3, 3);
    version[3] = '\0';

    if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0))
    {
        fprintf(stderr, "bad version number, not '87a' or '89a'\n");
        return(NULL);
    }

    if (! ReadOK(bp,buf,7))
    {
        fprintf(stderr, "failed to read screen descriptor\n");
        return(NULL);
    }

    GifScreen.Width           = LM_to_uint(buf[0],buf[1]);
    GifScreen.Height          = LM_to_uint(buf[2],buf[3]);
    GifScreen.BitPixel        = 2<<(buf[4]&0x07);
    GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
    GifScreen.Background      = buf[5];
    GifScreen.AspectRatio     = buf[6];
    GifScreen.xGreyScale      = 0;

    if (BitSet(buf[4], LOCALCOLORMAP))
    {    /* Global Colormap */

        if (!ReadColorMap(bp, image, GifScreen.BitPixel, GifScreen.colors,
                                                     &GifScreen.xGreyScale))
        {
             fprintf(stderr, "error reading global colormap\n");
             return(NULL);
        }
    }

    if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49)
        fprintf(stderr, "Warning:  non-square pixels!\n");

    while (data == NULL)
    {
        if (! ReadOK(bp,&c,1))
        {
            fprintf(stderr, "EOF / read error on image data\n");
            return(NULL);
        }

        if (c == ';')
        {         /* GIF terminator */

            if (imageCount < imageNumber)
            {
                fprintf(stderr, "No images found in file\n");
                return(NULL);
            }
            break;
        }

        if (c == '!')
        {         /* Extension */

            if (! ReadOK(bp,&c,1))
            {
                fprintf(stderr, "EOF / read error on extention function code\n");
                return(NULL);
            }

            DoExtension(bp, c);
            continue;
        }

        if (c != ',')
        {         /* Not a valid start character */
            fprintf(stderr, "bogus character 0x%02x, ignoring\n", (int)c);
            continue;
        }

        ++imageCount;

        if (! ReadOK(bp,buf,9))
        {
            fprintf(stderr,"couldn't read left/top/width/height\n");
            return(NULL);
        }

        useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);

        bitPixel = 1<<((buf[8]&0x07)+1);

     /* Just set width/height for the imageNumber we are requesting */

        if (imageCount == imageNumber)
        {
            image->width = LM_to_uint(buf[4],buf[5]);
            image->height = LM_to_uint(buf[6],buf[7]);
        }

        if (! useGlobalColormap)
        {
            if (!ReadColorMap(bp, image, bitPixel, colors, &greyScale))
            {
                fprintf(stderr, "error reading local colormap\n");
                return(NULL);
            }

            cmap = colors;
        }
        else
        {
            cmap = GifScreen.colors;
            bitPixel = GifScreen.BitPixel;
            greyScale = GifScreen.xGreyScale;
        }

        if (depth == 24 || depth == 12)
        {
            data = ReadImage24(bp, LM_to_uint(buf[4],buf[5]),
                             LM_to_uint(buf[6],buf[7]), cmap, bitPixel,
                 BitSet(buf[8], INTERLACE), imageCount != imageNumber);
        } 
	else if (depth == 16)
	{
            data = ReadImage16(bp, LM_to_uint(buf[4],buf[5]),
                             LM_to_uint(buf[6],buf[7]), cmap, bitPixel,
                 BitSet(buf[8], INTERLACE), imageCount != imageNumber);
        }
        else if (depth == 1 || depth == 2 || depth == 4)
        {
            data = ReadImage_1_2_4(bp, LM_to_uint(buf[4],buf[5]),
                             LM_to_uint(buf[6],buf[7]), cmap, greyScale,
                 BitSet(buf[8], INTERLACE), imageCount != imageNumber, depth);
        }
	else
        {
            data = ReadImage(bp, LM_to_uint(buf[4],buf[5]),
                             LM_to_uint(buf[6],buf[7]), cmap, greyScale,
                 BitSet(buf[8], INTERLACE), imageCount != imageNumber);
        }

        if (imageCount != imageNumber && data != NULL)
        {
            Free(data);
            data = NULL;
        }
    }

    return(data);
}
