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
    FILE:         Screen.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    General Screen class - maintains and displays a list of windows
*/

#ifndef Screen_h
#define Screen_h

#include "Types.h"
#include "Color.h"
#include "List.h"

// +--------------------------------------------------------------------+

class  Bitmap;
class  Window;
struct Rect;

// +--------------------------------------------------------------------+

class Screen
{
public:
    static const char* TYPENAME() { return "Screen"; }

    Screen(Video* v);
    virtual ~Screen();

    virtual bool      SetBackgroundColor(Color c);

    virtual bool      Resize(int w, int h);
    virtual bool      Refresh();
    virtual bool      AddWindow(Window* c);
    virtual bool      DelWindow(Window* c);

    int               Width()     const { return width;   }
    int               Height()    const { return height;  }

    virtual void      ClearAllFrames(bool clear_all);
    virtual void      ClearNextFrames(int num_frames);

    virtual Video*    GetVideo()  const { return video;   }

protected:
    int               width;
    int               height;
    int               clear;
    int               closed;

    Video*            video;

    List<Window>      window_list;
};

#endif Screen_h

