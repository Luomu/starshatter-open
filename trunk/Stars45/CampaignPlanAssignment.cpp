/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignPlanAssignment.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanAssignment creates combat assignments for
    assets within each combat zone as the third step in
    force tasking.
*/

#include "MemDebug.h"
#include "CampaignPlanAssignment.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Mission.h"

// +--------------------------------------------------------------------+

void
CampaignPlanAssignment::ExecFrame()
{
   if (campaign && campaign->IsActive()) {
      // once every few minutes is plenty:
      if (Campaign::Stardate() - exec_time < 300)
         return;

      ListIter<Combatant>  iter = campaign->GetCombatants();
      while (++iter) {
         ProcessCombatant(iter.value());
      }

      exec_time = Campaign::Stardate();
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanAssignment::ProcessCombatant(Combatant* c)
{
   CombatGroup* force = c->GetForce();
   if (force) {
      force->CalcValue();
      force->ClearAssignments();
   }

   ListIter<CombatZone> zone = campaign->GetZones();
   while (++zone) {
      ProcessZone(c, zone.value());
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanAssignment::BuildZoneList(CombatGroup* g, CombatZone* zone, List<CombatGroup>& groups)
{
   if (!g)
      return;

   if (g->GetAssignedZone() == zone)
      groups.append(g);

   ListIter<CombatGroup> iter = g->GetComponents();
   while (++iter)
      BuildZoneList(iter.value(), zone, groups);
}

// +--------------------------------------------------------------------+

void
CampaignPlanAssignment::BuildAssetList(const int*         pref,
                                       List<CombatGroup>& groups,
                                       List<CombatGroup>& assets)
{
   if (!pref)
      return;

   while (*pref) {
      ListIter<CombatGroup> g = groups;
      while (++g) {
         if (g->Type() == *pref && g->CountUnits() > 0)
            assets.append(g.value());
      }

      pref++;
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanAssignment::ProcessZone(Combatant* c, CombatZone* zone)
{
   List<CombatGroup> groups;
   BuildZoneList(c->GetForce(), zone, groups);

   ZoneForce* force = zone->FindForce(c->GetIFF());

   // defensive assignments:
   ListIter<CombatGroup> def = force->GetDefendList();
   while (++def) {
      List<CombatGroup> assets;
      BuildAssetList(CombatGroup::PreferredDefender(def->Type()), groups, assets);

      ListIter<CombatGroup> g = assets;
      while (++g) {
         CombatAssignment* a = new(__FILE__,__LINE__)
            CombatAssignment(Mission::DEFEND,
                             def.value(),
                             g.value());

         if (a)
            g->GetAssignments().append(a);
      }
   }

   // offensive assignments:
   ListIter<CombatGroup> tgt = force->GetTargetList();
   while (++tgt) {
      CombatGroup* target = tgt.value();

      List<CombatGroup> assets;
      BuildAssetList(CombatGroup::PreferredAttacker(tgt->Type()), groups, assets);

      ListIter<CombatGroup> g = assets;
      while (++g) {
         CombatGroup* asset = g.value();
         int          mtype = Mission::ASSAULT;

         if (target->IsStrikeTarget())
            mtype = Mission::STRIKE;

         else if (target->IsFighterGroup())
            mtype = Mission::SWEEP;

         else if (target->Type() == CombatGroup::LCA_SQUADRON)
            mtype = Mission::INTERCEPT;

         CombatAssignment* a = new(__FILE__,__LINE__)
            CombatAssignment(mtype, target, asset);

         if (a)
            g->GetAssignments().append(a);
      }
   }
}

