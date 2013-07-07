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
