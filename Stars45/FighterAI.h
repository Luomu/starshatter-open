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
    FILE:         FighterAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fighter (low-level) Artifical Intelligence class
*/

#ifndef FighterAI_h
#define FighterAI_h

#include "Types.h"
#include "ShipAI.h"

// +--------------------------------------------------------------------+

class Ship;
class Shot;
class InboundSlot;

// +--------------------------------------------------------------------+

class FighterAI : public ShipAI
{
public:
    FighterAI(SimObject* s);
    virtual ~FighterAI();

    virtual void      ExecFrame(double seconds);
    virtual int       Subframe()  const { return true; }

    // convert the goal point from world to local coords:
    virtual void      FindObjective();
    virtual void      FindObjectiveNavPoint();

protected:
    // behaviors:
    virtual Steer     AvoidTerrain();
    virtual Steer     SeekTarget();
    virtual Steer     EvadeThreat();
    virtual Point     ClosingVelocity();

    // accumulate behaviors:
    virtual void      Navigator();

    // steering functions:
    virtual Steer     Seek(const Point& point);
    virtual Steer     SeekFormationSlot();

    // fire on target if appropriate:
    virtual void      FireControl();
    virtual void      HelmControl();
    virtual void      ThrottleControl();

    virtual double    CalcDefensePerimeter(Ship* starship);
    virtual void      ReturnToBase(Ship* controller);

    Shot*             decoy_missile;
    double            missile_time;
    int               terrain_warning;
    int               drop_state;
    char              dir_info[32];
    double            brakes;
    double            z_shift;
    double            time_to_dock;
    InboundSlot*      inbound;
    int               rtb_code;
    bool              evading;
    DWORD             jink_time;
    Point             jink;
    bool              over_threshold;
    bool              form_up;
    bool              go_manual;
};

// +--------------------------------------------------------------------+

#endif FighterAI_h

