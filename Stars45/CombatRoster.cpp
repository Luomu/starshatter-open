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
    FILE:         CombatRoster.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    The complete roster of all known persistent entities
    for all combatants in the game.
*/

#include "MemDebug.h"
#include "CombatRoster.h"
#include "CombatGroup.h"

#include "Game.h"
#include "DataLoader.h"
#include "Text.h"

// +--------------------------------------------------------------------+

static CombatRoster* roster = 0;

// +--------------------------------------------------------------------+

CombatRoster::CombatRoster()
{
    DataLoader* loader = DataLoader::GetLoader();
    loader->SetDataPath("Campaigns/");

    List<Text> files;
    loader->ListFiles("*.def", files);

    for (int i = 0; i < files.size(); i++) {
        Text filename = *files[i];

        if (!filename.contains("/") && !filename.contains("\\")) {
            loader->SetDataPath("Campaigns/");
            CombatGroup* g = CombatGroup::LoadOrderOfBattle(filename, -1, 0);
            forces.append(g);
        }
    }

    files.destroy();
}

// +--------------------------------------------------------------------+

CombatRoster::~CombatRoster()
{
    forces.destroy();
}

// +--------------------------------------------------------------------+

CombatGroup*
CombatRoster::GetForce(const char* name)
{
    ListIter<CombatGroup> iter = forces;
    while (++iter) {
        CombatGroup* f = iter.value();

        if (f->Name() == name)
        return f;
    }

    return 0;
}

// +--------------------------------------------------------------------+

void
CombatRoster::Initialize()
{
    roster = new(__FILE__,__LINE__) CombatRoster();
}

void
CombatRoster::Close()
{
    delete roster;
    roster = 0;
}

CombatRoster*
CombatRoster::GetInstance()
{
    return roster;
}