/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImgView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Bitmap "billboard" Image View class
*/

#include "MemDebug.h"
#include "ImgView.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Bitmap.h"
#include "Screen.h"

// +--------------------------------------------------------------------+

ImgView::ImgView(Window* c, Bitmap* bmp)
   : View(c), img(bmp), width(0), height(0), x_offset(0), y_offset(0),
     blend(Video::BLEND_SOLID)
{
   if (img) {
      width  = img->Width();
      height = img->Height();
   }

   if (width < c->Width())
      x_offset = (c->Width() - width) / 2;

   if (height < c->Height())
      y_offset = (c->Height() - height) / 2;
}

ImgView::~ImgView()
{
}

// +--------------------------------------------------------------------+

void
ImgView::Refresh()
{
   if (img && width > 0 && height > 0)
      window->DrawBitmap(x_offset,
                         y_offset,
                         x_offset + width,
                         y_offset + height,
                         img,
                         blend);
}

// +--------------------------------------------------------------------+

void
ImgView::SetPicture(Bitmap* bmp)
{
   img      = bmp;
   width    = 0;
   height   = 0;
   x_offset = 0;
   y_offset = 0;

   if (img) {
      width  = img->Width();
      height = img->Height();
   }

   if (window) {
      x_offset = (window->Width()  - width)  / 2;
      y_offset = (window->Height() - height) / 2;
   }
}
