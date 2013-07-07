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
