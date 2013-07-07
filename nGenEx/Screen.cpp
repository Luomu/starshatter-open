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
    FILE:         Screen.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    General Screen class - maintains and displays a list of windows
*/

#include "MemDebug.h"
#include "Screen.h"
#include "Bitmap.h"
#include "Color.h"
#include "Window.h"
#include "Mouse.h"
#include "Pcx.h"
#include "Video.h"

// +--------------------------------------------------------------------+

Screen::Screen(Video* v)
: width(0), height(0), video(v), clear(0), closed(0)
{
    if (video) {
        width  = video->Width();
        height = video->Height();
    }

    Mouse::Create(this);
}

Screen::~Screen()
{
    Mouse::Close();

    closed = 1;
    window_list.destroy();
}

// +--------------------------------------------------------------------+

bool
Screen::AddWindow(Window* c)
{
    if (!c || closed) return false;

    if (c->X() < 0) return false;
    if (c->Y() < 0) return false;
    if (c->X() + c->Width()  > Width())  return false;
    if (c->Y() + c->Height() > Height()) return false;

    if (!window_list.contains(c))
    window_list.append(c);

    return true;
}

bool
Screen::DelWindow(Window* c)
{
    if (!c || closed) return false;

    return window_list.remove(c) == c;
}

// +--------------------------------------------------------------------+

void
Screen::ClearAllFrames(bool clear_all)
{
    if (clear_all)
    clear = -1;
    else
    clear = 0;
}   

void
Screen::ClearNextFrames(int num_frames)
{
    if (clear >= 0 && clear < num_frames)
    clear = num_frames;
}

// +--------------------------------------------------------------------+

bool
Screen::SetBackgroundColor(Color c)
{
    if (video)
    return video->SetBackgroundColor(c);
    else
    return false;
}

// +--------------------------------------------------------------------+

bool
Screen::Resize(int w, int h)
{
    // scale all root-level windows to new screen size:

    ListIter<Window> iter = window_list;
    while (++iter) {
        Window* win = iter.value();
        Rect tmprect = win->GetRect();

        double  w_x = tmprect.x / (double) width;
        double  w_y = tmprect.y / (double) height;
        double  w_w = tmprect.w / (double) width;
        double  w_h = tmprect.h / (double) height;

        Rect    r;

        r.x = (int) (w_x * w);
        r.y = (int) (w_y * h);
        r.w = (int) (w_w * w);
        r.h = (int) (w_h * h);

        win->MoveTo(r);
    }

    width  = w;
    height = h;

    return true;
}

// +--------------------------------------------------------------------+

bool
Screen::Refresh()
{
    if (clear && !video->ClearAll())
    return false;

    video->StartFrame();

    ListIter<Window> iter = window_list;
    while (++iter) {
        Window* win = iter.value();
        
        if (win->IsShown()) {
            win->Paint();
        }
    }

    Mouse::Paint();

    video->EndFrame();

    if (clear > 0) clear--;
    return true;
}




