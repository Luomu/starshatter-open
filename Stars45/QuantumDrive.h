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
    FILE:         QuantumDrive.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Quantum (JUMP) Drive (system) class
*/

#ifndef QuantumDrive_h
#define QuantumDrive_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;
class SimRegion;

// +--------------------------------------------------------------------+

class QuantumDrive : public System
{
public:
    enum SUBTYPE { QUANTUM, HYPER };

    QuantumDrive(SUBTYPE s, double capacity, double sink_rate);
    QuantumDrive(const QuantumDrive& rhs);
    virtual ~QuantumDrive();

    enum ACTIVE_STATES {
        ACTIVE_READY, ACTIVE_COUNTDOWN, ACTIVE_PREWARP, ACTIVE_POSTWARP
    };

    void              SetDestination(SimRegion* rgn, const Point& loc);
    bool              Engage(bool immediate=false);
    int               ActiveState() const { return active_state; }
    double            WarpFactor()  const { return warp_fov;     }
    double            JumpTime()    const { return jump_time;    }
    virtual void      PowerOff();

    virtual void      ExecFrame(double seconds);

    void              SetShip(Ship* s)  { ship = s;    }
    Ship*             GetShip() const   { return ship; }

    double            GetCountdown() const    { return countdown; }
    void              SetCountdown(double d)  { countdown = d;    }

protected:
    void              Jump();
    void              AbortJump();

    int               active_state;

    Ship*             ship;
    double            warp_fov;
    double            jump_time;
    double            countdown;

    SimRegion*        dst_rgn;
    Point             dst_loc;
};

#endif // QuantumDrive_h

