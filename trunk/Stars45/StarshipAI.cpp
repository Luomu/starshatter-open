/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         StarshipAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship (low-level) Artificial Intelligence class
*/

#include "MemDebug.h"
#include "StarshipAI.h"
#include "StarshipTacticalAI.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Mission.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "Contact.h"
#include "WeaponGroup.h"
#include "Drive.h"
#include "Sim.h"
#include "StarSystem.h"
#include "FlightComp.h"
#include "Farcaster.h"
#include "QuantumDrive.h"

#include "Game.h"
#include "Random.h"

// +----------------------------------------------------------------------+

StarshipAI::StarshipAI(SimObject* s)
   : ShipAI(s), sub_select_time(0), subtarget(0), tgt_point_defense(false)
{
   ai_type = STARSHIP;

   // signifies this ship is a dead hulk:
   if (ship && ship->Design()->auto_roll < 0) {
      Point torque(rand()-16000, rand()-16000, rand()-16000);
      torque.Normalize();
      torque *= ship->Mass() / 10;

      ship->SetFLCSMode(0);
      if (ship->GetFLCS())
         ship->GetFLCS()->PowerOff();

      ship->ApplyTorque(torque);
      ship->SetVelocity(RandomDirection() * Random(20, 50));

      for (int i = 0; i < 64; i++) {
         Weapon* w = ship->GetWeaponByIndex(i+1);
         if (w)
            w->DrainPower(0);
         else
            break;
      }
   }

   else {
      tactical = new(__FILE__,__LINE__) StarshipTacticalAI(this);
   }

   sub_select_time      = Game::GameTime() + (DWORD) Random(0, 2000);
   point_defense_time   = sub_select_time;
}


// +--------------------------------------------------------------------+

StarshipAI::~StarshipAI()
{ }

// +--------------------------------------------------------------------+

void
StarshipAI::FindObjective()
{
   distance = 0;

   int   order = ship->GetRadioOrders()->Action();

   if (order == RadioMessage::QUANTUM_TO ||
       order == RadioMessage::FARCAST_TO) {

      FindObjectiveQuantum();
      objective = Transform(obj_w);
      return;
   }

   bool  hold  = order == RadioMessage::WEP_HOLD ||
                 order == RadioMessage::FORM_UP;

   bool  form  = hold                  || 
                 (!order && !target)   || 
                 (farcaster);

   // if not the element leader, stay in formation:
   if (form && element_index > 1) {
      ship->SetDirectorInfo(Game::GetText("ai.formation"));

      if (navpt && navpt->Action() == Instruction::LAUNCH) {
         FindObjectiveNavPoint();
      }
      else {
         navpt = 0;
         FindObjectiveFormation();
      }

      // transform into camera coords:
      objective = Transform(obj_w);
      return;
   }

   // under orders?
   bool   directed      = false;
   double threat_level  = 0;
   double support_level = 1;
   Ship*  ward          = ship->GetWard();

   if (tactical) {
      directed      = (tactical->RulesOfEngagement() == TacticalAI::DIRECTED);
      threat_level  = tactical->ThreatLevel();
      support_level = tactical->SupportLevel();
   }

   // threat processing:
   if (hold || !directed && threat_level >= 2*support_level) {

      // seek support:
      if (support) {
         double d_support = Point(support->Location() - ship->Location()).length();
         if (d_support > 35e3) {
            ship->SetDirectorInfo(Game::GetText("ai.regroup"));
            FindObjectiveTarget(support);
            objective = Transform(obj_w);
            return;
         }
      }

      // run away:
      else if (threat && threat != target) {
         ship->SetDirectorInfo(Game::GetText("ai.retreat"));
         obj_w = ship->Location() + Point(ship->Location() - threat->Location()) * 100;
         objective = Transform(obj_w);
         return;
      }
   }

   // weapons hold:
   if (hold) {
      if (navpt) {
         ship->SetDirectorInfo(Game::GetText("ai.seek-navpt"));
         FindObjectiveNavPoint();
      }

      else if (patrol) {
         ship->SetDirectorInfo(Game::GetText("ai.patrol"));
         FindObjectivePatrol();
      }

      else {
         ship->SetDirectorInfo(Game::GetText("ai.holding"));
         objective = Point();
      }
   }

   // normal processing:
   else if (target) {
      ship->SetDirectorInfo(Game::GetText("ai.seek-target"));
      FindObjectiveTarget(target);
   }

   else if (patrol) {
      ship->SetDirectorInfo(Game::GetText("ai.patrol"));
      FindObjectivePatrol();
   }

   else if (ward) {
      ship->SetDirectorInfo(Game::GetText("ai.seek-ward"));
      FindObjectiveFormation();
   }

   else if (navpt) {
      ship->SetDirectorInfo(Game::GetText("ai.seek-navpt"));
      FindObjectiveNavPoint();
   }

   else if (rumor) {
      ship->SetDirectorInfo(Game::GetText("ai.search"));
      FindObjectiveTarget(rumor);
   }

   else {
      objective = Point();
   }

   // transform into camera coords:
   objective = Transform(obj_w);
}

// +--------------------------------------------------------------------+

void
StarshipAI::Navigator()
{
   // signifies this ship is a dead hulk:
   if (ship && ship->Design()->auto_roll < 0) {
      ship->SetDirectorInfo(Game::GetText("ai.dead"));
      return;
   }

   accumulator.Clear();
   magnitude = 0;

   hold = false;
   if ((ship->GetElement() && ship->GetElement()->GetHoldTime() > 0) || 
       (navpt && navpt->Status() == Instruction::COMPLETE && navpt->HoldTime() > 0))
      hold = true;

   ship->SetFLCSMode(Ship::FLCS_HELM);

   if (!ship->GetDirectorInfo()) {
      if (target)
         ship->SetDirectorInfo(Game::GetText("ai.seek-target"));
      else if (ship->GetWard())
         ship->SetDirectorInfo(Game::GetText("ai.seek-ward"));
      else
         ship->SetDirectorInfo(Game::GetText("ai.patrol"));
   }

   if (farcaster && distance < 25e3) {
      accumulator = SeekTarget();
   }
   else {
      accumulator = AvoidCollision();

      if (!other && !hold)
         accumulator = SeekTarget();
   }

   HelmControl();
   ThrottleControl();
   FireControl();
   AdjustDefenses();
}

// +--------------------------------------------------------------------+

void
StarshipAI::HelmControl()
{
   // signifies this ship is a dead hulk:
   if (ship && ship->Design()->auto_roll < 0) {
      return;
   }

   double trans_x          = 0;
   double trans_y          = 0;
   double trans_z          = 0;

   bool   station_keeping  = distance < 0;

   if (station_keeping) {
      accumulator.brake = 1;
      accumulator.stop  = 1;

      ship->SetHelmPitch(0);
   }

   else {
      Element* elem = ship->GetElement();

      Ship* ward     = ship->GetWard();
      Ship* s_threat = 0;
      if (threat && threat->Class() >= ship->Class())
         s_threat = threat;

      if (other || target || ward || s_threat || navpt || patrol || farcaster || element_index > 1) {
         ship->SetHelmHeading(accumulator.yaw);

         if (elem->Type() == Mission::FLIGHT_OPS) {
            ship->SetHelmPitch(0);

            if (ship->NumInbound() > 0) {
               ship->SetHelmHeading(ship->CompassHeading());
            }
         }

         else if (accumulator.pitch > 60*DEGREES) {
            ship->SetHelmPitch(60*DEGREES);
         }

         else if (accumulator.pitch < -60*DEGREES) {
            ship->SetHelmPitch(-60*DEGREES);
         }

         else {
            ship->SetHelmPitch(accumulator.pitch);
         }

      }
      else {
         ship->SetHelmPitch(0);
      }
   }

   ship->SetTransX(trans_x);
   ship->SetTransY(trans_y);
   ship->SetTransZ(trans_z);

   ship->ExecFLCSFrame();
}

void
StarshipAI::ThrottleControl()
{
   // signifies this ship is a dead hulk:
   if (ship && ship->Design()->auto_roll < 0) {
      return;
   }

   // station keeping:

   if (distance < 0) {
      old_throttle = 0;
      throttle     = 0;

      ship->SetThrottle(0);

      if (ship->GetFLCS())
         ship->GetFLCS()->FullStop();

      return;
   }

   // normal throttle processing:

   double   ship_speed  = ship->Velocity() * ship->Heading();
   double   brakes      = 0;
   Ship*    ward        = ship->GetWard();
   Ship*    s_threat    = 0;

   if (threat && threat->Class() >= ship->Class())
      s_threat = threat;

   if (target || s_threat) {  // target pursuit, or retreat
      throttle = 100;

      if (target && distance < 50e3) {
         double closing_speed = ship_speed;

         if (target) {
            Point delta = target->Location() - ship->Location();
            delta.Normalize();

            closing_speed = ship->Velocity() * delta;
         }

         if (closing_speed > 300) {
            throttle = 30;
            brakes   = 0.25;
         }
      }

      throttle *= (1 - accumulator.brake);

      if (throttle < 1 && ship->GetFLCS() != 0)
         ship->GetFLCS()->FullStop();
   }

   else if (ward) {           // escort, match speed of ward
      double speed = ward->Velocity().length();
      throttle = old_throttle;

      if (speed == 0) {
         double d = (ship->Location() - ward->Location()).length();

         if (d > 30e3)
            speed = (d - 30e3) / 100;
      }

      if (speed > 0) {
         if (ship_speed > speed) {
            throttle = old_throttle - 1;
            brakes   = 0.2;
         }
         else if (ship_speed < speed - 10) {
            throttle =  old_throttle + 1;
         }
      }
      else {
         throttle = 0;
         brakes   = 0.5;
      }
   }

   else if (patrol || farcaster) {  // seek patrol point
      throttle = 100;

      if (distance < 10 * ship_speed) {
         if (ship->Velocity().length() > 200)
            throttle = 5;
         else
            throttle = 50;
      }
   }

   else if (navpt) {          // lead only, get speed from navpt
      double speed = navpt->Speed();
      throttle = old_throttle;

      if (hold) {
         throttle = 0;
         brakes   = 1;
      }

      else {
         if (speed <= 0)
            speed = 300;

         if (ship_speed > speed) {
            if (throttle > 0 && old_throttle > 1)
               throttle = old_throttle - 1;

            brakes   = 0.25;
         }
         else if (ship_speed < speed - 10) {
            throttle =  old_throttle + 1;
         }
      }
   }

   else if (element_index > 1) { // wingman
      Ship*  lead = ship->GetElement()->GetShip(1);
      double lv = lead->Velocity().length();
      double sv = ship_speed;
      double dv = lv-sv;
      double dt = 0;

      if (dv > 0)       dt = dv * 1e-2 * seconds;
      else if (dv < 0)  dt = dv * 1e-2 * seconds;

      throttle = old_throttle + dt;
   }

   else {
      throttle = 0;
   }

   old_throttle = throttle;
   ship->SetThrottle(throttle);

   if (ship_speed > 1 && brakes > 0)
      ship->SetTransY(-brakes * ship->Design()->trans_y);

   else if (throttle > 10 && (ship->GetEMCON() < 2 || ship->GetFuelLevel() < 10))
      ship->SetTransY(ship->Design()->trans_y);
}

// +--------------------------------------------------------------------+

Steer
StarshipAI::SeekTarget()
{
   if (navpt) {
      SimRegion*     self_rgn = ship->GetRegion();
      SimRegion*     nav_rgn  = navpt->Region();
      QuantumDrive*  qdrive   = ship->GetQuantumDrive();

      if (self_rgn && !nav_rgn) {
         nav_rgn = self_rgn;
         navpt->SetRegion(nav_rgn);
      }

      bool use_farcaster      = self_rgn != nav_rgn && 
                                (navpt->Farcast() || 
                                 !qdrive ||
                                 !qdrive->IsPowerOn() ||
                                 qdrive->Status() < System::DEGRADED
                                );

      if (use_farcaster) {
         if (!farcaster) {
            ListIter<Ship> s = self_rgn->Ships();
            while (++s && !farcaster) {
               if (s->GetFarcaster()) {
                  const Ship* dest = s->GetFarcaster()->GetDest();
                  if (dest && dest->GetRegion() == nav_rgn) {
                     farcaster = s->GetFarcaster();
                  }
               }
            }
         }

         if (farcaster) {
            if (farcaster->GetShip()->GetRegion() != self_rgn)
               farcaster = farcaster->GetDest()->GetFarcaster();

            obj_w = farcaster->EndPoint();
            distance = Point(obj_w - ship->Location()).length();

            if (distance < 1000)
               farcaster = 0;
         }
      }
      else if (self_rgn != nav_rgn) {
         QuantumDrive* q = ship->GetQuantumDrive();

         if (q) {
            if (q->ActiveState() == QuantumDrive::ACTIVE_READY) {
               q->SetDestination(navpt->Region(), navpt->Location());
               q->Engage();
            }
         }
      }
   }

   return ShipAI::SeekTarget();
}

// +--------------------------------------------------------------------+

Steer
StarshipAI::AvoidCollision()
{
   if (!ship || ship->Velocity().length() < 25)
      return Steer();

   return ShipAI::AvoidCollision();
}

// +--------------------------------------------------------------------+

void
StarshipAI::FireControl()
{
   // identify unknown contacts:
   if (identify) {
      if (fabs(ship->GetHelmHeading() - ship->CompassHeading()) < 10*DEGREES) {
         Contact* contact = ship->FindContact(target);

         if (contact && !contact->ActLock()) {
            if (!ship->GetProbe()) {
               ship->LaunchProbe();
            }
         }
      }

      return;
   }

   // investigate last known location of enemy ship:
   if (rumor && !target && ship->GetProbeLauncher() && !ship->GetProbe()) {
      // is rumor in basket?
      Point rmr = Transform(rumor->Location());
      rmr.Normalize();

      double dx = fabs(rmr.x);
      double dy = fabs(rmr.y);

      if (dx < 10*DEGREES && dy < 10*DEGREES && rmr.z > 0) {
         ship->LaunchProbe();
      }
   }

   // Corvettes and Frigates are anti-air platforms.  They need to
   // target missile threats even when the threat is aimed at another
   // friendly ship.  Forward facing weapons must be on auto fire,
   // while lateral and aft facing weapons are set to point defense.

   if (ship->Class() == Ship::CORVETTE || ship->Class() == Ship::FRIGATE) {
      ListIter<WeaponGroup> iter = ship->Weapons();
      while (++iter) {
         WeaponGroup* group = iter.value();

         ListIter<Weapon> w_iter = group->GetWeapons();
         while (++w_iter) {
            Weapon* weapon = w_iter.value();

            double az = weapon->GetAzimuth();
            if (fabs(az) < 45*DEGREES) {
               weapon->SetFiringOrders(Weapon::AUTO);
               weapon->SetTarget(target, 0);
            }

            else {
               weapon->SetFiringOrders(Weapon::POINT_DEFENSE);
            }
         }
      }
   }

   // All other starships are free to engage ship targets.  Weapon
   // fire control is managed by the type of weapon.

   else {
      System* subtgt = SelectSubtarget();

      ListIter<WeaponGroup> iter = ship->Weapons();
      while (++iter) {
         WeaponGroup* weapon = iter.value();

         if (weapon->GetDesign()->target_type & Ship::DROPSHIPS) {   // anti-air weapon?
            weapon->SetFiringOrders(Weapon::POINT_DEFENSE);
         }
         else if (weapon->IsDrone()) {                               // torpedoes
            weapon->SetFiringOrders(Weapon::MANUAL);
            weapon->SetTarget(target, 0);

            if (target && target->GetRegion() == ship->GetRegion()) {
               Point  delta = target->Location() - ship->Location();
               double range = delta.length();

               if (range < weapon->GetDesign()->max_range * 0.9 && 
                   !AssessTargetPointDefense())
                  weapon->SetFiringOrders(Weapon::AUTO);

               else if (range < weapon->GetDesign()->max_range * 0.5)
                  weapon->SetFiringOrders(Weapon::AUTO);
            }
         }
         else {                                                      // anti-ship weapon
            weapon->SetFiringOrders(Weapon::AUTO);
            weapon->SetTarget(target, subtgt);
            weapon->SetSweep(subtgt ? Weapon::SWEEP_NONE : Weapon::SWEEP_TIGHT);
         }
      }
   }
}

// +--------------------------------------------------------------------+

System*
StarshipAI::SelectSubtarget()
{
   if (Game::GameTime() - sub_select_time < 2345)
      return subtarget;

   subtarget = 0;

   if (!target || target->Type() != SimObject::SIM_SHIP || GetAILevel() < 1)
      return subtarget;

   Ship* tgt_ship = (Ship*) target;

   if (!tgt_ship->IsStarship())
      return subtarget;

   Weapon*  subtgt = 0;
   double   dist   = 50e3;
   Point    svec   = ship->Location() - tgt_ship->Location();

   sub_select_time = Game::GameTime();

   // first pass: turrets
   ListIter<WeaponGroup> g_iter = tgt_ship->Weapons();
   while (++g_iter) {
      WeaponGroup* g = g_iter.value();

      if (g->GetDesign() && g->GetDesign()->turret_model) {
         ListIter<Weapon> w_iter = g->GetWeapons();
         while (++w_iter) {
            Weapon*  w     = w_iter.value();

            if (w->Availability() < 35)
               continue;

            if (w->GetAimVector() * svec < 0)
               continue;

            if (w->GetTurret()) {
               Point    tloc  = w->GetTurret()->Location();
               Point    delta = tloc - ship->Location();
               double   dlen  = delta.length();

               if (dlen < dist) {
                  subtgt = w;
                  dist   = dlen;
               }
            }
         }
      }
   }

   // second pass: major weapons
   if (!subtgt) {
      g_iter.reset();
      while (++g_iter) {
         WeaponGroup* g = g_iter.value();

         if (g->GetDesign() && !g->GetDesign()->turret_model) {
            ListIter<Weapon> w_iter = g->GetWeapons();
            while (++w_iter) {
               Weapon*  w     = w_iter.value();

               if (w->Availability() < 35)
                  continue;

               if (w->GetAimVector() * svec < 0)
                  continue;

               Point    tloc  = w->MountLocation();
               Point    delta = tloc - ship->Location();
               double   dlen  = delta.length();

               if (dlen < dist) {
                  subtgt = w;
                  dist   = dlen;
               }
            }
         }
      }
   }

   subtarget = subtgt;
   return subtarget;
}

// +--------------------------------------------------------------------+

bool
StarshipAI::AssessTargetPointDefense()
{
   if (Game::GameTime() - point_defense_time < 3500)
      return tgt_point_defense;

   tgt_point_defense = false;

   if (!target || target->Type() != SimObject::SIM_SHIP || GetAILevel() < 2)
      return tgt_point_defense;

   Ship* tgt_ship = (Ship*) target;

   if (!tgt_ship->IsStarship())
      return tgt_point_defense;

   Weapon*  subtgt = 0;
   Point    svec   = ship->Location() - tgt_ship->Location();

   point_defense_time = Game::GameTime();

   // first pass: turrets
   ListIter<WeaponGroup> g_iter = tgt_ship->Weapons();
   while (++g_iter && !tgt_point_defense) {
      WeaponGroup* g = g_iter.value();

      if (g->CanTarget(1)) {
         ListIter<Weapon> w_iter = g->GetWeapons();
         while (++w_iter && !tgt_point_defense) {
            Weapon* w = w_iter.value();

            if (w->Availability() > 35 && w->GetAimVector() * svec > 0)
               tgt_point_defense = true;
         }
      }
   }

   return tgt_point_defense;
}


// +--------------------------------------------------------------------+

Point
StarshipAI::Transform(const Point& point)
{
   return point - self->Location();
}

Steer
StarshipAI::Seek(const Point& point)
{
   // the point is in relative world coordinates
   //   x: distance east(-)  / west(+)
   //   y: altitude down(-)  / up(+)
   //   z: distance north(-) / south(+)

   Steer result;

   result.yaw   = atan2(point.x, point.z) + PI;

   double adjacent = sqrt(point.x*point.x + point.z*point.z);
   if (fabs(point.y) > ship->Radius() && adjacent > ship->Radius())
      result.pitch = atan(point.y / adjacent);

   if (!_finite(result.yaw))
      result.yaw = 0;

   if (!_finite(result.pitch))
      result.pitch = 0;

   return result;
}

Steer
StarshipAI::Flee(const Point& point)
{
   Steer result = Seek(point);
   result.yaw += PI;
   return result;
}

Steer
StarshipAI::Avoid(const Point& point, float radius)
{
   Steer result = Seek(point);

   if (point * ship->BeamLine() > 0)
      result.yaw -= PI/2;
   else
      result.yaw += PI/2;

   return result;
}


