/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignMissionStarship.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignMissionStarship generates missions and mission
    info for the player's STARSHIP GROUP as part of a
    dynamic campaign.
*/

#ifndef CampaignMissionStarship_h
#define CampaignMissionStarship_h

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

class CampaignMissionStarship
{
public:
   static const char* TYPENAME() { return "CampaignMissionStarship"; }

   CampaignMissionStarship(Campaign* c);
   virtual ~CampaignMissionStarship();
   
   virtual void   CreateMission(CampaignMissionRequest* request);

protected:
   virtual Mission*  GenerateMission(int id);
   virtual void      SelectType();
   virtual void      SelectRegion();
   virtual void      GenerateStandardElements();
   virtual void      GenerateMissionElements();
   virtual void      CreateElements(CombatGroup* g);
   virtual void      CreateSquadron(CombatGroup* g);
   virtual void      CreatePlayer();

   virtual void      CreateWards();
   virtual void      CreateWardFreight();

   virtual void      CreateEscorts();

   virtual void      CreateTargets();
   virtual void      CreateTargetsAssault();
   virtual void      CreateTargetsPatrol();
   virtual void      CreateTargetsCarrier();
   virtual void      CreateTargetsFreightEscort();
   virtual int       CreateRandomTarget(const char* rgn, Point base_loc);

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

   CombatUnit*       player_unit;
   CombatGroup*      player_group;
   CombatGroup*      strike_group;
   CombatGroup*      strike_target;
   Mission*          mission;
   List<MissionElement> player_group_elements;
   MissionElement*   player;
   MissionElement*   ward;
   MissionElement*   prime_target;
   MissionElement*   escort;

   int               ownside;
   int               enemy;
   int               mission_type;
};

#endif CampaignMissionStarship_h

