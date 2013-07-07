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

    SUBSYSTEM:    Stars
    FILE:         ShipKiller.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ShipKiller class implementation
*/

#include "MemDebug.h"
#include "ShipKiller.h"
#include "Sim.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "System.h"
#include "Weapon.h"
#include "Shot.h"
#include "Explosion.h"
#include "Debris.h"
#include "HUDSounds.h"

#include "Solid.h"
#include "Random.h"

// +----------------------------------------------------------------------+

ShipKiller::ShipKiller(Ship* s)
: ship(s), DEATH_CAM_LINGER(5.0f), time(0.0f), exp_time(0.0f), exp_index(0)
{
}

ShipKiller::~ShipKiller()
{
}

// +----------------------------------------------------------------------+

inline int random_index()
{
    return (int) (rand()/3277);
}

// +----------------------------------------------------------------------+

void
ShipKiller::BeginDeathSpiral()
{
    if (!ship) return;

    // shut down all ship systems:
    ListIter<System> iter = ship->Systems();
    while (++iter) {
        iter->PowerOff();
        iter->SetPowerLevel(0);

        if (iter->Type() == System::WEAPON) {
            Weapon* gun  = (Weapon*) iter.value();

            for (int i = 0; i < Weapon::MAX_BARRELS; i++) {
                Shot* beam = gun->GetBeam(i);
                if (beam)
                beam->Destroy();
            }
        }
    }

    if (ship->GetShieldRep())
    ship->GetShieldRep()->Hide();

    Sim*              sim    = Sim::GetSim();
    const ShipDesign* design = ship->Design();

    float  time_to_go = design->death_spiral_time;
    time = DEATH_CAM_LINGER + time_to_go;
    loc  = ship->Location() + ship->Velocity() * (time_to_go-1.0f);

    if (rand() < 16000)
    loc += ship->BeamLine() * -3 * ship->Radius();
    else
    loc += ship->BeamLine() *  3 * ship->Radius();

    if (rand() < 8000)
    loc += ship->LiftLine() * -1 * ship->Radius();
    else
    loc += ship->LiftLine() *  2 * ship->Radius();

    // stop on crash:
    if (ship->IsGroundUnit() || (ship->IsAirborne() && ship->AltitudeAGL() < ship->Radius()*2)) {
        time = DEATH_CAM_LINGER;
        loc  = ship->Location() + Point(6*ship->Radius(), 7*ship->Radius(), 8*ship->Radius());
        ship->SetVelocity(Point(0,0,0));
    }
    // else, slow tumble:
    else {
        Point torque = RandomVector(ship->Mass()/7);
        ship->ApplyTorque(torque);

        for (int i = 0; i < 5; i++) {
            exp_index = random_index() % ShipDesign::MAX_EXPLOSIONS;
            if (design->explosion[exp_index].type > 0 && !design->explosion[exp_index].final)
            break;
        }

        float exp_scale = design->explosion_scale;
        if (exp_scale <= 0)
        exp_scale = design->scale;

        exp_time  = design->explosion[exp_index].time;

        if (design->explosion[exp_index].type > 0) {
            Point exp_loc = ship->Location() + (design->explosion[exp_index].loc * ship->Cam().Orientation());
            sim->CreateExplosion(exp_loc,
            ship->Velocity(), 
            design->explosion[exp_index].type, 
            (float) ship->Radius(), 
            exp_scale, 
            ship->GetRegion(), 
            ship);
        }
    }

    ship->SetControls(0);
    ship->SetupAgility();
}

// +----------------------------------------------------------------------+

void
ShipKiller::ExecFrame(double seconds)
{
    Sim*              sim    = Sim::GetSim();
    const ShipDesign* design = ship->Design();

    time     -= (float) seconds;
    exp_time -= (float) seconds;

    float exp_scale = design->explosion_scale;
    if (exp_scale <= 0)
    exp_scale = design->scale;

    if (exp_time < 0) {
        exp_index++;
        if (exp_index >= ShipDesign::MAX_EXPLOSIONS || design->explosion[exp_index].final)
        exp_index = 0;

        exp_time  = design->explosion[exp_index].time;

        if (design->explosion[exp_index].type > 0) {
            Point exp_loc = ship->Location() + (design->explosion[exp_index].loc * ship->Cam().Orientation());
            sim->CreateExplosion(exp_loc,
            ship->Velocity(), 
            design->explosion[exp_index].type, 
            (float) ship->Radius(), 
            exp_scale, 
            ship->GetRegion(), 
            ship);
        }
    }

    if (time < DEATH_CAM_LINGER) {
        for (int i = 0; i < ShipDesign::MAX_EXPLOSIONS; i++) {
            if (design->explosion[i].final) {
                Point exp_loc = ship->Location() + (design->explosion[i].loc * ship->Cam().Orientation());
                sim->CreateExplosion(exp_loc,
                ship->Velocity(), 
                design->explosion[i].type, 
                (float) ship->Radius(), 
                exp_scale, 
                ship->GetRegion());
            }
        }

        for (int i = 0; i < ShipDesign::MAX_DEBRIS; i++) {
            if (design->debris[i].model) {
                Point debris_loc = ship->Location() + (design->debris[i].loc * ship->Cam().Orientation());
                Point debris_vel = debris_loc - ship->Location();
                debris_vel.Normalize();

                if (design->debris[i].speed > 0)
                debris_vel *= design->debris[i].speed;
                else
                debris_vel *= 200;

                if (ship->IsGroundUnit()) {
                    debris_vel *= 2;
                    
                    if (debris_vel.y < 0)
                    debris_vel.y *= -1;
                }

                for (int n = 0; n < design->debris[i].count; n++) {
                    Debris* debris = sim->CreateDebris(debris_loc,
                    debris_vel + ship->Velocity(),
                    design->debris[i].model,
                    design->debris[i].mass,
                    ship->GetRegion());

                    debris->SetLife(design->debris[i].life);
                    debris->SetDrag(design->debris[i].drag);

                    if (n == 0) {
                        debris->CloneCam(ship->Cam());
                        debris->MoveTo(debris_loc);
                    }

                    for (int fire = 0; fire < 5; fire++) {
                        if (design->debris[i].fire_loc[fire] == Vec3(0,0,0))
                        continue;

                        Point fire_loc = debris->Location() + (design->debris[i].fire_loc[fire] * debris->Cam().Orientation());

                        if (design->debris[i].fire_type > 0) {
                            sim->CreateExplosion(fire_loc,
                            ship->Velocity(), 
                            design->debris[i].fire_type, 
                            exp_scale,
                            exp_scale, 
                            ship->GetRegion(),
                            debris);
                        }
                        else {
                            sim->CreateExplosion(fire_loc,
                            ship->Velocity(), 
                            Explosion::SMALL_FIRE, 
                            exp_scale,
                            exp_scale, 
                            ship->GetRegion(),
                            debris);

                            sim->CreateExplosion(fire_loc,
                            ship->Velocity(), 
                            Explosion::SMOKE_TRAIL, 
                            exp_scale * 0.25f,
                            exp_scale * 0.25f, 
                            ship->GetRegion(),
                            debris);
                        }
                    }

                    if (n+1 < design->debris[i].count) {
                        debris_vel = RandomVector(1);

                        if (design->debris[i].speed > 0)
                        debris_vel *= design->debris[i].speed * Random(0.8, 1.2);
                        else
                        debris_vel *= 300 + rand()/50;
                    }
                }
            }
        }

        if (ship == sim->GetPlayerShip())
        HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);

        sim->CreateSplashDamage(ship);
        ship->Destroy(); // CAREFUL!!!  This will also delete this object!
    }
}
