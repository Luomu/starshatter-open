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
    FILE:         CampaignPlanEvent.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanEvent generates simulated combat
    events based on a statistical analysis of the
    combatants within the context of a dynamic
    campaign.
*/

#ifndef CampaignPlanEvent_h
#define CampaignPlanEvent_h

#include "Types.h"
#include "CampaignPlan.h"

// +--------------------------------------------------------------------+

class CombatAction;
class CombatAssignment;
class CombatEvent;
class CombatGroup;
class CombatUnit;
class CombatZone;

// +--------------------------------------------------------------------+

class CampaignPlanEvent : public CampaignPlan
{
public:
    static const char* TYPENAME() { return "CampaignPlanEvent"; }

    CampaignPlanEvent(Campaign* c);
    virtual ~CampaignPlanEvent();

    // operations:
    virtual void   ExecFrame();
    virtual void   SetLockout(int seconds);

    virtual bool   ExecScriptedEvents();
    virtual bool   ExecStatisticalEvents();

protected:
    virtual void   ProsecuteKills(CombatAction* action);

    virtual CombatAssignment*
    ChooseAssignment(CombatGroup* c);
    virtual bool   CreateEvent(CombatAssignment* a);

    virtual CombatEvent* CreateEventDefend(CombatAssignment* a);
    virtual CombatEvent* CreateEventFighterAssault(CombatAssignment* a);
    virtual CombatEvent* CreateEventFighterStrike(CombatAssignment* a);
    virtual CombatEvent* CreateEventFighterSweep(CombatAssignment* a);
    virtual CombatEvent* CreateEventStarship(CombatAssignment* a);

    virtual bool         IsFriendlyAssignment(CombatAssignment* a);
    virtual bool         Success(CombatAssignment* a);
    virtual Text         GetTeamName(CombatGroup* g);

    // attributes:
    int            event_time;
};

#endif CampaignPlanEvent_h

