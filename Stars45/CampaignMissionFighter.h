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
    FILE:         CampaignMissionFighter.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignMissionFighter generates missions and mission
    info for the player's FIGHTER SQUADRON as part of a
    dynamic campaign.
*/

#ifndef CampaignMissionFighter_h
#define CampaignMissionFighter_h

#include "Types.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Campaign;
class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;
class Mission;
class MissionElement;
class MissionInfo;
class MissionTemplate;

// +--------------------------------------------------------------------+

class CampaignMissionFighter
{
public:
    static const char* TYPENAME() { return "CampaignMissionFighter"; }

    CampaignMissionFighter(Campaign* c);
    virtual ~CampaignMissionFighter();

    virtual void   CreateMission(CampaignMissionRequest* request);

protected:
    virtual Mission*  GenerateMission(int id);
    virtual void      SelectType();
    virtual void      SelectRegion();
    virtual void      GenerateStandardElements();
    virtual void      GenerateMissionElements();
    virtual void      CreateElements(CombatGroup* g);
    virtual void      CreateSquadron(CombatGroup* g);
    virtual void      CreatePlayer(CombatGroup* g);

    virtual void      CreatePatrols();
    virtual void      CreateWards();
    virtual void      CreateWardFreight();
    virtual void      CreateWardShuttle();
    virtual void      CreateWardStrike();

    virtual void      CreateEscorts();

    virtual void      CreateTargets();
    virtual void      CreateTargetsPatrol();
    virtual void      CreateTargetsSweep();
    virtual void      CreateTargetsIntercept();
    virtual void      CreateTargetsFreightEscort();
    virtual void      CreateTargetsShuttleEscort();
    virtual void      CreateTargetsStrikeEscort();
    virtual void      CreateTargetsStrike();
    virtual void      CreateTargetsAssault();
    virtual int       CreateRandomTarget(const char* rgn, Point base_loc);

    virtual bool      IsGroundObjective(CombatGroup* obj);

    virtual void      PlanetaryInsertion(MissionElement* elem);
    virtual void      OrbitalInsertion(MissionElement* elem);

    virtual MissionElement*
    CreateSingleElement(CombatGroup*    g,
    CombatUnit*     u);
    virtual MissionElement*
    CreateFighterPackage(CombatGroup*   squadron, 
    int            count, 
    int            role);

    virtual CombatGroup* FindSquadron(int iff, int type);
    virtual CombatUnit*  FindCarrier(CombatGroup* g);

    virtual void         DefineMissionObjectives();
    virtual MissionInfo* DescribeMission();
    virtual void         Exit();

    Campaign*         campaign;
    CampaignMissionRequest* request;
    MissionInfo*      mission_info;

    CombatGroup*      squadron;
    CombatGroup*      strike_group;
    CombatGroup*      strike_target;
    Mission*          mission;
    MissionElement*   player_elem;
    MissionElement*   carrier_elem;
    MissionElement*   ward;
    MissionElement*   prime_target;
    MissionElement*   escort;
    Text              air_region;
    Text              orb_region;
    bool              airborne;
    bool              airbase;
    int               ownside;
    int               enemy;
    int               mission_type;
};

#endif CampaignMissionFighter_h

