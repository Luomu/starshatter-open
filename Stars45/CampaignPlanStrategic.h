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
    FILE:         CampaignPlanStrategic.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlanStrategic prioritizes targets and defensible
    allied forces as the first step in force tasking.  This
    algorithm computes which enemy resources are most important
    to attack, based on the AI value of each combat group, and
    strategic weighting factors that help shape the strategy
    to the objectives for the current campaign.
*/

#ifndef CampaignPlanStrategic_h
#define CampaignPlanStrategic_h

#include "Types.h"
#include "CampaignPlan.h"

// +--------------------------------------------------------------------+

class CampaignPlanStrategic : public CampaignPlan
{
public:
    static const char* TYPENAME() { return "CampaignPlanStrategic"; }

    CampaignPlanStrategic(Campaign* c) : CampaignPlan(c) { }
    virtual ~CampaignPlanStrategic()                     { }

    // operations:
    virtual void   ExecFrame();

protected:
    void           PlaceGroup(CombatGroup* g);

    void           ScoreCombatant(Combatant* c);

    void           ScoreDefensible(Combatant* c);
    void           ScoreDefend(Combatant* c, CombatGroup* g);

    void           ScoreTargets(Combatant* c, Combatant* t);
    void           ScoreTarget(Combatant* c, CombatGroup* g);

    void           ScoreNeeds(Combatant* c);

    // zone alocation:
    void           BuildGroupList(CombatGroup* g, List<CombatGroup>& groups);
    void           AssignZones(Combatant* c);
    void           ResolveZoneMovement(CombatGroup* g);
};

#endif CampaignPlanStrategic_h

