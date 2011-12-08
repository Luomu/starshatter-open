/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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
