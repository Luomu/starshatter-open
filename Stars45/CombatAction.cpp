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
    FILE:         CombatAction.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A significant (newsworthy) event in the dynamic campaign.
*/

#include "MemDebug.h"
#include "CombatAction.h"
#include "CombatGroup.h"
#include "Campaign.h"
#include "Combatant.h"
#include "Player.h"
#include "Random.h"

// +----------------------------------------------------------------------+

CombatAction::CombatAction(int n, int typ, int sub, int iff)
    : id(n), type(typ), subtype(sub), opp_type(-1), team(iff),
      status(PENDING), count(0), rval(-1), source(0), time(0),
      start_before((int) 1e9), start_after(0),
      min_rank(0), max_rank(100),
      delay(0), probability(100), asset_type(0), target_type(0)
{ }

CombatAction::~CombatAction()
{
    requirements.destroy();
    asset_kills.destroy();
    target_kills.destroy();
}

// +----------------------------------------------------------------------+

bool
CombatAction::IsAvailable() const
{
    CombatAction* pThis = (CombatAction*) this;

    if (rval < 0) {
        pThis->rval = (int) Random(0, 100);

        if (rval > probability)
        pThis->status = SKIPPED;
    }

    if (status != PENDING)
    return false;

    if (min_rank > 0 || max_rank < 100) {
        Player* player = Player::GetCurrentPlayer();

        if (player->Rank() < min_rank || player->Rank() > max_rank)
        return false;
    }

    Campaign* campaign = Campaign::GetCampaign();
    if (campaign) {
        if (campaign->GetTime() < start_after) {
            return false;
        }

        if (campaign->GetTime() > start_before) {
            pThis->status = FAILED; // too late!
            return false;
        }

        // check requirements against actions in current campaign:
        ListIter<CombatActionReq> iter = pThis->requirements;
        while (++iter) {
            CombatActionReq* r  = iter.value();
            bool             ok = false;

            if (r->action > 0) {
                ListIter<CombatAction> action = campaign->GetActions();
                while (++action) {
                    CombatAction* a = action.value();

                    if (a->Identity() == r->action) {
                        if (r->not) {
                            if (a->Status() == r->stat)
                            return false;
                        }
                        else {
                            if (a->Status() != r->stat)
                            return false;
                        }
                    }
                }
            }

            // group-based requirement
            else if (r->group_type > 0) {
                if (r->c1) {
                    CombatGroup* group = r->c1->FindGroup(r->group_type, r->group_id);

                    if (group) {
                        int test = 0;
                        int comp = 0;

                        if (r->intel) {
                            test = group->IntelLevel();
                            comp = r->intel;
                        }

                        else {
                            test = group->CalcValue();
                            comp = r->score;
                        }

                        switch (r->comp) {
                        case CombatActionReq::LT:  ok = (test <  comp); break;
                        case CombatActionReq::LE:  ok = (test <= comp); break;
                        case CombatActionReq::GT:  ok = (test >  comp); break;
                        case CombatActionReq::GE:  ok = (test >= comp); break;
                        case CombatActionReq::EQ:  ok = (test == comp); break;
                        }
                    }

                    if (!ok)
                    return false;
                }
            }

            // score-based requirement
            else {
                int test = 0;

                if (r->comp <= CombatActionReq::EQ) {  // absolute
                    if (r->c1) {
                        int test = r->c1->Score();

                        switch (r->comp) {
                        case CombatActionReq::LT:  ok = (test <  r->score); break;
                        case CombatActionReq::LE:  ok = (test <= r->score); break;
                        case CombatActionReq::GT:  ok = (test >  r->score); break;
                        case CombatActionReq::GE:  ok = (test >= r->score); break;
                        case CombatActionReq::EQ:  ok = (test == r->score); break;
                        }
                    }
                }

                else {                                 // relative
                    if (r->c1 && r->c2) {
                        int test = r->c1->Score() - r->c2->Score();

                        switch (r->comp) {
                        case CombatActionReq::RLT: ok = (test <  r->score); break;
                        case CombatActionReq::RLE: ok = (test <= r->score); break;
                        case CombatActionReq::RGT: ok = (test >  r->score); break;
                        case CombatActionReq::RGE: ok = (test >= r->score); break;
                        case CombatActionReq::REQ: ok = (test == r->score); break;
                        }
                    }
                }

                if (!ok)
                return false;
            }

            if (delay > 0) {
                pThis->start_after = (int) campaign->GetTime() + delay;
                pThis->delay       = 0;
                return IsAvailable();
            }
        }
    }

    return true;
}

// +----------------------------------------------------------------------+

void
CombatAction::FireAction()
{
    Campaign* campaign = Campaign::GetCampaign();
    if (campaign)
    time = (int) campaign->GetTime();

    if (count >= 1)
    count--;

    if (count < 1)
    status = COMPLETE;
}

void
CombatAction::FailAction()
{
    Campaign* campaign = Campaign::GetCampaign();
    if (campaign)
    time = (int) campaign->GetTime();

    count  = 0;
    status = FAILED;
}

// +----------------------------------------------------------------------+

void
CombatAction::AddRequirement(int action, int stat, bool not)
{
    requirements.append(new(__FILE__,__LINE__) CombatActionReq(action, stat, not));
}

void
CombatAction::AddRequirement(Combatant* c1, Combatant* c2, int comp, int score)
{
    requirements.append(new(__FILE__,__LINE__) CombatActionReq(c1, c2, comp, score));
}

void
CombatAction::AddRequirement(Combatant* c1, int group_type, int group_id, int comp, int score, int intel)
{
    requirements.append(new(__FILE__,__LINE__) CombatActionReq(c1, group_type, group_id, comp, score, intel));
}

// +----------------------------------------------------------------------+

int
CombatAction::TypeFromName(const char* n)
{
    int type = 0;

    if (!_stricmp(n, "NO_ACTION"))
        type = NO_ACTION;

    else if (!_stricmp(n, "MARKER"))
        type = NO_ACTION;

    else if (!_stricmp(n, "STRATEGIC_DIRECTIVE"))
        type = STRATEGIC_DIRECTIVE;

    else if (!_stricmp(n, "STRATEGIC"))
        type = STRATEGIC_DIRECTIVE;

    else if (!_stricmp(n, "ZONE_ASSIGNMENT"))
        type = ZONE_ASSIGNMENT;

    else if (!_stricmp(n, "ZONE"))
        type = ZONE_ASSIGNMENT;

    else if (!_stricmp(n, "SYSTEM_ASSIGNMENT"))
        type = SYSTEM_ASSIGNMENT;

    else if (!_stricmp(n, "SYSTEM"))
        type = SYSTEM_ASSIGNMENT;

    else if (!_stricmp(n, "MISSION_TEMPLATE"))
        type = MISSION_TEMPLATE;

    else if (!_stricmp(n, "MISSION"))
        type = MISSION_TEMPLATE;

    else if (!_stricmp(n, "COMBAT_EVENT"))
        type = COMBAT_EVENT;

    else if (!_stricmp(n, "EVENT"))
        type = COMBAT_EVENT;

    else if (!_stricmp(n, "INTEL_EVENT"))
        type = INTEL_EVENT;

    else if (!_stricmp(n, "INTEL"))
        type = INTEL_EVENT;

    else if (!_stricmp(n, "CAMPAIGN_SITUATION"))
        type = CAMPAIGN_SITUATION;

    else if (!_stricmp(n, "SITREP"))
        type = CAMPAIGN_SITUATION;

    else if (!_stricmp(n, "CAMPAIGN_ORDERS"))
        type = CAMPAIGN_ORDERS;

    else if (!_stricmp(n, "ORDERS"))
        type = CAMPAIGN_ORDERS;

    return type;
}

int
CombatAction::StatusFromName(const char* n)
{
    int stat = 0;

    if (!_stricmp(n, "PENDING"))
    stat = PENDING;

    else if (!_stricmp(n, "ACTIVE"))
    stat = ACTIVE;

    else if (!_stricmp(n, "SKIPPED"))
    stat = SKIPPED;

    else if (!_stricmp(n, "FAILED"))
    stat = FAILED;

    else if (!_stricmp(n, "COMPLETE"))
    stat = COMPLETE;

    return stat;
}


// +----------------------------------------------------------------------+

int
CombatActionReq::CompFromName(const char* n)
{
    int comp = 0;

    if (!_stricmp(n, "LT"))
    comp = LT;

    else if (!_stricmp(n, "LE"))
    comp = LE;

    else if (!_stricmp(n, "GT"))
    comp = GT;

    else if (!_stricmp(n, "GE"))
    comp = GE;

    else if (!_stricmp(n, "EQ"))
    comp = EQ;

    else if (!_stricmp(n, "RLT"))
    comp = RLT;

    else if (!_stricmp(n, "RLE"))
    comp = RLE;

    else if (!_stricmp(n, "RGT"))
    comp = RGT;

    else if (!_stricmp(n, "RGE"))
    comp = RGE;

    else if (!_stricmp(n, "REQ"))
    comp = REQ;

    return comp;
}
