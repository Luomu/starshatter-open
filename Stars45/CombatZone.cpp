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
    FILE:         CombatZone.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CombatZone is used by the dynamic campaign strategy
    and logistics algorithms to assign forces to locations
    within the campaign.  A CombatZone is a collection of
    closely related sectors, and the assets contained
    within them.
*/

#include "MemDebug.h"
#include "CombatZone.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "Campaign.h"
#include "ShipDesign.h"
#include "Ship.h"

#include "Game.h"
#include "DataLoader.h"
#include "ParseUtil.h"

// +----------------------------------------------------------------------+

CombatZone::CombatZone()
{
}

CombatZone::~CombatZone()
{
    regions.destroy();
    forces.destroy();
}

// +--------------------------------------------------------------------+

void
CombatZone::Clear()
{
    forces.destroy();
}

// +--------------------------------------------------------------------+

void
CombatZone::AddGroup(CombatGroup* group)
{
    if (group) {
        int iff = group->GetIFF();
        ZoneForce* f = FindForce(iff);
        f->AddGroup(group);
        group->SetCurrentZone(this);
    }
}

void
CombatZone::RemoveGroup(CombatGroup* group)
{
    if (group) {
        int iff = group->GetIFF();
        ZoneForce* f = FindForce(iff);
        f->RemoveGroup(group);
        group->SetCurrentZone(0);
    }
}

bool
CombatZone::HasGroup(CombatGroup* group)
{
    if (group) {
        int iff = group->GetIFF();
        ZoneForce* f = FindForce(iff);
        return f->HasGroup(group);
    }

    return false;
}

// +--------------------------------------------------------------------+

void
CombatZone::AddRegion(const char* rgn)
{
    if (rgn && *rgn) {
        regions.append(new (__FILE__,__LINE__) Text(rgn));

        if (name.length() < 1)
        name = rgn;
    }
}

// +--------------------------------------------------------------------+

bool
CombatZone::HasRegion(const char* rgn)
{
    if (rgn && *rgn && regions.size()) {
        Text test(rgn);
        return regions.contains(&test);
    }

    return false;
}

// +--------------------------------------------------------------------+

ZoneForce*
CombatZone::FindForce(int iff)
{
    ListIter<ZoneForce> f = forces;
    while (++f) {
        if (f->GetIFF() == iff)
        return f.value();
    }

    return MakeForce(iff);
}

// +--------------------------------------------------------------------+

ZoneForce*
CombatZone::MakeForce(int iff)
{
    ZoneForce* f = new(__FILE__,__LINE__) ZoneForce(iff);
    forces.append(f);
    return f;
}

// +--------------------------------------------------------------------+

static List<CombatZone> zonelist;

List<CombatZone>&
CombatZone::Load(const char* filename)
{
    zonelist.clear();

    DataLoader* loader   = DataLoader::GetLoader();
    BYTE*       block    = 0;

    loader->LoadBuffer(filename, block, true);
    Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

    Term*  term = parser.ParseTerm();

    if (!term) {
        return zonelist;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "ZONES") {
            return zonelist;
        }
    }

    do {
        delete term; term = 0;
        term = parser.ParseTerm();
        
        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "zone") {
                    if (!def->term() || !def->term()->isStruct()) {
                        ::Print("WARNING: zone struct missing in '%s%s'\n", loader->GetDataPath(), filename);
                    }
                    else {
                        TermStruct* val = def->term()->isStruct();

                        CombatZone* zone = new(__FILE__,__LINE__) CombatZone();
                        char        rgn[64];
                        ZeroMemory(rgn,  sizeof(rgn));

                        for (int i = 0; i < val->elements()->size(); i++) {
                            TermDef* pdef = val->elements()->at(i)->isDef();
                            if (pdef) {
                                if (pdef->name()->value() == "region") {
                                    GetDefText(rgn, pdef, filename);
                                    zone->AddRegion(rgn);
                                }
                                else if (pdef->name()->value() == "system") {
                                    GetDefText(rgn, pdef, filename);
                                    zone->system = rgn;
                                }
                            }
                        }

                        zonelist.append(zone);
                    }
                }
            }
        }
    }
    while (term);

    loader->ReleaseBuffer(block);

    return zonelist;
}

// +--------------------------------------------------------------------+

ZoneForce::ZoneForce(int i)
{
    iff = i;

    for (int n = 0; n < 8; n++)
    need[n] = 0;
}

void
ZoneForce::AddGroup(CombatGroup* group)
{
    if (group)
    groups.append(group);
}

void
ZoneForce::RemoveGroup(CombatGroup* group)
{
    if (group)
    groups.remove(group);
}

bool
ZoneForce::HasGroup(CombatGroup* group)
{
    if (group)
    return groups.contains(group);

    return false;
}

int
ZoneForce::GetNeed(int group_type) const
{
    switch (group_type) {
    case CombatGroup::CARRIER_GROUP:       return need[0];
    case CombatGroup::BATTLE_GROUP:        return need[1];
    case CombatGroup::DESTROYER_SQUADRON:  return need[2];
    case CombatGroup::ATTACK_SQUADRON:     return need[3];
    case CombatGroup::FIGHTER_SQUADRON:    return need[4];
    case CombatGroup::INTERCEPT_SQUADRON:  return need[5];
    }

    return 0;
}

void
ZoneForce::SetNeed(int group_type, int needed)
{
    switch (group_type) {
    case CombatGroup::CARRIER_GROUP:       need[0] = needed; break;
    case CombatGroup::BATTLE_GROUP:        need[1] = needed; break;
    case CombatGroup::DESTROYER_SQUADRON:  need[2] = needed; break;
    case CombatGroup::ATTACK_SQUADRON:     need[3] = needed; break;
    case CombatGroup::FIGHTER_SQUADRON:    need[4] = needed; break;
    case CombatGroup::INTERCEPT_SQUADRON:  need[5] = needed; break;
    }
}

void
ZoneForce::AddNeed(int group_type, int needed)
{
    switch (group_type) {
    case CombatGroup::CARRIER_GROUP:       need[0] += needed; break;
    case CombatGroup::BATTLE_GROUP:        need[1] += needed; break;
    case CombatGroup::DESTROYER_SQUADRON:  need[2] += needed; break;
    case CombatGroup::ATTACK_SQUADRON:     need[3] += needed; break;
    case CombatGroup::FIGHTER_SQUADRON:    need[4] += needed; break;
    case CombatGroup::INTERCEPT_SQUADRON:  need[5] += needed; break;
    }
}

