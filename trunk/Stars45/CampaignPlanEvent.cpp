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
    FILE:         CampaignPlanEvent.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanEvent generates simulated combat
    events based on a statistical analysis of the
    combatants within the context of a dynamic
    campaign.
*/

#include "MemDebug.h"
#include "CampaignPlanEvent.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAction.h"
#include "CombatAssignment.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Mission.h"
#include "Random.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+

CampaignPlanEvent::CampaignPlanEvent(Campaign* c)
    : CampaignPlan(c), event_time(0)
{
    if (campaign) {
        event_time = (int) campaign->GetTime();
    }
}

CampaignPlanEvent::~CampaignPlanEvent()
{ }

// +--------------------------------------------------------------------+

void
CampaignPlanEvent::ExecFrame()
{
    if (campaign && campaign->IsActive()) {
        if (!campaign->GetPlayerGroup())
        return;

        // once every twenty minutes is plenty:
        if (Campaign::Stardate() - exec_time < 1200)
        return;

        if (!ExecScriptedEvents())
        ExecStatisticalEvents();

        exec_time  = Campaign::Stardate();
        event_time = (int) campaign->GetTime();
    }
}

void
CampaignPlanEvent::SetLockout(int seconds)
{
    exec_time = Campaign::Stardate() + seconds;
}

// +--------------------------------------------------------------------+

bool
CampaignPlanEvent::ExecScriptedEvents()
{
    bool scripted_event = false;

    if (campaign) {
        ListIter<CombatAction> iter = campaign->GetActions();
        while (++iter) {
            CombatAction* action = iter.value();

            if (action->IsAvailable()) {

                switch (action->Type()) {
                case CombatAction::COMBAT_EVENT:
                    {
                        CombatEvent* event = new(__FILE__,__LINE__)
                        CombatEvent(campaign, 
                        action->Subtype(),
                        (int) campaign->GetTime(),
                        action->GetIFF(),
                        action->Source(),
                        action->Region());

                        if (!event)
                        return false;

                        event->SetTitle(action->GetText());

                        if (*action->Filename() != 0)
                        event->SetFilename(action->Filename());

                        if (*action->ImageFile() != 0)
                        event->SetImageFile(action->ImageFile());

                        if (*action->SceneFile() != 0)
                        event->SetSceneFile(action->SceneFile());

                        event->Load();

                        ProsecuteKills(action);
                        campaign->GetEvents().append(event);

                        action->FireAction();
                        scripted_event = true;

                        if (action->Subtype() == CombatEvent::CAMPAIGN_END) {
                            ::Print(">>>>> CAMPAIGN %d END  (Action %03d) <<<<<\n", campaign->GetCampaignId(), action->Identity());
                            campaign->SetStatus(Campaign::CAMPAIGN_SUCCESS);
                        }

                        else if (action->Subtype() == CombatEvent::CAMPAIGN_FAIL) {
                            ::Print(">>>>> CAMPAIGN %d FAIL (Action %03d) <<<<<\n", campaign->GetCampaignId(), action->Identity());
                            campaign->SetStatus(Campaign::CAMPAIGN_FAILED);
                        }
                    }
                    break;

                case CombatAction::STRATEGIC_DIRECTIVE:
                    {
                        CombatGroup* g = campaign->FindGroup(action->GetIFF(),
                        action->AssetType(),
                        action->AssetId());

                        if (g) {
                            g->SetStrategicDirection(action->GetText());
                            action->FireAction();
                        }
                        else {
                            action->FailAction();
                        }

                        scripted_event = true;
                    }
                    break;

                case CombatAction::CAMPAIGN_SITUATION:
                    {
                        campaign->SetSituation(action->GetText());
                        action->FireAction();
                        scripted_event = true;
                    }
                    break;

                case CombatAction::CAMPAIGN_ORDERS:
                    {
                        campaign->SetOrders(action->GetText());
                        action->FireAction();
                        scripted_event = true;
                    }
                    break;

                case CombatAction::INTEL_EVENT:
                    {
                        CombatGroup* g = campaign->FindGroup(action->GetIFF(),
                        action->AssetType(),
                        action->AssetId());

                        if (g) {
                            g->SetIntelLevel(action->Subtype());
                            action->FireAction();
                        }
                        else {
                            ::Print("WARNING: Action %d (intel level) Could not find group (IFF:%d, type:%d, id:%d)\n",
                            action->Identity(),
                            action->GetIFF(),
                            action->AssetType(),
                            action->AssetId());

                            action->FailAction();
                        }

                        scripted_event = true;
                    }
                    break;

                case CombatAction::ZONE_ASSIGNMENT:
                    {
                        CombatGroup* g = campaign->FindGroup(action->GetIFF(),
                        action->AssetType(),
                        action->AssetId());

                        if (g) {
                            bool found = false;

                            if (*action->Region()) {
                                CombatZone* zone = campaign->GetZone(action->Region());

                                if (zone) {
                                    g->SetAssignedZone(zone);
                                    g->SetZoneLock(true);
                                    found = true;

                                    // don't announce the move unless it's for the player's team:
                                    if (action->GetIFF() == campaign->GetPlayerIFF() && _stricmp(action->GetText(), "do-not-display")) {
                                        CombatEvent* event = new(__FILE__,__LINE__)
                                        CombatEvent(campaign,
                                        CombatEvent::MOVE_TO,
                                        (int) campaign->GetTime(),
                                        action->GetIFF(),
                                        CombatEvent::FORCOM,
                                        action->Region());

                                        if (!event)
                                            return false;

                                        Text title = Text(g->Name()) + " Orders: Proceed to " + action->Region() + " Sector";
                                        event->SetTitle(title);

                                        double   eta = campaign->GetTime() + 3600;
                                        eta -= fmod(eta, 1800);

                                        char     text[64];
                                        FormatDayTime(text, eta);

                                        Text info  = "ORDERS:\n\nEffective immediately, ";
                                        info += g->GetDescription();
                                        info += " and all associated units shall proceed to ";
                                        info += action->Region();
                                        info += " sector and commence spaceborne operations in that area.  ETA rendevous point ";
                                        info += text;
                                        info += ".\n\nFleet Admiral A. Evars FORCOM\nCommanding";

                                        event->SetInformation(info);

                                        if (*action->ImageFile() != 0)
                                        event->SetImageFile(action->ImageFile());

                                        if (*action->SceneFile() != 0)
                                        event->SetSceneFile(action->SceneFile());

                                        event->Load();
                                        campaign->GetEvents().append(event);
                                    }
                                }
                            }

                            if (!found) {
                                ::Print("WARNING: Action %d Could not find assigned zone '%s' for '%s'\n",
                                action->Identity(),
                                action->Region() ? action->Region() : "NULL",
                                g->Name().data());

                                g->SetAssignedZone(0);
                            }

                            action->FireAction();
                        }
                        else {
                            ::Print("WARNING: Action %d (zone assignment) Could not find group (IFF:%d, type:%d, id:%d)\n",
                            action->Identity(),
                            action->GetIFF(),
                            action->AssetType(),
                            action->AssetId());

                            action->FailAction();
                        }

                        scripted_event = true;
                    }
                    break;

                case CombatAction::SYSTEM_ASSIGNMENT:
                    {
                        CombatGroup* g = campaign->FindGroup(action->GetIFF(),
                        action->AssetType(),
                        action->AssetId());

                        if (g) {
                            bool found = false;

                            if (*action->System()) {
                                Text system = action->System();

                                if (campaign->GetSystem(system)) {
                                    g->SetAssignedSystem(system);
                                    found = true;

                                    // don't announce the move unless it's for the player's team:
                                    if (action->GetIFF() == campaign->GetPlayerIFF() && _stricmp(action->GetText(), "do-not-display")) {
                                        CombatEvent* event = new(__FILE__,__LINE__)
                                        CombatEvent(campaign,
                                        CombatEvent::MOVE_TO,
                                        (int) campaign->GetTime(),
                                        action->GetIFF(),
                                        CombatEvent::FORCOM,
                                        action->Region());

                                        if (!event)
                                            return false;

                                        Text title = Text(g->Name()) + " Orders: Proceed to " + action->System() + " System";
                                        event->SetTitle(title);

                                        double   eta = campaign->GetTime() + 3600;
                                        eta -= fmod(eta, 1800);

                                        char     text[64];
                                        FormatDayTime(text, eta);

                                        Text info  = "ORDERS:\n\nEffective immediately, ";
                                        info += g->GetDescription();
                                        info += " and all associated units shall proceed to the ";
                                        info += action->System();
                                        info += " star system and commence spaceborne operations in that area.  ETA rendevous point ";
                                        info += text;
                                        info += ".\n\nFleet Admiral A. Evars FORCOM\nCommanding";

                                        event->SetInformation(info);

                                        if (*action->ImageFile() != 0)
                                        event->SetImageFile(action->ImageFile());

                                        if (*action->SceneFile() != 0)
                                        event->SetSceneFile(action->SceneFile());

                                        event->Load();
                                        campaign->GetEvents().append(event);
                                    }
                                }
                            }

                            if (!found) {
                                ::Print("WARNING: Action %d Could not find assigned system '%s' for '%s'\n",
                                action->Identity(),
                                action->System() ? action->System() : "NULL",
                                g->Name().data());

                                g->SetAssignedSystem("");
                            }

                            action->FireAction();
                        }
                        else {
                            ::Print("WARNING: Action %d (system assignment) Could not find group (IFF:%d, type:%d, id:%d)\n",
                            action->Identity(),
                            action->GetIFF(),
                            action->AssetType(),
                            action->AssetId());

                            action->FailAction();
                        }

                        scripted_event = true;
                    }
                    break;

                case CombatAction::NO_ACTION:
                    action->FireAction();
                    scripted_event = true;
                    break;

                default:
                    break;
                }
            }
        }
    }

    return scripted_event;
}

// +--------------------------------------------------------------------+

void
CampaignPlanEvent::ProsecuteKills(CombatAction* action)
{
    if (action->AssetKills().size() > 0) {
        CombatGroup* g = campaign->FindGroup(action->GetIFF(),
        action->AssetType(),
        action->AssetId());

        if (g) {
            ListIter<Text> iter = action->AssetKills();
            while (++iter) {
                Text* name = iter.value();
                CombatUnit* asset = g->FindUnit(*name);

                if (asset) {
                    int value_killed = asset->Kill(1);

                    ListIter<Combatant> iter = campaign->GetCombatants();
                    while (++iter) {
                        Combatant* c = iter.value();
                        if (c->GetIFF() > 0 && c->GetIFF() != asset->GetIFF()) {
                            // damage to neutral assets must be scored to bad guys:
                            if (asset->GetIFF() > 0 || c->GetIFF() > 1) {
                                c->AddScore(value_killed);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (action->TargetKills().size() > 0) {
        CombatGroup* g = campaign->FindGroup(action->TargetIFF(),
        action->TargetType(),
        action->TargetId());

        if (g) {
            ListIter<Text> iter = action->TargetKills();
            while (++iter) {
                Text* name = iter.value();
                CombatUnit* target = g->FindUnit(*name);

                if (target) {
                    int value_killed = target->Kill(1);

                    ListIter<Combatant> iter = campaign->GetCombatants();
                    while (++iter) {
                        Combatant* c = iter.value();
                        if (c->GetIFF() > 0 && c->GetIFF() != target->GetIFF()) {
                            // damage to neutral assets must be scored to bad guys:
                            if (target->GetIFF() > 0 || c->GetIFF() > 1) {
                                c->AddScore(value_killed);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

bool
CampaignPlanEvent::ExecStatisticalEvents()
{
    bool result = false;

    if (campaign) {
        ListIter<Combatant> iter = campaign->GetCombatants();
        while (++iter && !result) {
            Combatant*        c = iter.value();
            CombatAssignment* a = ChooseAssignment(c->GetForce());

            // prefer assignments not in player's zone:
            if (a) {
                CombatGroup* objective  = a->GetObjective();
                CombatGroup* player     = campaign->GetPlayerGroup();

                if (objective && player && 
                        objective->GetCurrentZone() == player->GetCurrentZone())
                a = ChooseAssignment(c->GetForce());
            }

            if (a) {
                result = CreateEvent(a);
            }
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

bool
CampaignPlanEvent::CreateEvent(CombatAssignment* a)
{
    CombatEvent* event = 0;

    if (campaign && a && a->GetResource() && RandomChance(1,2)) {
        event_time = (int) Random(event_time, campaign->GetTime());

        CombatGroup*   group = a->GetResource();

        if (group == campaign->GetPlayerGroup()) {

            if (group->Type() == CombatGroup::DESTROYER_SQUADRON ||
                    group->Type() == CombatGroup::BATTLE_GROUP       ||
                    group->Type() == CombatGroup::CARRIER_GROUP) {

                return false;
            }
        }

        CombatGroup* target = a->GetObjective();

        if (target && target == campaign->GetPlayerGroup()) {

            if (target->Type() == CombatGroup::DESTROYER_SQUADRON ||
                    target->Type() == CombatGroup::BATTLE_GROUP       ||
                    target->Type() == CombatGroup::CARRIER_GROUP) {

                return false;
            }
        }

        switch (a->Type()) {
        case Mission::DEFEND:
            event = CreateEventDefend(a);
            break;

        case Mission::ASSAULT:
            if (group->IsStarshipGroup())
            event = CreateEventStarship(a);
            else
            event = CreateEventFighterAssault(a);
            break;

        case Mission::STRIKE:
            if (group->IsStarshipGroup())
            event = CreateEventStarship(a);
            else
            event = CreateEventFighterStrike(a);
            break;

        case Mission::SWEEP:
            event = CreateEventFighterSweep(a);
            break;
        }

        if (event) {
            campaign->GetEvents().append(event);
            return true;
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

static void FindAssignments(CombatGroup* g, List<CombatAssignment>& alist)
{
    if (!g) return;

    alist.append(g->GetAssignments());

    ListIter<CombatGroup> iter = g->GetComponents();
    while (++iter)
    FindAssignments(iter.value(), alist);
}

CombatAssignment*
CampaignPlanEvent::ChooseAssignment(CombatGroup* g)
{
    List<CombatAssignment> alist;
    FindAssignments(g, alist);

    int tries = 5;

    if (alist.size() > 0) {
        while (tries-- > 0) {
            int index = (int) Random(0, alist.size());

            if (index >= alist.size())
            index = 0;

            CombatAssignment* a = alist[index];

            if (!a) continue;

            CombatGroup* resource   = a->GetResource();
            CombatGroup* objective  = a->GetObjective();

            if (!resource || !objective)
            continue;

            if (resource->IsReserve() || objective->IsReserve())
            continue;

            if (resource->CalcValue() < 50 || objective->CalcValue() < 50)
            continue;

            if (resource == campaign->GetPlayerGroup() || objective == campaign->GetPlayerGroup())
            continue;

            return a;
        }
    }

    return 0;
}

// +--------------------------------------------------------------------+

CombatEvent*
CampaignPlanEvent::CreateEventDefend(CombatAssignment* a)
{
    bool friendly  = IsFriendlyAssignment(a);

    if (!friendly)
    return 0;

    CombatEvent*   event    = 0;
    CombatGroup*   group    = a->GetResource();
    CombatGroup*   obj      = a->GetObjective();
    CombatUnit*    unit     = group->GetRandomUnit();
    CombatUnit*    tgt      = obj->GetRandomUnit();

    if (!unit || !tgt)
    return 0;

    bool           success  = Success(a);
    Text           rgn      = group->GetRegion();
    Text           title    = Text(group->Name()) + " in Defensive Engagement";
    Text           info;

    event = new(__FILE__,__LINE__) CombatEvent(campaign,
    CombatEvent::DEFEND, 
    event_time,
    group->GetIFF(),
    CombatEvent::TACNET,
    rgn);

    if (!event)
    return 0;

    int tgt_count  = 0;
    int unit_count = 0;

    if (!success) {
        if (tgt) {
            if (tgt->Kill(1) > 0)
            tgt_count++;
            Combatant* c = group->GetCombatant();
            if (c)     c->AddScore(tgt->GetSingleValue());
        }

        if (unit && RandomChance(1,5)) {
            if (unit->Kill(1) > 0)
            unit_count++;
            Combatant* c = obj->GetCombatant();
            if (c)     c->AddScore(unit->GetSingleValue());
        }
    }

    CombatGroup*   us    = group;
    CombatGroup*   them  = obj;
    int            us_count = unit_count;
    int            them_count = tgt_count;

    if (obj->IsStrikeTarget()) {
        info = Text("EVENT:  ") + rgn + " Sector\n\n";
    }

    else {
        info = Text("MISSION:  Escort ") + obj->Name() + ", " + rgn + " Sector\n\n";
    }

    info += GetTeamName(group);
    info += Text(" ") + group->GetDescription();

    if (success)
    info += " successfully defended ";
    else
    info += " was unable to defend ";

    info += GetTeamName(obj);
    info += Text(" ") + obj->GetDescription() + ".\n\n";

    // need to find an enemy group to do the attacking...

    event->SetTitle(title);
    event->SetInformation(info);
    return event;
}

// +--------------------------------------------------------------------+

CombatEvent*
CampaignPlanEvent::CreateEventFighterAssault(CombatAssignment* a)
{
    CombatEvent*   event    = 0;
    CombatGroup*   group    = a->GetResource();
    CombatGroup*   obj      = a->GetObjective();
    CombatUnit*    unit     = group->GetRandomUnit();
    CombatUnit*    tgt      = obj->GetRandomUnit();

    if (!unit || !tgt)
    return 0;

    bool           success  = Success(a);
    Text           rgn      = group->GetRegion();
    Text           title    = Text(group->Name());
    Text           info;

    event = new(__FILE__,__LINE__) CombatEvent(campaign,
    CombatEvent::ATTACK, 
    event_time,
    group->GetIFF(),
    CombatEvent::TACNET,
    rgn);

    if (!event)
    return 0;

    title += Text(" Assault ") + obj->Name();

    int tgt_count  = 0;
    int unit_count = 0;

    if (success) {
        if (tgt) {
            int killed = tgt->Kill(1 + tgt->Count()/2);
            if (killed > 0)
            tgt_count += killed / tgt->GetSingleValue();
            Combatant* c = group->GetCombatant();
            if (c)     c->AddScore(tgt->GetSingleValue());
        }

        if (unit && RandomChance(1,5)) {
            if (unit->Kill(1) > 0)
            unit_count++;
            Combatant* c = obj->GetCombatant();
            if (c)     c->AddScore(unit->GetSingleValue());
        }
    }
    else {
        for (int i = 0; i < 2; i++) {
            if (unit && RandomChance(1,4)) {
                if (unit->Kill(1) > 0)
                unit_count++;
                Combatant* c = obj->GetCombatant();
                if (c)     c->AddScore(unit->GetSingleValue());
            }
        }
    }

    CombatGroup*   us    = group;
    CombatGroup*   them  = obj;
    int            us_count = unit_count;
    int            them_count = tgt_count;

    bool friendly  = IsFriendlyAssignment(a);

    if (friendly) {
        info = Text("MISSION:  Strike, ") + rgn + " Sector\n\n";
    }

    else {
        info = Text("EVENT:  ") + rgn + " Sector\n\n";

        us = obj;
        them = group;
        us_count = tgt_count;
        them_count = unit_count;
    }

    info += GetTeamName(group);
    info += Text(" ") + group->GetDescription();

    if (success)
    info += " successfully assault ";
    else if (!friendly)
    info += " assault averted against ";
    else
    info += " attempted assault on ";

    info += GetTeamName(obj);
    info += Text(" ") + obj->GetDescription() + ".\n\n";

    char text[256];

    if (them_count) {
        if (friendly) {
            if (them_count > 1)
                sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, tgt->Name().data());
            else
                sprintf_s(text, "ENEMY KILLED:\t %s destroyed\n", tgt->Name().data());
        } else {
            sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, them->Name().data());
        }

        info += text;
    } else {
        info += "ENEMY KILLED:\t 0\n";
    }

    if (us_count) {
        if (!friendly)
            sprintf_s(text, "ALLIED LOSSES:\t %s destroyed\n", tgt->Name().data());
        else
            sprintf_s(text, "ALLIED LOSSES:\t %d %s destroyed",    us_count, us->Name().data());

        info += text;
    }
    else {
        info += "ALLIED LOSSES:\t 0";
    }


    event->SetTitle(title);
    event->SetInformation(info);
    return event;
}

// +--------------------------------------------------------------------+

CombatEvent*
CampaignPlanEvent::CreateEventFighterStrike(CombatAssignment* a)
{
    CombatEvent*   event    = 0;
    CombatGroup*   group    = a->GetResource();
    CombatGroup*   obj      = a->GetObjective();
    CombatUnit*    unit     = group->GetRandomUnit();
    CombatUnit*    tgt      = obj->GetRandomUnit();

    if (!unit || !tgt)
    return 0;

    bool           success  = Success(a);
    Text           rgn      = group->GetRegion();
    Text           title    = Text(group->Name());
    Text           info;

    event = new(__FILE__,__LINE__) CombatEvent(campaign,
    CombatEvent::ATTACK, 
    event_time,
    group->GetIFF(),
    CombatEvent::TACNET,
    rgn);

    if (!event)
    return 0;

    if (unit)
    title += Text(" ") + unit->GetDesign()->abrv + "s";

    if (success) {
        title += " Successfully Strike " + obj->Name();
    }
    else {
        title += " Attempt Strike on " + obj->Name();
    }

    int tgt_count  = 0;
    int unit_count = 0;

    if (success) {
        if (tgt) {
            int killed = tgt->Kill(1 + tgt->Count()/2);
            if (killed > 0)
            tgt_count += killed / tgt->GetSingleValue();
            Combatant* c = group->GetCombatant();
            if (c)     c->AddScore(tgt->GetSingleValue());
        }

        if (unit && RandomChance(1,5)) {
            if (unit->Kill(1) > 0)
            unit_count++;
            Combatant* c = obj->GetCombatant();
            if (c)     c->AddScore(unit->GetSingleValue());
        }
    }
    else {
        for (int i = 0; i < 2; i++) {
            if (unit && RandomChance(1,4)) {
                if (unit->Kill(1) > 0)
                unit_count++;
                Combatant* c = obj->GetCombatant();
                if (c)     c->AddScore(unit->GetSingleValue());
            }
        }
    }

    CombatGroup*   us    = group;
    CombatGroup*   them  = obj;
    int            us_count = unit_count;
    int            them_count = tgt_count;

    bool friendly  = IsFriendlyAssignment(a);

    if (friendly) {
        info = Text("MISSION:  Strike, ") + rgn + " Sector\n\n";
    }

    else {
        info = Text("EVENT:  ") + rgn + " Sector\n\n";

        us = obj;
        them = group;
        us_count = tgt_count;
        them_count = unit_count;
    }

    info += GetTeamName(group);
    info += Text(" ") + group->GetDescription();

    if (success)
    info += " successfully strike ";
    else if (!friendly)
    info += " strike against ";
    else
    info += " attempted strike on ";

    info += GetTeamName(obj);
    info += Text(" ") + obj->GetDescription();

    if (!success && !friendly)
    info += " averted.\n\n";
    else
    info += ".\n\n";

    char text[256];

    if (them_count) {
        if (friendly) {
            if (them_count > 1)
                sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, tgt->Name().data());
            else
                sprintf_s(text, "ENEMY KILLED:\t %s destroyed\n", tgt->Name().data());
        } else {
            sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, them->Name().data());
        }

        info += text;
    } else {
        info += "ENEMY KILLED:\t 0\n";
    }

    if (us_count) {
        if (!friendly)
            sprintf_s(text, "ALLIED LOSSES:\t %s destroyed\n", tgt->Name().data());
        else
            sprintf_s(text, "ALLIED LOSSES:\t %d %s destroyed", us_count, us->Name().data());

        info += text;
    } else {
        info += "ALLIED LOSSES:\t 0";
    }

    event->SetTitle(title);
    event->SetInformation(info);
    return event;
}

// +--------------------------------------------------------------------+

CombatEvent*
CampaignPlanEvent::CreateEventFighterSweep(CombatAssignment* a)
{
    CombatEvent*   event    = 0;
    CombatGroup*   group    = a->GetResource();
    CombatGroup*   obj      = a->GetObjective();
    CombatUnit*    unit     = group->GetRandomUnit();
    CombatUnit*    tgt      = obj->GetRandomUnit();

    if (!unit || !tgt)
    return 0;

    bool           success  = Success(a);
    Text           rgn      = group->GetRegion();
    Text           title    = Text(group->Name());
    Text           info;

    event = new(__FILE__,__LINE__) CombatEvent(campaign,
    CombatEvent::ATTACK, 
    event_time,
    group->GetIFF(),
    CombatEvent::TACNET,
    rgn);

    if (!event)
    return 0;

    if (unit)
    title += Text(" ") + unit->GetDesign()->abrv + "s";
    else
    title += " Fighters";

    if      (RandomChance(1, 4))  title += " Clash with ";
    else if (RandomChance(1, 4))  title += " Engage ";
    else if (RandomChance(1, 4))  title += " Intercept ";
    else                          title += " Encounter ";

    title += obj->Name();

    int tgt_count  = 0;
    int unit_count = 0;

    if (success) {
        for (int i = 0; i < 2; i++) {
            if (tgt && RandomChance(3,4)) {
                if (tgt->Kill(1) > 0)
                tgt_count++;
                Combatant* c = group->GetCombatant();
                if (c)     c->AddScore(tgt->GetSingleValue());
            }
        }

        if (tgt_count > 1) {
            if (tgt && RandomChance(1,4)) {
                if (tgt->Kill(1) > 0)
                tgt_count++;
                Combatant* c = group->GetCombatant();
                if (c)     c->AddScore(tgt->GetSingleValue());
            }
        }

        else {
            if (unit && RandomChance(1,5)) {
                if (unit->Kill(1) > 0)
                unit_count++;
                Combatant* c = obj->GetCombatant();
                if (c)     c->AddScore(unit->GetSingleValue());
            }
        }
    }
    else {
        for (int i = 0; i < 2; i++) {
            if (unit && RandomChance(3,4)) {
                if (unit->Kill(1) > 0)
                unit_count++;
                Combatant* c = obj->GetCombatant();
                if (c)     c->AddScore(unit->GetSingleValue());
            }
        }

        if (tgt && RandomChance(1,4)) {
            if (tgt->Kill(1) > 0)
            tgt_count++;
            Combatant* c = group->GetCombatant();
            if (c)     c->AddScore(tgt->GetSingleValue());
        }
    }

    CombatGroup*   us    = group;
    CombatGroup*   them  = obj;
    int            us_count = unit_count;
    int            them_count = tgt_count;

    bool friendly  = IsFriendlyAssignment(a);

    if (!friendly) {
        us = obj;
        them = group;
        us_count = tgt_count;
        them_count = unit_count;
    }

    if (friendly) {
        if (RandomChance())
        info = Text("MISSION:  OCA Sweep, ") + rgn + " Sector\n\n";
        else
        info = Text("MISSION:  FORCAP, ") + rgn + " Sector\n\n";

        info += GetTeamName(group);
        info += Text(" ") + group->GetDescription();
        info += Text(" engaged ") + GetTeamName(obj);
        info += Text(" ") + obj->GetDescription() + ".\n\n";
    }
    else {
        info = Text("MISSION:  Patrol, ") + rgn + " Sector\n\n";

        info += GetTeamName(obj);
        info += Text(" ") + obj->GetDescription();
        info += Text(" engaged ") + GetTeamName(group);
        info += Text(" ") + group->GetDescription() + ".\n\n";
    }

    char text[256];

    if (them_count) {
        sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, them->Name().data());

        info += text;
    } else {
        info += "ENEMY KILLED:\t 0\n";
    }

    if (us_count) {
        sprintf_s(text, "ALLIED LOSSES:\t %d %s destroyed",    us_count, us->Name().data());
        info += text;
    } else {
        info += "ALLIED LOSSES:\t 0";
    }

    event->SetTitle(title);
    event->SetInformation(info);
    return event;
}

// +--------------------------------------------------------------------+

CombatEvent*
CampaignPlanEvent::CreateEventStarship(CombatAssignment* a)
{
    CombatEvent*   event    = 0;
    CombatGroup*   group    = a->GetResource();
    CombatGroup*   obj      = a->GetObjective();
    CombatUnit*    unit     = group->GetRandomUnit();
    CombatUnit*    tgt      = obj->GetRandomUnit();

    if (!unit || !tgt)
    return 0;

    bool           success  = Success(a);
    Text           rgn      = group->GetRegion();
    Text           title    = Text(group->Name());
    Text           info;

    event = new(__FILE__,__LINE__) CombatEvent(campaign,
    CombatEvent::ATTACK, 
    event_time,
    group->GetIFF(),
    CombatEvent::TACNET,
    group->GetRegion());

    if (!event)
    return 0;

    title += Text(" Assaults ") + a->GetObjective()->Name();

    int tgt_count  = 0;
    int unit_count = 0;

    if (success) {
        if (tgt) {
            if (tgt->Kill(1) > 0)
            tgt_count++;
            Combatant* c = group->GetCombatant();
            if (c)     c->AddScore(tgt->GetSingleValue());
        }

        if (unit && RandomChance(1,5)) {
            if (unit->Kill(1) > 0)
            unit_count++;
            Combatant* c = obj->GetCombatant();
            if (c)     c->AddScore(unit->GetSingleValue());
        }
    }
    else {
        for (int i = 0; i < 2; i++) {
            if (unit && RandomChance(1,4)) {
                if (unit->Kill(1) > 0)
                unit_count++;
                Combatant* c = obj->GetCombatant();
                if (c)     c->AddScore(unit->GetSingleValue());
            }
        }
    }

    CombatGroup*   us    = group;
    CombatGroup*   them  = obj;
    int            us_count = unit_count;
    int            them_count = tgt_count;

    bool friendly  = IsFriendlyAssignment(a);

    if (friendly) {
        info = Text("MISSION:  Fleet Action, ") + rgn + " Sector\n\n";
    }

    else {
        info = Text("EVENT:  ") + rgn + " Sector\n\n";

        us = obj;
        them = group;
        us_count = tgt_count;
        them_count = unit_count;
    }

    info += GetTeamName(group);
    info += Text(" ") + group->GetDescription();

    if (success)
    info += " successfully assaulted ";
    else if (!friendly)
    info += " assault against ";
    else
    info += " attempted assault on ";

    info += GetTeamName(obj);
    info += Text(" ") + obj->GetDescription();

    if (!success && !friendly)
    info += " failed.\n\n";
    else
    info += ".\n\n";

    char text[256];

    if (them_count) {
        if (friendly) {
            if (tgt->Count() > 1) {
                sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, tgt->Name().data());
            } else {
                sprintf_s(text, "ENEMY KILLED:\t %s destroyed\n", tgt->Name().data());
            }
        } else {
            if (unit->Count() > 1) {
                sprintf_s(text, "ENEMY KILLED:\t %d %s destroyed\n", them_count, unit->Name().data());
            } else {
                sprintf_s(text, "ENEMY KILLED:\t %s destroyed\n", unit->Name().data());
            }
        }

        info += text;
    } else {
        info += "ENEMY KILLED:\t 0\n";
    }

    if (us_count) {
        if (!friendly)
            sprintf_s(text, "ALLIED LOSSES:\t %s destroyed\n", tgt->Name().data());
        else
            sprintf_s(text, "ALLIED LOSSES:\t %s destroyed", unit->Name().data());

        info += text;
    } else {
        info += "ALLIED LOSSES:\t 0";
    }

    event->SetTitle(title);
    event->SetInformation(info);
    return event;
}

// +--------------------------------------------------------------------+

bool
CampaignPlanEvent::IsFriendlyAssignment(CombatAssignment* a)
{
    if (!campaign || !a || !a->GetResource())
    return false;

    int            a_team = a->GetResource()->GetIFF();
    CombatGroup*   player = campaign->GetPlayerGroup();

    if (player && (player->GetIFF() == a_team))
    return true;

    return false;
}

bool
CampaignPlanEvent::Success(CombatAssignment* a)
{
    if (!campaign || !a || !a->GetResource())
    return false;

    int odds  = 6 - campaign->GetCampaignId();

    if (odds < 1)
    odds = 1;

    bool success = RandomChance(odds, 5);

    if (!IsFriendlyAssignment(a))
    success = !success;

    return success;
}

// +--------------------------------------------------------------------+

Text
CampaignPlanEvent::GetTeamName(CombatGroup* g)
{
    while (g->GetParent())
    g = g->GetParent();

    return g->Name();
}
