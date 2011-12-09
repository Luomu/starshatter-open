/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CampaignMissionFighter.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	CampaignMissionFighter generates missions and mission
	info for the player's FIGHTER SQUADRON as part of a
	dynamic campaign.
*/

#include "MemDebug.h"
#include "CampaignMissionFighter.h"
#include "CampaignMissionRequest.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Callsign.h"
#include "Galaxy.h"
#include "Mission.h"
#include "MissionTemplate.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Starshatter.h"
#include "StarSystem.h"
#include "Player.h"

#include "Random.h"

static int pkg_id = 1000;
extern int dump_missions;

// +--------------------------------------------------------------------+

CampaignMissionFighter::CampaignMissionFighter(Campaign* c)
: campaign(c), squadron(0), mission(0), player_elem(0),
strike_group(0), strike_target(0), prime_target(0),
carrier_elem(0), ward(0), escort(0), airborne(false), airbase(false),
ownside(0), enemy(-1), mission_type(0), mission_info(0)
{
	if (!campaign || !campaign->GetPlayerGroup()) {
		::Print("ERROR - CMF campaign=0x%08x player_group=0x%08x\n",
		campaign, (DWORD) campaign?campaign->GetPlayerGroup():0);
		return;
	}

	CombatGroup* player_group = campaign->GetPlayerGroup();

	switch (player_group->Type()) {
	case CombatGroup::WING: {
			CombatGroup*          wing = player_group;
			ListIter<CombatGroup> iter = wing->GetComponents();

			while (++iter) {
				if (iter->Type() == CombatGroup::FIGHTER_SQUADRON) {
					squadron = iter.value();
				}
			}
		}
		break;

	case CombatGroup::FIGHTER_SQUADRON:
	case CombatGroup::INTERCEPT_SQUADRON:
	case CombatGroup::ATTACK_SQUADRON:
		squadron = player_group;
		break;

	default:
		::Print("ERROR - CMF invalid player group: %s IFF %d\n",
		player_group->GetDescription(),
		player_group->GetIFF());
		break;
	}

	if (squadron) {
		CombatGroup* carrier = squadron->FindCarrier();

		if (carrier && carrier->Type() == CombatGroup::STARBASE) {
			airbase = true;
		}
	}
}

CampaignMissionFighter::~CampaignMissionFighter() {}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateMission(CampaignMissionRequest* req)
{
	if (!campaign || !squadron || !req)
	return;

	::Print("\n-----------------------------------------------\n");
	if (req->Script().length())
	::Print("CMF CreateMission() request: %s '%s'\n", 
	Mission::RoleName(req->Type()),
	(const char*) req->Script());

	else
	::Print("CMF CreateMission() request: %s %s\n", 
	Mission::RoleName(req->Type()),
	req->GetObjective() ? req->GetObjective()->Name() : "(no target)");

	request       = req;
	mission_info  = 0;

	if (request->GetPrimaryGroup()) {
		switch (request->GetPrimaryGroup()->Type()) {
		case CombatGroup::FIGHTER_SQUADRON:
		case CombatGroup::INTERCEPT_SQUADRON:
		case CombatGroup::ATTACK_SQUADRON:
			squadron = request->GetPrimaryGroup();
			break;
		}
	}

	ownside = squadron->GetIFF();

	for (int i = 0; i < campaign->GetCombatants().size(); i++) {
		int iff = campaign->GetCombatants().at(i)->GetIFF();
		if (iff > 0 && iff != ownside) {
			enemy = iff;
			break;
		}
	}

	static int id_key = 1;
	GenerateMission(id_key++);
	DefineMissionObjectives();

	MissionInfo* info = DescribeMission();

	if (info) {
		campaign->GetMissionList().append(info);

		::Print("CMF Created %03d '%s' %s\n\n",
		info->id,
		(const char*) info->name,
		Mission::RoleName(mission->Type()));

		if (dump_missions) {
			Text script = mission->Serialize();
			char fname[32];

			sprintf_s(fname, "msn%03d.def", info->id);
			FILE* f;
			fopen_s(&f, fname, "w");
			if (f) {
				fprintf(f, "%s\n", script.data());
				fclose(f);
			}
		}
	}
	else {
		::Print("CMF failed to create mission.\n");
	}
}

// +--------------------------------------------------------------------+

Mission*
CampaignMissionFighter::GenerateMission(int id)
{
	bool found = false;

	SelectType();

	if (request && request->Script().length()) {
		MissionTemplate* mt = new(__FILE__,__LINE__) MissionTemplate(id, request->Script(), campaign->Path());
		if (mt)
		mt->SetPlayerSquadron(squadron);
		mission = mt;
		found   = true;
	}

	else {
		mission_info = campaign->FindMissionTemplate(mission_type, squadron);
		found        = mission_info != 0;

		if (found) {
			MissionTemplate* mt = new(__FILE__,__LINE__) MissionTemplate(id, mission_info->script, campaign->Path());
			if (mt)
			mt->SetPlayerSquadron(squadron);
			mission = mt;
		}
		else {
			mission = new(__FILE__,__LINE__) Mission(id);
			if (mission)
			mission->SetType(mission_type);
		}
	}

	if (!mission) {
		Exit();
		return 0;
	}

	char name[64];
	sprintf_s(name, "Fighter Mission %d", id);

	mission->SetName(name);
	mission->SetTeam(squadron->GetIFF());
	mission->SetStart(request->StartTime());

	SelectRegion();
	GenerateStandardElements();
	CreatePatrols();

	if (!found) {
		GenerateMissionElements();
		mission->SetOK(true);
		mission->Validate();
	}

	else {
		mission->Load();

		if (mission->IsOK()) {
			player_elem  = mission->GetPlayer();
			prime_target = mission->GetTarget();
			ward         = mission->GetWard();

			Player* p = Player::GetCurrentPlayer();

			if (player_elem && p)
			player_elem->SetAlert(!p->FlyingStart());
		}

		// if there was a problem, scrap the mission
		// and start over:
		else {
			delete mission;

			mission = new(__FILE__,__LINE__) Mission(id);
			mission->SetType(mission_type);
			mission->SetName(name);
			mission->SetTeam(squadron->GetIFF());
			mission->SetStart(request->StartTime());

			SelectRegion();
			GenerateStandardElements();
			GenerateMissionElements();

			mission->SetOK(true);
			mission->Validate();
		}
	}

	return mission;
}

// +--------------------------------------------------------------------+

bool
CampaignMissionFighter::IsGroundObjective(CombatGroup* obj)
{
	bool ground = false;

	if (obj) {
		CombatGroup* pgroup = campaign->GetPlayerGroup();

		if (pgroup) {
			CombatZone* zone = pgroup->GetAssignedZone();

			if (zone) {
				StarSystem* system = campaign->GetSystem(zone->System());

				if (system) {
					OrbitalRegion* region = system->FindRegion(obj->GetRegion());

					if (region && region->Type() == Orbital::TERRAIN) {
						ground = true;
					}
				}
			}
		}
	}

	return ground;
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::SelectType()
{
	int type = Mission::PATROL;

	if (request) {
		type = request->Type();
		if (type == Mission::STRIKE) {
			strike_group = request->GetPrimaryGroup();

			// verify that objective is a ground target:
			if (!IsGroundObjective(request->GetObjective())) {
				type = Mission::ASSAULT;
			}
		}

		else if (type == Mission::ESCORT_STRIKE) {
			strike_group = request->GetSecondaryGroup();
			if (!strike_group || strike_group->CalcValue() < 1) {
				type = Mission::SWEEP;
				strike_group = 0;
			}
		}
	}

	mission_type = type;
}

void
CampaignMissionFighter::SelectRegion()
{
	CombatZone* zone = squadron->GetAssignedZone();

	if (!zone)
	zone = squadron->GetCurrentZone();

	if (zone) {
		mission->SetStarSystem(campaign->GetSystem(zone->System()));
		mission->SetRegion(*zone->GetRegions().at(0));

		orb_region = mission->GetRegion();

		if (zone->GetRegions().size() > 1) {
			air_region = *zone->GetRegions().at(1);

			StarSystem*    system   = mission->GetStarSystem();
			OrbitalRegion* rgn      = 0;
			
			if (system)
			rgn = system->FindRegion(air_region);

			if (!rgn || rgn->Type() != Orbital::TERRAIN)
			air_region = "";
		}

		if (air_region.length() > 0) {
			if (request && IsGroundObjective(request->GetObjective())) {
				airborne = true;
			}

			else if (mission->Type() >= Mission::AIR_PATROL &&
					mission->Type() <= Mission::AIR_INTERCEPT) {
				airborne = true;
			}

			else if (mission->Type() == Mission::STRIKE ||
					mission->Type() == Mission::ESCORT_STRIKE) {
				if (strike_group) {
					strike_target = campaign->FindStrikeTarget(ownside, strike_group);

					if (strike_target && strike_target->GetRegion() == air_region)
					airborne = true;
				}
			}

			if (airbase) {
				mission->SetRegion(air_region);
			}
		}
	}

	else {
		::Print("WARNING: CMF - No zone for '%s'\n", squadron->Name().data());

		StarSystem* s = campaign->GetSystemList()[0];

		mission->SetStarSystem(s);
		mission->SetRegion(s->Regions()[0]->Name());
	}

	if (!airborne) {
		switch (mission->Type()) {
		case Mission::AIR_PATROL:     mission->SetType(Mission::PATROL);     break;
		case Mission::AIR_SWEEP:      mission->SetType(Mission::SWEEP);      break;
		case Mission::AIR_INTERCEPT:  mission->SetType(Mission::INTERCEPT);  break;
		default:                                                             break;
		}
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::GenerateStandardElements()
{
	ListIter<CombatZone> z_iter = campaign->GetZones();
	while (++z_iter) {
		CombatZone* z = z_iter.value();

		ListIter<ZoneForce>  iter = z->GetForces();
		while (++iter) {
			ZoneForce* force = iter.value();
			ListIter<CombatGroup> group = force->GetGroups();

			while (++group) {
				CombatGroup* g = group.value();

				switch (g->Type()) {
				case CombatGroup::INTERCEPT_SQUADRON:
				case CombatGroup::FIGHTER_SQUADRON:
				case CombatGroup::ATTACK_SQUADRON:
				case CombatGroup::LCA_SQUADRON:
					CreateSquadron(g);
					break;

				case CombatGroup::DESTROYER_SQUADRON:
				case CombatGroup::BATTLE_GROUP:
				case CombatGroup::CARRIER_GROUP:
					CreateElements(g);
					break;

				case CombatGroup::MINEFIELD:
				case CombatGroup::BATTERY:
				case CombatGroup::MISSILE:
				case CombatGroup::STATION:
				case CombatGroup::STARBASE:
				case CombatGroup::SUPPORT:
				case CombatGroup::COURIER:
				case CombatGroup::MEDICAL:
				case CombatGroup::SUPPLY:
				case CombatGroup::REPAIR:
					CreateElements(g);
					break;

				case CombatGroup::CIVILIAN:
				case CombatGroup::WAR_PRODUCTION:
				case CombatGroup::FACTORY:
				case CombatGroup::REFINERY:
				case CombatGroup::RESOURCE:
				case CombatGroup::INFRASTRUCTURE:
				case CombatGroup::TRANSPORT:
				case CombatGroup::NETWORK:
				case CombatGroup::HABITAT:
				case CombatGroup::STORAGE:
				case CombatGroup::NON_COM:
					CreateElements(g);
					break;
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::GenerateMissionElements()
{
	CreateWards();
	CreatePlayer(squadron);
	CreateTargets();
	CreateEscorts();

	if (player_elem) {
		Instruction* obj = new(__FILE__,__LINE__) Instruction(mission->GetRegion(),
		Point(0,0,0),
		Instruction::RTB);

		if (obj)
		player_elem->AddObjective(obj);
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateElements(CombatGroup* g)
{
	MissionElement*   elem  = 0;
	List<CombatUnit>& units = g->GetUnits();

	CombatUnit* cmdr = 0;

	for (int i = 0; i < units.size(); i++) {
		elem = CreateSingleElement(g, units[i]);

		if (elem) {
			if (!cmdr) {
				cmdr = units[i];
			}
			else {
				elem->SetCommander(cmdr->Name());

				if (g->Type() == CombatGroup::CARRIER_GROUP &&
						elem->MissionRole() == Mission::ESCORT) {
					Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, cmdr->Name());
					if (obj)
					elem->AddObjective(obj);
				}
			}

			mission->AddElement(elem);
		}
	}
}

MissionElement*
CampaignMissionFighter::CreateSingleElement(CombatGroup* g, CombatUnit* u)
{
	if (!g || g->IsReserve())     return 0;
	if (!u || u->LiveCount() < 1) return 0;

	// make sure this unit is actually in the right star system:
	Galaxy* galaxy = Galaxy::GetInstance();
	if (galaxy) {
		if (galaxy->FindSystemByRegion(u->GetRegion()) !=
				galaxy->FindSystemByRegion(squadron->GetRegion())) {
			return 0;
		}
	}

	// make sure this unit isn't already in the mission:
	ListIter<MissionElement> e_iter = mission->GetElements();
	while (++e_iter) {
		MissionElement* elem = e_iter.value();

		if (elem->GetCombatUnit() == u)
		return 0;
	}

	MissionElement* elem = new(__FILE__,__LINE__) MissionElement;
	if (!elem) {
		Exit();
		return 0;
	}

	if (u->Name().length())
	elem->SetName(u->Name());
	else
	elem->SetName(u->DesignName());

	elem->SetElementID(pkg_id++);

	elem->SetDesign(u->GetDesign());
	elem->SetCount(u->LiveCount());
	elem->SetIFF(u->GetIFF());
	elem->SetIntelLevel(g->IntelLevel());
	elem->SetRegion(u->GetRegion());
	elem->SetHeading(u->GetHeading());

	int   unit_index = g->GetUnits().index(u);
	Point base_loc   = u->Location();
	bool  exact      = u->IsStatic(); // exact unit-level placement

	if (base_loc.length() < 1) {
		base_loc = g->Location();
		exact = false;
	}

	if (unit_index < 0 || unit_index > 0 && !exact) {
		Point loc = RandomDirection();

		if (!u->IsStatic()) {
			while (fabs(loc.y) > fabs(loc.x))
			loc = RandomDirection();

			loc *= 10e3 + 9e3 * unit_index;
		}
		else {
			loc *= 2e3 + 2e3 * unit_index;
		}

		elem->SetLocation(base_loc + loc);
	}
	else {
		elem->SetLocation(base_loc);
	}

	if (g->Type() == CombatGroup::CARRIER_GROUP) {
		if (u->Type() == Ship::CARRIER) {
			elem->SetMissionRole(Mission::FLIGHT_OPS);

			if (squadron && elem->GetCombatGroup() == squadron->FindCarrier())
			carrier_elem = elem;

			else if (!carrier_elem && u->GetIFF() == squadron->GetIFF())
			carrier_elem = elem;
		}
		else {
			elem->SetMissionRole(Mission::ESCORT);
		}
	}
	else if (u->Type() == Ship::STATION || 
			u->Type() == Ship::STARBASE) {
		elem->SetMissionRole(Mission::FLIGHT_OPS);

		if (squadron && elem->GetCombatGroup() == squadron->FindCarrier()) {
			carrier_elem = elem;

			if (u->Type() == Ship::STARBASE)
			airbase = true;
		}
	}
	else if (u->Type() == Ship::FARCASTER) {
		elem->SetMissionRole(Mission::OTHER);

		// link farcaster to other terminus:
		Text name = u->Name();
		int  dash = -1;

		for (int i = 0; i < (int) name.length(); i++)
		if (name[i] == '-')
		dash = i;

		Text src = name.substring(0, dash);
		Text dst = name.substring(dash+1, name.length() - (dash+1));

		Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::VECTOR, dst + "-" + src);
		elem->AddObjective(obj);
	}
	else if ((u->Type() & Ship::STARSHIPS) != 0) {
		elem->SetMissionRole(Mission::FLEET);
	}

	elem->SetCombatGroup(g);
	elem->SetCombatUnit(u);

	return elem;
}

CombatUnit*
CampaignMissionFighter::FindCarrier(CombatGroup* g)
{
	CombatGroup* carrier = g->FindCarrier();

	if (carrier && carrier->GetUnits().size()) {
		MissionElement* carrier_elem = mission->FindElement(carrier->Name());

		if (carrier_elem)
		return carrier->GetUnits().at(0);
	}

	return 0;
}

void
CampaignMissionFighter::CreateSquadron(CombatGroup* g)
{
	if (!g || g->IsReserve()) return;

	CombatUnit* fighter = g->GetUnits().at(0);
	CombatUnit* carrier = FindCarrier(g);

	if (!fighter || !carrier) return;

	int live_count  = fighter->LiveCount();
	int maint_count = (live_count > 4) ? live_count / 2 : 0;

	MissionElement* elem = new(__FILE__,__LINE__) MissionElement;

	if (!elem) {
		Exit();
		return;
	}

	elem->SetName(g->Name());
	elem->SetElementID(pkg_id++);

	elem->SetDesign(fighter->GetDesign());
	elem->SetCount(fighter->Count());
	elem->SetDeadCount(fighter->DeadCount());
	elem->SetMaintCount(maint_count);
	elem->SetIFF(fighter->GetIFF());
	elem->SetIntelLevel(g->IntelLevel());
	elem->SetRegion(fighter->GetRegion());

	elem->SetCarrier(carrier->Name());
	elem->SetCommander(carrier->Name());
	elem->SetLocation(carrier->Location() + RandomPoint());

	elem->SetCombatGroup(g);
	elem->SetCombatUnit(fighter);

	mission->AddElement(elem);
}

void
CampaignMissionFighter::CreatePlayer(CombatGroup* g)
{
	int pkg_size = 2;

	if (mission->Type() == Mission::STRIKE || mission->Type() == Mission::ASSAULT) {
		if (request && request->GetObjective()) {
			int tgt_type = request->GetObjective()->Type();

			if (tgt_type >= CombatGroup::FLEET && tgt_type <= CombatGroup::CARRIER_GROUP)
			pkg_size = 4;

			if (tgt_type == CombatGroup::STATION || tgt_type == CombatGroup::STARBASE)
			pkg_size = 4;
		}
	}

	MissionElement* elem = CreateFighterPackage(g, pkg_size, mission->Type());

	if (elem) {
		Player* p = Player::GetCurrentPlayer();
		elem->SetAlert(p ? !p->FlyingStart() : true);
		elem->SetPlayer(1);

		if (ward) {
			Point approach = elem->Location() - ward->Location();
			approach.Normalize();

			Point pickup = ward->Location() + approach * 50e3;
			double delta = (pickup - elem->Location()).length();

			if (delta > 30e3) {
				Instruction* n = new(__FILE__,__LINE__) Instruction(elem->Region(), pickup, Instruction::ESCORT);
				n->SetTarget(ward->Name());
				n->SetSpeed(750);
				elem->AddNavPoint(n);
			}

			Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, ward->Name());

			switch (mission->Type()) {
			case Mission::ESCORT_FREIGHT:
				obj->SetTargetDesc(Text("the star freighter ") + ward->Name());
				break;

			case Mission::ESCORT_SHUTTLE:
				obj->SetTargetDesc(Text("the shuttle ") + ward->Name());
				break;

			case Mission::ESCORT_STRIKE:
				obj->SetTargetDesc(Text("the ") + ward->Name() + Text(" strike package"));
				break;

			case Mission::DEFEND:
				obj->SetTargetDesc(Text("the ") + ward->Name());
				break;

			default:
				if (ward->GetCombatGroup()) {
					obj->SetTargetDesc(Text("the ") + ward->GetCombatGroup()->GetDescription());
				}
				else {
					obj->SetTargetDesc(Text("the ") + ward->Name());
				}
				break;
			}

			elem->AddObjective(obj);
		}

		mission->AddElement(elem);

		player_elem = elem;
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreatePatrols()
{
	List<MissionElement> patrols;

	ListIter<MissionElement>   iter = mission->GetElements();
	while (++iter) {
		MissionElement*   squad_elem  = iter.value();
		CombatGroup*      squadron    = squad_elem->GetCombatGroup();
		CombatUnit*       unit        = squad_elem->GetCombatUnit();

		if (!squad_elem->IsSquadron() || !squadron || !unit || unit->LiveCount() < 4)
		continue;

		if (squadron->Type() == CombatGroup::INTERCEPT_SQUADRON ||
				squadron->Type() == CombatGroup::FIGHTER_SQUADRON) {

			StarSystem*    system      = mission->GetStarSystem();
			CombatGroup*   base        = squadron->FindCarrier();
			OrbitalRegion* region      = system->FindRegion(base->GetRegion());
			int            patrol_type = Mission::PATROL;
			Point          base_loc;

			if (!base || !region)
			continue;

			if (region->Type() == Orbital::TERRAIN) {
				patrol_type = Mission::AIR_PATROL;

				if (RandomChance(2,3))
				continue;
			}

			base_loc = base->Location() + RandomPoint() * 1.5;

			if (region->Type() == Orbital::TERRAIN)
			base_loc += Point(0, 0, 14.0e3);

			MissionElement* elem = CreateFighterPackage(squadron, 2, patrol_type);
			if (elem) {
				elem->SetIntelLevel(Intel::KNOWN);
				elem->SetRegion(base->GetRegion());
				elem->SetLocation(base_loc);
				patrols.append(elem);
			}
		}
	}

	iter.attach(patrols);
	while (++iter)
	mission->AddElement(iter.value());
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateWards()
{
	switch (mission->Type()) {
	case Mission::ESCORT_FREIGHT:    CreateWardFreight(); break;
	case Mission::ESCORT_SHUTTLE:    CreateWardShuttle(); break;
	case Mission::ESCORT_STRIKE:     CreateWardStrike();  break;
	default:                                              break;
	}
}

void
CampaignMissionFighter::CreateWardFreight()
{
	if (!mission || !mission->GetStarSystem()) return;

	CombatUnit*    carrier  = FindCarrier(squadron);
	CombatGroup*   freight  = 0;

	if (request)
	freight = request->GetObjective();

	if (!freight)
	freight = campaign->FindGroup(ownside, CombatGroup::FREIGHT);

	if (!freight || freight->CalcValue() < 1)  return;

	CombatUnit* unit = freight->GetNextUnit();
	if (!unit) return;

	MissionElement* elem = CreateSingleElement(freight, unit);
	if (!elem) return;

	elem->SetMissionRole(Mission::CARGO);
	elem->SetIntelLevel(Intel::KNOWN);
	elem->SetRegion(squadron->GetRegion());

	if (carrier)
	elem->SetLocation(carrier->Location() + RandomPoint() * 2);

	ward = elem;
	mission->AddElement(elem);


	StarSystem*    system   = mission->GetStarSystem();
	OrbitalRegion* rgn1     = system->FindRegion(elem->Region());
	Point          delta    = rgn1->Location() - rgn1->Primary()->Location();
	Point          npt_loc  = elem->Location();
	Instruction*   n        = 0;

	delta.Normalize();
	delta *= 200.0e3;

	npt_loc += delta;

	n = new(__FILE__,__LINE__) Instruction(elem->Region(),
	npt_loc,
	Instruction::VECTOR);

	if (n) {
		n->SetSpeed(500);
		elem->AddNavPoint(n);
	}

	Text rgn2 = elem->Region();
	List<CombatZone>& zones = campaign->GetZones();
	if (zones[zones.size()-1]->HasRegion(rgn2))
	rgn2 = *zones[0]->GetRegions()[0];
	else
	rgn2 = *zones[zones.size()-1]->GetRegions()[0];

	n = new(__FILE__,__LINE__) Instruction(rgn2,
	Point(0, 0, 0),
	Instruction::VECTOR);

	if (n) {
		n->SetSpeed(750);
		elem->AddNavPoint(n);
	}
}

void
CampaignMissionFighter::CreateWardShuttle()
{
	if (!mission || !mission->GetStarSystem()) return;

	CombatUnit*    carrier  = FindCarrier(squadron);
	CombatGroup*   shuttle  = campaign->FindGroup(ownside, CombatGroup::LCA_SQUADRON);

	if (!shuttle || shuttle->CalcValue() < 1)  return;

	List<CombatUnit>& units = shuttle->GetUnits();

	MissionElement* elem = CreateFighterPackage(shuttle, 1, Mission::CARGO);
	if (!elem) return;

	elem->SetIntelLevel(Intel::KNOWN);
	elem->SetRegion(orb_region);
	elem->Loadouts().destroy();

	if (carrier)
	elem->SetLocation(carrier->Location() + RandomPoint() * 2);

	ward = elem;
	mission->AddElement(elem);

	// if there is terrain nearby, then have the shuttle fly down to it:
	if (air_region.length() > 0) {
		StarSystem*    system   = mission->GetStarSystem();
		OrbitalRegion* rgn1     = system->FindRegion(elem->Region());
		Point          delta    = rgn1->Location() - rgn1->Primary()->Location();
		Point          npt_loc  = elem->Location();
		Instruction*   n        = 0;

		delta.Normalize();
		delta *= -200.0e3;

		npt_loc += delta;

		n = new(__FILE__,__LINE__) Instruction(elem->Region(),
		npt_loc,
		Instruction::VECTOR);

		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}

		n = new(__FILE__,__LINE__) Instruction(air_region,
		Point(0, 0, 10.0e3),
		Instruction::VECTOR);

		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}
	}

	// otherwise, escort the shuttle in for a landing on the carrier:
	else if (carrier) {
		Point          src = carrier->Location() + RandomDirection() * 150e3;
		Point          dst = carrier->Location() + RandomDirection() * 25e3;
		Instruction*   n   = 0;

		elem->SetLocation(src);

		n = new(__FILE__,__LINE__) Instruction(elem->Region(), dst, Instruction::DOCK);
		if (n) {
			n->SetTarget(carrier->Name());
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}
	}
}

void
CampaignMissionFighter::CreateWardStrike()
{
	if (!mission || !mission->GetStarSystem()) return;

	CombatUnit*    carrier  = FindCarrier(squadron);
	CombatGroup*   strike   = strike_group;

	if (!strike || strike->CalcValue() < 1)  return;

	List<CombatUnit>& units = strike->GetUnits();

	int type = Mission::ASSAULT;

	if (airborne)
	type = Mission::STRIKE;

	MissionElement* elem = CreateFighterPackage(strike, 2, type);
	if (!elem) return;

	if (strike->GetParent() == squadron->GetParent()) {
		Player* p = Player::GetCurrentPlayer();
		elem->SetAlert(p ? !p->FlyingStart() : true);
	}

	elem->SetIntelLevel(Intel::KNOWN);
	elem->SetRegion(squadron->GetRegion());

	if (strike_target) {
		Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ASSAULT, strike_target->Name());

		if (obj) {
			if (airborne)
			obj->SetAction(Instruction::STRIKE);

			elem->AddObjective(obj);
		}
	}

	ward = elem;
	mission->AddElement(elem);


	StarSystem*    system   = mission->GetStarSystem();
	OrbitalRegion* rgn1     = system->FindRegion(elem->Region());
	Point          delta    = rgn1->Location() - rgn1->Primary()->Location();
	Point          npt_loc  = elem->Location();
	Instruction*   n        = 0;

	if (airborne) {
		delta.Normalize();
		delta *= -30.0e3;
		npt_loc += delta;

		n = new(__FILE__,__LINE__) Instruction(elem->Region(),
		npt_loc,
		Instruction::VECTOR);
		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}

		npt_loc = Point(0, 0, 10.0e3);

		n = new(__FILE__,__LINE__) Instruction(air_region,
		npt_loc,
		Instruction::VECTOR);
		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}
	}

	// IP:
	if (strike_target) {
		delta = strike_target->Location() - npt_loc;
		delta.Normalize();
		delta *= 15.0e3;

		npt_loc = strike_target->Location() + delta + Point(0, 0, 8.0e3);

		n = new(__FILE__,__LINE__) Instruction(strike_target->GetRegion(),
		npt_loc,
		Instruction::STRIKE);
		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}
	}

	if (airborne) {
		n = new(__FILE__,__LINE__) Instruction(air_region,
		Point(0, 0, 30.0e3),
		Instruction::VECTOR);
		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}
	}

	if (carrier) {
		n = new(__FILE__,__LINE__) Instruction(elem->Region(),
		carrier->Location() - Point(0, -20.0e3, 0),
		Instruction::VECTOR);
		if (n) {
			n->SetSpeed(500);
			elem->AddNavPoint(n);
		}
	}

	// find the strike target element:
	if (strike_target) {
		prime_target = mission->FindElement(strike_target->Name());
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateEscorts()
{
	bool escort_needed = false;

	if (mission->Type() == Mission::STRIKE || mission->Type() == Mission::ASSAULT) {
		if (request && request->GetObjective()) {
			int tgt_type = request->GetObjective()->Type();

			if (tgt_type == CombatGroup::CARRIER_GROUP ||
					tgt_type == CombatGroup::STATION       ||
					tgt_type == CombatGroup::STARBASE)

			escort_needed = true;
		}
	}

	if (player_elem && escort_needed) {
		CombatGroup* s = FindSquadron(ownside, CombatGroup::INTERCEPT_SQUADRON);

		if (s && s->IsAssignable()) {
			MissionElement* elem = CreateFighterPackage(s, 2, Mission::ESCORT_STRIKE);

			if (elem) {
				Point offset = Point(2.0e3, 2.0e3, 1.0e3);

				ListIter<Instruction> npt_iter = player_elem->NavList();
				while (++npt_iter) {
					Instruction* npt = npt_iter.value();
					Instruction* n   = new(__FILE__,__LINE__) Instruction(npt->RegionName(),
					npt->Location() + offset,
					Instruction::ESCORT);
					if (n) {
						n->SetSpeed(npt->Speed());
						elem->AddNavPoint(n);
					}
				}

				mission->AddElement(elem);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateTargets()
{
	switch (mission->Type()) {
	default:
	case Mission::DEFEND:
	case Mission::PATROL:         CreateTargetsPatrol();        break;
	case Mission::SWEEP:          CreateTargetsSweep();         break;
	case Mission::INTERCEPT:      CreateTargetsIntercept();     break;
	case Mission::ESCORT_FREIGHT: CreateTargetsFreightEscort(); break;

	case Mission::ESCORT:
	case Mission::ESCORT_SHUTTLE: CreateTargetsShuttleEscort(); break;
	case Mission::ESCORT_STRIKE:  CreateTargetsStrikeEscort();  break;
	case Mission::STRIKE:         CreateTargetsStrike();        break;
	case Mission::ASSAULT:        CreateTargetsAssault();       break;
	}
}

void
CampaignMissionFighter::CreateTargetsPatrol()
{
	if (!squadron || !player_elem) return;

	Text   region     = squadron->GetRegion();
	Point  base_loc   = player_elem->Location();
	Point  patrol_loc;

	if (airborne)
	base_loc = RandomPoint() * 2 + Point(0, 0, 12.0e3);

	else if (carrier_elem)
	base_loc = carrier_elem->Location();

	if (airborne) {
		if (!airbase)
		PlanetaryInsertion(player_elem);
		region     = air_region;
		patrol_loc = base_loc + 
		RandomDirection() * Random( 60e3, 100e3);
	}
	else {
		patrol_loc = base_loc +
		RandomDirection() * Random(110e3, 160e3);
	}

	Instruction* n = new(__FILE__,__LINE__) Instruction(region,
	patrol_loc,
	Instruction::PATROL);
	if (n)
	player_elem->AddNavPoint(n);

	int ntargets = (int) Random(2.0,5.1);

	while (ntargets > 0) {
		int t = CreateRandomTarget(region, patrol_loc);
		ntargets -= t;
		if (t < 1) break;
	}

	if (airborne && !airbase) {
		OrbitalInsertion(player_elem);
	}

	Instruction* obj = new(__FILE__,__LINE__) Instruction(*n);
	obj->SetTargetDesc("inbound enemy units");
	player_elem->AddObjective(obj);

	if (carrier_elem && !airborne) {
		obj = new(__FILE__,__LINE__) Instruction(Instruction::DEFEND, carrier_elem->Name());
		if (obj) {
			obj->SetTargetDesc(Text("the ") + carrier_elem->Name() + " battle group");
			player_elem->AddObjective(obj);
		}
	}
}

void
CampaignMissionFighter::CreateTargetsSweep()
{
	if (!squadron || !player_elem) return;

	double traverse      = PI;
	double a             = Random(-PI/2, PI/2);
	Point  base_loc      = player_elem->Location();
	Point  sweep_loc     = base_loc;
	Text   region        = player_elem->Region();
	Instruction* n          = 0;

	if (carrier_elem)
	base_loc = carrier_elem->Location();

	if (airborne) {
		PlanetaryInsertion(player_elem);
		region = air_region;
		sweep_loc = RandomPoint() + Point(0, 0, 10.0e3);   // keep it airborne!
	}

	sweep_loc += Point(sin(a), -cos(a), 0) * 100.0e3;

	n = new(__FILE__,__LINE__) Instruction(region,
	sweep_loc,
	Instruction::VECTOR);
	if (n) {
		n->SetSpeed(750);
		player_elem->AddNavPoint(n);
	}

	int index    = 0;
	int ntargets = 6;

	while (traverse > 0) {
		double a1 = Random(PI/4, PI/2);
		traverse  -= a1;
		a         += a1;

		sweep_loc += Point(sin(a), -cos(a), 0) * 80.0e3;

		n = new(__FILE__,__LINE__) Instruction(region,
		sweep_loc,
		Instruction::SWEEP);
		if (n) {
			n->SetSpeed(750);
			n->SetFormation(Instruction::SPREAD);
			player_elem->AddNavPoint(n);
		}

		if (ntargets && RandomChance()) {
			ntargets -= CreateRandomTarget(region, sweep_loc);
		}

		index++;
	}

	if (ntargets > 0)
	CreateRandomTarget(region, sweep_loc);

	if (airborne && !airbase) {
		OrbitalInsertion(player_elem);
		region = player_elem->Region();
	}

	sweep_loc = base_loc;
	sweep_loc.y += 30.0e3;

	n = new(__FILE__,__LINE__) Instruction(region,
	sweep_loc,
	Instruction::VECTOR);
	if (n) {
		n->SetSpeed(750);
		player_elem->AddNavPoint(n);
	}

	Instruction* obj = new(__FILE__,__LINE__) Instruction(region,
	sweep_loc,
	Instruction::SWEEP);
	if (obj) {
		obj->SetTargetDesc("enemy patrols");
		player_elem->AddObjective(obj);
	}

	if (carrier_elem && !airborne) {
		obj = new(__FILE__,__LINE__) Instruction(Instruction::DEFEND, carrier_elem->Name());
		if (obj) {
			obj->SetTargetDesc(Text("the ") + carrier_elem->Name() + " battle group");
			player_elem->AddObjective(obj);
		}
	}
}

void
CampaignMissionFighter::CreateTargetsIntercept()
{
	if (!squadron || !player_elem) return;

	CombatUnit*    carrier  = FindCarrier(squadron);
	CombatGroup*   s        = FindSquadron(enemy, CombatGroup::ATTACK_SQUADRON);
	CombatGroup*   s2       = FindSquadron(enemy, CombatGroup::FIGHTER_SQUADRON);

	if (!s || !s2) return;

	int   ninbound = 2 + (int) (RandomIndex() < 5);
	bool  second   = ninbound > 2;
	Text  attacker;

	while (ninbound--) {
		MissionElement* elem = CreateFighterPackage(s, 4, Mission::ASSAULT);
		if (elem) {
			elem->SetIntelLevel(Intel::KNOWN);
			elem->Loadouts().destroy();
			elem->Loadouts().append(new(__FILE__,__LINE__) MissionLoad(-1, "Hvy Ship Strike"));

			if (carrier) {
				Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ASSAULT, carrier->Name());
				if (obj) {
					elem->AddObjective(obj);
					elem->SetLocation(carrier->Location() + RandomPoint() * 6);
				}
			}
			else {
				elem->SetLocation(squadron->Location() + RandomPoint() * 5);
			}

			mission->AddElement(elem);

			attacker = elem->Name();

			if (!prime_target) {
				prime_target = elem;
				Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::INTERCEPT, attacker);
				if (obj) {
					obj->SetTargetDesc(Text("inbound strike package '") + elem->Name() + "'");
					player_elem->AddObjective(obj);
				}
			}

			MissionElement* e2 = CreateFighterPackage(s2, 2, Mission::ESCORT);
			if (e2) {
				e2->SetIntelLevel(Intel::KNOWN);
				e2->SetLocation(elem->Location() + RandomPoint() * 0.25);

				Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, elem->Name());
				if (obj)
				e2->AddObjective(obj);
				mission->AddElement(e2);
			}
		}
	}

	if (second) {
		// second friendly fighter package
		CombatGroup* s = FindSquadron(ownside, CombatGroup::FIGHTER_SQUADRON);

		if (s) {
			MissionElement* elem = CreateFighterPackage(s, 2, Mission::INTERCEPT);
			if (elem) {
				Player* p = Player::GetCurrentPlayer();
				elem->SetAlert(p ? !p->FlyingStart() : true);

				Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::INTERCEPT, attacker);
				if (obj)
				elem->AddObjective(obj);
				mission->AddElement(elem);
			}
		}
	}

	if (carrier && !airborne) {
		Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::DEFEND, carrier->Name());
		if (obj) {
			obj->SetTargetDesc(Text("the ") + carrier->Name() + " battle group");
			player_elem->AddObjective(obj);
		}
	}
}

void
CampaignMissionFighter::CreateTargetsFreightEscort()
{
	if (!squadron || !player_elem) return;

	if (!ward) {
		CreateTargetsPatrol();
		return;
	}

	CombatUnit*    carrier  = FindCarrier(squadron);
	CombatGroup*   s        = FindSquadron(enemy, CombatGroup::ATTACK_SQUADRON);
	CombatGroup*   s2       = FindSquadron(enemy, CombatGroup::FIGHTER_SQUADRON);

	if (!s) s = s2;

	if (!s || !s2) return;

	MissionElement* elem = CreateFighterPackage(s, 2, Mission::ASSAULT);
	if (elem) {
		elem->SetIntelLevel(Intel::KNOWN);

		elem->SetLocation(ward->Location() + RandomPoint() * 5);

		Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ASSAULT, ward->Name());
		if (obj)
		elem->AddObjective(obj);
		mission->AddElement(elem);

		MissionElement* e2 = CreateFighterPackage(s2, 2, Mission::ESCORT);
		if (e2) {
			e2->SetIntelLevel(Intel::KNOWN);
			e2->SetLocation(elem->Location() + RandomPoint() * 0.25);

			Instruction* obj2 = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, elem->Name());
			if (obj2)
			e2->AddObjective(obj2);
			mission->AddElement(e2);
		}
	}

	Instruction* obj3 = new(__FILE__,__LINE__) Instruction(mission->GetRegion(),
	Point(0,0,0),
	Instruction::PATROL);

	if (obj3) {
		obj3->SetTargetDesc("enemy patrols");
		player_elem->AddObjective(obj3);
	}
}

void
CampaignMissionFighter::CreateTargetsShuttleEscort()
{
	CreateTargetsFreightEscort();
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateTargetsStrikeEscort()
{
	if (!squadron || !player_elem) return;

	if (ward) {
		Point offset = Point(2.0e3, 2.0e3, 1.0e3);

		ListIter<Instruction> npt_iter = ward->NavList();
		while (++npt_iter) {
			Instruction* npt = npt_iter.value();
			Instruction* n   = new(__FILE__,__LINE__) Instruction(npt->RegionName(),
			npt->Location() + offset,
			Instruction::ESCORT);
			if (n) {
				n->SetSpeed(npt->Speed());
				player_elem->AddNavPoint(n);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateTargetsStrike()
{
	if (!squadron || !player_elem) return;

	if (request && request->GetObjective())
	strike_target = request->GetObjective();

	if (strike_target && strike_group) {
		CreateElements(strike_target);

		ListIter<MissionElement> e_iter = mission->GetElements();
		while (++e_iter) {
			MissionElement* elem = e_iter.value();

			if (elem->GetCombatGroup() == strike_target) {
				prime_target = elem;
				Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::STRIKE, elem->Name());
				if (obj) {
					obj->SetTargetDesc(Text("preplanned target '") + elem->Name() + "'");
					player_elem->AddObjective(obj);
				}

				// create flight plan:
				RLoc           rloc;
				Point          loc   = Point(0, 0, 15e3);
				Instruction*   n     = 0;

				PlanetaryInsertion(player_elem);

				// target approach and strike:
				Point delta = prime_target->Location() - loc;

				if (delta.length() >= 100e3) {
					Point mid = loc + delta * 0.5;
					mid.z = 10.0e3;

					rloc.SetReferenceLoc(0);
					rloc.SetBaseLocation(mid);
					rloc.SetDistance(20e3);
					rloc.SetDistanceVar(5e3);
					rloc.SetAzimuth(90*DEGREES);
					rloc.SetAzimuthVar(25*DEGREES);

					n = new(__FILE__,__LINE__) Instruction(prime_target->Region(),
					Point(),
					Instruction::VECTOR);
					if (n) {
						n->SetSpeed(750);
						n->GetRLoc() = rloc;
						player_elem->AddNavPoint(n);
					}

					loc = mid;
				}

				delta = loc - prime_target->Location();
				delta.Normalize();
				delta *= 25.0e3;

				loc = prime_target->Location() + delta;
				loc.z = 8.0e3;

				n = new(__FILE__,__LINE__) Instruction(prime_target->Region(),
				loc,
				Instruction::STRIKE);
				if (n) {
					n->SetSpeed(500);
					player_elem->AddNavPoint(n);
				}

				// exeunt:
				rloc.SetReferenceLoc(0);
				rloc.SetBaseLocation(Point(0, 0, 30.0e3));
				rloc.SetDistance(50e3);
				rloc.SetDistanceVar(5e3);
				rloc.SetAzimuth(-90*DEGREES);
				rloc.SetAzimuthVar(25*DEGREES);

				n = new(__FILE__,__LINE__) Instruction(prime_target->Region(),
				Point(),
				Instruction::VECTOR);
				if (n) {
					n->SetSpeed(750);
					n->GetRLoc() = rloc;
					player_elem->AddNavPoint(n);
				}

				if (carrier_elem) {
					rloc.SetReferenceLoc(0);
					rloc.SetBaseLocation(carrier_elem->Location());
					rloc.SetDistance(60e3);
					rloc.SetDistanceVar(10e3);
					rloc.SetAzimuth(180*DEGREES);
					rloc.SetAzimuthVar(30*DEGREES);

					n = new(__FILE__,__LINE__) Instruction(carrier_elem->Region(),
					Point(), 
					Instruction::RTB);
					if (n) {
						n->SetSpeed(750);
						n->GetRLoc() = rloc;
						player_elem->AddNavPoint(n);
					}
				}

				break;
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::CreateTargetsAssault()
{
	if (!squadron || !player_elem) return;

	CombatGroup* assigned  = 0;

	if (request)
	assigned = request->GetObjective();

	if (assigned) {
		if (assigned->Type() > CombatGroup::WING && assigned->Type() < CombatGroup::FLEET) {
			mission->AddElement(CreateFighterPackage(assigned, 2, Mission::CARGO));
		}
		else {
			CreateElements(assigned);
		}

		// select the prime target element - choose the lowest ranking
		// unit of a DESRON, CBG, or CVBG:

		ListIter<MissionElement> e_iter = mission->GetElements();
		while (++e_iter) {
			MissionElement* elem = e_iter.value();

			if (elem->GetCombatGroup() == assigned) {
				if (!prime_target || assigned->Type() <= CombatGroup::CARRIER_GROUP) {
					prime_target = elem;
				}
			}
		}

		if (prime_target) {
			MissionElement* elem = prime_target;

			Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ASSAULT, elem->Name());
			if (obj) {
				obj->SetTargetDesc(Text("preplanned target '") + elem->Name() + "'");
				player_elem->AddObjective(obj);
			}

			// create flight plan:
			RLoc           rloc;
			Vec3           dummy(0,0,0);
			Instruction*   instr = 0;
			Point          loc   = player_elem->Location();
			Point          tgt   = elem->Location();
			Point          mid;

			CombatGroup* tgt_group = elem->GetCombatGroup();
			if (tgt_group && tgt_group->GetFirstUnit() && tgt_group->IsMovable()) {
				tgt = tgt_group->GetFirstUnit()->Location();
			}

			if (carrier_elem)
			loc = carrier_elem->Location();

			mid = loc + (elem->Location() - loc) * 0.5;

			rloc.SetReferenceLoc(0);
			rloc.SetBaseLocation(mid);
			rloc.SetDistance(40e3);
			rloc.SetDistanceVar(5e3);
			rloc.SetAzimuth(90*DEGREES);
			rloc.SetAzimuthVar(45*DEGREES);

			instr = new(__FILE__,__LINE__) Instruction(elem->Region(), dummy, Instruction::VECTOR);
			if (instr) {
				instr->SetSpeed(750);
				instr->GetRLoc() = rloc;

				player_elem->AddNavPoint(instr);

				if (RandomChance()) {
					CreateRandomTarget(elem->Region(), rloc.Location());
				}
			}

			rloc.SetReferenceLoc(0);
			rloc.SetBaseLocation(tgt);
			rloc.SetDistance(60e3);
			rloc.SetDistanceVar(5e3);
			rloc.SetAzimuth(120*DEGREES);
			rloc.SetAzimuthVar(15*DEGREES);

			instr = new(__FILE__,__LINE__) Instruction(elem->Region(), dummy, Instruction::ASSAULT);
			if (instr) {
				instr->SetSpeed(750);
				instr->GetRLoc() = rloc;
				instr->SetTarget(elem->Name());

				player_elem->AddNavPoint(instr);
			}

			if (carrier_elem) {
				rloc.SetReferenceLoc(0);
				rloc.SetBaseLocation(loc);
				rloc.SetDistance(30e3);
				rloc.SetDistanceVar(0);
				rloc.SetAzimuth(180*DEGREES);
				rloc.SetAzimuthVar(60*DEGREES);

				instr = new(__FILE__,__LINE__) Instruction(carrier_elem->Region(), dummy, Instruction::RTB);
				if (instr) {
					instr->SetSpeed(500);
					instr->GetRLoc() = rloc;

					player_elem->AddNavPoint(instr);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

int
CampaignMissionFighter::CreateRandomTarget(const char* rgn, Point base_loc)
{
	if (!mission) return 0;

	int  ntargets = 0;
	int  ttype    = RandomIndex();
	bool oca      = (mission->Type() == Mission::SWEEP);

	if (ttype < 8) {
		CombatGroup* s = 0;
		
		if (ttype < 4)
		s = FindSquadron(enemy, CombatGroup::INTERCEPT_SQUADRON);
		else
		s = FindSquadron(enemy, CombatGroup::FIGHTER_SQUADRON);
		
		if (s) {
			MissionElement* elem = CreateFighterPackage(s, 2, Mission::SWEEP);
			if (elem) {
				elem->SetIntelLevel(Intel::KNOWN);
				elem->SetRegion(rgn);
				elem->SetLocation(base_loc + RandomPoint() * 1.5);
				mission->AddElement(elem);
				ntargets++;
			}
		}
	}
	else if (ttype < 12) {
		if (oca) {
			CombatGroup* s = FindSquadron(enemy, CombatGroup::LCA_SQUADRON);

			if (s) {
				MissionElement* elem = CreateFighterPackage(s, 1, Mission::CARGO);
				if (elem) {
					elem->SetIntelLevel(Intel::KNOWN);
					elem->SetRegion(rgn);
					elem->SetLocation(base_loc + RandomPoint() * 2);
					mission->AddElement(elem);
					ntargets++;

					CombatGroup* s2 = FindSquadron(enemy, CombatGroup::FIGHTER_SQUADRON);

					if (s2) {
						MissionElement* e2 = CreateFighterPackage(s2, 2, Mission::ESCORT);
						if (e2) {
							e2->SetIntelLevel(Intel::KNOWN);
							e2->SetRegion(rgn);
							e2->SetLocation(elem->Location() + RandomPoint() * 0.5);

							Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, elem->Name());
							if (obj)
							e2->AddObjective(obj);
							mission->AddElement(e2);
							ntargets++;
						}
					}
				}
			}
		}
		else {
			CombatGroup* s = FindSquadron(enemy, CombatGroup::ATTACK_SQUADRON);

			if (s) {
				MissionElement* elem = CreateFighterPackage(s, 2, Mission::ASSAULT);
				if (elem) {
					elem->SetIntelLevel(Intel::KNOWN);
					elem->SetRegion(rgn);
					elem->SetLocation(base_loc + RandomPoint() * 1.3);
					mission->AddElement(elem);
					ntargets++;
				}
			}
		}
	}
	else if (ttype < 15) {
		if (oca) {
			CombatGroup* s = 0;
			
			if (airborne)
			s = FindSquadron(enemy, CombatGroup::LCA_SQUADRON);
			else
			s = FindSquadron(enemy, CombatGroup::FREIGHT);

			if (s) {
				MissionElement* elem = CreateFighterPackage(s, 1, Mission::CARGO);
				if (elem) {
					elem->SetIntelLevel(Intel::KNOWN);
					elem->SetRegion(rgn);
					elem->SetLocation(base_loc + RandomPoint() * 2);
					mission->AddElement(elem);
					ntargets++;

					CombatGroup* s2 = FindSquadron(enemy, CombatGroup::INTERCEPT_SQUADRON);

					if (s2) {
						MissionElement* e2 = CreateFighterPackage(s2, 2, Mission::ESCORT);
						if (e2) {
							e2->SetIntelLevel(Intel::KNOWN);
							e2->SetRegion(rgn);
							e2->SetLocation(elem->Location() + RandomPoint() * 0.5);

							Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, elem->Name());
							if (obj)
							e2->AddObjective(obj);
							mission->AddElement(e2);
							ntargets++;
						}
					}
				}
			}
		}
		else {
			CombatGroup* s = FindSquadron(enemy, CombatGroup::ATTACK_SQUADRON);

			if (s) {
				MissionElement* elem = CreateFighterPackage(s, 2, Mission::ASSAULT);
				if (elem) {
					elem->SetIntelLevel(Intel::KNOWN);
					elem->SetRegion(rgn);
					elem->SetLocation(base_loc + RandomPoint() * 1.1);
					mission->AddElement(elem);
					ntargets++;

					CombatGroup* s2 = FindSquadron(enemy, CombatGroup::FIGHTER_SQUADRON);

					if (s2) {
						MissionElement* e2 = CreateFighterPackage(s2, 2, Mission::ESCORT);
						if (e2) {
							e2->SetIntelLevel(Intel::KNOWN);
							e2->SetRegion(rgn);
							e2->SetLocation(elem->Location() + RandomPoint() * 0.5);

							Instruction* obj = new(__FILE__,__LINE__) Instruction(Instruction::ESCORT, elem->Name());
							if (obj)
							e2->AddObjective(obj);
							mission->AddElement(e2);
							ntargets++;
						}
					}
				}
			}
		}
	}
	else {
		CombatGroup* s = FindSquadron(enemy, CombatGroup::LCA_SQUADRON);

		if (s) {
			MissionElement* elem = CreateFighterPackage(s, 2, Mission::CARGO);
			if (elem) {
				elem->SetIntelLevel(Intel::KNOWN);
				elem->SetRegion(rgn);
				elem->SetLocation(base_loc + RandomPoint() * 2);
				mission->AddElement(elem);
				ntargets++;
			}
		}
	}

	return ntargets;
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::PlanetaryInsertion(MissionElement* elem)
{
	if (!mission || !elem)         return;
	if (!mission->GetStarSystem()) return;

	MissionElement* carrier = mission->FindElement(elem->Commander());
	StarSystem*    system   = mission->GetStarSystem();
	OrbitalRegion* rgn1     = system->FindRegion(elem->Region());
	OrbitalRegion* rgn2     = system->FindRegion(air_region);
	Point          npt_loc  = elem->Location();
	Instruction*   n        = 0;
	Player*        p        = Player::GetCurrentPlayer();

	int            flying_start = p ? p->FlyingStart() : 0;

	if (carrier && !flying_start) {
		npt_loc  = carrier->Location() + Point(1e3, -5e3, 0);
	}

	if (rgn1 && rgn2) {
		double   delta_t  = mission->Start() - campaign->GetTime();
		Point    r1       = rgn1->PredictLocation(delta_t);
		Point    r2       = rgn2->PredictLocation(delta_t);

		Point delta = r2 - r1;

		delta.y *= -1;
		delta.Normalize();
		delta *= 10e3;

		npt_loc += delta;

		n = new(__FILE__,__LINE__) Instruction(elem->Region(),
		npt_loc,
		Instruction::VECTOR);
		if (n) {
			n->SetSpeed(750);
			elem->AddNavPoint(n);
		}
	}

	n = new(__FILE__,__LINE__) Instruction(air_region,
	Point(0, 0, 15e3),
	Instruction::VECTOR);
	if (n) {
		n->SetSpeed(750);
		elem->AddNavPoint(n);
	}
}

void
CampaignMissionFighter::OrbitalInsertion(MissionElement* elem)
{
	Instruction* n = new(__FILE__,__LINE__) Instruction(air_region,
	Point(0, 0, 30.0e3),
	Instruction::VECTOR);
	if (n) {
		n->SetSpeed(750);
		elem->AddNavPoint(n);
	}
}

// +--------------------------------------------------------------------+

MissionElement*
CampaignMissionFighter::CreateFighterPackage(CombatGroup* squadron, int count, int role)
{
	if (!squadron) return 0;

	CombatUnit* fighter = squadron->GetUnits().at(0);
	CombatUnit* carrier = FindCarrier(squadron);

	if (!fighter)
	return 0;

	int avail  = fighter->LiveCount();
	int actual = count;

	if (avail < actual)
	actual = avail;

	if (avail < 1) {
		::Print("CMF - Insufficient fighters in squadron '%s' - %d required, %d available\n",
		squadron->Name().data(), count, avail);
		return 0;
	}

	MissionElement* elem = new(__FILE__,__LINE__) MissionElement;
	if (!elem) {
		Exit();
		return 0;
	}

	elem->SetName(Callsign::GetCallsign(fighter->GetIFF()));
	elem->SetElementID(pkg_id++);

	if (carrier) {
		elem->SetCommander(carrier->Name());
		elem->SetHeading(carrier->GetHeading());
	}
	else {
		elem->SetHeading(fighter->GetHeading());
	}

	elem->SetDesign(fighter->GetDesign());
	elem->SetCount(actual);
	elem->SetIFF(fighter->GetIFF());
	elem->SetIntelLevel(squadron->IntelLevel());
	elem->SetRegion(fighter->GetRegion());
	elem->SetSquadron(squadron->Name());
	elem->SetMissionRole(role);

	switch (role) {
	case Mission::ASSAULT:
		if (request->GetObjective() &&
				request->GetObjective()->Type() == CombatGroup::MINEFIELD)
		elem->Loadouts().append(new(__FILE__,__LINE__) MissionLoad(-1, "Rockets"));
		else
		elem->Loadouts().append(new(__FILE__,__LINE__) MissionLoad(-1, "Ship Strike"));
		break;

	case Mission::STRIKE:
		elem->Loadouts().append(new(__FILE__,__LINE__) MissionLoad(-1, "Ground Strike"));
		break;

	default:
		elem->Loadouts().append(new(__FILE__,__LINE__) MissionLoad(-1, "ACM Medium Range"));
		break;
	}

	if (carrier) {
		Point offset = RandomPoint() * 0.3;
		offset.y = fabs(offset.y);
		offset.z += 2e3;
		elem->SetLocation(carrier->Location() + offset);
	}
	else {
		elem->SetLocation(fighter->Location() + RandomPoint());
	}

	elem->SetCombatGroup(squadron);
	elem->SetCombatUnit(fighter);

	return elem;
}

// +--------------------------------------------------------------------+

static CombatGroup* FindCombatGroup(CombatGroup* g, int type)
{
	if (g->IntelLevel() <= Intel::RESERVE)
	return 0;

	if (g->GetUnits().size() > 0) {
		for (int i = 0; i < g->GetUnits().size(); i++) {
			CombatUnit* u = g->GetUnits().at(i);
			if (g->Type() == type && u->LiveCount() > 0)
			return g;
		}
	}

	CombatGroup* result = 0;

	ListIter<CombatGroup> subgroup = g->GetComponents();
	while (++subgroup && !result)
	result = FindCombatGroup(subgroup.value(), type);

	return result;
}

// +--------------------------------------------------------------------+

CombatGroup*
CampaignMissionFighter::FindSquadron(int iff, int type)
{
	if (!squadron) return 0;

	CombatGroup*   result   = 0;
	Campaign*      campaign = Campaign::GetCampaign();

	if (campaign) {
		ListIter<Combatant> combatant = campaign->GetCombatants();
		while (++combatant && !result) {
			if (combatant->GetIFF() == iff) {
				result = ::FindCombatGroup(combatant->GetForce(), type);

				if (result && result->CountUnits() < 1) {
					result = 0;
				}
			}
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::DefineMissionObjectives()
{
	if (!mission || !player_elem) return;

	if (prime_target)    mission->SetTarget(prime_target);
	if (ward)            mission->SetWard(ward);

	Text objectives;

	for (int i = 0; i < player_elem->Objectives().size(); i++) {
		Instruction* obj = player_elem->Objectives().at(i);
		objectives += "* ";
		objectives += obj->GetDescription();
		objectives += ".\n";
	}

	mission->SetObjective(objectives);
}

// +--------------------------------------------------------------------+

MissionInfo*
CampaignMissionFighter::DescribeMission()
{
	if (!mission || !player_elem) return 0;

	char name[256];
	char player_info[256];

	if (mission_info && mission_info->name.length())
		sprintf_s(name, "MSN-%03d %s", mission->Identity(), mission_info->name.data());
	else if (ward)
		sprintf_s(name, "MSN-%03d %s %s", mission->Identity(), Game::GetText(mission->TypeName()).data(), ward->Name().data());
	else if (prime_target)
		sprintf_s(name, "MSN-%03d %s %s %s", mission->Identity(), Game::GetText(mission->TypeName()).data(),
					Ship::ClassName(prime_target->GetDesign()->type),
					prime_target->Name().data());
	else
		sprintf_s(name, "MSN-%03d %s", mission->Identity(), Game::GetText(mission->TypeName()).data());

	if (player_elem) {
		sprintf_s(player_info, "%d x %s %s '%s'", 
			player_elem->Count(), 
			(const char*) player_elem->GetDesign()->abrv,
			(const char*) player_elem->GetDesign()->name,
			(const char*) player_elem->Name());
	}

	MissionInfo* info = new(__FILE__,__LINE__) MissionInfo;

	info->id          = mission->Identity();
	info->mission     = mission;
	info->name        = name;
	info->type        = mission->Type();
	info->player_info = player_info;
	info->description = mission->Objective();
	info->start       = mission->Start();

	if (mission->GetStarSystem())
	info->system      = mission->GetStarSystem()->Name();
	info->region      = mission->GetRegion();

	mission->SetName(name);

	return info;
}

// +--------------------------------------------------------------------+

void
CampaignMissionFighter::Exit()
{
	Starshatter* stars = Starshatter::GetInstance();
	if (stars)
	stars->SetGameMode(Starshatter::MENU_MODE);
}
