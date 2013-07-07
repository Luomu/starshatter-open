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
    FILE:         LandingGear.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    LandingGear System class
*/

#include "MemDebug.h"
#include "LandingGear.h"
#include "Ship.h"
#include "Sim.h"
#include "AudioConfig.h"

#include "DataLoader.h"
#include "Physical.h"
#include "Scene.h"
#include "Sound.h"
#include "Game.h"

static Sound* gear_transit_sound = 0;

// +----------------------------------------------------------------------+

LandingGear::LandingGear()
: System(MISC_SYSTEM, 0, "Landing Gear", 1, 1, 1, 1),
state(GEAR_UP), transit(0), ngear(0), clearance(0)
{
    name = Game::GetText("sys.landing-gear");
    abrv = Game::GetText("sys.landing-gear.abrv");

    for (int i = 0; i < MAX_GEAR; i++) {
        models[i] = 0;
        gear[i] = 0;
    }
}

// +----------------------------------------------------------------------+

LandingGear::LandingGear(const LandingGear& g)
: System(g), state(GEAR_UP), transit(0), ngear(g.ngear), clearance(0)
{
    Mount(g);
    SetAbbreviation(g.Abbreviation());
    int i;

    for (i = 0; i < ngear; i++) {
        models[i] = 0;
        gear[i]   = new(__FILE__,__LINE__) Solid;
        start[i]  = g.start[i];
        end[i]    = g.end[i];

        gear[i]->UseModel(g.models[i]);

        if (clearance > end[i].y)
        clearance = end[i].y;
    }

    while (i < MAX_GEAR) {
        models[i] = 0;
        gear[i]   = 0;
        i++;
    }

    clearance += mount_rel.y;
}

// +--------------------------------------------------------------------+

LandingGear::~LandingGear()
{
    for (int i = 0; i < MAX_GEAR; i++) {
        delete models[i];

        if (gear[i]) {
            Solid* g = gear[i];

            if (g->GetScene())
            g->GetScene()->DelGraphic(g);
            delete g;
        }
    }
}

// +--------------------------------------------------------------------+

void
LandingGear::Initialize()
{
    if (!gear_transit_sound) {
        DataLoader* loader = DataLoader::GetLoader();
        loader->SetDataPath("Sounds/");
        loader->LoadSound("GearTransit.wav", gear_transit_sound);
    }
}

// +--------------------------------------------------------------------+

void
LandingGear::Close()
{
    delete gear_transit_sound;
    gear_transit_sound = 0;
}

// +--------------------------------------------------------------------+

int
LandingGear::AddGear(Model* m, const Point& s, const Point& e)
{
    if (ngear < MAX_GEAR) {
        models[ngear] = m;
        start[ngear]  = s;
        end[ngear]    = e;

        ngear++;
    }

    return ngear;
}

// +--------------------------------------------------------------------+

void
LandingGear::SetState(GEAR_STATE s)
{
    if (state != s) {
        state = s;

        if (ship && ship == Sim::GetSim()->GetPlayerShip()) {
            if (state == GEAR_LOWER || state == GEAR_RAISE) {
                if (gear_transit_sound) {
                    Sound* sound = gear_transit_sound->Duplicate();
                    sound->SetVolume(AudioConfig::EfxVolume());
                    sound->Play();
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
LandingGear::ExecFrame(double seconds)
{
    System::ExecFrame(seconds);

    switch (state) {
    case GEAR_UP:
        transit = 0;
        break;

    case GEAR_DOWN:
        transit = 1;
        break;

    case GEAR_LOWER:
        if (transit < 1) {
            transit += seconds;

            Scene* s = 0;
            if (ship && ship->Rep())
            s = ship->Rep()->GetScene();

            if (s) {
                for (int i = 0; i < ngear; i++) {
                    if (gear[i] && !gear[i]->GetScene()) {
                        s->AddGraphic(gear[i]);
                    }
                }
            }
        }
        else {
            transit = 1;
            state   = GEAR_DOWN;
        }
        break;

    case GEAR_RAISE:
        if (transit > 0) {
            transit -= seconds;
        }
        else {
            transit = 0;
            state   = GEAR_UP;

            for (int i = 0; i < ngear; i++) {
                if (gear[i]) {
                    Scene* s = gear[i]->GetScene();
                    if (s) s->DelGraphic(gear[i]);
                }
            }
        }
        break;
    }
}

// +--------------------------------------------------------------------+

void
LandingGear::Orient(const Physical* rep)
{
    System::Orient(rep);

    const Matrix& orientation = rep->Cam().Orientation();
    Point         ship_loc    = rep->Location();

    for (int i = 0; i < ngear; i++) {
        Point gloc;
        if (transit < 1)  gloc = start[i] + (end[i]-start[i])*transit;
        else              gloc = end[i];

        Point projector = (gloc * orientation) + ship_loc;
        if (gear[i]) {
            gear[i]->MoveTo(projector);
            gear[i]->SetOrientation(orientation);
        }
    }
}

// +--------------------------------------------------------------------+

Solid*
LandingGear::GetGear(int index)
{
    if (index >= 0 && index < ngear) {
        Solid* g = gear[index];
        return g;
    }

    return 0;
}

// +--------------------------------------------------------------------+

Point
LandingGear::GetGearStop(int index)
{
    if (index >= 0 && index < ngear) {
        Solid* g = gear[index];

        if (g)
        return g->Location();
    }

    return Point();
}

// +--------------------------------------------------------------------+

double
LandingGear::GetTouchDown()
{
    double down = 0;

    if (ship) {
        down = ship->Location().y;

        if (state != GEAR_UP) {
            for (int i = 0; i < ngear; i++) {
                if (gear[i]) {
                    Point stop = gear[i]->Location();

                    if (stop.y < down)
                    down = stop.y;
                }
            }
        }
    }

    return down;
}