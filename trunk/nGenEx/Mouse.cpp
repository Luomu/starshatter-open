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
    FILE:         Mouse.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mouse class
*/

#include "MemDebug.h"
#include "Mouse.h"
#include "DataLoader.h"
#include "Window.h"
#include "Screen.h"
#include "Bitmap.h"
#include "Video.h"

// +--------------------------------------------------------------------+

int     Mouse::show        = 1;
int     Mouse::cursor      = Mouse::ARROW;

int     Mouse::x           = 320;
int     Mouse::y           = 240;
int     Mouse::l           = 0;
int     Mouse::m           = 0;
int     Mouse::r           = 0;
int     Mouse::w           = 0;

Bitmap* Mouse::image[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
int     Mouse::hotspot[8]  = { 0, 0, 0, 0, 0, 0, 0, 0 };
Window* Mouse::window      = 0;

// +--------------------------------------------------------------------+

void Mouse::Create(Screen* screen)
{
    if (screen) {
        delete window;
        window = new(__FILE__,__LINE__) Window(screen, 0, 0, screen->Width(), screen->Height());
    }
}

// +--------------------------------------------------------------------+

void Mouse::Resize(Screen* screen)
{
    if (screen) {
        delete window;
        window = new(__FILE__,__LINE__) Window(screen, 0, 0, screen->Width(), screen->Height());
    }
}

// +--------------------------------------------------------------------+

void Mouse::Close()
{
    for (int i = 0; i < 8; i++) {
        delete image[i];
        image[i] = 0;
    }

    delete window;
    window = 0;

    show   = 0;
    cursor = ARROW;
}

// +--------------------------------------------------------------------+

void
Mouse::SetCursorPos(int ax, int ay)
{
    POINT p;
    int   dx = 0;
    int   dy = 0;

    ::GetCursorPos(&p);

    dx = p.x - x;
    dy = p.y - y;

    x = ax;
    y = ay;

    ::SetCursorPos(x+dx,y+dy);
}

// +--------------------------------------------------------------------+

int
Mouse::SetCursor(CURSOR c)
{
    int old = cursor;
    cursor = c;
    return old;
}

// +--------------------------------------------------------------------+

int 
Mouse::LoadCursor(CURSOR c, const char* name, HOTSPOT hs)
{
    int result = 0;

    delete image[c];
    image[c] = 0;

    if (name && *name) {
        image[c] = new(__FILE__,__LINE__) Bitmap;
        result = DataLoader::GetLoader()->LoadBitmap(name, *image[c], Bitmap::BMP_TRANSPARENT);

        if (result) {
            Bitmap* bmp = image[c];

            if (bmp && bmp->HiPixels())
            image[c]->CopyAlphaRedChannel(image[c]->Width(), image[c]->Height(), (LPDWORD) image[c]->HiPixels());

            hotspot[c] = hs;
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

void
Mouse::Show(int s)
{
    show = s;
}

// +--------------------------------------------------------------------+

void
Mouse::Paint()
{
    if (!show || !window) return;

    // draw using bitmap:
    if (image[cursor]) {
        int w2 = image[cursor]->Width()/2;
        int h2 = image[cursor]->Height()/2;

        if (hotspot[cursor] == HOTSPOT_NW)
        window->DrawBitmap(x,    y,    x+w2*2, y+h2*2, image[cursor], Video::BLEND_ALPHA);
        else
        window->DrawBitmap(x-w2, y-h2, x+w2,   y+h2,   image[cursor], Video::BLEND_ALPHA);
    }

    // draw using primitives:
    /***
else {
    switch (cursor) {
    case ARROW:
    case CROSS:
    case USER1:
    case USER2:
    case USER3:
    default:
        window->DrawLine(x-7, y,   x+8, y,   Color::White);
        window->DrawLine(x,   y-7, x,   y+8, Color::White);
        break;

    case WAIT:
        window->DrawLine(x-7, y-7, x+8, y-7, Color::White);
        window->DrawLine(x-7, y-7, x+8, y+8, Color::White);
        window->DrawLine(x-7, y+8, x+8, y-7, Color::White);
        window->DrawLine(x-7, y+8, x+8, y+8, Color::White);
        break;

    case NOT:
        window->DrawEllipse(x-7,y-7,x+8,y+8, Color::White);
        window->DrawLine(   x-7,y-7,x+8,y+8, Color::White);
        break;

    case DRAG:
        window->DrawRect(x-7, y-6, x+8, y-3, Color::White);
        window->DrawRect(x-7, y-1, x+8, y+2, Color::White);
        window->DrawRect(x-7, y+4, x+8, y+7, Color::White);
        break;
    }
}
***/
}
