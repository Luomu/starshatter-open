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
    FILE:         LandingGear.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fighter undercarriage (landing gear) system class
*/

#ifndef LandingGear_h
#define LandingGear_h

#include "Types.h"
#include "System.h"
#include "Solid.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class LandingGear : public System
{
public:
    enum CONSTANTS  { MAX_GEAR = 4 };
    enum GEAR_STATE { GEAR_UP, GEAR_LOWER, GEAR_DOWN, GEAR_RAISE };

    LandingGear();
    LandingGear(const LandingGear& rhs);
    virtual ~LandingGear();

    virtual int       AddGear(Model* m, const Point& s, const Point& e);
    virtual void      ExecFrame(double seconds);
    virtual void      Orient(const Physical* rep);

    GEAR_STATE        GetState()        const { return state; }
    void              SetState(GEAR_STATE s);
    int               NumGear()         const { return ngear; }
    Solid*            GetGear(int i);
    Point             GetGearStop(int i);
    double            GetTouchDown();
    double            GetClearance()    const { return clearance; }

    static void       Initialize();
    static void       Close();

protected:
    GEAR_STATE        state;
    double            transit;
    double            clearance;

    int               ngear;
    Model*            models[MAX_GEAR];
    Solid*            gear[MAX_GEAR];
    Point             start[MAX_GEAR];
    Point             end[MAX_GEAR];
};

#endif LandingGear_h

