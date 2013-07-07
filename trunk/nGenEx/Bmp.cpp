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
    FILE:         Bmp.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    BMP image file loader
*/


#include "MemDebug.h"
#include "BMP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// +--------------------------------------------------------------------+

BmpImage::BmpImage()
    : width(0), height(0), image(0)
{ }

BmpImage::BmpImage(short w, short h, unsigned long* hibits)
{
    ZeroMemory(this, sizeof(BmpImage));

    width                = w;
    height               = h;

    file_hdr.type        = 19778;    // 'BM'
    file_hdr.size        = sizeof(BmpFileHeader) +
    sizeof(BmpInfoHeader) +
    w * h * 3;

    file_hdr.offset      = sizeof(BmpFileHeader) +
    sizeof(BmpInfoHeader);

    info_hdr.hdr_size    = sizeof(BmpInfoHeader);
    info_hdr.width       = width;
    info_hdr.height      = height;
    info_hdr.planes      = 1;
    info_hdr.bit_count   = 24;

    int pixels           = width * height;

    image = new(__FILE__,__LINE__) DWORD [pixels];

    if (image && pixels) {
        for (int i = 0; i < pixels; i++)
        image[i] = hibits[i];
    }
}

BmpImage::~BmpImage()
{
    delete [] image;
}

// +--------------------------------------------------------------------+

int BmpImage::Load(char *filename)
{
    int   status = BMP_INVALID;
    FILE* f;

    fopen_s(&f, filename,"rb");
    if (f == NULL)
    return BMP_NOFILE;

    fread(&file_hdr.type,   sizeof(WORD),      1, f);
    fread(&file_hdr.size,   sizeof(DWORD),     1, f);
    fread(&file_hdr.rsvd1,  sizeof(WORD),      1, f);
    fread(&file_hdr.rsvd2,  sizeof(WORD),      1, f);
    fread(&file_hdr.offset, sizeof(DWORD),     1, f);
    fread(&info_hdr,        BMP_INFO_HDR_SIZE, 1, f);

    if (info_hdr.width > 32768 || info_hdr.height > 32768 || 
            (info_hdr.width&3) || (info_hdr.height&3) || 
            info_hdr.compression != 0) {
        fclose(f);
        return BMP_INVALID;
    }

    width    = (WORD) info_hdr.width;
    height   = (WORD) info_hdr.height;

    // read 256 color BMP file
    if (info_hdr.bit_count == 8) {
        fread(palette, sizeof(palette), 1, f);

        int pixels = width*height;

        delete [] image;
        image = new(__FILE__,__LINE__) DWORD[pixels];
        if (image == NULL)
        return BMP_NOMEM;

        for (int row = height-1; row >= 0; row--) {
            for (int col = 0; col < width; col++) {
                BYTE index = fgetc(f);
                image[row*width+col] = palette[index];
            }
        }

        status = BMP_OK;
    }

    // read 24-bit (true COLOR) BMP file
    else if (info_hdr.bit_count == 24) {
        int pixels = width*height;

        delete [] image;
        image = new(__FILE__,__LINE__) DWORD[pixels];
        if (image == NULL)
        return BMP_NOMEM;

        for (int row = height-1; row >= 0; row--) {
            for (int col = 0; col < width; col++) {
                DWORD blue  = fgetc(f);
                DWORD green = fgetc(f);
                DWORD red   = fgetc(f);

                image[row*width+col] = 0xff000000 | (red << 16) | (green << 8) | blue;
            }
        }

        status = BMP_OK;
    }

    fclose(f);
    return status;
}

// +--------------------------------------------------------------------+

int BmpImage::LoadBuffer(unsigned char* buf, int len)
{
    int   status = BMP_INVALID;
    BYTE* fp;

    if (buf == NULL)
    return BMP_NOFILE;

    fp = buf;
    memcpy(&info_hdr, buf + BMP_FILE_HDR_SIZE, BMP_INFO_HDR_SIZE);
    fp += BMP_FILE_HDR_SIZE + BMP_INFO_HDR_SIZE;

    if (info_hdr.width > 32768 || info_hdr.height > 32768 || 
            (info_hdr.width&3) || (info_hdr.height&3) || 
            info_hdr.compression != 0) {
        return BMP_INVALID;
    }

    width    = (WORD) info_hdr.width;
    height   = (WORD) info_hdr.height;

    // read 256 color BMP file
    if (info_hdr.bit_count == 8) {
        memcpy(palette, fp, sizeof(palette));
        fp += sizeof(palette);

        int pixels = width*height;

        delete [] image;
        image = new(__FILE__,__LINE__) DWORD[pixels];
        if (image == NULL)
        return BMP_NOMEM;

        for (int row = height-1; row >= 0; row--) {
            for (int col = 0; col < width; col++) {
                BYTE index = *fp++;
                image[row*width+col] = palette[index];
            }
        }

        status = BMP_OK;
    }

    // read 24-bit (true COLOR) BMP file
    else if (info_hdr.bit_count == 24) {
        int pixels = width*height;

        delete [] image;
        image = new(__FILE__,__LINE__) DWORD[pixels];
        if (image == NULL)
        return BMP_NOMEM;

        for (int row = height-1; row >= 0; row--) {
            for (int col = 0; col < width; col++) {
                DWORD blue  = *fp++;
                DWORD green = *fp++;
                DWORD red   = *fp++;

                image[row*width+col] = 0xff000000 | (red << 16) | (green << 8) | blue;
            }
        }

        status = BMP_OK;
    }

    return status;
}

// +--------------------------------------------------------------------+

int BmpImage::Save(char *filename)
{
    int   status = BMP_INVALID;
    FILE* f;

    fopen_s(&f, filename,"wb");
    if (f == NULL)
    return BMP_NOFILE;

    info_hdr.bit_count   = 24;
    info_hdr.compression = 0;

    fwrite(&file_hdr.type,   sizeof(WORD),      1, f);
    fwrite(&file_hdr.size,   sizeof(DWORD),     1, f);
    fwrite(&file_hdr.rsvd1,  sizeof(WORD),      1, f);
    fwrite(&file_hdr.rsvd2,  sizeof(WORD),      1, f);
    fwrite(&file_hdr.offset, sizeof(DWORD),     1, f);
    fwrite(&info_hdr,        BMP_INFO_HDR_SIZE, 1, f);

    // write 24-bit (TRUE COLOR) BMP file
    for (int row = height-1; row >= 0; row--) {
        for (int col = 0; col < width; col++) {
            DWORD pixel = image[row*width+col];

            BYTE blue  = (BYTE) ((pixel & 0x000000ff) >>  0);
            BYTE green = (BYTE) ((pixel & 0x0000ff00) >>  8);
            BYTE red   = (BYTE) ((pixel & 0x00ff0000) >> 16);

            fwrite(&blue,  1, 1, f);
            fwrite(&green, 1, 1, f);
            fwrite(&red,   1, 1, f);
        }
    }

    status = BMP_OK;

    fclose(f);
    return status;
}

