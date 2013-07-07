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
    FILE:         MultiController.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    MultiController Input class
*/

#include "MemDebug.h"
#include "MultiController.h"

// +--------------------------------------------------------------------+

MultiController::MultiController()
: x(0), y(0), z(0), p(0), r(0), w(0), c(0), p1(0), r1(0), w1(0), t(0)
{
    for (int i = 0; i < MotionController::MaxActions; i++)
    action[i] = 0;

    nctrl = 0;
    for (int i = 0; i < 4; i++)
    ctrl[i] = 0;
}

MultiController::~MultiController()
{
    for (int i = 0; i < 4; i++)
    delete ctrl[i];
}

// +--------------------------------------------------------------------+

void
MultiController::AddController(MotionController* c)
{
    if (nctrl < 4 && c)
    ctrl[nctrl++] = c;
}

void
MultiController::MapKeys(KeyMapEntry* mapping, int nkeys)
{
    for (int i = 0; i < nctrl; i++)
    ctrl[i]->MapKeys(mapping, nkeys);
}

int
MultiController::GetSwapYawRoll() const
{
    if (nctrl)
    return ctrl[0]->GetSwapYawRoll();

    return 0;
}

void
MultiController::SwapYawRoll(int swap)
{
    for (int i = 0; i < nctrl; i++)
    ctrl[i]->SwapYawRoll(swap);
}

// +--------------------------------------------------------------------+

inline void clamp(double& x) { if (x<-1)x=-1; else if (x>1)x=1; }

void
MultiController::Acquire()
{
    t = x = y = z = p = r = w = c = 0;

    for (int i = 0; i < MotionController::MaxActions; i++)
    action[i] = 0;

    for (int i = 0; i < nctrl; i++) {
        ctrl[i]->Acquire();

        x += ctrl[i]->X();
        y += ctrl[i]->Y();
        z += ctrl[i]->Z();

        r += ctrl[i]->Roll();
        p += ctrl[i]->Pitch();
        w += ctrl[i]->Yaw();
        c += ctrl[i]->Center();
        t += ctrl[i]->Throttle();
        
        for (int a = 0; a < MotionController::MaxActions; a++)
        action[a] += ctrl[i]->Action(a);
    }

    clamp(x);
    clamp(y);
    clamp(z);
    clamp(r);
    clamp(p);
    clamp(w);
    clamp(t);
}

// +--------------------------------------------------------------------+

void
MultiController::SetThrottle(double throttle)
{
    for (int i = 0; i < nctrl; i++)
    ctrl[i]->SetThrottle(throttle);
}

// +--------------------------------------------------------------------+

int
MultiController::ActionMap(int key)
{
    for (int i = 0; i < nctrl; i++) {
        int result = ctrl[i]->ActionMap(key);

        if (result)
        return result;
    }

    return 0;
}

