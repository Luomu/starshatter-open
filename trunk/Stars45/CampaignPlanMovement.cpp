/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignPlanMovement.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanMovement simulates random patrol movements
    of starship groups between missions.
*/

#include "MemDebug.h"
#include "CampaignPlanMovement.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Random.h"
#include "ShipDesign.h"

// +--------------------------------------------------------------------+

void
CampaignPlanMovement::ExecFrame()
{
   if (campaign && campaign->IsActive()) {
      if (Campaign::Stardate() - exec_time < 7200)
         return;

      campaign->GetAllCombatUnits(-1, all_units);

      ListIter<CombatUnit> iter = all_units;
      while (++iter) {
         CombatUnit* u = iter.value();

         if (u->IsStarship() && !u->IsStatic())
            MoveUnit(u);
      }

      all_units.clear();

      exec_time = Campaign::Stardate();
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanMovement::MoveUnit(CombatUnit* u)
{
   if (u) {
      // starship repair:
      double damage  = u->GetSustainedDamage();

      if (damage > 0 && u->GetDesign()) {
         int    percent = (int) (100 * damage / u->GetDesign()->integrity);

         if (percent > 50) {
            u->SetSustainedDamage(0.90 * damage);
         }
      }

      Point  loc  = u->Location();
      Point  dir  = loc;
      double dist = dir.Normalize();

      const double MAX_RAD  = 320e3;
      const double MIN_DIST = 150e3;

      if (dist < MAX_RAD) {
         double scale = 1 - dist/MAX_RAD;

         loc += dir * (Random(30e3, 90e3) * scale) + RandomDirection() * 10e3;

         if (fabs(loc.z) > 20e3)
            loc.z *= 0.1;

         u->MoveTo(loc);

         CombatGroup* g = u->GetCombatGroup();
         if (g && g->Type() > CombatGroup::FLEET && g->GetFirstUnit() == u) {
            g->MoveTo(loc);

            if (g->IntelLevel() > Intel::KNOWN)
               g->SetIntelLevel(Intel::KNOWN);
         }
      }

      else if (dist > 1.25 * MAX_RAD) {
         double scale = 1 - dist/MAX_RAD;

         loc += dir * (Random(80e3, 120e3) * scale) + RandomDirection() * 3e3;

         if (fabs(loc.z) > 20e3)
            loc.z *= 0.1;

         u->MoveTo(loc);

         CombatGroup* g = u->GetCombatGroup();
         if (g && g->Type() > CombatGroup::FLEET && g->GetFirstUnit() == u) {
            g->MoveTo(loc);

            if (g->IntelLevel() > Intel::KNOWN)
               g->SetIntelLevel(Intel::KNOWN);
         }
      }

      else {
         loc += RandomDirection() * 30e3;

         if (fabs(loc.z) > 20e3)
            loc.z *= 0.1;

         u->MoveTo(loc);

         CombatGroup* g = u->GetCombatGroup();
         if (g && g->Type() > CombatGroup::FLEET && g->GetFirstUnit() == u) {
            g->MoveTo(loc);

            if (g->IntelLevel() > Intel::KNOWN)
               g->SetIntelLevel(Intel::KNOWN);
         }
      }

      CombatUnit* closest_unit = 0;
      double      closest_dist = 1e6;

      ListIter<CombatUnit> iter = all_units;
      while (++iter) {
         CombatUnit* unit = iter.value();

         if (unit->GetCombatGroup() != u->GetCombatGroup() && unit->GetRegion() == u->GetRegion() && !unit->IsDropship()) {
            Point  delta = loc - unit->Location();
            dist  = delta.Normalize();

            if (dist < closest_dist) {
               closest_unit = unit;
               closest_dist = dist;
            }
         }
      }

      if (closest_unit && closest_dist < MIN_DIST) {
         Point  delta = loc - closest_unit->Location();
         dist  = delta.Normalize();

         loc += delta * 1.1 * (MIN_DIST - closest_dist);

         if (fabs(loc.z) > 20e3)
            loc.z *= 0.1;

         u->MoveTo(loc);

         CombatGroup* g = u->GetCombatGroup();
         if (g && g->Type() > CombatGroup::FLEET && g->GetFirstUnit() == u) {
            g->MoveTo(loc);

            if (g->IntelLevel() > Intel::KNOWN)
               g->SetIntelLevel(Intel::KNOWN);
         }
      }
   }
}
