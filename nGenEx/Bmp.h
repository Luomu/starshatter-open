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
    FILE:         Bmp.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    BMP image file loader
*/

#ifndef BMP_H
#define BMP_H

// +--------------------------------------------------------------------+

enum { BMP_OK, BMP_NOMEM, BMP_INVALID, BMP_NOFILE };

struct BmpFileHeader
{
    WORD  type;
    DWORD size;
    WORD  rsvd1;
    WORD  rsvd2;
    DWORD offset;
};

struct BmpInfoHeader
{
    DWORD hdr_size;
    DWORD width;
    DWORD height;
    WORD  planes;
    WORD  bit_count;
    DWORD compression;
    DWORD img_size;
    DWORD x_pixels_per_meter;
    DWORD y_pixels_per_meter;
    DWORD colors_used;
    DWORD colors_important;
};

const int BMP_FILE_HDR_SIZE = 14;
const int BMP_INFO_HDR_SIZE = 40;

// +--------------------------------------------------------------------+

struct BmpImage
{
    static const char* TYPENAME() { return "BmpImage"; }

    BmpImage(short w, short h, unsigned long* hibits);

    BmpImage();
    ~BmpImage();

    int Load(char *filename);
    int Save(char *filename);

    int LoadBuffer(unsigned char* buf, int len);

    BmpFileHeader  file_hdr;
    BmpInfoHeader  info_hdr;
    DWORD          palette[256];
    DWORD*         image;
    WORD           width;
    WORD           height;
};

// +--------------------------------------------------------------------+


#endif BMP_H
