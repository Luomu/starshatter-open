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
    FILE:         Farcaster.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#ifndef Farcaster_h
#define Farcaster_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"
#include "SimObject.h"
#include "text.h"

// +----------------------------------------------------------------------+

class Ship;
class ShipDesign;
class Farcaster;

// +----------------------------------------------------------------------+

class Farcaster : public System, public SimObserver
{
public:
    Farcaster(double capacity, double sink_rate);
    Farcaster(const Farcaster& rhs);
    virtual ~Farcaster();

    enum CONSTANTS { NUM_APPROACH_PTS = 4 };

    virtual void   ExecFrame(double seconds);
    void           SetShip(Ship* s)     { ship  = s; }
    void           SetDest(Ship* d)     { dest  = d; }

    Point          ApproachPoint(int i) const { return approach_point[i];               }
    Point          StartPoint()         const { return start_point;                     }
    Point          EndPoint()           const { return end_point;                       }

    virtual void   SetApproachPoint(int i, Point loc);
    virtual void   SetStartPoint(Point loc);
    virtual void   SetEndPoint(Point loc);
    virtual void   SetCycleTime(double time);

    virtual void   Orient(const Physical* rep);

    // SimObserver:
    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    // accessors:
    const Ship*    GetShip()            const { return ship; }
    const Ship*    GetDest()            const { return dest; }

    int            ActiveState()        const { return active_state; }
    double         WarpFactor()         const { return warp_fov;     }

protected:
    virtual void      Jump();
    virtual void      Arrive(Ship* s);

    Ship*             ship;
    Ship*             dest;
    Ship*             jumpship;

    Point             start_rel;
    Point             end_rel;
    Point             approach_rel[NUM_APPROACH_PTS];

    Point             start_point;
    Point             end_point;
    Point             approach_point[NUM_APPROACH_PTS];

    double            cycle_time;
    int               active_state;
    double            warp_fov;

    bool              no_dest;
};

// +----------------------------------------------------------------------+

#endif Farcaster_h

