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
    FILE:         CampaignPlan.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlan defines the interface for all campaign
    planning algorithms.  Known subclasses:
    CampaignPlanStrategic  - strategic planning
    CampaignPlanAssignment - logistics planning
    CampaignPlanMission    - mission planning
    CampaignPlanMovement   - starship movement
    CampaignPlanEvent      - scripted events
*/

#ifndef CampaignPlan_h
#define CampaignPlan_h

#include "Types.h"
#include "Text.h"
#include "Term.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Campaign;
class Combatant;
class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+

class CampaignPlan
{
public:
    static const char* TYPENAME() { return "CampaignPlan"; }

    CampaignPlan(Campaign* c) : campaign(c), exec_time(-1e6) { }
    virtual ~CampaignPlan()                                  { }

    int operator == (const CampaignPlan& p) const { return this == &p; }

    // operations:
    virtual void      ExecFrame() { }
    virtual void      SetLockout(int seconds) { }

protected:
    Campaign*         campaign;
    double            exec_time;
};

#endif CampaignPlan_h

