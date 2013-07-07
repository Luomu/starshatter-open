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
    FILE:         PCX.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    PCX image file loader
*/

#ifndef PCX_H
#define PCX_H

// +--------------------------------------------------------------------+

enum { PCX_OK, PCX_NOMEM, PCX_TOOBIG, PCX_NOFILE };

struct PcxHeader
{
    char  manufacturer;    // Always set to 10
    char  version;         // Always 5 for 256-color files
    char  encoding;        // Always set to 1
    char  bits_per_pixel;  // Should be 8 for 256-color files
    short xmin,ymin;       // Coordinates for top left corner
    short xmax,ymax;       // Width and height of image
    short hres;            // Horizontal resolution of image
    short vres;            // Vertical resolution of image
    char  palette16[48];   // EGA palette; not used for 256-color files
    char  reserved;        // Reserved for future use
    char  color_planes;    // Color planes
    short bytes_per_line;  // Number of bytes in 1 line of pixels
    short palette_type;    // Should be 2 for color palette
    char  filler[58];      // Nothing but junk
};

// +--------------------------------------------------------------------+

struct PcxImage
{
    static const char* TYPENAME() { return "PcxImage"; }

    PcxImage(short w, short h, unsigned long* hibits);
    PcxImage(short w, short h, unsigned char* bits, unsigned char* colors);

    PcxImage();
    ~PcxImage();

    int Load(char *filename);
    int Save(char *filename);

    int LoadBuffer(unsigned char* buf, int len);

    PcxHeader      hdr;
    unsigned char* bitmap;
    unsigned long* himap;
    unsigned char  pal[768];
    unsigned long  imagebytes;
    unsigned short width, height;
};

// +--------------------------------------------------------------------+


#endif
