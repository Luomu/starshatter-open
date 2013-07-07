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
    FILE:         Mouse.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mouse class
*/

#ifndef Mouse_h
#define Mouse_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Screen;
class Window;

// +--------------------------------------------------------------------+

class Mouse
{
    friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    friend class Game;

public:
    static const char* TYPENAME() { return "Mouse"; }

    enum CURSOR    { ARROW, CROSS, WAIT, NOT, DRAG, USER1, USER2, USER3 };
    enum HOTSPOT   { HOTSPOT_CTR, HOTSPOT_NW };

    static int     X()               { return x; }
    static int     Y()               { return y; }
    static int     LButton()         { return l; }
    static int     MButton()         { return m; }
    static int     RButton()         { return r; }
    static int     Wheel()           { return w; }

    static void    Paint();

    static void    SetCursorPos(int x, int y);
    static void    Show(int s=1);
    static int     SetCursor(CURSOR c);
    static int     LoadCursor(CURSOR c, const char* name, HOTSPOT hs = HOTSPOT_CTR);

    static void    Create(Screen* screen);
    static void    Resize(Screen* screen);
    static void    Close();

private:
    static int     show;
    static int     cursor;

    static int     x;
    static int     y;
    static int     l;
    static int     m;
    static int     r;
    static int     w;

    static Bitmap* image[8];
    static int     hotspot[8];

    static Window* window;
};

#endif Mouse_h

