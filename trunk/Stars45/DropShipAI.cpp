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
    FILE:         DropShipAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Drop Ship (orbit/surface and surface/orbit) AI class
*/

#include "MemDebug.h"
#include "DropShipAI.h"
#include "TacticalAI.h"
#include "Ship.h"
#include "ShipCtrl.h"
#include "Drive.h"
#include "Sim.h"
#include "StarSystem.h"
#include "KeyMap.h"

#include "Game.h"

// +----------------------------------------------------------------------+

DropShipAI::DropShipAI(Ship* s)
    : ShipAI(s)
{
    seek_gain = 20;
    seek_damp = 0.5;

    delete tactical;
    tactical = 0;
}

DropShipAI::~DropShipAI()
{
}

// +--------------------------------------------------------------------+

void
DropShipAI::FindObjective()
{
    distance = 0;

    if (!ship)  return;

    Sim*        sim = Sim::GetSim();
    SimRegion*  self_rgn = ship->GetRegion();

    // if making orbit, go up:
    if (self_rgn->Type() == Sim::AIR_SPACE) {
        obj_w = self->Location() + Point(0, 1e3, 0);
    }

    // if breaking orbit, head for terrain region:
    else {
        SimRegion* dst_rgn = sim->FindNearestTerrainRegion(ship);
        Point      dst     = dst_rgn->GetOrbitalRegion()->Location() -
        self_rgn->GetOrbitalRegion()->Location() +
        Point(0, 0, -1e6);
        
        obj_w =    dst.OtherHand();
    }

    // distance from self to navpt:
    distance = Point(obj_w - self->Location()).length();

    // transform into camera coords:
    objective = Transform(obj_w);
    objective.Normalize();
}

// +--------------------------------------------------------------------+

void
DropShipAI::Navigator()
{
    accumulator.Clear();
    magnitude = 0;

    if (other)
    ship->SetFLCSMode(Ship::FLCS_AUTO);
    else
    ship->SetFLCSMode(Ship::FLCS_MANUAL);

    Accumulate(AvoidCollision());
    Accumulate(Seek(objective));

    // are we being asked to flee?
    if (fabs(accumulator.yaw) == 1.0 && accumulator.pitch == 0.0) {
        accumulator.pitch  = -0.7f;
        accumulator.yaw   *= 0.25f;
    }

    self->ApplyRoll((float) (accumulator.yaw * -0.4));
    self->ApplyYaw((float) (accumulator.yaw * 0.2));

    if (fabs(accumulator.yaw) > 0.5 && fabs(accumulator.pitch) < 0.1)
    accumulator.pitch -= 0.1f;

    if (accumulator.pitch != 0)
    self->ApplyPitch((float) accumulator.pitch);

    // if not turning, roll to orient with world coords:
    if (fabs(accumulator.yaw) < 0.1) {
        Point vrt = ((Camera*) &(self->Cam()))->vrt();
        double deflection = vrt.y;
        if (deflection != 0) {
            double theta = asin(deflection/vrt.length());
            self->ApplyRoll(-theta);
        }
    }

    ship->SetThrottle(100);
    ship->ExecFLCSFrame();
}

