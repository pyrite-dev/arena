#include <stdio.h>

#define Byte WWWByte  
#include "png.h"  
#include "www.h" 

extern Visual *visual;
extern int RPixelShift;   /* GRR */
extern int GPixelShift;   /* GRR */
extern int BPixelShift;   /* GRR */
extern int RPixelMask, GPixelMask, BPixelMask;
extern int depth;         /* Depth of display in bits */
extern Colormap colormap;
extern int imaging;       /* set to COLOR888, COLOR232, GREY4 or MONO */
extern int tileWidth, tileHeight;
extern unsigned char *tileData;
extern unsigned long windowColor;
extern unsigned long stdcmap[128];  /* 2/3/2 color maps for gifs etc */
extern unsigned long greymap[16];
extern debug;
extern unsigned long transparent;
extern double Gamma;
extern XColor papercols[256];
extern unsigned char *gamma_table; /* For gamma correcting paper */

/* defined in module dither */
extern int Magic256[256];
extern int Magic16[256];
extern int Magic32[256];
extern int Magic64[256];

/*png_rw_ptr */
static void
arena_png_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
  Block *bp=(Block*)png_get_io_ptr(png_ptr);
  
  if (bp->size > bp->next) {
    if (bp->next + length > bp->size)
      length = bp->size - bp->next;
    
    memcpy(data, bp->buffer + bp->next, length);
    bp->next += length;
    return;
  } else {
    png_error(png_ptr, "Read Error");
  }
}

static void
arena_prog_png_read_data(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
  Block *bp=(Block*)png_get_io_ptr(png_ptr);
  
  if (bp->size > bp->next) {
    if (bp->next + length > bp->size)
      length = bp->size - bp->next;
    
    memcpy(data, bp->buffer + bp->next, length);
    bp->next += length;
    return;
  } else {
      memset(data, 0, length);
      return;
  }
}

void
png_error(png_struct *png_ptr, char *message)
{
  fprintf(stderr,"libpng error: %s\n", message);
  /* Warn("libpng error: %s\n", message); */
  longjmp(png_ptr->jmpbuf, 1);
}

void
png_warning(png_struct *png_ptr, char *message)
{
  if (!png_ptr)
    return;

  fprintf(stderr,"libpng warning: %s\n", message);
  /* Warn("libpng warning: %s\n", message); */
}


/* Read image into displays 1,2 or 4 bit deep */
unsigned char *ReadPNGImage_1_2_4(png_struct *png_ptr, png_info *info_ptr, png_byte *png_image) 
{
  png_byte *pp;
  unsigned char *data, *dp;
  int ypos;
  int ppb, bpl;
  int len=info_ptr->width;
  int color_type=info_ptr->color_type;
  int alpha;
  int newbyte;
  
  ppb = 8/depth; /* pixels per byte */
  bpl = len/ppb + (len%ppb ? 1 : 0); /* bytes per line */
  
  data = (unsigned char *)malloc(bpl * info_ptr->height);
  if (data == NULL)
    png_error (png_ptr, "couldn't alloc space for X image data");
  
  /* Remove alpha channel from type,  but remember it */
  alpha=color_type & PNG_COLOR_MASK_ALPHA;
  color_type &= ~PNG_COLOR_MASK_ALPHA;
  
  pp=png_image;
  dp=data;
  newbyte = 1;
  for (ypos = 0; ypos < png_ptr->height; ypos++)
    {
      int col = ypos & 0x0f;
      int shift=0;
      int xpos;
      
      for (xpos = 0; xpos < len; ++xpos)
	{
	  int row = xpos & 0x0f;
	  int gr;
	  int cr=0, cg=0, cb=0, cgr, a;
	  int isgrey = 0;
	  
	  if (newbyte) {
	    *dp = 0;
	    newbyte = 0;
	  }
  
/*
(From gif.c)
Some quick notes to remember how shift is calculated:

0 1 2 3 4 5 6 7 8 9 	xpos
0 1 2 3 4 5 6 7 0 1     xpos %8
7 6 5 4 3 2 1 0 7 6     7 - (xpos % 8)

1 0 1 0 1 0 1 0 1 0     (7 - (xpos % 8)) % ppb               pixels perl byte = 2
4 0 4 0 4 0 4 0 4 0     ((7 - (xpos % 8)) % ppb) * depth     depth = 4

3 2 1 0 3 2 1                                                pixels per byte = 4
6 4 2 0 6 4 2                                                depth = 2

*/
	  shift = (((7 - (xpos % 8)) % ppb) * depth);
	  
	  if (color_type == PNG_COLOR_TYPE_GRAY) {
	    cr=cg=cb=cgr=(*pp++);
	    isgrey=1;
	  } else if (color_type == PNG_COLOR_TYPE_RGB) {
	    int drb, dgb, dbr;
	    
	    cr= (*pp++);
	    cg= (*pp++);
	    cb= (*pp++);
	    cgr = (0.59*cr) + (0.20*cg) + (0.11*cb);

	    drb = abs(cr - cb);
	    dgb = abs(cg - cb);
	    dbr = abs(cb - cr);
	    if (drb < 20 && dgb < 20 && dbr < 20)
	      isgrey=1;
	  } else {
	    png_error(png_ptr, "Unknown PNG color type seen (ReadPNGImage_1_2_4)");
	  }
	  
	  /* Process alpha channel if present */
	  if (alpha)
	    a=(*pp++);
	  
	  if (alpha && a != 0xff) {
	    int tr,tg,tb;
	    int drb, dgb, dbr;
	    
	    /* From www.c - sorry! */
	    if(depth==1) {
	      tr=255; tg=255; tb=255;
	    } else {
	      tr=220; tg=209; tb=186;
	    }
      
            /* Add picture pixel to background pixel */
	    cr=(a/255.0)*cr + ((255.0-a)/255.0) * tr;
	    cg=(a/255.0)*cg + ((255.0-a)/255.0) * tg;
	    cb=(a/255.0)*cb + ((255.0-a)/255.0) * tb;
	    
	    /* Recalculate grayness */
	    cgr = (0.59*cr) + (0.20*cg) + (0.11*cb);
	    isgrey=0;
	    drb = abs(cr - cb);
	    dgb = abs(cg - cb);
	    dbr = abs(cb - cr);
	    if (drb < 20 && dgb < 20 && dbr < 20)
	      isgrey=1;
	  }
	  
	  if (alpha && !a)
	    *dp |= transparent << shift;
	  else if (imaging == COLOR232)
	    {
	      if (isgrey) {
                int gr = cgr & 0xF0;

                if (cgr - gr > Magic16[(row << 4) + col])
                    gr += 16;

                gr = min(gr, 0xF0);
                
		*dp |= greymap[gr >> 4] << shift;
	      } else {
		int r = cr & 0xC0;
		int g = cg & 0xE0;
		int b = cb & 0xC0;
		int v = (row << 4) + col;

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
		  
		*dp |= stdcmap[(r >> 6) | (g >> 3) | (b >> 1)] << shift;
	      }
	    }
	  else if (imaging == MONO)
	    {
	      if (cgr < Magic256[(row << 4) + col])
		*dp |= greymap[0] << shift;
	      else
		*dp |= greymap[15] << shift;
	    }
	  else /* Not COLOR232 or MONO */
	    {
	      gr = cgr & 0xF0;
	      if (cgr - gr > Magic16[(row << 4) + col])
		gr += 16;

	      gr = min(gr, 0xF0);
	      *dp |= greymap[gr >> 4] << shift;
	    }
	  
	  if (shift == 0) {
	    dp++;
	    newbyte = 1;
	  }
	  
	} /* xpos */
      
      if (shift) {
	  dp++;   /* make sure we start on a new byte for the next line */
	  newbyte = 1;
	}
    } /* ypos */
  
  return(data);
  
}


/* Read image into displays 24 bit deep */
unsigned char *ReadPNGImage_24(png_struct *png_ptr, png_info *info_ptr, png_byte *png_image) 
{
  png_byte *pp;
  unsigned char *data, *dp;
  unsigned long int ulp;
  int ypos;
  int len=info_ptr->width;
  int color_type=info_ptr->color_type;
  int alpha;
  
  data = (unsigned char *)malloc(4 * len * info_ptr->height);
  if (data == NULL)
    png_error (png_ptr, "couldn't alloc space for X image data");
  
  /* Remove alpha channel from type,  but remember it */
  alpha=color_type & PNG_COLOR_MASK_ALPHA;
  color_type &= ~PNG_COLOR_MASK_ALPHA;
  
  pp=png_image;
  dp=data;
  for (ypos = 0; ypos < png_ptr->height; ypos++)
    {
      int xpos;
      
      for (xpos = 0; xpos < len; ++xpos)
	{
	  int cr, cg, cb, cgr, a;
	  
	  if (color_type == PNG_COLOR_TYPE_GRAY) {
	    cr=cg=cb=cgr=(*pp++);
	  } else if (color_type == PNG_COLOR_TYPE_RGB) {
	    cr= (*pp++);
	    cg= (*pp++);
	    cb= (*pp++);
	    cgr = (0.59*cr) + (0.20*cg) + (0.11*cb);
	  } else {
	    png_error(png_ptr, "Unknown PNG color type seen (ReadPNGImage_24)");
	  }
	  
	  /* Process alpha channel if present */
	  if (alpha)
	    a=(*pp++);
	  
	   /* Assume COLOR888 */
	  if (alpha && a != 0xff)
	    {
	      int tr,tg,tb;
	    
	      /* Get tile colour without gamma - from www.c */
	      if (tileData)
		{
		  int x = xpos % tileWidth;
		  int y = ypos % tileHeight;
		  int i = y * tileWidth + x;
		  unsigned char *s = tileData + 4 * i;
		  tr=s[(RPixelShift) ? 1 : 3];
		  tg=s[2];
		  tb=s[(RPixelShift) ? 3 : 1];
		}
	      else
		{
		  tr = (transparent  >> 16) & 0xFF;
		  tg = (transparent  >> 8) & 0xFF;
		  tb = transparent & 0xFF;
		}
	      
	      /* Add picture pixel to background pixel */
	      cr=(a/255.0)*cr + ((255.0-a)/255.0) * tr;
	      cg=(a/255.0)*cg + ((255.0-a)/255.0) * tg;
	      cb=(a/255.0)*cb + ((255.0-a)/255.0) * tb;
	    }

	  GetPngColor(cr, cg, cb, &ulp);
	  *dp++ = '\0';
	  *dp++ = (ulp >> 16) & 0xff; 
	  *dp++ = (ulp >> 8) & 0xff; 
	  *dp++ = ulp & 0xff; 
	  
	} /* xpos */
      
    } /* ypos */      
  
  return data;
}

/* Read image into displays 16 bit deep */
unsigned char *ReadPNGImage_16(png_struct *png_ptr, png_info *info_ptr, png_byte *png_image) 
{
  png_byte *pp;
  unsigned char *data, *dp;
  unsigned long int ulp;
  int ypos;
  int len=info_ptr->width;
  int color_type=info_ptr->color_type;
  int alpha;
  
  data = (unsigned char *)malloc(2 * len * info_ptr->height);
  if (data == NULL)
    png_error (png_ptr, "couldn't alloc space for X image data");
  
  /* Remove alpha channel from type,  but remember it */
  alpha=color_type & PNG_COLOR_MASK_ALPHA;
  color_type &= ~PNG_COLOR_MASK_ALPHA;
  
  pp=png_image;
  dp=data;
  for (ypos = 0; ypos < png_ptr->height; ypos++)
    {
      int xpos;
      
      for (xpos = 0; xpos < len; ++xpos)
	{
	  int cr=0, cg=0, cb=0, cgr, a;
	  
	  if (color_type == PNG_COLOR_TYPE_GRAY) {
	    cr=cg=cb=cgr=(*pp++);
	  } else if (color_type == PNG_COLOR_TYPE_RGB) {
	    cr= (*pp++);
	    cg= (*pp++);
	    cb= (*pp++);
	    cgr = (0.59*cr) + (0.20*cg) + (0.11*cb);
	  } else {
	    png_error(png_ptr, "Unknown PNG color type seen (ReadPNGImage_16)");
	  }
	  
	  /* Process alpha channel if present */
	  if (alpha)
	    a=(*pp++);
	  
	  if (alpha && a != 0xff)
	    {
	      int tr,tg,tb;
	    
	      /* Get tile colour without gamma - from www.c */
	      if (tileData)
	      {
		  int x = xpos % tileWidth;
		  int y = ypos % tileHeight;
		  int i = y * tileWidth + x;
		  unsigned char *s = (tileData + 2 * i);
		  unsigned short val;
		  val = *s * 256  + *(s+1);
		  tr= (((val & visual->red_mask) >> RPixelShift) << 8) / (RPixelMask+1);
		  tg= (((val & visual->green_mask) >> GPixelShift) << 8) /  (GPixelMask+1);
		  tb= (((val & visual->blue_mask) >> BPixelShift) << 8 ) / (BPixelMask+1);
	      }
	      else
	      {
 		  tr = (((transparent & visual->red_mask) >> RPixelShift) << 8) / (RPixelMask+1);
 		  tg = (((transparent & visual->green_mask) >> GPixelShift) << 8) /  (GPixelMask+1);
		  tb = (((transparent & visual->blue_mask)  >> BPixelShift) << 8 ) / (BPixelMask+1);
	      }
	      
	      /* Add picture pixel to background pixel */
	      cr=(a/255.0)*cr + ((255.0-a)/255.0) * tr;
	      cg=(a/255.0)*cg + ((255.0-a)/255.0) * tg;
	      cb=(a/255.0)*cb + ((255.0-a)/255.0) * tb;
	    }
	  
	  GetPngColor(cr, cg, cb, &ulp);
	  *dp++ = (unsigned char)((ulp >> 8) & 0xff);
	  *dp++ = (unsigned char)((ulp) & 0xff);
	  
	} /* xpos */
      
    } /* ypos */      
  
  return data;
}

/* Read image into displays, not 1,2,4,16 or 24 bit deep */
unsigned char *ReadPNGImage(png_struct *png_ptr, png_info *info_ptr, png_byte *png_image) 
{
  png_byte *pp;
  unsigned char *data, *dp;
  int ypos;
  int len=info_ptr->width;
  int color_type=info_ptr->color_type;
  int alpha;
  
  data = (unsigned char *)malloc(len * info_ptr->height);
  if (data == NULL)
    png_error (png_ptr, "couldn't alloc space for X image data");
  
  /* Remove alpha channel from type,  but remember it */
  alpha=color_type & PNG_COLOR_MASK_ALPHA;
  color_type &= ~PNG_COLOR_MASK_ALPHA;
  
  pp=png_image;
  dp=data;
  for (ypos = 0; ypos < png_ptr->height; ypos++)
    {
      int col = ypos & 0x0f;
      int xpos;
      
      for (xpos = 0; xpos < len; ++xpos)
	{
	  int row = xpos & 0x0f;
	  int cr=0, cg=0, cb=0, cgr, a;
	  int gr;
	  int isgrey = 0;
	  
	  if (color_type == PNG_COLOR_TYPE_GRAY) {
	    cr=cg=cb=cgr=Voltage2Brightness(*pp++);
	    isgrey=1;
	  } else if (color_type == PNG_COLOR_TYPE_RGB) {
	    int drb, dgb, dbr;
	    
	    cr= Voltage2Brightness(*pp++);
	    cg= Voltage2Brightness(*pp++);
	    cb= Voltage2Brightness(*pp++);
	    cgr = (0.59*cr) + (0.20*cg) + (0.11*cb);

	    drb = abs(cr - cb);
	    dgb = abs(cg - cb);
	    dbr = abs(cb - cr);
	    if (drb < 20 && dgb < 20 && dbr < 20)
	      isgrey=1;
	  } else {
	    png_error(png_ptr, "Unknown PNG color type seen (ReadPNGImage)");
	  }
	  
	  
	  /* Process alpha channel if present */
	  if (alpha) {
	    a=(*pp++);
	  
	    if(!a) {
	      dp=Transparent(dp, xpos, ypos);
	      continue;
	    } else if(a != 0xff) {
	      int tcolor, tr,tg,tb;
	      int drb, dgb, dbr;
	      
	      /* Get tile colour without gamma - from www.c */
	      if (tileData)
		{
		  unsigned int x = xpos % tileWidth;
		  unsigned int y = ypos % tileHeight;
		  unsigned int i = y * tileWidth + x;
		  tcolor=tileData[i];
		}
	      else
		tcolor=transparent;
	      
	      tr=papercols[tcolor].red;
	      tg=papercols[tcolor].green;
	      tb=papercols[tcolor].blue;

	      /* Add picture pixel to background pixel */
	      cr=(a/255.0)*cr + ((255.0-a)/255.0) * tr;
	      cg=(a/255.0)*cg + ((255.0-a)/255.0) * tg;
	      cb=(a/255.0)*cb + ((255.0-a)/255.0) * tb;
	      
	      /* Recalculate grayness */
	      cgr = (0.59*cr) + (0.20*cg) + (0.11*cb);
	      isgrey=0;
	      drb = abs(cr - cb);
	      dgb = abs(cg - cb);
	      dbr = abs(cb - cr);
	      if (drb < 20 && dgb < 20 && dbr < 20)
		isgrey=1;
	    }
	  }
	  
	  if (imaging == COLOR232)
	    {
	      if (isgrey) {
                int gr = cgr & 0xF0;

                if (cgr - gr > Magic16[(row << 4) + col])
                    gr += 16;

                gr = min(gr, 0xF0);
                
		*dp++ = greymap[gr >> 4];
	      } else {
	      int r = cr & 0xC0;
	      int g = cg & 0xE0;
	      int b = cb & 0xC0;
	      int v = (row << 4) + col;

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
	      }
	    }
	  else if (imaging == MONO)
	    {
	      if (cgr < Magic256[(row << 4) + col])
		*dp++ = greymap[0];
	      else
		*dp++ = greymap[15];
	    }
	  else /* Not COLOR232 or MONO */
	    {
	      gr = cgr & 0xF0;
	      if (cgr - gr > Magic16[(row << 4) + col])
		gr += 16;
	      
	      gr = min(gr, 0xF0);
	      *dp++ = greymap[gr >> 4];
	    }

	} /* xpos */
      
    } /* ypos */      
  
  return data;
}


static char *png_color_type[] = {"grayscale", "undefined type", "RGB",
				 "colormap", "grayscale+alpha",
				 "undefined type", "RGB+alpha"};

unsigned char *LoadPNGImage(Image *image, Block *bp, unsigned int depth) {
  png_struct *png_ptr; 
  png_info *info_ptr;
  png_byte *row_pointer;
  png_byte *png_image=NULL;
  int number_passes, pass, row, bit_depth, image_width;
  unsigned char *ximage=NULL;
  int color_type, orig_color_type;
  double image_gamma;
  
  png_ptr = malloc(sizeof (png_struct));
  if (!png_ptr)
    return 0;
  info_ptr = malloc(sizeof (png_info));
  if (!info_ptr) {
    free(png_ptr);
    return 0;
  }
   
  if (setjmp(png_ptr->jmpbuf)) {
    png_read_destroy(png_ptr, info_ptr, (png_info *)0);
    free(png_ptr);
    free(info_ptr);
    if (ximage)
      free(ximage);
    if (png_image)
      free(png_image);
    return 0;
  }
  
  png_read_init(png_ptr);
  png_info_init(info_ptr);
  /* No need to init PNG I/O - only ever do a read */
  png_set_read_fn(png_ptr, bp, &arena_png_read_data);
  png_read_info(png_ptr, info_ptr);
  bit_depth=info_ptr->bit_depth;
  orig_color_type = color_type = info_ptr->color_type;

  if (IMAGE_TRACE)
    fprintf(stderr, "PNG Image: %ld x %ld, %d-bit%s, %s, %sinterlaced\n",
	    info_ptr->width, info_ptr->height,
	    bit_depth, bit_depth>1 ? "s" : "",
	    (color_type>6) ? png_color_type[1] : png_color_type[color_type],
	    info_ptr->interlace_type ? "" : "non-");
  

  /* expand to RGB or 8 bit grayscale (with alpha if present) */
  png_set_expand(png_ptr);
  
  /* tell libpng to handle the gamma conversion */
  if (info_ptr->valid & PNG_INFO_gAMA)
    image_gamma=info_ptr->gamma;
  else
    image_gamma=(double)0.45; /* FIXME: OR: 1/Gamma ? */
  
  /* Only gamma correct if >1.1% difference */
/*  if (abs((Gamma * image_gamma) -1) > 0.011) */
    png_set_gamma(png_ptr, Gamma, image_gamma);
#if 0 /* GRR */  
  if (info_ptr->valid & PNG_INFO_bKGD)
    png_set_background(png_ptr, &(png_ptr->background),
		       PNG_BACKGROUND_GAMMA_FILE, 1, image_gamma);
#endif 
  /* tell libpng to convert 16 bits per channel to 8 */
  if (info_ptr->bit_depth == 16)
      png_set_strip_16(png_ptr); 
  
  /* read all the image into the rows allocated above */
  /* (Automatically handles interlacing) */
  number_passes = png_set_interlace_handling(png_ptr);
  
  /* Optional call to update the info_ptr area with the new settings */
  png_read_update_info(png_ptr, info_ptr);
  bit_depth=info_ptr->bit_depth;
  color_type=info_ptr->color_type;
  image_width= info_ptr->width * info_ptr->channels;

  png_image = malloc (info_ptr->height * image_width);
  if (png_image == NULL)
    png_error (png_ptr, "couldn't alloc space for PNG image");


  /* optional call to update palette with transformations, except
     it doesn't do any! */
  png_start_read_image(png_ptr);
  
  for (pass = 0; pass < number_passes; pass++)
    {
/*      if ((depth == 24)||(depth == 16))
	row_pointer=(png_byte*)ximage;
	else    --patch dsr 24-bits display 8-Dec-95 */
	row_pointer=png_image;
	
	for (row = 0; row < info_ptr->height; row++)
	{
	    png_read_row(png_ptr, NULL, row_pointer);
	    row_pointer += image_width;
	}
    }
  
  /* Now convert PNG image to X one */
  if (depth == 1 || depth == 2 || depth == 4)
      ximage = ReadPNGImage_1_2_4(png_ptr, info_ptr, png_image);
  else if (depth == 24)
      ximage = ReadPNGImage_24(png_ptr, info_ptr, png_image);
  else if (depth == 16) 
      ximage = ReadPNGImage_16(png_ptr, info_ptr, png_image);
  else
      ximage = ReadPNGImage(png_ptr, info_ptr, png_image);
  
  image->npixels = 0; /* Set to 0 in gif.c, icon.c, image.c  */
  image->width = info_ptr->width;
  image->height = info_ptr->height;

  /* Finish */
  png_read_end(png_ptr, info_ptr);
  png_read_destroy(png_ptr, info_ptr, (png_info *)0);

  free(png_ptr);   /* howcome 23/10/95: applied patch from Dave Beckett */
  free(info_ptr);  /* howcome 23/10/95: applied patch from Dave Beckett */

  free(png_image);

  
  return(ximage);
}

unsigned char *LoadProgressivePNGImage(Image *image, Block *bp, unsigned int depth, unsigned long length) {
  png_struct *png_ptr; 
  png_info *info_ptr;
  png_byte *row_pointer;
  png_byte *png_image=NULL;
  int number_passes, pass, row, bit_depth, image_width;
  unsigned char *ximage=NULL;
  int color_type, orig_color_type;
  double image_gamma;
  
  png_ptr = malloc(sizeof (png_struct));
  if (!png_ptr)
    return 0;
  info_ptr = malloc(sizeof (png_info));
  if (!info_ptr) {
    free(png_ptr);
    return 0;
  }
   
  if (setjmp(png_ptr->jmpbuf))
  {
      if (depth == 1 || depth == 2 || depth == 4)
	  ximage = ReadPNGImage_1_2_4(png_ptr, info_ptr, png_image);
      else if (depth == 24)
	  ximage = ReadPNGImage_24(png_ptr, info_ptr, png_image);
      else if (depth == 16) 
	  ximage = ReadPNGImage_16(png_ptr, info_ptr, png_image);
      else
	  ximage = ReadPNGImage(png_ptr, info_ptr, png_image);
      
      image->npixels = 0; /* Set to 0 in gif.c, icon.c, image.c  */
      image->width = info_ptr->width;
      image->height = info_ptr->height;
      
      /* Finish */

      png_read_destroy(png_ptr, info_ptr, (png_info *)0);

      free(png_ptr);  
      free(info_ptr); 
      if(png_image)
      {
	  free(png_image);
	  return(ximage);
      }
      else
	  return 0;
  }
    
  png_read_init(png_ptr);
  png_info_init(info_ptr);
  /* No need to init PNG I/O - only ever do a read */
  png_set_read_fn(png_ptr, bp, &arena_prog_png_read_data);
  png_read_info(png_ptr, info_ptr);
  bit_depth=info_ptr->bit_depth;
  orig_color_type = color_type = info_ptr->color_type;

  if (IMAGE_TRACE)
    fprintf(stderr, "PNG Image: %ld x %ld, %d-bit%s, %s, %sinterlaced\n",
	    info_ptr->width, info_ptr->height,
	    bit_depth, bit_depth>1 ? "s" : "",
	    (color_type>6) ? png_color_type[1] : png_color_type[color_type],
	    info_ptr->interlace_type ? "" : "non-");
  

  /* expand to RGB or 8 bit grayscale (with alpha if present) */
  png_set_expand(png_ptr);
  
  /* tell libpng to handle the gamma conversion */
  if (info_ptr->valid & PNG_INFO_gAMA)
    image_gamma=info_ptr->gamma;
  else
    image_gamma=(double)0.45; /* FIXME: OR: 1/Gamma ? */
  
  /* Only gamma correct if >1.1% difference */
  if (abs((Gamma * image_gamma) -1) > 0.011)
    png_set_gamma(png_ptr, Gamma, image_gamma);
  
  if (info_ptr->valid & PNG_INFO_bKGD)
    png_set_background(png_ptr, &(png_ptr->background),
		       PNG_BACKGROUND_GAMMA_FILE, 1, image_gamma);

  /* tell libpng to convert 16 bits per channel to 8 */
  if (info_ptr->bit_depth == 16)
      png_set_strip_16(png_ptr); 
  
  /* read all the image into the rows allocated above */
  /* (Automatically handles interlacing) */
  number_passes = png_set_interlace_handling(png_ptr);
  
  /* Optional call to update the info_ptr area with the new settings */
  png_read_update_info(png_ptr, info_ptr);
  bit_depth=info_ptr->bit_depth;
  color_type=info_ptr->color_type;
  image_width= info_ptr->width * info_ptr->channels;

  png_image = malloc (info_ptr->height * image_width);
  if (png_image == NULL)
    png_error (png_ptr, "couldn't alloc space for PNG image");


  /* optional call to update palette with transformations, except
     it doesn't do any! */
  png_start_read_image(png_ptr);
  
  for (pass = 0; pass < number_passes; pass++)
    {
/*      if ((depth == 24)||(depth == 16))
	row_pointer=(png_byte*)ximage;
	else    --patch dsr 24-bits display 8-Dec-95 */
	row_pointer=png_image;
	
	for (row = 0; row < info_ptr->height; row++)
	{
	    png_read_row(png_ptr, NULL, row_pointer);
	    row_pointer += image_width;
	}
    }
  

  /* Now convert PNG image to X one */
  if (depth == 1 || depth == 2 || depth == 4)
      ximage = ReadPNGImage_1_2_4(png_ptr, info_ptr, png_image);
  else if (depth == 24)
      ximage = ReadPNGImage_24(png_ptr, info_ptr, png_image);
  else if (depth == 16) 
      ximage = ReadPNGImage_16(png_ptr, info_ptr, png_image);
  else
      ximage = ReadPNGImage(png_ptr, info_ptr, png_image);
  
  image->npixels = 0; /* Set to 0 in gif.c, icon.c, image.c  */
  image->width = info_ptr->width;
  image->height = info_ptr->height;

  /* Finish */
  png_read_end(png_ptr, info_ptr);
  png_read_destroy(png_ptr, info_ptr, (png_info *)0);

  free(png_ptr);   /* howcome 23/10/95: applied patch from Dave Beckett */
  free(info_ptr);  /* howcome 23/10/95: applied patch from Dave Beckett */

  free(png_image);

  
  return(ximage);
}

