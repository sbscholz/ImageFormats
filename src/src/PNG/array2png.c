#include "imagesac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <png.h>

static int
array2png_file (FILE *fp, int height, int width, int *arr_data)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  png_bytep img_data;
  png_int_32 *bptr;
  int x, y, aroffset, ioffset;

  if (fp == NULL) {
    SAC_RuntimeError("SAC_PNG_array2png: Failed to open image file: %s",
                     strerror(errno));
    return errno;
  }


  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL)
  {
    SAC_RuntimeError("SAC_PNG_array2png: Failed to create png structure: %s",
                    strerror(errno));
    return errno;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
    SAC_RuntimeError("SAC_PNG_array2png: Failed to create png info structure: %s",
                    strerror(errno));
    return errno;
  }

    if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    SAC_RuntimeError("SAC_PNG_array2png: Failed to read PNG image");
    return errno;
  }

  png_init_io(png_ptr, fp);
  png_set_IHDR(png_ptr, info_ptr, width, height, 8,     /* bit depth */
               PNG_COLOR_TYPE_RGB,      /* color type */
               PNG_INTERLACE_NONE,      /* interlace type */
               PNG_COMPRESSION_TYPE_DEFAULT,    /* compression type */
               PNG_FILTER_TYPE_DEFAULT);        /* filter type */

  png_write_info(png_ptr, info_ptr);

  row_pointers = malloc(height * sizeof(png_bytep));
  img_data = malloc(height * width * 4);
  for (y = 0; y < height; y++)
  {
    row_pointers[y] = img_data + (y * width * 4);
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
         ( (((png_int_32)arr_data[aroffset+0] & 0xff) <<  8)
         | (((png_int_32)arr_data[aroffset+1] & 0xff) <<  16)
         | (((png_int_32)arr_data[aroffset+2] & 0xff) <<  24)
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
  png_write_end(png_ptr, info_ptr);

  /*
   * Clean up
   */
  free(row_pointers);
  free(img_data);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);

  return 0;
}


void SAC_PNG_array2png_file(SAC_ND_PARAM_out_nodesc( res_nt, int),
                            SAC_ND_PARAM_in_nodesc( filep_nt, FILE*),
                            SAC_ND_PARAM_in( ar_nt, int))
{
  int res = 0;
  int height = SAC_ND_A_DESC_SHAPE(ar_nt, 0);
  int width = SAC_ND_A_DESC_SHAPE(ar_nt, 1);
  int *arr_data = SAC_ND_A_FIELD(ar_nt);
  FILE *fp = SAC_ND_A_FIELD(filep_nt);

  res = array2png_file (fp, height, width, arr_data);

  *SAC_NAMEP( SAC_ND_A_FIELD( res_nt)) = res;
  SAC_ND_DEC_RC_FREE( ar_nt, 1,)

}

void SAC_PNG_array2png(SAC_ND_PARAM_out_nodesc( res_nt, int),
                       SAC_ND_PARAM_in_nodesc( string_nt, string),
                       SAC_ND_PARAM_in( ar_nt, int))
{
  FILE *fp;
  int res = 0;
  int height = SAC_ND_A_DESC_SHAPE(ar_nt, 0);
  int width = SAC_ND_A_DESC_SHAPE(ar_nt, 1);
  int *arr_data = SAC_ND_A_FIELD(ar_nt);

  fp = fopen(NT_NAME(string_nt), "wb");

  res = array2png_file (fp, height, width, arr_data);

  *SAC_NAMEP( SAC_ND_A_FIELD( res_nt)) = res;
  SAC_ND_DEC_RC_FREE( ar_nt, 1,)
}

void SAC_PNG_array2png_stdout(SAC_ND_PARAM_out_nodesc( res_nt, int),
                              SAC_ND_PARAM_in( ar_nt, int))
{
  FILE *fp;
  int res = 0;
  int height = SAC_ND_A_DESC_SHAPE(ar_nt, 0);
  int width = SAC_ND_A_DESC_SHAPE(ar_nt, 1);
  int *arr_data = SAC_ND_A_FIELD(ar_nt);

  res = array2png_file (stdout, height, width, arr_data);

  *SAC_NAMEP( SAC_ND_A_FIELD( res_nt)) = res;
  SAC_ND_DEC_RC_FREE( ar_nt, 1,)
}
