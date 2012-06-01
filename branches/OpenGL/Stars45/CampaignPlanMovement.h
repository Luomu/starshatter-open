/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CampaignPlanMovement.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	CampaignPlanMovement simulates random patrol movements
	of starship groups between missions.  This agitation
	keeps the ships from bunching up in the middle of a
	sector.
*/

#ifndef CampaignPlanMovement_h
#define CampaignPlanMovement_h

#include "Types.h"
#include "CampaignPlan.h"

// +--------------------------------------------------------------------+

class CampaignPlanMovement : public CampaignPlan
{
public:
	static const char* TYPENAME() { return "CampaignPlanMovement"; }

	CampaignPlanMovement(Campaign* c) : CampaignPlan(c) { }
	virtual ~CampaignPlanMovement()                     { }

	// operations:
	virtual void      ExecFrame();

protected:
	void              MoveUnit(CombatUnit* u);

	List<CombatUnit>  all_units;
};

#endif CampaignPlanMovement_h

