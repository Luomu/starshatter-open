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
    FILE:         Window.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Window class (a region of a screen or buffer)
*/

#ifndef Window_h
#define Window_h

#include "Types.h"
#include "Geometry.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Color;
class Bitmap;
class Font;
class Screen;
class View;

// +--------------------------------------------------------------------+

class Window
{
    friend class Screen;

public:
    static const char* TYPENAME() { return "Window"; }

    Window(Screen* s, int ax, int ay, int aw, int ah);
    virtual ~Window();

    int operator == (const Window& that) const { return this == &that; }

    // Screen dimensions:
    Screen*           GetScreen()          const { return screen;  }
    const Rect&       GetRect()            const { return rect;    }
    int               X()                  const { return rect.x;  }
    int               Y()                  const { return rect.y;  }
    int               Width()              const { return rect.w;  }
    int               Height()             const { return rect.h;  }

    // Operations:
    virtual void      Paint();
    virtual void      Show()                     { shown = true;   }
    virtual void      Hide()                     { shown = false;  }
    virtual bool      IsShown()            const { return shown;   }

    virtual void      MoveTo(const Rect& r);

    virtual bool      AddView(View* v);
    virtual bool      DelView(View* v);

    Rect              ClipRect(const Rect& r);
    bool              ClipLine(int& x1, int& y1, int& x2, int& y2);
    bool              ClipLine(double& x1, double& y1, double& x2, double& y2);

    void              DrawLine(int x1, int y1, int x2, int y2, Color color, int blend=0);
    void              DrawRect(int x1, int y1, int x2, int y2, Color color, int blend=0);
    void              DrawRect(const Rect& r, Color color, int blend=0);
    void              FillRect(int x1, int y1, int x2, int y2, Color color, int blend=0);
    void              FillRect(const Rect& r, Color color, int alpha=0);
    void              DrawBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend=0);
    void              FadeBitmap(int x1, int y1, int x2, int y2, Bitmap* img, Color c, int blend);
    void              ClipBitmap(int x1, int y1, int x2, int y2, Bitmap* img, Color c, int blend, const Rect& clip);
    void              TileBitmap(int x1, int y1, int x2, int y2, Bitmap* img, int blend=0);
    void              DrawLines(int nPts, POINT* pts, Color color, int blend=0);
    void              DrawPoly(int nPts, POINT* pts, Color color, int blend=0);
    void              FillPoly(int nPts, POINT* pts, Color color, int blend=0);

    void              DrawEllipse(int x1, int y1, int x2, int y2, Color color, int blend=0);
    void              FillEllipse(int x1, int y1, int x2, int y2, Color color, int blend=0);

    // text methods:
    void              SetFont(Font* f)           { font = f;    }
    Font*             GetFont()            const { return font; }

    void              Print(int x1, int y1, const char* fmt, ...);
    void              DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags);

protected:
    // translate screen coords into window relative coords
    virtual void      ScreenToWindow(int& x, int& y) { }
    virtual void      ScreenToWindow(Rect& r)        { }

    Rect              rect;
    Screen*           screen;
    bool              shown;
    Font*             font;

    List<View>        view_list;
};

#endif Window_h

