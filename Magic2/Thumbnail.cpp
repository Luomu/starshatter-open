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
