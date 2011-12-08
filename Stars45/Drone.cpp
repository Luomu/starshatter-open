/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Drone.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Laser and Missile class
*/

#include "MemDebug.h"
#include "Drone.h"
#include "Weapon.h"
#include "Ship.h"
#include "Sim.h"
#include "Explosion.h"

#include "Game.h"
#include "Bolt.h"
#include "Sprite.h"
#include "Solid.h"
#include "Light.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Sound.h"

// +--------------------------------------------------------------------+

Drone::Drone(const Point& pos, const Camera& shot_cam, WeaponDesign* dsn, const Ship* ship)
   : Shot(pos, shot_cam, dsn, ship), 
     decoy_type(0), iff_code(0)
{
   obj_type       = SimObject::SIM_DRONE;

   if (dsn) {
      decoy_type  = dsn->decoy_type;
      probe       = dsn->probe;
      integrity   = dsn->integrity;
      sprintf(name, "Drone %04d", Identity());
   }
}

// +--------------------------------------------------------------------+

Drone::~Drone()
{
}

// +--------------------------------------------------------------------+

void
Drone::SeekTarget(SimObject* target, System* sub)
{
   if (!probe)
   Shot::SeekTarget(target, sub);
}

// +--------------------------------------------------------------------+

void
Drone::ExecFrame(double seconds)
{
   Shot::ExecFrame(seconds);
}

// +--------------------------------------------------------------------+

void
Drone::Disarm()
{
   Shot::Disarm();
}

// +--------------------------------------------------------------------+

void
Drone::Destroy()
{
   Shot::Destroy();
}

// +--------------------------------------------------------------------+

double
Drone::PCS() const
{
   if (decoy_type == 0 && !probe)
      return 10e3;

   return 0;
}

double
Drone::ACS() const
{
   if (decoy_type == 0 && !probe)
      return 1e3;

   return 0;
}

// +--------------------------------------------------------------------+

const char*
Drone::ClassName() const
{
   return Ship::ClassName(decoy_type);
}

int
Drone::Class() const
{
   return decoy_type;
}

// +--------------------------------------------------------------------+

int
Drone::HitBy(Shot* shot, Point& impact)
{
   if (life == 0 || !shot->IsArmed()) return 0;

   const int HIT_NOTHING   = 0;
   const int HIT_HULL      = 1;

   Point    hull_impact;
   int      hit_type = HIT_NOTHING;
   Point    shot_loc = shot->Location();
   Point    shot_org = shot->Origin();
   Point    delta    = shot_loc - Location();
   double   dlen     = delta.length();
   double   dscale   = 1;
   float    scale    = design->explosion_scale;
   Sim*     sim      = Sim::GetSim();

   if (scale <= 0)
      scale = design->scale;

   // MISSILE PROCESSING ------------------------------------------------

   if (shot->IsMissile()) {
      if (dlen < 10 * Radius()) {
         hull_impact = impact = shot_loc;
         sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH,   0.3f * scale, scale, region);
         sim->CreateExplosion(impact, Point(),    Explosion::SHOT_BLAST,   2.0f,         scale, region);
         hit_type = HIT_HULL;
      }
   }

   // ENERGY WEP PROCESSING ---------------------------------------------

   else {
      if (shot->IsBeam()) {
         // check right angle spherical distance:
         Point  d0   = Location() - shot_org;
         Point  w    = shot_loc   - shot_org; w.Normalize();
         Point  test = shot_org + w * (d0 * w);
         Point  d1   = test - Location();
         double dlen = d1.length();          // distance of point from line

         if (dlen < 2*Radius()) {
            hull_impact = impact = test;
            shot->SetBeamPoints(shot_org, impact);
            sim->CreateExplosion(impact, Velocity(), Explosion::BEAM_FLASH, 0.30f * scale, scale, region);
            hit_type = HIT_HULL;
         }
      }
      else if (dlen < 2*Radius()) {
         hull_impact = impact = shot_loc;
         sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH, 0.30f * scale, scale, region);
         hit_type = HIT_HULL;
      }
   }
 
   // DAMAGE RESOLUTION -------------------------------------------------

   if (hit_type != HIT_NOTHING) {
      double effective_damage = shot->Damage() * dscale;

      if (shot->IsBeam()) {
         effective_damage *= Game::FrameTime();
      }
      else {
         ApplyTorque(shot->Velocity() * (float) effective_damage * 1e-6f);
      }

      if (effective_damage > 0)
         Physical::InflictDamage(effective_damage);
   }
   
   return hit_type;
}
