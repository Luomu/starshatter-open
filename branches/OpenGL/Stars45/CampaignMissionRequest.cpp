/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CampaignMissionRequest.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	CampaignMissionRequest 
*/

#include "MemDebug.h"
#include "CampaignMissionRequest.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Mission.h"
#include "Instruction.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Random.h"

// +--------------------------------------------------------------------+

CampaignMissionRequest::CampaignMissionRequest(Campaign*    c,
int          t,
int          s,
CombatGroup* p,
CombatGroup* tgt)
: campaign(c), type(t), opp_type(-1), start(s), 
primary_group(p), secondary_group(0),
objective(tgt), use_loc(false)
{ }
