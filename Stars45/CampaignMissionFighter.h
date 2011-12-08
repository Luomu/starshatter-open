/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

