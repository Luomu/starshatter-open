/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         PngImage.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    BMP image file loader
*/


#include "MemDebug.h"
#include "PngImage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "png.h"

// +--------------------------------------------------------------------+

PngImage::PngImage()
   : width(0), height(0), bpp(0), alpha_loaded(false), image(0)
{ }

PngImage::~PngImage()
{
   delete [] image;
}

// +--------------------------------------------------------------------+

int PngImage::Load(char *filename)
{
   int   status = PNG_INVALID;
   FILE* f;

   f = fopen(filename,"rb");
   if (f == NULL)
      return PNG_NOFILE;

   BYTE buf[12];
   fread(buf, 8, 1, f);
   fseek(f, 0, SEEK_SET);

   if (png_sig_cmp(buf, (png_size_t)0, 8))
      return PNG_INVALID;

   png_structp png_ptr;
   png_infop   info_ptr;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

   if (!png_ptr) {
      return PNG_NOMEM;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return PNG_NOMEM;
   }

   png_init_io(png_ptr, f);
   png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);

   status = CreateImage((void*) png_ptr, (void*) info_ptr);

   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

   fclose(f);
   return status;
}

// +--------------------------------------------------------------------+

struct PngIOStruct
{
   BYTE* fp;
   BYTE* buffer;
   DWORD length;
};

static void png_user_read_data(png_structp read_ptr, png_bytep data, png_size_t length)
{
   PngIOStruct* read_io_ptr = (PngIOStruct*) png_get_io_ptr(read_ptr);

   if (!read_io_ptr)
      return;

   if (read_io_ptr->fp + length < read_io_ptr->buffer + read_io_ptr->length) {
      memcpy(data, read_io_ptr->fp, length);
      read_io_ptr->fp += length;
   }
}

static void png_user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
}

static void png_user_flush_data(png_structp png_ptr)
{
}

int PngImage::LoadBuffer(unsigned char* buf, int len)
{
   int status = PNG_INVALID;
   PngIOStruct io;

   if (buf == NULL)
      return PNG_NOFILE;

   if (png_sig_cmp(buf, (png_size_t)0, 8))
      return PNG_INVALID;

   io.buffer = buf;
   io.fp     = buf;
   io.length = len;

   png_structp png_ptr;
   png_infop   info_ptr;

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

   if (!png_ptr) {
      return PNG_NOMEM;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
      return PNG_NOMEM;
   }

   png_set_read_fn(png_ptr, (void *) (&io), png_user_read_data);
   png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, png_voidp_NULL);

   status = CreateImage((void*) png_ptr, (void*) info_ptr);

   png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

   return status;
}

// +--------------------------------------------------------------------+

int
PngImage::CreateImage(void* pptr, void* iptr)
{
   int status = PNG_INVALID;

   png_structp png_ptr  = (png_structp) pptr;
   png_infop   info_ptr = (png_infop)   iptr;

   width  = info_ptr->width;
   height = info_ptr->height;
   bpp    = info_ptr->pixel_depth;

   if (width > 0 && width < 32768 && height > 0 && height < 32768) {
      // true-color:
      if (bpp >= 24) {
         status = PNG_OK;

         if (info_ptr->channels == 4)
            alpha_loaded = true;

         image = new DWORD[width*height];
         BYTE** rows = png_get_rows(png_ptr, info_ptr);

         for (DWORD row = 0; row < height; row++) {
            BYTE* p = rows[row];

            for (DWORD col = 0; col < width; col++) {
               DWORD red   = *p++;
               DWORD green = *p++;
               DWORD blue  = *p++;
               DWORD alpha = 0xff;

               if (info_ptr->channels == 4)
                  alpha = *p++;

               image[row*width+col] = (alpha << 24) | (red << 16) | (green << 8) | blue;
            }
         }
      }

      // paletted:
      else if (bpp == 8 && info_ptr->num_palette > 0) {
         DWORD pal[256];

         if (info_ptr->num_trans > 0)
            alpha_loaded = true;

         for (int i = 0; i < 256; i++) {
            if (i < info_ptr->num_palette) {
               DWORD red   = info_ptr->palette[i].red;
               DWORD green = info_ptr->palette[i].green;
               DWORD blue  = info_ptr->palette[i].blue;
               DWORD alpha = 0xff;

               if (i < info_ptr->num_trans)
                  alpha = info_ptr->trans[i];
               
               pal[i] = (alpha << 24) | (red << 16) | (green << 8) | blue;
            }

            else {
               pal[i] = 0;
            }
         }

         image = new DWORD[width*height];
         BYTE** rows = png_get_rows(png_ptr, info_ptr);

         for (DWORD row = 0; row < height; row++) {
            BYTE* p = rows[row];

            for (DWORD col = 0; col < width; col++) {
               BYTE index = *p++;
               image[row*width+col] = pal[index];
            }
         }
      }
   }

   return status;
}