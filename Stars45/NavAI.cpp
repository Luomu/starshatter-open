/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NavAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Automatic Navigator Artificial Intelligence class
*/

#include "MemDebug.h"
#include "NavAI.h"
#include "TacticalAI.h"
#include "Instruction.h"
#include "NavSystem.h"
#include "QuantumDrive.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "ShipCtrl.h"
#include "Drive.h"
#include "Farcaster.h"
#include "Shield.h"
#include "Sim.h"
#include "StarSystem.h"
#include "KeyMap.h"
#include "HUDView.h"
#include "HUDSounds.h"

#include "Game.h"

// +----------------------------------------------------------------------+

NavAI::NavAI(Ship* s)
   : ShipAI(s), complete(false), brakes(0),
     drop_state(0), quantum_state(0), farcaster(0), terrain_warning(false)
{
   seek_gain = 20;
   seek_damp = 0.55;

   delete tactical;
   tactical = 0;
}

// +--------------------------------------------------------------------+

NavAI::~NavAI()
{ }

// +--------------------------------------------------------------------+

void
NavAI::ExecFrame(double s)
{
   if (!ship) return;

   seconds = s;

   ship->SetDirectorInfo(" ");

   if (ship->GetFlightPhase() == Ship::TAKEOFF)
      takeoff = true;

   else if (takeoff && ship->MissionClock() > 10000)
      takeoff = false;

   FindObjective();
   Navigator();

   // watch for disconnect:
   if (ShipCtrl::Toggled(KEY_AUTO_NAV)) {
      NavSystem* navsys = ship->GetNavSystem();
      if (navsys) {
         HUDView::GetInstance()->SetHUDMode(HUDView::HUD_MODE_TAC);
         navsys->DisengageAutoNav();

         Sim* sim = Sim::GetSim();
         if (sim) {
            ship->SetControls(sim->GetControls());
            return;
         }
      }
   }

   static double time_til_change = 0.0;

   if (time_til_change < 0.001) {
      if (ship->GetShield()) {
         Shield* shield = ship->GetShield();
         double  level  = shield->GetPowerLevel();

         if (ShipCtrl::KeyDown(KEY_SHIELDS_FULL)) {
            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(100);
            time_til_change = 0.5f;
         }

         else if (ShipCtrl::KeyDown(KEY_SHIELDS_ZERO)) {
            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(0);
            time_til_change = 0.5f;
         }

         else if (ShipCtrl::KeyDown(KEY_SHIELDS_UP)) {
            if (level < 25)      level =  25;
            else if (level < 50) level =  50;
            else if (level < 75) level =  75;
            else                 level = 100;

            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(level);
            time_til_change = 0.5f;
         }

         else if (ShipCtrl::KeyDown(KEY_SHIELDS_DOWN)) {
            if (level > 75)      level =  75;
            else if (level > 50) level =  50;
            else if (level > 25) level =  25;
            else                 level =   0;

            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(level);
            time_til_change = 0.5f;
         }

      }
   }
   else {
      time_til_change -= seconds;
   }

   if (ShipCtrl::Toggled(KEY_DECOY))
      ship->FireDecoy();

   if (ShipCtrl::Toggled(KEY_LAUNCH_PROBE))
      ship->LaunchProbe();

   if (ShipCtrl::Toggled(KEY_GEAR_TOGGLE))
      ship->ToggleGear();

   if (ShipCtrl::Toggled(KEY_NAVLIGHT_TOGGLE))
      ship->ToggleNavlights();

   if (drop_state < 0) {
      ship->DropOrbit();
      return;
   }

   if (drop_state > 0) {
      ship->MakeOrbit();
      return;
   }
}

// +--------------------------------------------------------------------+

void
NavAI::FindObjective()
{
   navpt    = 0;
   distance = 0;

   // runway takeoff:
   if (takeoff) {
      obj_w   = ship->Location()   + ship->Heading() * 10e3;
      obj_w.y = ship->Location().y + 2e3;

      // transform into camera coords:
      objective = Transform(obj_w);
      ship->SetDirectorInfo(Game::GetText("ai.takeoff"));
      return;
   }

   // PART I: Find next NavPoint:
   if (ship->GetNavSystem())
      navpt = ship->GetNextNavPoint();

   complete = !navpt;
   if (complete) return;

   // PART II: Compute Objective from NavPoint:
   Point       npt = navpt->Location();
   Sim*        sim = Sim::GetSim();
   SimRegion*  self_rgn = ship->GetRegion();
   SimRegion*  nav_rgn = navpt->Region();

   if (self_rgn && !nav_rgn) {
      nav_rgn = self_rgn;
      navpt->SetRegion(nav_rgn);
   }

   if (self_rgn == nav_rgn) {
      if (farcaster) {
         if (farcaster->GetShip()->GetRegion() != self_rgn)
            farcaster = farcaster->GetDest()->GetFarcaster();

         obj_w = farcaster->EndPoint();
      }

      else {
         obj_w = npt.OtherHand();
      }
   
      // distance from self to navpt:
      distance = Point(obj_w - self->Location()).length();
   
      // transform into camera coords:
      objective = Transform(obj_w);

      if (!ship->IsStarship())
         objective.Normalize();

      if (farcaster && distance < 1000)
         farcaster = 0;
   }

   // PART III: Deal with orbital transitions:
   else if (ship->IsDropship()) {
      if (nav_rgn->GetOrbitalRegion()->Primary() ==
          self_rgn->GetOrbitalRegion()->Primary()) {

         Point npt = nav_rgn->Location() - self_rgn->Location();
         obj_w = npt.OtherHand();

         // distance from self to navpt:
         distance = Point(obj_w - ship->Location()).length();

         // transform into camera coords:
         objective = Transform(obj_w);

         if (nav_rgn->IsAirSpace()) {
            drop_state = -1;
         }
         else if (nav_rgn->IsOrbital()) {
            drop_state = 1;
         }
      }

      // PART IIIa: Deal with farcaster jumps:
      else if (nav_rgn->IsOrbital() && self_rgn->IsOrbital()) {
         ListIter<Ship> s = self_rgn->Ships();
         while (++s && !farcaster) {
            if (s->GetFarcaster()) {
               const Ship* dest = s->GetFarcaster()->GetDest();
               if (dest && dest->GetRegion() == nav_rgn) {
                  farcaster = s->GetFarcaster();
               }
            }
         }

         if (farcaster) {
            Point apt   = farcaster->ApproachPoint(0);
            Point npt   = farcaster->StartPoint();
            double r1   = (ship->Location() - npt).length();

            if (r1 > 50e3) {
               obj_w     = apt;
               distance  = r1;
               objective = Transform(obj_w);
            }

            else {
               double r2 = (ship->Location() - apt).length();
               double r3 = (npt - apt).length();

               if (r1+r2 < 1.2*r3) {
                  obj_w     = npt;
                  distance  = r1;
                  objective = Transform(obj_w);
               }
               else {
                  obj_w     = apt;
                  distance  = r2;
                  objective = Transform(obj_w);
               }
            }
         }
      }
   }

   // PART IV: Deal with quantum jumps:
   else if (ship->IsStarship()) {
      quantum_state = 1;

      Point npt = nav_rgn->Location() + navpt->Location();
      npt -= self_rgn->Location();
      obj_w = npt.OtherHand();

      // distance from self to navpt:
      distance = Point(obj_w - ship->Location()).length();

      // transform into camera coords:
      objective = Transform(obj_w);
   }
}

// +--------------------------------------------------------------------+

void
NavAI::Navigator()
{
   accumulator.Clear();
   magnitude = 0;
   brakes    = 0;
   hold      = false;

   if (navpt) {
      if (navpt->Status() == Instruction::COMPLETE && navpt->HoldTime() > 0) {
         ship->SetDirectorInfo(Game::GetText("ai.auto-hold"));
         hold = true;
      }
      else {
         ship->SetDirectorInfo(Game::GetText("ai.auto-nav"));
      }
   }
   else {
      ship->SetDirectorInfo(Game::GetText("ai.auto-stop"));
   }

   Accumulate(AvoidTerrain());
   Accumulate(AvoidCollision());

   if (!hold)
      accumulator = SeekTarget();

   HelmControl();
   ThrottleControl();
}

// +--------------------------------------------------------------------+

void
NavAI::HelmControl()
{
   // ----------------------------------------------------------
   // STARSHIP HELM MODE
   // ----------------------------------------------------------

   if (ship->IsStarship()) {
      ship->SetFLCSMode(Ship::FLCS_HELM);
      ship->SetHelmHeading(accumulator.yaw);

      if (accumulator.pitch > 45*DEGREES)
         ship->SetHelmPitch(45*DEGREES);

      else if (accumulator.pitch < -45*DEGREES)
         ship->SetHelmPitch(-45*DEGREES);

      else
         ship->SetHelmPitch(accumulator.pitch);
   }

   // ----------------------------------------------------------
   // FIGHTER FLCS AUTO MODE
   // ----------------------------------------------------------

   else {
      ship->SetFLCSMode(Ship::FLCS_AUTO);

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

      if (!ship->IsAirborne() || ship->AltitudeAGL() > 100)
         ship->RaiseGear();
   }

   ship->SetTransX(0);
   ship->SetTransY(0);
   ship->SetTransZ(0);
   ship->ExecFLCSFrame();
}

// +--------------------------------------------------------------------+

void
NavAI::ThrottleControl()
{
   double ship_speed = ship->Velocity() * ship->Heading();
   bool   augmenter  = false;

   if (hold) {
      throttle = 0;
      brakes   = 1;
   }

   else if (navpt) {
      double speed      = navpt->Speed();

      if (speed < 10)
         speed = 250;

      throttle = 0;

      if (Ship::GetFlightModel() > 0) {
         if (ship_speed > speed + 10)
            throttle =  old_throttle - 0.25;

         else if (ship_speed < speed - 10)
            throttle =  old_throttle + 0.25;

         else
            throttle = old_throttle;
      }

      else {
         if (ship_speed > speed+5)
            brakes   = 0.25;

         else if (ship_speed < speed-5)
            throttle = 50;
      }
   }
   else {
      throttle = 0;
      brakes   = 0.25;
   }

   if (ship->IsAirborne() && ship->Class() < Ship::LCA) {
      if (ship_speed < 250) {
         throttle = 100;
         brakes   = 0;

         if (ship_speed < 200)
            augmenter = true;
      }

      else if (throttle < 20) {
         throttle = 20;
      }
   }

   old_throttle = throttle;
   ship->SetThrottle(throttle);
   ship->SetAugmenter(augmenter);

   if (ship_speed > 1 && brakes > 0)
      ship->SetTransY(-brakes * ship->Design()->trans_y);
}

// +--------------------------------------------------------------------+

Steer
NavAI::SeekTarget()
{
   if (!ship)
      return Steer();

   if (takeoff)
      return Seek(objective);

   if (navpt) {
      if (quantum_state == 1) {
         QuantumDrive* q = ship->GetQuantumDrive();

         if (q) {
            if (q->ActiveState() == QuantumDrive::ACTIVE_READY) {
               q->SetDestination(navpt->Region(), navpt->Location());
               q->Engage();
            }

            else if (q->ActiveState() == QuantumDrive::ACTIVE_POSTWARP) {
               quantum_state = 0;
            }
         }
      }

      if (distance <  2 * self->Radius()) {
         ship->SetNavptStatus(navpt, Instruction::COMPLETE);

         return Steer();
      }
      else {
         return Seek(objective);
      }
   }

   return Steer();
}

// +--------------------------------------------------------------------+

void
NavAI::Disengage()
{
   throttle = 0;
}

// +--------------------------------------------------------------------+

Point
NavAI::Transform(const Point& point)
{
   if (ship && ship->IsStarship())
      return point - self->Location();

   return SteerAI::Transform(point);
}

Steer
NavAI::Seek(const Point& point)
{
   // if ship is starship, the point is in relative world coordinates
   //   x: distance east(-)  / west(+)
   //   y: altitude down(-)  / up(+)
   //   z: distance north(-) / south(+)

   if (ship && ship->IsStarship()) {
      Steer result;

      result.yaw = atan2(point.x, point.z) + PI;

      double adjacent = sqrt(point.x * point.x + point.z * point.z);
      if (fabs(point.y) > ship->Radius() && adjacent > ship->Radius())
         result.pitch = atan(point.y / adjacent);

      if (_isnan(result.yaw))
         result.yaw = 0;

      if (_isnan(result.pitch))
         result.pitch = 0;

      return result;
   }

   return SteerAI::Seek(point);
}

Steer
NavAI::Flee(const Point& point)
{
   if (ship && ship->IsStarship()) {
      Steer result = Seek(point);
      result.yaw += PI;
      result.pitch *= -1;
      return result;
   }

   return SteerAI::Flee(point);
}

Steer
NavAI::Avoid(const Point& point, float radius)
{
   if (ship && ship->IsStarship()) {
      Steer result = Seek(point);

      if (point * ship->BeamLine() > 0)
         result.yaw -= PI/2;
      else
         result.yaw += PI/2;

      return result;
   }

   return SteerAI::Avoid(point, radius);
}

// +--------------------------------------------------------------------+

Steer
NavAI::AvoidTerrain()
{
   Steer avoid;

   terrain_warning = false;

   if (!ship || !ship->GetRegion() || !ship->GetRegion()->IsActive() ||
         takeoff || (navpt && navpt->Action() == Instruction::LAUNCH))
      return avoid;

   if (ship->IsAirborne() && ship->GetFlightPhase() == Ship::ACTIVE) {
      // too low?
      if (ship->AltitudeAGL() < 1000) {
         terrain_warning = true;
         ship->SetDirectorInfo(Game::GetText("ai.too-low"));

         // way too low?
         if (ship->AltitudeAGL() < 750) {
            ship->SetDirectorInfo(Game::GetText("ai.way-too-low"));
         }

         // where will we be?
         Point selfpt = ship->Location() + ship->Velocity() + Point(0, 10e3, 0);

         // transform into camera coords:
         Point obj = Transform(selfpt);

         // pull up!
         avoid = Seek(obj);
      }
   }

   return avoid;
}



