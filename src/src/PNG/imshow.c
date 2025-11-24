#include "imagesac.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <png.h>

void imshow( SAC_ND_PARAM_out_nodesc( res_nt, int),
	     SAC_ND_PARAM_in( ar_nt, int))
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep  *row_pointers;
  png_bytep   img_data;
  png_int_32 *bptr;
  int height, width;
  int x, y, i, aroffset, ioffset;
  char *sys;

  int fd;
  FILE *fp;
  char fname[] = "img_XXXXXX";

  if (-1 == (fd = mkstemp( fname)))
    SAC_RuntimeError( "mkstemp failed: %s", strerror( errno));

  if (NULL == (fp = fdopen( fd, "wb"))) {
    SAC_RuntimeError( "Failed to create temporary image file");
  }

  png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if ( png_ptr == NULL) {
    SAC_RuntimeError( "Failed to create png structure");
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr,
			     (png_infopp)NULL);
    SAC_RuntimeError( "Failed to create png info structure");
  }

  height = SAC_ND_A_DESC_SHAPE( ar_nt, 0);
  width  = SAC_ND_A_DESC_SHAPE( ar_nt, 1);

  png_init_io( png_ptr, fp);
  png_set_IHDR( png_ptr, info_ptr,
		width,
		height,
		8,                              /* bit depth */
		PNG_COLOR_TYPE_RGB,             /* color type */
		PNG_INTERLACE_NONE,             /* interlace type */
		PNG_COMPRESSION_TYPE_BASE,   /* compression type */
		PNG_FILTER_TYPE_BASE);       /* filter type */

  png_write_info(png_ptr, info_ptr);

  row_pointers = malloc( height*sizeof( png_bytep));
  img_data = malloc( height*width*4);
  for ( i = 0; i < height; i++) {
    row_pointers[i] = img_data + (4*i*width);
  }

  /*
   * draw
   */
  bptr = (png_int_32 *)img_data;
  aroffset = 0;
  ioffset = 0;
  for (y = 0; y < height; y++) {
    for (x = 0; x < width; x++) {
      bptr[ioffset] =
	 ( (((png_int_32)SAC_ND_A_FIELD( ar_nt)[aroffset+0] & 0xff) <<  8)
	 | (((png_int_32)SAC_ND_A_FIELD( ar_nt)[aroffset+1] & 0xff) <<  16)
	 | (((png_int_32)SAC_ND_A_FIELD( ar_nt)[aroffset+2] & 0xff) <<  24)
         );
      ioffset +=1;
      aroffset+=3;
    }
  }
  png_set_rows(png_ptr, info_ptr, row_pointers);

  /*
   * Write the image
   */
  png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_FILLER, NULL);

  /*
   * Clean up
   */
  free( img_data);
  free( row_pointers);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose( fp);


  /*
   * show image
   */
  sys = malloc( 1024*sizeof( char));
  sprintf( sys, "(display \"%s\" && rm -f \"%s\") > /dev/null 2>&1 &",
	   fname, fname);
  if (EXIT_SUCCESS != system( sys))
    SAC_RuntimeError("Failed to execute '%s': %s", sys, strerror(errno));

  free( sys);

  /*
   * return 0
   */
  *SAC_NAMEP( SAC_ND_A_FIELD( res_nt)) = 0;

  SAC_ND_DEC_RC( ar_nt, 1)
}
