#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"
#include "jerror.h"
#include <setjmp.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */


#include <X11/Intrinsic.h>

#include "HTUtils.h"	/* WWW general purpose macros */
#include "tcp.h"
#include "HTList.h"
#include "HTAccess.h"

#include "www.h"

#ifdef JPEG

JSAMPLE *image_buffer;	/* Points to large array of R,G,B-order data */
int image_height;	/* Number of rows in image */
int image_width;	/* Number of columns in image */

#define MAX_BUFFER 10000
#define MAX_ROWS   16 /* matches one or two MCU rows in most images */
#define SIZEOF(object)  ((size_t) sizeof(object))   /* from jinclude.h */

typedef struct {
   unsigned char	*buffer;
   int			filled;
   } buffer_struct;

typedef struct {
  struct jpeg_source_mgr pub;	/* public fields */

  buffer_struct			*buffer;
  int				state;
  JOCTET			terminal[2];
} my_source_mgr;

typedef my_source_mgr * my_src_ptr;

#define	IMAGE_COLOUR_SPACE_UNKNOWN	0
#define	IMAGE_COLOUR_SPACE_GREYSCALE	1
#define	IMAGE_COLOUR_SPACE_RGB		2




static void my_input_manager  (j_decompress_ptr cinfo, buffer_struct *buffer);
int image_transform_jpeg (
		void *buffer_data, 
		int buffer_size,
		int declare_data_frame (void *userdata, image_data *data),
		int put_scanline_someplace (void *userdata, image_data *data),
		void *userdata
		);

extern int ImageOutputInit (void *output,
			image_data *data);
extern int ImageOutputScanlines (void *output, 
			image_data *data);

unsigned char *LoadJPEGImage(Image *image, Block *bp, unsigned int depth) {
    output_image_data	_output_data, *output_data;

    output_data = &_output_data;

    output_data->depth = depth;
    image_transform_jpeg (
		(void*)bp->buffer, bp->size, 
		ImageOutputInit,
		ImageOutputScanlines, 
		output_data);

    image->width = output_data->width;
    image->height = output_data->height;
    
    if(output_data->depth == 24)
    {
	unsigned char   *dp, *scan;
	int i,j;
	extern unsigned char *gamma_table;

	dp = output_data->data;
	scan = dp;
	for(i=0;i<output_data->width;i++)
	    for(j=0;j<output_data->height;j++)
	    {
		*dp++ = gamma_table[*scan++];
		*dp++ = gamma_table[*scan++];
		*dp++ = gamma_table[*scan++];
		*dp++ = gamma_table[*scan++];
	    }
    }
      
    return output_data->data;
}


#ifdef STANDALONE_TEST

extern int put_scanline_someplace (void *userdata, image_data *data);
extern int declare_data_frame (void *userdata, image_data *data);

extern int main (int argc, char *argv[], char *envp[]) {
   char 		*filename = argv[1];
   int 			file;
   int			status;
   unsigned char	buffer [MAX_BUFFER];
   void 		*userdata;

   printf ("Process file %s\n", filename);

   file = open (filename, O_RDONLY, 0);
   status = read (file, buffer, MAX_BUFFER);
   close (file);

   printf ("file read %d\n", status);

   image_transform_jpeg (
		(void*)buffer, status, 
		declare_data_frame,
		put_scanline_someplace, 
		userdata);

   printf ("finished :-)\n");
   }

extern int declare_data_frame (void *userdata, image_data *data) {
   printf ("The picture size is %d, %d components %d\n", 
		data->height, data->width, data->components);
   }
extern int put_scanline_someplace (void *userdata, image_data *data) {
   int	count;
   int	total=0;
   unsigned char *buffer = data->buffer;

   for (count=0; count<(data->components*data->width*data->rows); count++) {
        total = total + buffer[count];
	}
   printf ("[%d=%d]", data->rows, total);

   return (1);
   }
#endif


struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

static void my_error_exit (j_common_ptr cinfo) {
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  longjmp(myerr->setjmp_buffer, 1);
}


int image_transform_jpeg (
		void *buffer_data, 
		int buffer_size,
		int declare_data_frame (void *userdata, image_data *data),
		int put_scanline_someplace (void *userdata, image_data *data),
		void *userdata
		) {

  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */
  int	rows;

  image_data	_idata, *idata;

  buffer_struct		_buffer_in, *buffer_in;

  buffer_in = &_buffer_in;
  idata = &_idata;

  buffer_in->buffer = buffer_data;
  buffer_in->filled = buffer_size;

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    return 0;
  }

    jpeg_create_decompress(&cinfo);
    my_input_manager (&cinfo, buffer_in);

    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    idata->width = cinfo.output_width;
    idata->height = cinfo.output_height;
    idata->components = cinfo.output_components;

    if (cinfo.out_color_space == JCS_GRAYSCALE) {
	idata->colour_space = IMAGE_COLOUR_SPACE_GREYSCALE;
	}
    else if (cinfo.out_color_space == JCS_RGB) {
	idata->colour_space = IMAGE_COLOUR_SPACE_RGB;
	}
    else {
	idata->colour_space = IMAGE_COLOUR_SPACE_UNKNOWN;
	}
    idata->row_interlace = 1;

    declare_data_frame (userdata, idata);


    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, MAX_ROWS);

    idata->buffer = (unsigned char*) buffer [0];

    while (cinfo.output_scanline < cinfo.output_height) {
        rows = jpeg_read_scanlines(&cinfo, buffer, MAX_ROWS);
	idata->rows = rows;
        put_scanline_someplace(userdata, idata);
        }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 1;
    }


static void init_source (j_decompress_ptr cinfo) {
    my_src_ptr src = (my_src_ptr) cinfo->src;
    src->state = 0;
    }

static boolean fill_input_buffer (j_decompress_ptr cinfo) {
  my_src_ptr src = (my_src_ptr) cinfo->src;
  /* janet 21/07/95: not used:  size_t nbytes; */

  /* Since we have given all we have got already we simply fake an end of
	file */
  WARNMS(cinfo, JWRN_JPEG_EOF);
  src->pub.next_input_byte = src->terminal;
  src->pub.bytes_in_buffer = 2;
  src->terminal[0] = (JOCTET) 0xFF;
  src->terminal[0] = (JOCTET) JPEG_EOI;

  return TRUE;
}

static void skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
    my_src_ptr src = (my_src_ptr) cinfo->src;

    src->pub.next_input_byte = src->pub.next_input_byte + num_bytes;
}


static void term_source (j_decompress_ptr cinfo) {
    }


static void my_input_manager (j_decompress_ptr cinfo, buffer_struct *buffer) {
  my_src_ptr src;

  if (cinfo->src == NULL) {	/* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_source_mgr));
  }

  src = (my_src_ptr) cinfo->src;
  src->pub.init_source 		= init_source;
  src->pub.fill_input_buffer 	= fill_input_buffer;
  src->pub.skip_input_data 	= skip_input_data;
  src->pub.resync_to_restart 	= jpeg_resync_to_restart; 
  src->pub.term_source 		= term_source;

  src->pub.bytes_in_buffer 	= buffer->filled;
  src->pub.next_input_byte 	= buffer->buffer;

  src->buffer = buffer;
}


#endif /* JPEG */
