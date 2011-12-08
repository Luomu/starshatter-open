/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Debris.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Debris Sprite animation class
*/

#include "MemDebug.h"
#include "Debris.h"
#include "Shot.h"
#include "Explosion.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Terrain.h"

#include "Solid.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"
#include "Random.h"

// +--------------------------------------------------------------------+

Debris::Debris(Model* model, const Vec3& pos, const Vec3& vel, double m)
   : SimObject("Debris", SimObject::SIM_DEBRIS) 
{
   MoveTo(pos);

   velocity  = vel;
   mass      = (float) m;
   integrity = mass * 10.0f;
   life      = 300;

   Solid* solid = new(__FILE__,__LINE__) Solid;

   if (solid) {
      solid->UseModel(model);
      solid->MoveTo(pos);

      rep = solid;

      radius = solid->Radius();
   }

   Point torque = RandomVector(Mass()/2);

   if (Mass() < 10)
      torque *= (rand() / 3200);
   else if (Mass() > 10e3)
      torque *= 0.25;
   else if (Mass() > 10e6)
      torque *= 0.005;

   ApplyTorque(torque);
}

// +--------------------------------------------------------------------+

int
Debris::HitBy(Shot* shot, Point& impact)
{
   if (!shot->IsArmed()) return 0;

   const int HIT_NOTHING   = 0;
   const int HIT_HULL      = 1;

   Point    hull_impact;
   int      hit_type = HIT_NOTHING;
   bool     hit_hull = true;
   Point    shot_loc = shot->Location();
   Point    delta    = shot_loc - Location();
   double   dlen     = delta.length();
   double   dscale   = 1;
   float    scale    = 1.0f;
   Sim*     sim      = Sim::GetSim();

   // MISSILE PROCESSING ------------------------------------------------

   if (shot->IsMissile()) {
      if (dlen < Radius()) {
         hull_impact = impact = shot_loc;
         sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH,   0.3f * scale, scale, region, this);
         sim->CreateExplosion(impact, Point(),    Explosion::SHOT_BLAST,   2.0f,         scale, region);
         hit_type = HIT_HULL;
      }
   }

   // ENERGY WEP PROCESSING ---------------------------------------------

   else {

      Solid* solid = (Solid*) rep;
   
      Point  shot_loc = shot->Location();
      Point  shot_vpn = shot_loc - shot->Origin();
      double shot_len = shot_vpn.Normalize();
      if (shot_len == 0) shot_len = 1000;
   
      // impact:
      if (solid) {
         if (solid->CheckRayIntersection(shot->Origin(), shot_vpn, shot_len, impact)) {
            // trim beam shots to impact point:
            if (shot->IsBeam())
               shot->SetBeamPoints(shot->Origin(), impact);

            hull_impact = impact;

            if (shot->IsBeam())
               sim->CreateExplosion(impact, Velocity(), Explosion::BEAM_FLASH, 0.30f * scale, scale, region, this);
            else
               sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH, 0.30f * scale, scale, region, this);

            Point burst_vel = hull_impact - Location();
            burst_vel.Normalize();
            burst_vel *= Radius() * 0.5;
            burst_vel += Velocity();

            sim->CreateExplosion(hull_impact, burst_vel, Explosion::HULL_BURST, 0.50f * scale, scale, region, this);

            hit_type = HIT_HULL;
            hit_hull = true;
         }
      }

      else {
         if (dlen < Radius()) {
            hull_impact = impact = shot_loc;

            if (shot->IsBeam())
               sim->CreateExplosion(impact, Velocity(), Explosion::BEAM_FLASH, 0.30f * scale, scale, region, this);
            else
               sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH, 0.30f * scale, scale, region, this);

            hit_type = HIT_HULL;
         }
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

// +--------------------------------------------------------------------+

void
Debris::ExecFrame(double seconds)
{
   if (GetRegion()->Type() == SimRegion::AIR_SPACE) {
      if (AltitudeAGL() < Radius()) {
         velocity          = Point();
         arcade_velocity   = Point();

         Terrain* terrain  = region->GetTerrain();

         if (terrain) {
            Point loc = Location();
            MoveTo(Point(loc.x, terrain->Height(loc.x, loc.z), loc.z));
         }
      }
      else {
         if (mass > 100) {
            Orbital* primary = GetRegion()->GetOrbitalRegion()->Primary();

            const double   GRAV = 6.673e-11;
            double         m0   = primary->Mass();
            double         r    = primary->Radius();

            SetDrag(0.001);
            SetGravity(6 * GRAV * m0 / (r*r));  // accentuate gravity
            SetBaseDensity(1.0f);
         }

         AeroFrame(seconds);
      }
   }
   else {
      Physical::ExecFrame(seconds);
   }
}

// +--------------------------------------------------------------------+

double
Debris::AltitudeAGL() const
{
   Point    loc          = Location();
   double   altitude_agl = loc.y;

   Terrain* terrain      = region->GetTerrain();

   if (terrain)
      altitude_agl -= terrain->Height(loc.x, loc.z);

   if (!_finite(altitude_agl))
      altitude_agl = 0;

   return altitude_agl;
}