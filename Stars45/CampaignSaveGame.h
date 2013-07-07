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
    FILE:         CampaignSaveGame.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignSaveGame contains the logic needed to save and load
    campaign games in progress.
*/

#ifndef CampaignSaveGame_h
#define CampaignSaveGame_h

#include "Types.h"
#include "Geometry.h"
#include "text.h"
#include "term.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Campaign;
class CampaignPlan;
class Combatant;
class CombatGroup;
class CombatZone;
class DataLoader;
class Mission;
class Player;
class StarSystem;

// +--------------------------------------------------------------------+

class CampaignSaveGame
{
public:
    static const char* TYPENAME() { return "CampaignSaveGame"; }

    CampaignSaveGame(Campaign* c=0);
    virtual ~CampaignSaveGame();

    virtual Campaign* GetCampaign()     { return campaign;   }

    virtual void      Load(const char* name);
    virtual void      Save(const char* name);
    static  void      Delete(const char* name);
    static  void      RemovePlayer(Player* p);

    virtual void      LoadAuto();
    virtual void      SaveAuto();

    static  Text      GetResumeFile();
    static  int       GetSaveGameList(List<Text>& save_list);

private:
    static  Text      GetSaveDirectory();
    static  Text      GetSaveDirectory(Player* p);
    static  void      CreateSaveDirectory();

    Campaign*   campaign;
};

#endif CampaignSaveGame_h

