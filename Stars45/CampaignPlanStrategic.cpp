/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignPlanStrategic.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanStrategic prioritizes targets and defensible
    allied forces as the first step in force tasking.
*/

#include "MemDebug.h"
#include "CampaignPlanStrategic.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Random.h"

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::ExecFrame()
{
   if (campaign && campaign->IsActive()) {
      if (Campaign::Stardate() - exec_time < 300)
         return;

      ListIter<CombatZone> zone = campaign->GetZones();
      while (++zone)
         zone->Clear();

      ListIter<Combatant>  iter = campaign->GetCombatants();
      while (++iter) {
         Combatant*     c     = iter.value();
         CombatGroup*   force = c->GetForce();

         force->CalcValue();

         PlaceGroup(force);
         ScoreCombatant(c);
         ScoreNeeds(c);

         force->ClearUnlockedZones();
         AssignZones(c);
         ResolveZoneMovement(force);
      }

      exec_time = Campaign::Stardate();
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::PlaceGroup(CombatGroup* g)
{
   if (!g)
      return;

   Text        rgn  = g->GetRegion();
   CombatZone* zone = campaign->GetZone(rgn);

   // if we couldn't find anything suitable,
   // just pick a zone at random:
   if (!zone && g->IsMovable()) {
      int nzones = campaign->GetZones().size();
      int n      = RandomIndex() % nzones;
      zone       = campaign->GetZones().at(n);

      Text assigned_rgn;
      if (!campaign->GetZone(rgn)) {
         assigned_rgn = *zone->GetRegions().at(0);
         g->AssignRegion(assigned_rgn);
      }
   }

   if (zone && !zone->HasGroup(g))
      zone->AddGroup(g);

   ListIter<CombatGroup> iter = g->GetComponents();
   while (++iter)
      PlaceGroup(iter.value());
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::ScoreCombatant(Combatant* c)
{
   // prep lists:
   c->GetDefendList().clear();
   c->GetTargetList().clear();

   ScoreDefensible(c);

   ListIter<Combatant> iter = campaign->GetCombatants();
   while (++iter) {
      if (iter->GetIFF() > 0 && iter->GetIFF() != c->GetIFF())
         ScoreTargets(c, iter.value());
   }

   // sort lists:
   c->GetDefendList().sort();
   c->GetTargetList().sort();
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::ScoreDefensible(Combatant* c)
{
   if (c->GetForce())
      ScoreDefend(c, c->GetForce());
}

void
CampaignPlanStrategic::ScoreDefend(Combatant* c, CombatGroup* g)
{
   if (!g || g->IsReserve())
      return;

   if (g->IsDefensible()) {
      g->SetPlanValue(g->Value());
      c->GetDefendList().append(g);

      CombatZone* zone  = campaign->GetZone(g->GetRegion());
      ZoneForce*  force = 0;

      if (zone)
         force = zone->FindForce(c->GetIFF());

      if (force)
         force->GetDefendList().append(g);
   }

   ListIter<CombatGroup> iter = g->GetComponents();
   while (++iter) {
      ScoreDefend(c, iter.value());
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::ScoreTargets(Combatant* c, Combatant* t)
{
   if (t->GetForce())
      ScoreTarget(c, t->GetForce());
}

void
CampaignPlanStrategic::ScoreTarget(Combatant* c, CombatGroup* g)
{
   if (!g || g->IntelLevel() <= Intel::SECRET)
      return;

   if (g->IsTargetable()) {
      g->SetPlanValue(g->Value() * c->GetTargetStratFactor(g->Type()));
      c->GetTargetList().append(g);

      CombatZone* zone  = campaign->GetZone(g->GetRegion());
      ZoneForce*  force = 0;

      if (zone)
         force = zone->FindForce(c->GetIFF());

      if (force)
         force->GetTargetList().append(g);
   }

   ListIter<CombatGroup> iter = g->GetComponents();
   while (++iter) {
      ScoreTarget(c, iter.value());
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::ScoreNeeds(Combatant* c)
{
   ListIter<CombatZone> zone = campaign->GetZones();
   while (++zone) {
      ZoneForce* force = zone->FindForce(c->GetIFF());

      // clear needs:
      force->SetNeed(CombatGroup::CARRIER_GROUP,      0);
      force->SetNeed(CombatGroup::BATTLE_GROUP,       0);
      force->SetNeed(CombatGroup::DESTROYER_SQUADRON, 0);
      force->SetNeed(CombatGroup::ATTACK_SQUADRON,    0);
      force->SetNeed(CombatGroup::FIGHTER_SQUADRON,   0);
      force->SetNeed(CombatGroup::INTERCEPT_SQUADRON,   0);

      // what defensive assets are needed in this zone?
      ListIter<CombatGroup> def = force->GetDefendList();
      while (++def) {
         int defender_type = *CombatGroup::PreferredDefender(def->Type());
         force->AddNeed(defender_type, def->Value());
      }

      // what offensive assets are needed in this zone?
      ListIter<CombatGroup> tgt = force->GetTargetList();
      while (++tgt) {
         int attacker_type = *CombatGroup::PreferredAttacker(tgt->Type());
         force->AddNeed(attacker_type, tgt->Value());
      }
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::BuildGroupList(CombatGroup* g, List<CombatGroup>& groups)
{
   if (!g || g->IsReserve())
      return;

   if (g->IsAssignable())
      groups.append(g);

   ListIter<CombatGroup> iter = g->GetComponents();
   while (++iter)
      BuildGroupList(iter.value(), groups);
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::AssignZones(Combatant* c)
{
   // find the list of assignable groups, in priority order:
   List<CombatGroup> groups;
   BuildGroupList(c->GetForce(), groups);
   groups.sort();

   // for each group, assign a zone:
   ListIter<CombatGroup> g_iter = groups;

   // first pass: fighter and attack squadrons assigned to star bases
   while (++g_iter) {
      CombatGroup* g     = g_iter.value();
      int          gtype = g->Type();

      if (gtype == CombatGroup::ATTACK_SQUADRON    ||
          gtype == CombatGroup::FIGHTER_SQUADRON   ||
          gtype == CombatGroup::INTERCEPT_SQUADRON) {
         CombatGroup* parent = g->GetParent();

         if (parent && parent->Type() == CombatGroup::WING)
            parent = parent->GetParent();

         if (!parent || parent->Type() == CombatGroup::CARRIER_GROUP)
            continue;

         // these groups are attached to fixed resources,
         // so they must be assigned to the parent's zone:
         CombatZone* parent_zone = campaign->GetZone(parent->GetRegion());

         if (parent_zone) {
            ZoneForce*  parent_force = parent_zone->FindForce(g->GetIFF());

            if (parent_force) {
               g->SetAssignedZone(parent_zone);
               parent_force->AddNeed(g->Type(), -(g->Value()));
            }
         }
      }
   }

   // second pass: carrier groups
   g_iter.reset();
   while (++g_iter) {
      CombatGroup* g     = g_iter.value();
      int          gtype = g->Type();

      if (gtype == CombatGroup::CARRIER_GROUP) {
         int         current_zone_need = 0;
         int         highest_zone_need = 0;
         CombatZone* highest_zone      = 0;
         ZoneForce*  highest_force     = 0;
         CombatZone* current_zone      = 0;
         ZoneForce*  current_force     = 0;

         List<CombatZone> possible_zones;

         if (g->IsZoneLocked()) {
            current_zone  = g->GetAssignedZone();
            current_force = current_zone->FindForce(g->GetIFF());
         }

         else {
            ListIter<CombatZone> z_iter = campaign->GetZones();
            while (++z_iter) {
               CombatZone* zone  = z_iter.value();
               ZoneForce*  force = zone->FindForce(g->GetIFF());
               int         need  = force->GetNeed(CombatGroup::CARRIER_GROUP)     +
                                   force->GetNeed(CombatGroup::ATTACK_SQUADRON)   +
                                   force->GetNeed(CombatGroup::FIGHTER_SQUADRON)  +
                                   force->GetNeed(CombatGroup::INTERCEPT_SQUADRON);

               if (g->IsSystemLocked() && zone->System() != g->GetAssignedSystem())
                  continue;

               possible_zones.append(zone);

               if (zone->HasRegion(g->GetRegion())) {
                  current_zone_need = need;
                  current_zone      = zone;
                  current_force     = force;
               }

               if (need > highest_zone_need) {
                  highest_zone_need = need;
                  highest_zone      = zone;
                  highest_force     = force;
               }
            }
         }

         CombatZone* assigned_zone  = current_zone;
         ZoneForce*  assigned_force = current_force;

         if (highest_zone_need > current_zone_need) {
            assigned_zone  = highest_zone;
            assigned_force = highest_force;
         }

         // if we couldn't find anything suitable,
         // just pick a zone at random:
         if (!assigned_zone) {
            if (possible_zones.isEmpty())
               possible_zones.append(campaign->GetZones());

            int nzones = possible_zones.size();
            int n      = RandomIndex() % nzones;

            assigned_zone  = possible_zones.at(n);
            assigned_force = assigned_zone->FindForce(g->GetIFF());
         }

         if (assigned_force && assigned_zone) {
            Text assigned_rgn;
            if (!campaign->GetZone(g->GetRegion())) {
               assigned_rgn = *assigned_zone->GetRegions().at(0);
               g->AssignRegion(assigned_rgn);
            }

            g->SetAssignedZone(assigned_zone);
            assigned_force->AddNeed(g->Type(), -(g->Value()));

            // also assign the carrier's wing and squadrons to the same zone:
            ListIter<CombatGroup> squadron = g->GetComponents();
            while (++squadron) {
               squadron->SetAssignedZone(assigned_zone);
               assigned_force->AddNeed(squadron->Type(), -(squadron->Value()));

               if (squadron->Type() == CombatGroup::WING) {
                  ListIter<CombatGroup> s = squadron->GetComponents();
                  while (++s) {
                     s->SetAssignedZone(assigned_zone);
                     assigned_force->AddNeed(s->Type(), -(s->Value()));
                  }
               }
            }
         }
      }
   }

   // third pass: everything else
   g_iter.reset();
   while (++g_iter) {
      CombatGroup* g     = g_iter.value();
      int          gtype = g->Type();

      if (gtype == CombatGroup::BATTLE_GROUP || gtype == CombatGroup::DESTROYER_SQUADRON) {
         int         current_zone_need = 0;
         int         highest_zone_need = 0;
         CombatZone* highest_zone      = 0;
         ZoneForce*  highest_force     = 0;
         CombatZone* current_zone      = 0;
         ZoneForce*  current_force     = 0;

         List<CombatZone> possible_zones;

         if (g->IsZoneLocked()) {
            current_zone  = g->GetAssignedZone();
            current_force = current_zone->FindForce(g->GetIFF());
         }

         else {
            ListIter<CombatZone> z_iter = campaign->GetZones();
            while (++z_iter) {
               CombatZone* zone  = z_iter.value();
               ZoneForce*  force = zone->FindForce(g->GetIFF());
               int         need  = force->GetNeed(g->Type());

               if (g->IsSystemLocked() && zone->System() != g->GetAssignedSystem())
                  continue;

               possible_zones.append(zone);

               // battle groups can do double-duty:
               if (gtype == CombatGroup::BATTLE_GROUP)
                  need += force->GetNeed(CombatGroup::DESTROYER_SQUADRON);

               if (zone->HasRegion(g->GetRegion())) {
                  current_zone_need = need;
                  current_zone      = zone;
                  current_force     = force;
               }

               if (need > highest_zone_need) {
                  highest_zone_need = need;
                  highest_zone      = zone;
                  highest_force     = force;
               }
            }
         }

         if (highest_zone_need > current_zone_need) {
            g->SetAssignedZone(highest_zone);

            if (highest_force)
               highest_force->AddNeed(g->Type(), -(g->Value()));
         }
         else {
            if (!current_zone) {
               if (possible_zones.isEmpty())
                  possible_zones.append(campaign->GetZones());

               int nzones = possible_zones.size();
               int n      = RandomIndex() % nzones;

               current_zone  = possible_zones.at(n);
               current_force = current_zone->FindForce(g->GetIFF());
            }

            g->SetAssignedZone(current_zone);

            if (current_force)
               current_force->AddNeed(g->Type(), -(g->Value()));

            Text assigned_rgn;
            if (!campaign->GetZone(g->GetRegion())) {
               assigned_rgn = *current_zone->GetRegions().at(0);
               g->AssignRegion(assigned_rgn);
            }
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
CampaignPlanStrategic::ResolveZoneMovement(CombatGroup* g)
{
   CombatZone* zone = g->GetAssignedZone();
   bool        move = false;

   if (zone && !zone->HasRegion(g->GetRegion())) {
      move = true;
      CombatZone* old_zone = g->GetCurrentZone();
      if (old_zone)
         old_zone->RemoveGroup(g);
      zone->AddGroup(g);
   }

   ListIter<CombatGroup> comp = g->GetComponents();
   while (++comp)
      ResolveZoneMovement(comp.value());

   // assign region last, to allow components to
   // resolve their zones:
   if (zone && move)
      g->AssignRegion(*zone->GetRegions().at(0));
}
