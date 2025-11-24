#include "imagesac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <png.h>

/*
 * PNG versions 1.2.41 and 1.4.0 and later have this defined.
 * If your systems png.h doesn't have this then it is highly
 * recommended to upgrade because older PNG libraries have
 * a serious security issue as documented in:
 * http://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2011-3026
 */
#ifndef PNG_TRANSFORM_GRAY_TO_RGB
#define PNG_TRANSFORM_GRAY_TO_RGB       0
#endif

#undef array_nt
#undef ret_nt
#define array_nt  (array, T_OLD((AKD, (NHD, (NUQ, )))))
#define ret_nt    (ret,   T_OLD((AKD, (NHD, (NUQ, )))))

void SAC_PNG_png2array( SAC_ND_PARAM_out(array_nt, int),
                        SAC_ND_PARAM_in_nodesc(string_nt, string))
{
  FILE *fp;
  char *filename = NT_NAME(string_nt);
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;
  int x, y;
  int height;
  int width;
  int *buf, *ptr;

  fp = fopen(filename, "rb");
  if (fp == NULL)
  {
    SAC_RuntimeError("SAC_PNG_png2array: Failed to open image file '%s': %s",
                     filename, strerror(errno));
    return;
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL)
  {
    SAC_RuntimeError("SAC_PNG_png2array: Failed to create png structure: %s",
                    strerror(errno));
    fclose(fp);
    return;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    SAC_RuntimeError("SAC_PNG_png2array: Failed to create png info structure: %s",
                    strerror(errno));
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(fp);
    return;
  }

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    SAC_RuntimeError("SAC_PNG_png2array: Failed to read PGN image");
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fclose(fp);
    return;
  }

  png_init_io(png_ptr, fp);
  png_read_png(png_ptr, info_ptr,
               PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_STRIP_ALPHA |
               PNG_TRANSFORM_PACKING | PNG_TRANSFORM_GRAY_TO_RGB,
               NULL);

  row_pointers = png_get_rows(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  width = png_get_image_width(png_ptr, info_ptr);

  buf = SAC_MALLOC(width * height * sizeof(int) * 3);
  ptr = buf;
  for (y = 0; y < height; ++y)
  {
    unsigned char *row = (unsigned char *) row_pointers[y];
    for (x = 0; x < width * 3; ++x)
    {
      *ptr++ = *row++;
    }
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  SAC_ND_DECL__DATA(ret_nt, int,)
  SAC_ND_DECL__DESC(ret_nt,)
  int SAC_ND_A_MIRROR_DIM(ret_nt) = 3;
  SAC_ND_ALLOC__DESC(ret_nt, dims)
  SAC_ND_SET__RC(ret_nt, 1)
  SAC_ND_A_DESC_SHAPE(ret_nt, 0) = height;
  SAC_ND_A_DESC_SHAPE(ret_nt, 1) = width;
  SAC_ND_A_DESC_SHAPE(ret_nt, 2) = 3;
  SAC_ND_A_FIELD(ret_nt) = buf;
  SAC_ND_RET_out(array_nt, ret_nt)
}
