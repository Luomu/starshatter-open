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
    FILE:         FadeView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fading Bitmap "billboard" Image View class
*/

#include "MemDebug.h"
#include "FadeView.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "Game.h"

// +--------------------------------------------------------------------+

FadeView::FadeView(Window* c, double in, double out, double hold)
: View(c),
fade_in(in * 1000),
fade_out(out * 1000),
hold_time(hold * 1000),
step_time(0),
fast(1),
time(0)
{
    state = StateStart;
}

FadeView::~FadeView()
{ }

// +--------------------------------------------------------------------+

void FadeView::FadeIn(double in)       { fade_in   = in  * 1000; }
void FadeView::FadeOut(double out)     { fade_out  = out * 1000; }
void FadeView::FastFade(int fade_fast) { fast      = fade_fast;  }
void FadeView::StopHold()
{
    //Print("  FadeView::StopHold()\n");
    hold_time = 0;
}

// +--------------------------------------------------------------------+

void
FadeView::Refresh()
{
    double msec = 0;

    if (state == StateStart) {
        time = Game::RealTime();
    }
    else if (state != StateDone) {
        double new_time = Game::RealTime();
        msec = new_time - time;
        time = new_time;
    }

    switch (state) {
    case StateStart:
        if (fade_in) {
            //Print("  *  FadeView: %f, %f, %f\n", fade_in, fade_out, hold_time);
            Color::SetFade(0);
            //Print("  1. FadeView SetFade to   0  (%6.1f)\n", time);
        }

        step_time = 0;
        state = State2;
        break;

    case State2:
        if (fade_in) {
            Color::SetFade(0);
            //Print("  1. FadeView SetFade to   0  (%6.1f)\n", time);
        }

        step_time = 0;
        state = StateIn;
        break;

    case StateIn:
        if (step_time < fade_in) {
            double fade = step_time / fade_in;
            Color::SetFade(fade);
            //Print("  2. FadeView SetFade to %3d  (%6.1f) %6.1f\n", (int) (fade * 100), time, step_time);
            step_time += msec;
        }
        else {
            Color::SetFade(1);
            //Print("  2. FadeView SetFade to %3d  (%6.1f) %6.1f => HOLDING\n", 100, time, step_time);
            step_time = 0;
            state = StateHold;
        }
        break;

    case StateHold:
        if (step_time < hold_time) {
            step_time += msec;
            //Print("  3. FadeView holding at %3d  (%6.1f) %6.1f\n", 100, time, step_time);
        }
        else {
            //Print("  3. FadeView HOLD COMPLETE   (%6.1f) %6.1f\n", time, step_time);
            step_time = 0;
            state = StateOut;
        }
        break;

    case StateOut:
        if (fade_out > 0) {
            if (step_time < fade_out) {
                double fade = 1 - step_time / fade_out;
                Color::SetFade(fade);
                //Print("  4. FadeView SetFade to %3d  (%6.1f) %6.1f\n", (int) (fade*100), time, step_time);
                step_time += msec;
            }
            else {
                Color::SetFade(0);
                //Print("  4. FadeView SetFade to %3d  (%6.1f) %6.1f\n", 0, time, step_time);
                step_time = 0;
                state = StateDone;
            }
        }
        else {
            Color::SetFade(1);
            //Print("  4. FadeView SetFade to %3d  (%6.1f) %6.1f\n", 0, time, step_time);
            step_time = 0;
            state = StateDone;
        }
        break;

    default:
    case StateDone:
        //Print("  5. FadeView done  (%6.1f) %6.1f\n", time, step_time);
        break;
    }
}

