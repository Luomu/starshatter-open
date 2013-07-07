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
    FILE:         Farcaster.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "Farcaster.h"
#include "QuantumDrive.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Explosion.h"
#include "Sim.h"
#include "Element.h"
#include "Instruction.h"

#include "Game.h"
#include "Solid.h"
#include "Light.h"
#include "Sound.h"
#include "DataLoader.h"

// +======================================================================+

Farcaster::Farcaster(double cap, double rate)
    : System(FARCASTER, 0, "Farcaster", 1, (float) cap, (float) cap, (float) rate),
      ship(0), dest(0), jumpship(0), cycle_time(10),
      active_state(QuantumDrive::ACTIVE_READY), warp_fov(1), no_dest(false)
{
    name = Game::GetText("sys.farcaster");
    abrv = Game::GetText("sys.farcaster.abrv");
}

// +----------------------------------------------------------------------+

Farcaster::Farcaster(const Farcaster& s)
    : System(s),
      ship(0), dest(0), start_rel(s.start_rel),
      end_rel(s.end_rel), jumpship(0), cycle_time(s.cycle_time),
      active_state(QuantumDrive::ACTIVE_READY), warp_fov(1), no_dest(false)
{
    Mount(s);
    SetAbbreviation(s.Abbreviation());

    for (int i = 0; i < NUM_APPROACH_PTS; i++)
    approach_rel[i] = s.approach_rel[i];
}

// +--------------------------------------------------------------------+

Farcaster::~Farcaster()
{
}

// +--------------------------------------------------------------------+

void
Farcaster::ExecFrame(double seconds)
{
    System::ExecFrame(seconds);

    if (ship && !no_dest) {
        if (!dest) {
            Element* elem = ship->GetElement();

            if (elem->NumObjectives()) {
                Sim*           sim = Sim::GetSim();
                Instruction*   obj = elem->GetObjective(0);

                if (obj)
                dest = sim->FindShip(obj->TargetName());
            }

            if (!dest)
            no_dest = true;
        }
        else {
            if (dest->IsDying() || dest->IsDead()) {
                dest = 0;
                no_dest = true;
            }
        }
    }

    // if no destination, show red nav lights:
    if (no_dest)
    energy = 0.0f;

    if (active_state == QuantumDrive::ACTIVE_READY && energy >= capacity &&
            ship && ship->GetRegion() && dest && dest->GetRegion()) {
        SimRegion*     rgn      = ship->GetRegion();
        SimRegion*     dst      = dest->GetRegion();
        ListIter<Ship> s_iter   = rgn->Ships();

        jumpship = 0;

        while (++s_iter) {
            Ship* s = s_iter.value();

            if (s == ship || s->IsStatic() || s->WarpFactor() > 1)
            continue;

            Point delta = s->Location() - ship->Location();

            // activate:
            if (delta.length() < 1000) {
                active_state = QuantumDrive::ACTIVE_PREWARP;
                jumpship = s;
                Observe(jumpship);
                break;
            }
        }
    }

    if (active_state == QuantumDrive::ACTIVE_READY)
    return;

    if (ship) {
        bool warping = false;

        if (active_state == QuantumDrive::ACTIVE_PREWARP) {
            if (warp_fov < 5000) {
                warp_fov *= 1.5;
            }
            else {
                Jump();
            }

            warping = true;
        }

        else if (active_state == QuantumDrive::ACTIVE_POSTWARP) {
            if (warp_fov > 1) {
                warp_fov *= 0.75;
            }
            else {
                warp_fov     = 1;
                active_state = QuantumDrive::ACTIVE_READY;
            }

            warping = true;
        }

        if (jumpship) {
            if (warping) {
                jumpship->SetWarp(warp_fov);

                SimRegion*     r        = ship->GetRegion();
                ListIter<Ship> neighbor = r->Ships();

                while (++neighbor) {
                    if (neighbor->IsDropship()) {
                        Ship* s     = neighbor.value();
                        Point delta = s->Location() - ship->Location();

                        if (delta.length() < 5e3)
                        s->SetWarp(warp_fov);
                    }
                }
            }
            else {
                warp_fov = 1;
                jumpship->SetWarp(warp_fov);
            }
        }
    }
}

void
Farcaster::Jump()
{
    Sim*        sim = Sim::GetSim();
    SimRegion*  rgn = ship->GetRegion();
    SimRegion*  dst = dest->GetRegion();

    sim->CreateExplosion(jumpship->Location(), Point(0,0,0), 
    Explosion::QUANTUM_FLASH, 1.0f, 0, rgn);
    sim->RequestHyperJump(jumpship, dst, dest->Location().OtherHand(), 0, ship, dest);

    energy = 0.0f;

    Farcaster* f = dest->GetFarcaster();
    if (f) f->Arrive(jumpship);

    active_state = QuantumDrive::ACTIVE_READY;
    warp_fov     = 1;
    jumpship     = 0;
}

void
Farcaster::Arrive(Ship* s)
{
    energy = 0.0f;

    active_state = QuantumDrive::ACTIVE_POSTWARP;
    warp_fov     = 5000;
    jumpship     = s;

    if (jumpship && jumpship->Velocity().length() < 500) {
        jumpship->SetVelocity(jumpship->Heading() * 500);
    }
}

// +----------------------------------------------------------------------+

void
Farcaster::SetApproachPoint(int i, Point loc)
{
    if (i >= 0 && i < NUM_APPROACH_PTS)
    approach_rel[i] = loc;
}

void
Farcaster::SetStartPoint(Point loc)
{
    start_rel = loc;
}

void
Farcaster::SetEndPoint(Point loc)
{
    end_rel = loc;
}

// +----------------------------------------------------------------------+

void
Farcaster::SetCycleTime(double t)
{
    cycle_time = t;
}

// +----------------------------------------------------------------------+

void
Farcaster::Orient(const Physical* rep)
{
    System::Orient(rep);

    Matrix        orientation = rep->Cam().Orientation();
    Point         loc         = rep->Location();

    start_point = (start_rel   * orientation) + loc;
    end_point   = (end_rel     * orientation) + loc;

    for (int i = 0; i < NUM_APPROACH_PTS; i++)
    approach_point[i] = (approach_rel[i] * orientation) + loc;
}

// +----------------------------------------------------------------------+

bool
Farcaster::Update(SimObject* obj)
{
    if (obj == jumpship) {
        jumpship->SetWarp(1);

        SimRegion*     r        = ship->GetRegion();
        ListIter<Ship> neighbor = r->Ships();

        while (++neighbor) {
            if (neighbor->IsDropship()) {
                Ship* s     = neighbor.value();
                Point delta = s->Location() - ship->Location();

                if (delta.length() < 5e3)
                s->SetWarp(1);
            }
        }

        jumpship = 0;
    }

    return SimObserver::Update(obj);
}

const char*
Farcaster::GetObserverName() const
{
    return Name();
}
