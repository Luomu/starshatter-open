/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FighterTacticalAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fighter-specific mid-level (tactical) AI class
*/

#include "MemDebug.h"
#include "FighterTacticalAI.h"
#include "ShipAI.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shot.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "Sensor.h"
#include "Contact.h"
#include "WeaponGroup.h"
#include "Drive.h"
#include "Sim.h"
#include "StarSystem.h"

#include "Game.h"

// +----------------------------------------------------------------------+

const int WINCHESTER_FIGHTER = 0;
const int WINCHESTER_ASSAULT = 1;
const int WINCHESTER_STRIKE  = 2;
const int WINCHESTER_STATIC  = 3;

// +----------------------------------------------------------------------+

FighterTacticalAI::FighterTacticalAI(ShipAI* ai)
   : TacticalAI(ai), secondary_selection_time(0)
{
   for (int i = 0; i < 4; i++)
      winchester[i] = false;

   ai_level = ai->GetAILevel();

   switch (ai_level) {
   default:
   case 2:  THREAT_REACTION_TIME =  1000; break;
   case 1:  THREAT_REACTION_TIME =  3000; break;
   case 0:  THREAT_REACTION_TIME =  6000; break;
   }
}


// +--------------------------------------------------------------------+

FighterTacticalAI::~FighterTacticalAI()
{ }

// +--------------------------------------------------------------------+

bool
FighterTacticalAI::CheckFlightPlan()
{
   navpt = ship->GetNextNavPoint();

   int order = Instruction::PATROL;
   roe       = FLEXIBLE;

   if (navpt) {
      order = navpt->Action();

      switch (order)  {
      case Instruction::LAUNCH:
      case Instruction::DOCK:
      case Instruction::RTB:     roe = NONE;
                                 break;

      case Instruction::VECTOR:
                                 roe = SELF_DEFENSIVE;
                                 if (element_index > 1)
                                    roe = DEFENSIVE;
                                 break;

      case Instruction::DEFEND:
      case Instruction::ESCORT:  roe = DEFENSIVE;
                                 break;

      case Instruction::INTERCEPT:
                                 if (element_index > 1)
                                    roe = DEFENSIVE;
                                 else
                                    roe = DIRECTED;
                                 break;

      case Instruction::RECON:
      case Instruction::STRIKE:
      case Instruction::ASSAULT: roe = DIRECTED;
                                 break;

      case Instruction::PATROL:
      case Instruction::SWEEP:   roe = FLEXIBLE;
                                 break;

      default: break;
      }

      if (order == Instruction::STRIKE) {
         ship->SetSensorMode(Sensor::GM);

         if (IsStrikeComplete(navpt)) {
            ship->SetNavptStatus(navpt, Instruction::COMPLETE);
         }
      }

      else if (order == Instruction::ASSAULT) {
         if (ship->GetSensorMode() == Sensor::GM)
            ship->SetSensorMode(Sensor::STD);

         if (IsStrikeComplete(navpt)) {
            ship->SetNavptStatus(navpt, Instruction::COMPLETE);
         }
      }

      else {
         if (ship->GetSensorMode() == Sensor::GM)
            ship->SetSensorMode(Sensor::STD);
      }
   }

   switch (roe) {
   case NONE:              ship->SetDirectorInfo(Game::GetText("ai.none"));            break;
   case SELF_DEFENSIVE:    ship->SetDirectorInfo(Game::GetText("ai.self-defensive"));  break;
   case DEFENSIVE:         ship->SetDirectorInfo(Game::GetText("ai.defensive"));       break;
   case DIRECTED:          ship->SetDirectorInfo(Game::GetText("ai.directed"));        break;
   case FLEXIBLE:          ship->SetDirectorInfo(Game::GetText("ai.flexible"));        break;
   default:                ship->SetDirectorInfo(Game::GetText("ai.default"));         break;
   }

   return (navpt != 0);
}

// +--------------------------------------------------------------------+

void
FighterTacticalAI::SelectTarget()
{
   TacticalAI::SelectTarget();

   SimObject* target = ship_ai->GetTarget();

   if (target && (target->Type() == SimObject::SIM_SHIP) &&
      (Game::GameTime() - secondary_selection_time) > THREAT_REACTION_TIME) {
      SelectSecondaryForTarget((Ship*) target);
      secondary_selection_time = Game::GameTime();
   }
}

// +--------------------------------------------------------------------+

void
FighterTacticalAI::SelectTargetDirected(Ship* tgt)
{
   Ship* potential_target = tgt;

   if (!tgt) {
      // try to target one of the element's objectives
      // (if it shows up in the contact list)

      Element* elem = ship->GetElement();

      if (elem) {
         Instruction* objective = elem->GetTargetObjective();

         if (objective) {
            SimObject* obj_sim_obj = objective->GetTarget();
            Ship*      obj_tgt     = 0;

            if (obj_sim_obj && obj_sim_obj->Type() == SimObject::SIM_SHIP)
               obj_tgt = (Ship*) obj_sim_obj;

            if (obj_tgt && ship->FindContact(obj_tgt))
               potential_target = obj_tgt;
         }
      }
   }

   if (!CanTarget(potential_target))
      potential_target = 0;

   ship_ai->SetTarget(potential_target);
   SelectSecondaryForTarget(potential_target);
}

// +--------------------------------------------------------------------+

void
FighterTacticalAI::SelectTargetOpportunity()
{
   // NON-COMBATANTS do not pick targets of opportunity:
   if (ship->GetIFF() == 0)
      return;

   Ship* potential_target    = 0;
   Shot* current_shot_target = 0;

   // pick the closest combatant ship with a different IFF code:
   double target_dist = 1.0e15;
   double min_dist    = 5.0e3;

   // FIGHTERS are primarily anti-air platforms, but may
   // also attack smaller starships:

   Ship* ward = 0;
   if (element_index > 1)
      ward = ship->GetLeader();

   // commit range for patrol/sweep is 80 Km
   // (about 2 minutes for a fighter at max speed)
   if (roe == FLEXIBLE || roe == AGRESSIVE)
      target_dist = ship->Design()->commit_range;

   if (roe < FLEXIBLE)
      target_dist = 0.5 * ship->Design()->commit_range;

   int class_limit = Ship::LCA;

   if (ship->Class() == Ship::ATTACK)
      class_limit = Ship::DESTROYER;

   ListIter<Contact> c_iter = ship->ContactList();
   while (++c_iter) {
      Contact* contact = c_iter.value();
      Ship*    c_ship  = contact->GetShip();
      Shot*    c_shot  = contact->GetShot();
      int      c_iff   = contact->GetIFF(ship);
      bool     rogue   = false;

      if (c_ship)
         rogue = c_ship->IsRogue();

      if (!rogue && (c_iff <= 0 || c_iff == ship->GetIFF() || c_iff == 1000))
         continue;

      // reasonable target?
      if (c_ship && c_ship->Class() <= class_limit && !c_ship->InTransition()) {
         if (!rogue) {
            SimObject* ttgt = c_ship->GetTarget();

            // if we are self-defensive, is this contact engaging us?
            if (roe == SELF_DEFENSIVE && ttgt != ship)
               continue;

            // if we are defending, is this contact engaging us or our ward?
            if (roe == DEFENSIVE && ttgt != ship && ttgt != ward)
               continue;
         }

         // found an enemy, check distance:
         double dist = (ship->Location() - c_ship->Location()).length();

         if (dist < 0.75 * target_dist) {

            // if on patrol, check target distance from navpoint:
            if (roe == FLEXIBLE && navpt) {
               double ndist = (navpt->Location().OtherHand() - c_ship->Location()).length();
               if (ndist > 80e3)
                  continue;
            }

            potential_target = c_ship;
            target_dist = dist;
         }
      }

      else if (c_shot && c_shot->IsDrone()) {
         // found an enemy shot, do we have enough time to engage?
         if (c_shot->GetEta() < 10)
            continue;

         // found an enemy shot, check distance:
         double dist = (ship->Location() - c_shot->Location()).length();

         if (!current_shot_target) {
            current_shot_target = c_shot;
            target_dist = dist;
         }

         // is this shot a better target than the one we've found?
         else {
            Ship* ward = ship_ai->GetWard();

            if ((c_shot->IsTracking(ward) && !current_shot_target->IsTracking(ward)) ||
                (dist < target_dist)) {
               current_shot_target = c_shot;
               target_dist = dist;
            }
         }
      }
   }

   if (current_shot_target) {
      ship_ai->SetTarget(current_shot_target);
   }
   else {
      ship_ai->SetTarget(potential_target);
      SelectSecondaryForTarget(potential_target);
   }
}

// +--------------------------------------------------------------------+

int
FighterTacticalAI::ListSecondariesForTarget(Ship* tgt, List<WeaponGroup>& weps)
{
   weps.clear();

   if (tgt) {
      ListIter<WeaponGroup> iter = ship->Weapons();
      while (++iter) {
         WeaponGroup* w = iter.value();

         if (w->Ammo() && w->CanTarget(tgt->Class()))
            weps.append(w);
      }
   }

   return weps.size();
}

void
FighterTacticalAI::SelectSecondaryForTarget(Ship* tgt)
{
   if (tgt) {
      int wix = WINCHESTER_FIGHTER;

      if (tgt->IsGroundUnit())      wix = WINCHESTER_STRIKE;
      else if (tgt->IsStatic())     wix = WINCHESTER_STATIC;
      else if (tgt->IsStarship())   wix = WINCHESTER_ASSAULT;

      WeaponGroup*      best = 0;
      List<WeaponGroup> weps;
      
      if (ListSecondariesForTarget(tgt, weps)) {
         winchester[wix] = false;

         // select best weapon for the job:
         double range       = (ship->Location() - tgt->Location()).length();
         double best_range  = 0;
         double best_damage = 0;

         ListIter<WeaponGroup> iter  = weps;
         while (++iter) {
            WeaponGroup* w = iter.value();

            if (!best) {
               best = w;

               WeaponDesign* d = best->GetDesign();
               best_range  = d->max_range;
               best_damage = d->damage * d->ripple_count;

               if (best_range < range)
                  best = 0;
            }

            else {
               WeaponDesign* d = w->GetDesign();
               double w_range  = d->max_range;
               double w_damage = d->damage * d->ripple_count;

               if (w_range > range) {
                  if (w_range < best_range || w_damage > best_damage)
                     best = w;
               }
            }
         }

         // now cycle weapons until you pick the best one:
         WeaponGroup* current_missile = ship->GetSecondaryGroup();

         if (current_missile && best && current_missile != best) {
            ship->CycleSecondary();
            WeaponGroup* m = ship->GetSecondaryGroup();

            while (m != current_missile && m != best) {
               ship->CycleSecondary();
               m = ship->GetSecondaryGroup();
            }
         }
      }
      
      else {
         winchester[wix] = true;

         // if we have NO weapons that can hit this target,
         // just drop it:

         Weapon* primary = ship->GetPrimary();
         if (!primary || !primary->CanTarget(tgt->Class())) {
            ship_ai->DropTarget(3);
            ship->DropTarget();
         }
      }

      if (tgt->IsGroundUnit())
         ship->SetSensorMode(Sensor::GM);

      else if (ship->GetSensorMode() == Sensor::GM)
         ship->SetSensorMode(Sensor::STD);
   }
}

// +--------------------------------------------------------------------+

void
FighterTacticalAI::FindFormationSlot(int formation)
{
   // find the formation delta:
   int s = element_index - 1;
   Point delta(5*s, 0, -5*s);

   // diamond:
   if (formation == Instruction::DIAMOND) {
      switch (element_index) {
      case 2:  delta = Point( 12, -1, -10); break;
      case 3:  delta = Point(-12, -1, -10); break;
      case 4:  delta = Point(  0, -2, -20); break;
      }
   }

   // spread:
   if (formation == Instruction::SPREAD) {
      switch (element_index) {
      case 2:  delta = Point( 15, 0, 0); break;
      case 3:  delta = Point(-15, 0, 0); break;
      case 4:  delta = Point(-30, 0, 0); break;
      }
   }

   // box:
   if (formation == Instruction::BOX) {
      switch (element_index) {
      case 2:  delta = Point(15,  0,   0); break;
      case 3:  delta = Point( 0, -2, -20); break;
      case 4:  delta = Point(15, -2, -20); break;
      }
   }

   // trail:
   if (formation == Instruction::TRAIL) {
      delta = Point(0, s, -20*s);
   }

   ship_ai->SetFormationDelta(delta * ship->Radius() * 2);
}

// +--------------------------------------------------------------------+

void
FighterTacticalAI::FindThreat()
{
   // pick the closest contact on Threat Warning System:
   Ship*       threat_ship          = 0;
   Shot*       threat_missile       = 0;
   double      threat_dist          = 1e9;

   ListIter<Contact> c_iter = ship->ContactList();

   while (++c_iter) {
      Contact* contact = c_iter.value();

      if (contact->Threat(ship) && 
          (Game::GameTime() - contact->AcquisitionTime()) > THREAT_REACTION_TIME) {
         
         double rng = contact->Range(ship);

         if (contact->GetShot()) {
            threat_missile = contact->GetShot();
         }

         else if (rng < threat_dist && contact->GetShip()) {
            Ship* candidate = contact->GetShip();

            if (candidate->InTransition())
               continue;

            if (candidate->IsStarship() && rng < 50e3) {
               threat_ship = candidate;
               threat_dist = rng;
            }

            else if (candidate->IsDropship() && rng < 25e3) {
               threat_ship = candidate;
               threat_dist = rng;
            }

            // static and ground units:
            else if (rng < 30e3) {
               threat_ship = candidate;
               threat_dist = rng;
            }
         }
      }
   }

   ship_ai->SetThreat(threat_ship);
   ship_ai->SetThreatMissile(threat_missile);
}

// +--------------------------------------------------------------------+

bool
FighterTacticalAI::IsStrikeComplete(Instruction* instr)
{
   // wingmen can not call a halt to a strike:
   if (!ship || element_index > 1)
      return false;

   // if there's nothing to shoot at, we must be done:
   if (!instr || !instr->GetTarget() || instr->GetTarget()->Life() == 0 ||
       instr->GetTarget()->Type() != SimObject::SIM_SHIP)
      return true;

   // break off strike only when ALL weapons are expended:
   // (remember to check all relevant wingmen)
   Element*    element = ship->GetElement();
   Ship*       target  = (Ship*) instr->GetTarget();

   if (!element)
      return true;

   for (int i = 0; i < element->NumShips(); i++) {
      Ship* s = element->GetShip(i+1);

      if (!s || s->Integrity() < 25) // || (s->Location() - target->Location()).length() > 250e3)
         continue;

      ListIter<WeaponGroup> g_iter = s->Weapons();
      while (++g_iter) {
         WeaponGroup* w = g_iter.value();

         if (w->Ammo() && w->CanTarget(target->Class())) {
            ListIter<Weapon> w_iter = w->GetWeapons();

            while (++w_iter) {
               Weapon* weapon = w_iter.value();

               if (weapon->Status() > System::CRITICAL)
                  return false;
            }
         }
      }
   }

   // still here?  we must be done!
   return true;
}