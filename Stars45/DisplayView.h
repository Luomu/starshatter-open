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

    SUBSYSTEM:    Stars.exe
    FILE:         DisplayView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#ifndef DisplayView_h
#define DisplayView_h

#include "Types.h"
#include "View.h"
#include "SimObject.h"
#include "Color.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Bitmap;
class DisplayElement;
class Font;

// +--------------------------------------------------------------------+

class DisplayView : public View
{
public:
    DisplayView(Window* c);
    virtual ~DisplayView();

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      ExecFrame();
    virtual void      ClearDisplay();

    virtual void      AddText(const char*  txt, 
    Font*       font, 
    Color       color, 
    const Rect& rect,
    double      hold     = 1e9,
    double      fade_in  = 0,
    double      fade_out = 0);

    virtual void      AddImage(Bitmap*     bmp,
    Color       color, 
    int         blend,
    const Rect& rect,
    double      hold     = 1e9,
    double      fade_in  = 0,
    double      fade_out = 0);

    static DisplayView* GetInstance();

protected:
    int         width, height;
    double      xcenter, ycenter;

    List<DisplayElement>
    elements;
};

#endif DisplayView_h

