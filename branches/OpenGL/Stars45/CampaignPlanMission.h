/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CampaignPlanMission.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	CampaignPlanMission generates missions and mission
	info for the player's combat group as part of a
	dynamic campaign.
*/

#ifndef CampaignPlanMission_h
#define CampaignPlanMission_h

#include "Types.h"
#include "CampaignPlan.h"

// +--------------------------------------------------------------------+

class CampaignMissionRequest;
class CombatGroup;
class CombatUnit;
class CombatZone;

// +--------------------------------------------------------------------+

class CampaignPlanMission : public CampaignPlan
{
public:
	static const char* TYPENAME() { return "CampaignPlanMission"; }

	CampaignPlanMission(Campaign* c) : CampaignPlan(c), start(0), slot(0) { }
	virtual ~CampaignPlanMission() { }

	// operations:
	virtual void   ExecFrame();

protected:
	virtual void                     SelectStartTime();
	virtual CampaignMissionRequest*  PlanCampaignMission();
	virtual CampaignMissionRequest*  PlanStrategicMission();
	virtual CampaignMissionRequest*  PlanRandomStarshipMission();
	virtual CampaignMissionRequest*  PlanRandomFighterMission();
	virtual CampaignMissionRequest*  PlanStarshipMission();
	virtual CampaignMissionRequest*  PlanFighterMission();

	CombatGroup*   player_group;
	int            start;
	int            slot;
};

#endif CampaignPlanMission_h

