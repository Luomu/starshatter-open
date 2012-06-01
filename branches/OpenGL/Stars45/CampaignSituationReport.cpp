/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CampaignSituationReport.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	CampaignSituationReport generates the situation report
	portion of the briefing for a dynamically generated
	mission in a dynamic campaign.
*/

#include "MemDebug.h"
#include "CampaignSituationReport.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Callsign.h"
#include "Mission.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Random.h"
#include "Player.h"

// +--------------------------------------------------------------------+

CampaignSituationReport::CampaignSituationReport(Campaign* c, Mission* m)
: campaign(c), mission(m)
{}

CampaignSituationReport::~CampaignSituationReport() {}

// +--------------------------------------------------------------------+

void
CampaignSituationReport::GenerateSituationReport()
{
	if (!campaign || !mission)
	return;

	sitrep = Text();

	GlobalSituation();
	MissionSituation();

	mission->SetSituation(sitrep);
}

// +--------------------------------------------------------------------+

static const char* outlooks[4] = { "good", "fluid", "poor", "bleak" };

void
CampaignSituationReport::GlobalSituation()
{
	if (campaign->GetTime() < 40 * 3600)
	sitrep = Text(campaign->Name()) + Text(" is still in its early stages and the situation is ");
	else
	sitrep = Text("The overall outlook for ") + Text(campaign->Name()) + Text(" is ");

	int score = campaign->GetPlayerTeamScore();

	if (score > 1000)
	sitrep += outlooks[0];
	else if (score > -1000)
	sitrep += outlooks[1];
	else if (score > -2000)
	sitrep += outlooks[2];
	else
	sitrep += outlooks[3];

	sitrep += ".  ";

	Text strat_dir;

	CombatGroup* pg = campaign->GetPlayerGroup();

	if (pg)
	strat_dir = pg->GetStrategicDirection();

	if (strat_dir.length())
	sitrep += strat_dir;
	else
	sitrep += Text("Establishing and maintaining military control of the ") +
	mission->GetStarSystem()->Name() + " System remains a key priority.";
}

// +--------------------------------------------------------------------+

void
CampaignSituationReport::MissionSituation()
{
	if (mission) {
		MissionElement* player = mission->GetPlayer();
		MissionElement* target = mission->GetTarget();
		MissionElement* ward   = mission->GetWard();
		MissionElement* escort = FindEscort(player);
		Text            threat = GetThreatInfo();
		Text            sector = mission->GetRegion();

		sector += " sector.";

		switch (mission->Type()) {
		case Mission::PATROL:
		case Mission::AIR_PATROL:
			sitrep += "\n\nThis mission is a routine patrol of the ";
			sitrep += sector;
			break;

		case Mission::SWEEP:
		case Mission::AIR_SWEEP:
			sitrep += "\n\nFor this mission, you will be performing a fighter sweep of the ";
			sitrep += sector;
			break;

		case Mission::INTERCEPT:
		case Mission::AIR_INTERCEPT:
			sitrep += "\n\nWe have detected hostile elements inbound.  ";
			sitrep += "Your mission is to intercept them before they are able to engage their targets.";
			break;


		case Mission::STRIKE:
			sitrep += "\n\nThe goal of this mission is to perform a strike on preplanned targets in the ";
			sitrep += sector;

			if (target) {
				sitrep += "  Your package has been assigned to strike the ";

				if (target->GetCombatGroup())
				sitrep += target->GetCombatGroup()->GetDescription();
				else
				sitrep += target->Name();

				sitrep += ".";
			}
			break;

		case Mission::ASSAULT:
			sitrep += "\n\nThis mission is to assault preplanned targets in the ";
			sitrep += sector;

			if (target) {
				sitrep += "  Your package has been assigned to strike the ";

				if (target->GetCombatGroup())
				sitrep += target->GetCombatGroup()->GetDescription();
				else
				sitrep += target->Name();

				sitrep += ".";
			}
			break;

		case Mission::DEFEND:
			if (ward) {
				sitrep += "\n\nFor this mission, you will need to defend ";
				sitrep += ward->Name();
				sitrep += " in the ";
				sitrep += sector;
			}
			else {
				sitrep += "\n\nThis is a defensive patrol mission in the ";
				sitrep += sector;
			}
			break;

		case Mission::ESCORT:
			if (ward) {
				sitrep += "\n\nFor this mission, you will need to escort the ";
				sitrep += ward->Name();
				sitrep += " in the ";
				sitrep += sector;
			}
			else {
				sitrep += "\n\nThis is an escort mission in the ";
				sitrep += sector;
			}
			break;

		case Mission::ESCORT_FREIGHT:
			if (ward) {
				sitrep += "\n\nFor this mission, you will need to escort the freighter ";
				sitrep += ward->Name();
				sitrep += ".";
			}
			else {
				sitrep += "\n\nThis is a freight escort mission in the ";
				sitrep += sector;
			}
			break;

		case Mission::ESCORT_SHUTTLE:
			if (ward) {
				sitrep += "\n\nFor this mission, you will need to escort the shuttle ";
				sitrep += ward->Name();
				sitrep += ".";
			}
			else {
				sitrep += "\n\nThis is a shuttle escort mission in the ";
				sitrep += sector;
			}
			break;

		case Mission::ESCORT_STRIKE:
			if (ward) {
				sitrep += "\n\nFor this mission, you will need to protect the ";
				sitrep += ward->Name();
				sitrep += " strike package from hostile interceptors.";
			}
			else {
				sitrep += "\n\nFor this mission, you will be responsible for strike escort duty.";
			}
			break;

		case Mission::INTEL:
		case Mission::SCOUT:
		case Mission::RECON:
			sitrep += "\n\nThis is an intelligence gathering mission in the ";
			sitrep += sector;
			break;

		case Mission::BLOCKADE:
			sitrep += "\n\nThis mission is part of the blockade operation in the ";
			sitrep += sector;
			break;

		case Mission::FLEET:
			sitrep += "\n\nThis mission is a routine fleet patrol of the ";
			sitrep += sector;
			break;

		case Mission::BOMBARDMENT:
			sitrep += "\n\nOur goal for this mission is to engage and destroy preplanned targets in the ";
			sitrep += sector;
			break;

		case Mission::FLIGHT_OPS:
			sitrep += "\n\nFor this mission, the ";
			sitrep += player->Name();
			sitrep += " will be conducting combat flight operations in the ";
			sitrep += sector;
			break;

		case Mission::TRAINING:
			sitrep += "\n\nThis will be a training mission.";
			break;

		case Mission::TRANSPORT:
		case Mission::CARGO:
		case Mission::OTHER:
		default:
			break;
		}

		if (threat.length()) {
			sitrep += "  ";
			sitrep += threat;
			sitrep += "\n\n";
		}
	}
	else {
		sitrep += "\n\n";
	}

	Text rank;
	Text name;

	Player* p = Player::GetCurrentPlayer();

	if (p) {
		if (p->Rank() > 6)
		rank = ", Admiral";
		else
		rank = Text(", ") + Player::RankName(p->Rank());

		name = Text(", ") + p->Name();
	}

	sitrep += "You have a mission to perform.  ";

	switch (RandomIndex()) {
	case  0: sitrep += "You'd better go get to it!";                     break;
	case  1: sitrep += "And let's be careful out there!";                break;
	case  2: sitrep += "Good luck, sir!";                                break;
	case  3: sitrep += "Let's keep up the good work out there.";         break;
	case  4: sitrep += "Don't lose your focus.";                         break;
	case  5: sitrep += "Good luck out there.";                           break;
	case  6: sitrep += "What are you waiting for, cocktail hour?";       break;
	case  7: sitrep += Text("Godspeed") + rank + "!";                    break;
	case  8: sitrep += Text("Good luck") + rank + "!";                   break;
	case  9: sitrep += Text("Good luck") + name + "!";                   break;
	case 10: sitrep += "If everything is clear, get your team ready and get underway.";
		break;
	case 11: sitrep += Text("Go get to it") + rank + "!";                break;
	case 12: sitrep += "The clock is ticking, so let's move it!";        break;
	case 13: sitrep += "Stay sharp out there!";                          break;
	case 14: sitrep += Text("Go get 'em") + rank + "!";                  break;
	case 15: sitrep += "Now get out of here and get to work!";           break;
	}
}

// +--------------------------------------------------------------------+

MissionElement*
CampaignSituationReport::FindEscort(MissionElement* player)
{
	MissionElement* escort = 0;

	if (!mission || !player) return escort;

	ListIter<MissionElement> iter = mission->GetElements();
	while (++iter) {
		MissionElement* elem = iter.value();

	}

	return escort;
}

// +--------------------------------------------------------------------+

Text
CampaignSituationReport::GetThreatInfo()
{
	Text threat_info;

	int   enemy_fighters  = 0;
	int   enemy_starships = 0;
	int   enemy_sites     = 0;

	if (mission && mission->GetPlayer()) {
		MissionElement* player = mission->GetPlayer();
		Text            rgn0   = player->Region();
		Text            rgn1;
		int             iff    = player->GetIFF();

		ListIter<Instruction> nav = player->NavList();
		while (++nav) {
			if (rgn0 != nav->RegionName())
			rgn1 = nav->RegionName();
		}

		ListIter<MissionElement> elem = mission->GetElements();
		while (++elem) {
			if (elem->GetIFF() > 0 && elem->GetIFF() != iff && elem->IntelLevel() > Intel::SECRET) {
				if (elem->IsGroundUnit()) {
					if (!elem->GetDesign() || 
							elem->GetDesign()->type != Ship::SAM)
					continue;

					if (elem->Region() != rgn0 && 
							elem->Region() != rgn1)
					continue;
				}

				int mission_role = elem->MissionRole();

				if (mission_role == Mission::STRIKE    ||
						mission_role == Mission::INTEL     ||
						mission_role == Mission::CARGO     ||
						mission_role == Mission::TRANSPORT)
				continue;

				if (elem->GetDesign()->type >= Ship::MINE && elem->GetDesign()->type <= Ship::SWACS)
				enemy_sites += elem->Count();

				else if (elem->IsDropship())
				enemy_fighters += elem->Count();

				else if (elem->IsStarship())
				enemy_starships += elem->Count();

				else if (elem->IsGroundUnit())
				enemy_sites += elem->Count();
			}
		}
	}

	if (enemy_starships > 0) {
		threat_info = "We have reports of several enemy starships in the vicinity.";

		if (enemy_fighters > 0) {
			threat_info += "  Also be advised that enemy fighters may be operating nearby.";
		}

		else if (enemy_sites > 0) {
			threat_info += "  And be on the lookout for mines and defense satellites.";
		}
	}

	else if (enemy_fighters > 0) {
		threat_info = "We have reports of several enemy fighters in your operating area.";
	}

	else if (enemy_sites > 0) {
		if (mission->Type() >= Mission::AIR_PATROL && mission->Type() <= Mission::STRIKE)
		threat_info = "Remember to check air-to-ground sensors for SAM and AAA sites.";
		else
		threat_info = "Be on the lookout for mines and defense satellites.";
	}
	else {
		threat_info = "We have no reliable information on threats in your operating area.";
	}

	return threat_info;
}
