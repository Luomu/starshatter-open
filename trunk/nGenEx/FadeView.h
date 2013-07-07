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
    FILE:         FadeView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Non-rendering view class that controls the fade level (fade-in/fade-out)
*/

#ifndef FadeView_h
#define FadeView_h

#include "Types.h"
#include "View.h"

// +--------------------------------------------------------------------+

class FadeView : public View
{
public:
    static const char* TYPENAME() { return "FadeView"; }

    enum FadeState { StateStart, State2, StateIn, StateHold, StateOut, StateDone };

    FadeView(Window* c, double fade_in=1, double fade_out=1, double hold_time=4);
    virtual ~FadeView();

    // Operations:
    virtual void      Refresh();
    virtual bool      Done()    const   { return state == StateDone; }
    virtual bool      Holding() const   { return state == StateHold; }

    // Control:
    virtual void      FastFade(int fade_fast);
    virtual void      FadeIn(double fade_in);
    virtual void      FadeOut(double fade_out);
    virtual void      StopHold();

protected:
    double      fade_in;
    double      fade_out;
    double      hold_time;
    double      time;
    double      step_time;

    int         fast;
    FadeState   state;
};

#endif FadeView_h

