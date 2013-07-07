/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

// Needed for compatibility with libpng 1.4++
#define png_infopp_NULL (png_infopp)NULL
#define png_voidp_NULL (png_voidp)NULL
#define int_p_NULL (int*)NULL

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

    fopen_s(&f, filename,"rb");
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

    status = CreateImage(png_ptr, info_ptr);

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

    status = CreateImage(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    return status;
}

// +--------------------------------------------------------------------+

int
PngImage::CreateImage(png_structp png_ptr, png_infop info_ptr)
{
    int status = PNG_INVALID;

    width  = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    bpp    = png_get_bit_depth(png_ptr, info_ptr);

    if (width > 0 && width < 32768 && height > 0 && height < 32768) {
        // true-color:
        if (bpp >= 24) {
            status = PNG_OK;

            if ( png_get_channels(png_ptr, info_ptr) == 4)
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

                    if ( png_get_channels(png_ptr, info_ptr) == 4)
                    alpha = *p++;

                    image[row*width+col] = (alpha << 24) | (red << 16) | (green << 8) | blue;
                }
            }
        }

        // paletted:
        else if (bpp == 8) {
            DWORD pal[256];
            
            png_bytep trans_alpha; int num_trans; png_color_16p trans_color;
            png_colorp palette;
            int num_palette;

            png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &num_trans, &trans_color);
            png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);

            if (num_trans > 0)
            alpha_loaded = true;

            for (int i = 0; i < 256; i++) {
                if (i < num_palette) {
                    DWORD red   = palette[i].red;
                    DWORD green = palette[i].green;
                    DWORD blue  = palette[i].blue;
                    DWORD alpha = 0xff;

                    if (i < num_trans)
                    alpha = trans_alpha[i];
                    
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