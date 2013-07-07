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
    FILE:         CampaignPlanMission.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanMission generates missions and mission
    info for the player's combat group as part of a
    dynamic campaign.
*/

#include "MemDebug.h"
#include "CampaignPlanMission.h"
#include "CampaignMissionRequest.h"
#include "CampaignMissionFighter.h"
#include "CampaignMissionStarship.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAction.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Mission.h"
#include "StarSystem.h"
#include "Random.h"


// +--------------------------------------------------------------------+

void
CampaignPlanMission::ExecFrame()
{
    if (campaign && campaign->IsActive()) {
        player_group = campaign->GetPlayerGroup();
        if (!player_group) return;

        int missionCount = campaign->GetMissionList().size();

        if (missionCount > 0) {
            // starships only get one mission to pick from:
            if (player_group->IsStarshipGroup())
            return;

            // fighters get a maximum of five missions:
            if (missionCount >= 5)
            return;

            // otherwise, once every few seconds is plenty:
            if (Campaign::Stardate() - exec_time < 1)
            return;
        }

        SelectStartTime();

        if (player_group->IsFighterGroup()) {
            slot++;
            if (slot > 2) slot = 0;

            CampaignMissionRequest* request = PlanFighterMission();
            CampaignMissionFighter  generator(campaign);
            generator.CreateMission(request);
            delete request;
        }

        else if (player_group->IsStarshipGroup()) {
            // starships should always check for campaign and strategic missions
            slot = 0;

            CampaignMissionRequest* request = PlanStarshipMission();
            CampaignMissionStarship generator(campaign);
            generator.CreateMission(request);
            delete request;
        }

        exec_time = Campaign::Stardate();
    }
}

// +--------------------------------------------------------------------+

void
CampaignPlanMission::SelectStartTime()
{
    const int   HOUR           = 3600;  // 60 minutes
    const int   MISSION_DELAY  = 1800;  // 30 minutes
    double      base_time      = 0;

    List<MissionInfo>& info_list = campaign->GetMissionList();

    if (info_list.size() > 0) {
        MissionInfo* info = info_list[info_list.size()-1];
        base_time = info->start;
    }

    if (base_time == 0)
    base_time = campaign->GetTime() + MISSION_DELAY;

    start = (int) base_time + MISSION_DELAY;
    start -= start % MISSION_DELAY;
}

// +--------------------------------------------------------------------+

CampaignMissionRequest*
CampaignPlanMission::PlanStarshipMission()
{
    CampaignMissionRequest* request = 0;

    if (!request)  request = PlanCampaignMission();
    if (!request)  request = PlanStrategicMission();
    if (!request)  request = PlanRandomStarshipMission();

    return request;
}

// +--------------------------------------------------------------------+

CampaignMissionRequest*
CampaignPlanMission::PlanFighterMission()
{
    CampaignMissionRequest* request = 0;

    if (!request)  request = PlanCampaignMission();
    if (!request)  request = PlanStrategicMission();
    if (!request)  request = PlanRandomFighterMission();

    return request;
}

// +--------------------------------------------------------------------+

CampaignMissionRequest*
CampaignPlanMission::PlanCampaignMission()
{
    CampaignMissionRequest* request = 0;

    ListIter<CombatAction> iter = campaign->GetActions();
    while (++iter && !request) {
        CombatAction* action = iter.value();

        if (action->Type() != CombatAction::MISSION_TEMPLATE)
        continue;

        if (action->IsAvailable()) {

            // only fire each action once every two hours:
            if (action->ExecTime() > 0 && campaign->GetTime() - action->ExecTime() < 7200)
            continue;

            CombatGroup* g = campaign->FindGroup(action->GetIFF(),
            action->AssetType(),
            action->AssetId());

            if (g && (g == player_group || 
                        (player_group->Type() == CombatGroup::WING &&
                            player_group->FindGroup(g->Type(), g->GetID())))) {

                request = new(__FILE__,__LINE__)
                CampaignMissionRequest(campaign, 
                action->Subtype(),
                start, 
                g);

                if (request) {
                    request->SetOpposingType(action->OpposingType());
                    request->SetScript(action->GetText());
                }

                action->FireAction();
            }
        }
    }

    return request;
}

// +--------------------------------------------------------------------+

CampaignMissionRequest*
CampaignPlanMission::PlanStrategicMission()
{
    CampaignMissionRequest* request = 0;

    if (slot > 1)
    return request;

    // build list of assignments:
    List<CombatAssignment> assignments;
    assignments.append(player_group->GetAssignments());

    if (player_group->Type() == CombatGroup::WING) {
        ListIter<CombatGroup> iter = player_group->GetComponents();
        while (++iter) {
            CombatGroup* g = iter.value();
            assignments.append(g->GetAssignments());
        }
    }

    // pick next assignment as basis for mission:
    static int assignment_index = 0;

    if (assignments.size()) {
        if (assignment_index >= assignments.size())
        assignment_index = 0;

        CombatAssignment* a = assignments[assignment_index++];

        request = new(__FILE__,__LINE__)
        CampaignMissionRequest(campaign, 
        a->Type(),
        start, 
        a->GetResource());

        if (request)
        request->SetObjective(a->GetObjective());
    }

    return request;
}

// +--------------------------------------------------------------------+

static int mission_type_index = -1;
static int mission_types[16] = {
    Mission::PATROL,
    Mission::PATROL,
    Mission::ESCORT_FREIGHT,
    Mission::PATROL,
    Mission::ESCORT_FREIGHT,
    Mission::PATROL,
    Mission::ESCORT_FREIGHT,
    Mission::ESCORT_FREIGHT,
    Mission::PATROL,
    Mission::ESCORT_FREIGHT,
    Mission::PATROL,
    Mission::ESCORT_FREIGHT,
    Mission::PATROL,
    Mission::PATROL,
    Mission::ESCORT_FREIGHT,
    Mission::PATROL
};

// +--------------------------------------------------------------------+

CampaignMissionRequest*
CampaignPlanMission::PlanRandomStarshipMission()
{
    int type    = Mission::PATROL;
    int r       = RandomIndex();
    int ownside = player_group->GetIFF();

    if (mission_type_index < 0)
    mission_type_index = r;

    else if (mission_type_index >= 16)
    mission_type_index = 0;

    type = mission_types[mission_type_index++];

    if (type == Mission::ESCORT_FREIGHT) {
        CombatGroup*  freight  = campaign->FindGroup(ownside, CombatGroup::FREIGHT);
        if (!freight || freight->CountUnits() < 1)
        type = Mission::PATROL;
    }

    CampaignMissionRequest* request = 0;
    request = new(__FILE__,__LINE__)
    CampaignMissionRequest(campaign, type, start, player_group);

    return request;
}

// +--------------------------------------------------------------------+

static int fighter_mission_index     = 0;
static int fighter_mission_types[16] = {
    Mission::PATROL,
    Mission::SWEEP,
    Mission::ESCORT_SHUTTLE,
    Mission::AIR_PATROL,
    Mission::SWEEP,
    Mission::ESCORT_SHUTTLE,
    Mission::PATROL,
    Mission::PATROL,
    Mission::AIR_SWEEP,
    Mission::PATROL,
    Mission::AIR_PATROL,
    Mission::ESCORT_SHUTTLE,
    Mission::PATROL,
    Mission::SWEEP,
    Mission::PATROL,
    Mission::AIR_SWEEP
};

CampaignMissionRequest*
CampaignPlanMission::PlanRandomFighterMission()
{
    CampaignMissionRequest* request  = 0;
    int                     type     = fighter_mission_types[fighter_mission_index++];
    int                     ownside  = player_group->GetIFF();
    CombatGroup*            primary  = player_group;
    CombatGroup*            obj      = 0;

    if (fighter_mission_index > 15)
    fighter_mission_index = 0;

    if (type == Mission::ESCORT_FREIGHT) {
        CombatGroup*  freight  = campaign->FindGroup(ownside, CombatGroup::FREIGHT);
        if (!freight || freight->CalcValue() < 1)
        type = Mission::PATROL;
        else
        obj  = freight;
    }

    else if (type == Mission::ESCORT_SHUTTLE) {
        CombatGroup*  shuttle = campaign->FindGroup(ownside, CombatGroup::LCA_SQUADRON);
        if (!shuttle || shuttle->CalcValue() < 1)
        type = Mission::PATROL;
        else
        obj  = shuttle;
    }

    else if (primary->Type() == CombatGroup::WING) {
        if (RandomChance())
        primary = primary->FindGroup(CombatGroup::INTERCEPT_SQUADRON);
        else
        primary = primary->FindGroup(CombatGroup::FIGHTER_SQUADRON);
    }

    if (type >= Mission::AIR_PATROL && type <= Mission::AIR_INTERCEPT) {
        CombatZone* zone     = 0;
        bool        airborne = false;

        if (primary)
        zone = primary->GetAssignedZone();

        if (zone && zone->GetRegions().size() > 1) {
            Text        air_region = *zone->GetRegions().at(1);
            StarSystem* system     = campaign->GetSystem(zone->System());

            if (system) {
                OrbitalRegion* rgn = system->FindRegion(air_region);

                if (rgn && rgn->Type() == Orbital::TERRAIN)
                airborne = true;
            }
        }

        if (!airborne) {
            if (type == Mission::AIR_INTERCEPT)
            type = Mission::INTERCEPT;

            else if (type == Mission::AIR_SWEEP)
            type = Mission::SWEEP;

            else
            type = Mission::PATROL;
        }
    }

    request = new(__FILE__,__LINE__)
    CampaignMissionRequest(campaign, type, start, primary);

    if (request)
    request->SetObjective(obj);

    return request;
}
