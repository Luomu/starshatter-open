/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Thumbnail.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Thumbnail.cpp : implementation file
*/

#include "stdafx.h"
#include "Bitmap.h"

// +--------------------------------------------------------------------+
// Preview a bitmap image in the desired window
// +--------------------------------------------------------------------+

void ThumbPreview(HWND hprev, Bitmap* bitmap)
{
   HDC         hdc      = ::GetDC(hprev);
   RECT        rect;
   BITMAPINFO* pbmiDIB  = new BITMAPINFO;

   if (!pbmiDIB)
      return;

   ::GetClientRect(hprev, &rect);

   pbmiDIB->bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
   pbmiDIB->bmiHeader.biWidth          = bitmap->Width();
   pbmiDIB->bmiHeader.biHeight         = -bitmap->Height();
   pbmiDIB->bmiHeader.biPlanes         = 1;
   pbmiDIB->bmiHeader.biBitCount       = 32;
   pbmiDIB->bmiHeader.biCompression    = BI_RGB;
   pbmiDIB->bmiHeader.biSizeImage      = 0;
   pbmiDIB->bmiHeader.biXPelsPerMeter  = 0;
   pbmiDIB->bmiHeader.biYPelsPerMeter  = 0;
   pbmiDIB->bmiHeader.biClrUsed        = 0;
   pbmiDIB->bmiHeader.biClrImportant   = 0;

   int result =
   ::StretchDIBits(hdc, 
                   1,         1,        128,             128,
                   0,         0,        bitmap->Width(), bitmap->Height(),
                   bitmap->HiPixels(),
                   pbmiDIB,
                   DIB_RGB_COLORS,
                   SRCCOPY);

   ::ReleaseDC(hprev, hdc);

   delete pbmiDIB;
}
