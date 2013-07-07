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
    FILE:         Slider.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Slider/Gauge ActiveWindow class
*/

#ifndef Slider_h
#define Slider_h

#include "Types.h"
#include "ActiveWindow.h"

// +--------------------------------------------------------------------+

class Slider : public ActiveWindow
{
public:
    static const char* TYPENAME() { return "Slider"; }

    enum { MAX_VAL=8, ORIENT_HORIZONTAL=0, ORIENT_VERTICAL=1 };

    Slider(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid);
    Slider(Screen* s,       int ax, int ay, int aw, int ah, DWORD aid);
    virtual ~Slider();

    // Operations:
    virtual void      Draw();

    // Event Target Interface:
    virtual int       OnMouseMove(int x, int y);
    virtual int       OnLButtonDown(int x, int y);
    virtual int       OnLButtonUp(int x, int y);
    virtual int       OnClick();

    // Property accessors:
    bool  GetActive();
    void  SetActive(bool bNewValue);
    Color GetFillColor();
    void  SetFillColor(Color c);
    bool  GetBorder();
    void  SetBorder(bool bNewValue);
    Color GetBorderColor();
    void  SetBorderColor(Color c);

    int   GetNumLeds();
    void  SetNumLeds(int nNewValue);
    int   GetOrientation();
    void  SetOrientation(int nNewValue);

    int   GetRangeMin();
    void  SetRangeMin(int nNewValue);
    int   GetRangeMax();
    void  SetRangeMax(int nNewValue);
    int   GetStepSize();
    void  SetStepSize(int nNewValue);
    int   GetThumbSize();
    void  SetThumbSize(int nNewValue);
    bool  GetShowThumb();
    void  SetShowThumb(bool bNewValue);

    int   NumValues();
    int   GetValue(int index=0);
    void  SetValue(int nNewValue, int index=0);
    double FractionalValue(int index=0);

    void  SetMarker(int nNewValue, int index=0);

    // Methods:
    void  StepUp(int index=0);
    void  StepDown(int index=0);

protected:
    int            captured;
    int            dragging;
    int            mouse_x;
    int            mouse_y;

    bool           active;        // true => slider; false => gauge
    bool           border;
    Color          border_color;
    Color          fill_color;    // default: dark blue

    int            num_leds;      // default: 1
    int            orientation;   // 0 => horizontal; !0 => vertical

    int            range_min;
    int            range_max;
    int            step_size;
    int            show_thumb;
    int            thumb_size;
    int            thumb_pos;

    int            nvalues;
    int            value[MAX_VAL];
    int            marker[2];
};

#endif Slider_h

