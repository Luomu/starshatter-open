/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Campaign.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Campaign defines a strategic military scenario.
*/

#include "MemDebug.h"
#include "Campaign.h"
#include "CampaignPlanStrategic.h"
#include "CampaignPlanAssignment.h"
#include "CampaignPlanEvent.h"
#include "CampaignPlanMission.h"
#include "CampaignPlanMovement.h"
#include "CampaignSituationReport.h"
#include "CampaignSaveGame.h"
#include "Combatant.h"
#include "CombatAction.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatRoster.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Galaxy.h"
#include "Mission.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "Player.h"

#include "Game.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Random.h"
#include "FormatUtil.h"


const int TIME_NEVER = (int) 1e9;
const int ONE_DAY    = (int) 24 * 3600;

// +====================================================================+

MissionInfo::MissionInfo()
: mission(0), start(0), type(0), id(0), min_rank(0), max_rank(0),
action_id(0), action_status(0), exec_once(0),
start_before(TIME_NEVER), start_after(0)
{ }

MissionInfo::~MissionInfo()
{
	delete mission;
}

bool
MissionInfo::IsAvailable()
{
	Campaign*      campaign     = Campaign::GetCampaign();
	Player*        player       = Player::GetCurrentPlayer();
	CombatGroup*   player_group = campaign->GetPlayerGroup();

	if (campaign->GetTime() < start_after)
	return false;

	if (campaign->GetTime() > start_before)
	return false;

	if (region.length() && player_group->GetRegion() != region)
	return false;

	if (min_rank && player->Rank() < min_rank)
	return false;

	if (max_rank && player->Rank() > max_rank)
	return false;

	if (exec_once < 0)
	return false;

	if (exec_once > 0)
	exec_once = -1;

	return true;
}

// +====================================================================+

TemplateList::TemplateList()
: mission_type(0), group_type(0), index(0)
{ }

TemplateList::~TemplateList()
{
	missions.destroy();
}

// +====================================================================+

static List<Campaign>   campaigns;
static Campaign*        current_campaign = 0;

// +--------------------------------------------------------------------+

Campaign::Campaign(int id, const char* n)
: campaign_id(id), name(n), mission_id(-1), mission(0), net_mission(0),
scripted(false), sequential(false), time(0), startTime(0), loadTime(0),
player_group(0), player_unit(0), status(CAMPAIGN_INIT), lockout(0),
loaded_from_savegame(false)
{
	ZeroMemory(path, sizeof(path));
	Load();
}

Campaign::Campaign(int id, const char* n, const char* p)
: campaign_id(id), name(n), mission_id(-1), mission(0), net_mission(0),
scripted(false), sequential(false), time(0), startTime(0), loadTime(0),
player_group(0), player_unit(0), status(CAMPAIGN_INIT), lockout(0),
loaded_from_savegame(false)
{
	ZeroMemory(path, sizeof(path));
	strncpy(path, p, sizeof(path));
	Load();
}

// +--------------------------------------------------------------------+

Campaign::~Campaign()
{
	for (int i = 0; i < NUM_IMAGES; i++)
	image[i].ClearImage();

	delete net_mission;

	actions.destroy();
	events.destroy();
	missions.destroy();
	templates.destroy();
	planners.destroy();
	zones.destroy();
	combatants.destroy();
}

// +--------------------------------------------------------------------+

void
Campaign::Initialize()
{
	Campaign*   c        = 0;
	DataLoader* loader   = DataLoader::GetLoader();

	for (int i = 1; i < 100; i++) {
		char path[256];
		sprintf_s(path, "Campaigns/%02d/", i);

		loader->UseFileSystem(true);
		loader->SetDataPath(path);

		if (loader->FindFile("campaign.def")) {
			char txt[256];
			sprintf_s(txt, "Dynamic Campaign %02d", i);
			c = new(__FILE__,__LINE__) Campaign(i, txt);

			if (c) {
				campaigns.insertSort(c);
			}
		}
	}

	c = new(__FILE__,__LINE__) Campaign(SINGLE_MISSIONS, "Single Missions");
	if (c) {
		campaigns.insertSort(c);
		current_campaign = c;
	}

	c = new(__FILE__,__LINE__) Campaign(MULTIPLAYER_MISSIONS, "Multiplayer Missions");
	if (c) {
		campaigns.insertSort(c);
	}

	c = new(__FILE__,__LINE__) Campaign(CUSTOM_MISSIONS, "Custom Missions");
	if (c) {
		campaigns.insertSort(c);
	}
}

void
Campaign::Close()
{
	Print("Campaign::Close() - destroying all campaigns\n");
	current_campaign = 0;
	campaigns.destroy();
}

Campaign*
Campaign::GetCampaign()
{
	return current_campaign;
}

Campaign*
Campaign::SelectCampaign(const char* name)
{
	Campaign*            c     = 0;
	ListIter<Campaign>   iter  = campaigns;

	while (++iter && !c) {
		if (!_stricmp(iter->Name(), name))
		c = iter.value();
	}

	if (c) {
		Print("Campaign: Selected '%s'\n", c->Name());
		current_campaign = c;
	}
	else {
		Print("Campaign: could not find '%s'\n", name);
	}

	return c;
}

Campaign*
Campaign::CreateCustomCampaign(const char* name, const char* path)
{
	int       id = 0;

	if (name && *name && path && *path) {
		ListIter<Campaign> iter = campaigns;

		while (++iter) {
			Campaign* c = iter.value();
			if (c->GetCampaignId() >= id)
			id = c->GetCampaignId() + 1;

			if (!strcmp(c->Name(), name)) {
				Print("Campaign: custom campaign '%s' already exists.\n", name);
				return 0;
			}
		}
	}

	if (id == 0)
	id = CUSTOM_MISSIONS + 1;

	Campaign* c = new(__FILE__,__LINE__) Campaign(id, name, path);
	Print("Campaign: created custom campaign %d '%s'\n", id, name);
	campaigns.append(c);

	return c;
}

List<Campaign>&
Campaign::GetAllCampaigns()
{
	return campaigns;
}

int
Campaign::GetLastCampaignId()
{
	int result = 0;

	for (int i = 0; i < campaigns.size(); i++) {
		Campaign* c = campaigns.at(i);

		if (c->IsDynamic() && c->GetCampaignId() > result) {
			result = c->GetCampaignId();
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

CombatEvent*
Campaign::GetLastEvent()
{
	CombatEvent* result = 0;

	if (!events.isEmpty())
	result = events.last();

	return result;
}

// +--------------------------------------------------------------------+

int
Campaign::CountNewEvents() const
{
	int result = 0;

	for (int i = 0; i < events.size(); i++)
	if (/*events[i]->Source() != CombatEvent::TACNET &&*/ !events[i]->Visited())
	result++;

	return result;
}

// +--------------------------------------------------------------------+

void
Campaign::Clear()
{
	missions.destroy();
	planners.destroy();
	combatants.destroy();
	events.destroy();
	actions.destroy();

	player_group = 0;
	player_unit  = 0;

	updateTime = time;
}

// +--------------------------------------------------------------------+

void
Campaign::Load()
{
	// first, unload any existing data:
	Unload();

	if (!path[0]) {
		// then load the campaign from files:
		switch (campaign_id) {
		case SINGLE_MISSIONS:      strcpy_s(path, "Missions/");       break;
		case CUSTOM_MISSIONS:      strcpy_s(path, "Mods/Missions/");  break;
		case MULTIPLAYER_MISSIONS: strcpy_s(path, "Multiplayer/");    break;
		default:                   sprintf_s(path, "Campaigns/%02d/",	campaign_id);     break;
		}
	}

	DataLoader* loader   = DataLoader::GetLoader();
	loader->UseFileSystem(true);
	loader->SetDataPath(path);
	systems.clear();

	if (loader->FindFile("zones.def"))
	zones.append(CombatZone::Load("zones.def"));

	for (int i = 0; i < zones.size(); i++) {
		Text s = zones[i]->System();
		bool found = false;

		for (int n = 0; !found && n < systems.size(); n++) {
			if (s == systems[n]->Name())
			found = true;
		}

		if (!found)
		systems.append(Galaxy::GetInstance()->GetSystem(s));
	}

	loader->UseFileSystem(Starshatter::UseFileSystem());

	if (loader->FindFile("campaign.def"))
	LoadCampaign(loader);

	if (campaign_id == CUSTOM_MISSIONS) {
		loader->SetDataPath(path);
		LoadCustomMissions(loader);
	}
	else {
		bool found = false;

		if (loader->FindFile("missions.def")) {
			loader->SetDataPath(path);
			LoadMissionList(loader);
			found = true;
		}

		if (loader->FindFile("templates.def")) {
			loader->SetDataPath(path);
			LoadTemplateList(loader);
			found = true;
		}

		if (!found) {
			loader->SetDataPath(path);
			LoadCustomMissions(loader);
		}
	}

	loader->UseFileSystem(true);
	loader->SetDataPath(path);

	if (loader->FindFile("image.pcx")) {
		loader->LoadBitmap("image.pcx",        image[ 0]);
		loader->LoadBitmap("selected.pcx",     image[ 1]);
		loader->LoadBitmap("unavail.pcx",      image[ 2]);
		loader->LoadBitmap("banner.pcx",       image[ 3]);
	}

	loader->SetDataPath(0);
	loader->UseFileSystem(Starshatter::UseFileSystem());
}

void
Campaign::Unload()
{
	SetStatus(CAMPAIGN_INIT);

	Game::ResetGameTime();
	StarSystem::SetBaseTime(0);

	startTime = Stardate();
	loadTime  = startTime;
	lockout   = 0;

	for (int i = 0; i < NUM_IMAGES; i++)
	image[i].ClearImage();

	Clear();

	zones.destroy();
}

void
Campaign::LoadCampaign(DataLoader* loader, bool full)
{
	BYTE*       block    = 0;
	const char* filename = "campaign.def";

	loader->UseFileSystem(true);
	loader->LoadBuffer(filename, block, true);
	loader->UseFileSystem(Starshatter::UseFileSystem());
	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

	Term*  term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "CAMPAIGN") {
			return;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					if (!def->term() || !def->term()->isText()) {
						::Print("WARNING: name missing in '%s/%s'\n", loader->GetDataPath(), filename);
					}
					else {
						name = def->term()->isText()->value();
						name = Game::GetText(name);
					}
				}
				else if (def->name()->value() == "desc") {
					if (!def->term() || !def->term()->isText()) {
						::Print("WARNING: description missing in '%s/%s'\n", loader->GetDataPath(), filename);
					}
					else {
						description = def->term()->isText()->value();
						description = Game::GetText(description);
					}
				}
				else if (def->name()->value() == "situation") {
					if (!def->term() || !def->term()->isText()) {
						::Print("WARNING: situation missing in '%s/%s'\n", loader->GetDataPath(), filename);
					}
					else {
						situation = def->term()->isText()->value();
						situation = Game::GetText(situation);
					}
				}
				else if (def->name()->value() == "orders") {
					if (!def->term() || !def->term()->isText()) {
						::Print("WARNING: orders missing in '%s/%s'\n", loader->GetDataPath(), filename);
					}
					else {
						orders = def->term()->isText()->value();
						orders = Game::GetText(orders);
					}
				}
				else if (def->name()->value() == "scripted") {
					if (def->term() && def->term()->isBool()) {
						scripted = def->term()->isBool()->value();
					}
				}
				else if (def->name()->value() == "sequential") {
					if (def->term() && def->term()->isBool()) {
						sequential = def->term()->isBool()->value();
					}
				}
				else if (full && def->name()->value() == "combatant") {
					if (!def->term() || !def->term()->isStruct()) {
						::Print("WARNING: combatant struct missing in '%s/%s'\n", loader->GetDataPath(), filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						char           name[64];
						int            iff   = 0;
						CombatGroup*   force = 0;
						CombatGroup*   clone = 0;

						ZeroMemory(name,  sizeof(name));

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "name") {
									GetDefText(name, pdef, filename);

									force = CombatRoster::GetInstance()->GetForce(name);

									if (force)
									clone = force->Clone(false); // shallow copy
								}

								else if (pdef->name()->value() == "group") {
									ParseGroup(pdef->term()->isStruct(), force, clone, filename);
								}
							}
						}

						loader->SetDataPath(path);
						Combatant* c = new(__FILE__,__LINE__) Combatant(name, clone);
						if (c) {
							combatants.append(c);
						}
						else {
							Unload();
							return;
						}
					}
				}
				else if (full && def->name()->value() == "action") {
					if (!def->term() || !def->term()->isStruct()) {
						::Print("WARNING: action struct missing in '%s/%s'\n", loader->GetDataPath(), filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseAction(val, filename);
					}
				}
			}
		}
	}
	while (term);

	loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
Campaign::ParseGroup(TermStruct*    val,
CombatGroup*   force,
CombatGroup*   clone,
const char*    filename)
{
	if (!val) {
		::Print("invalid combat group in campaign %s\n", name.data());
		return;
	}

	int   type = 0;
	int   id   = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "type") {
				char type_name[64];
				GetDefText(type_name, pdef, filename);
				type = CombatGroup::TypeFromName(type_name);
			}

			else if (pdef->name()->value() == "id") {
				GetDefNumber(id, pdef, filename);
			}
		}
	}

	if (type && id) {
		CombatGroup* g = force->FindGroup(type, id);

		// found original group, now clone it over
		if (g && g->GetParent()) {
			CombatGroup* parent = CloneOver(force, clone, g->GetParent());
			parent->AddComponent(g->Clone());
		}
	}
}

// +--------------------------------------------------------------------+

void
Campaign::ParseAction(TermStruct* val, const char* filename)
{
	if (!val) {
		::Print("invalid action in campaign %s\n", name.data());
		return;
	}

	int   id             = 0;
	int   type           = 0;
	int   subtype        = 0;
	int   opp_type       = -1;
	int   team           = 0;
	int   source         = 0;
	Vec3  loc(0.0f, 0.0f, 0.0f);
	Text  system;
	Text  region;
	Text  file;
	Text  image;
	Text  scene;
	Text  text;
	int   count          = 1;
	int   start_before   = TIME_NEVER;
	int   start_after    = 0;
	int   min_rank       = 0;
	int   max_rank       = 100;
	int   delay          = 0;
	int   probability    = 100;

	int   asset_type     = 0;
	int   asset_id       = 0;
	int   target_type    = 0;
	int   target_id      = 0;
	int   target_iff     = 0;

	CombatAction* action = 0;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			if (pdef->name()->value() == "id") {
				GetDefNumber(id, pdef, filename);
			}
			else if (pdef->name()->value() == "type") {
				char txt[64];
				GetDefText(txt, pdef, filename);
				type = CombatAction::TypeFromName(txt);
			}
			else if (pdef->name()->value() == "subtype") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(subtype, pdef, filename);
				}

				else if (pdef->term()->isText()) {
					char txt[64];
					GetDefText(txt, pdef, filename);

					if (type == CombatAction::MISSION_TEMPLATE) {
						subtype = Mission::TypeFromName(txt);
					}
					else if (type == CombatAction::COMBAT_EVENT) {
						subtype = CombatEvent::TypeFromName(txt);
					}
					else if (type == CombatAction::INTEL_EVENT) {
						subtype = Intel::IntelFromName(txt);
					}
				}
			}
			else if (pdef->name()->value() == "opp_type") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(opp_type, pdef, filename);
				}

				else if (pdef->term()->isText()) {
					char txt[64];
					GetDefText(txt, pdef, filename);

					if (type == CombatAction::MISSION_TEMPLATE) {
						opp_type = Mission::TypeFromName(txt);
					}
				}
			}
			else if (pdef->name()->value() == "source") {
				char txt[64];
				GetDefText(txt, pdef, filename);
				source = CombatEvent::SourceFromName(txt);
			}
			else if (pdef->name()->value() == "team") {
				GetDefNumber(team, pdef, filename);
			}
			else if (pdef->name()->value() == "iff") {
				GetDefNumber(team, pdef, filename);
			}
			else if (pdef->name()->value() == "count") {
				GetDefNumber(count, pdef, filename);
			}
			else if (pdef->name()->value().contains("before")) {
				if (pdef->term()->isNumber()) {
					GetDefNumber(start_before, pdef, filename);
				}
				else {
					GetDefTime(start_before, pdef, filename);
					start_before -= ONE_DAY;
				}
			}
			else if (pdef->name()->value().contains("after")) {
				if (pdef->term()->isNumber()) {
					GetDefNumber(start_after, pdef, filename);
				}
				else {
					GetDefTime(start_after, pdef, filename);
					start_after -= ONE_DAY;
				}
			}
			else if (pdef->name()->value() == "min_rank") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(min_rank, pdef, filename);
				}
				else {
					char rank_name[64];
					GetDefText(rank_name, pdef, filename);
					min_rank = Player::RankFromName(rank_name);
				}
			}
			else if (pdef->name()->value() == "max_rank") {
				if (pdef->term()->isNumber()) {
					GetDefNumber(max_rank, pdef, filename);
				}
				else {
					char rank_name[64];
					GetDefText(rank_name, pdef, filename);
					max_rank = Player::RankFromName(rank_name);
				}
			}
			else if (pdef->name()->value() == "delay") {
				GetDefNumber(delay, pdef, filename);
			}
			else if (pdef->name()->value() == "probability") {
				GetDefNumber(probability, pdef, filename);
			}
			else if (pdef->name()->value() == "asset_type") {
				char type_name[64];
				GetDefText(type_name, pdef, filename);
				asset_type = CombatGroup::TypeFromName(type_name);
			}
			else if (pdef->name()->value() == "target_type") {
				char type_name[64];
				GetDefText(type_name, pdef, filename);
				target_type = CombatGroup::TypeFromName(type_name);
			}
			else if (pdef->name()->value() == "location" ||
					pdef->name()->value() == "loc") {
				GetDefVec(loc, pdef, filename);
			}
			else if (pdef->name()->value() == "system" ||
					pdef->name()->value() == "sys") {
				GetDefText(system, pdef, filename);
			}
			else if (pdef->name()->value() == "region" ||
					pdef->name()->value() == "rgn"    ||
					pdef->name()->value() == "zone") {
				GetDefText(region, pdef, filename);
			}
			else if (pdef->name()->value() == "file") {
				GetDefText(file, pdef, filename);
			}
			else if (pdef->name()->value() == "image") {
				GetDefText(image, pdef, filename);
			}
			else if (pdef->name()->value() == "scene") {
				GetDefText(scene, pdef, filename);
			}
			else if (pdef->name()->value() == "text") {
				GetDefText(text, pdef, filename);
				text = Game::GetText(text);
			}
			else if (pdef->name()->value() == "asset_id") {
				GetDefNumber(asset_id, pdef, filename);
			}
			else if (pdef->name()->value() == "target_id") {
				GetDefNumber(target_id, pdef, filename);
			}
			else if (pdef->name()->value() == "target_iff") {
				GetDefNumber(target_iff, pdef, filename);
			}

			else if (pdef->name()->value() == "asset_kill") {
				if (!action)
				action = new(__FILE__,__LINE__) CombatAction(id, type, subtype, team);

				if (action) {
					char txt[64];
					GetDefText(txt, pdef, filename);
					action->AssetKills().append(new (__FILE__,__LINE__) Text(txt));
				}
			}

			else if (pdef->name()->value() == "target_kill") {
				if (!action)
				action = new(__FILE__,__LINE__) CombatAction(id, type, subtype, team);

				if (action) {
					char txt[64];
					GetDefText(txt, pdef, filename);
					action->TargetKills().append(new (__FILE__,__LINE__) Text(txt));
				}
			}

			else if (pdef->name()->value() == "req") {
				if (!action)
				action = new(__FILE__,__LINE__) CombatAction(id, type, subtype, team);

				if (!pdef->term() || !pdef->term()->isStruct()) {
					::Print("WARNING: action req struct missing in '%s'\n", filename);
				}
				else if (action) {
					TermStruct* val2 = pdef->term()->isStruct();

					int  act  = 0;
					int  stat = CombatAction::COMPLETE;
					bool not  = false;

					Combatant*  c1    = 0;
					Combatant*  c2    = 0;
					int         comp  = 0;
					int         score = 0;
					int         intel = 0;
					int         gtype = 0;
					int         gid   = 0;

					for (int i = 0; i < val2->elements()->size(); i++) {
						TermDef* pdef2 = val2->elements()->at(i)->isDef();
						if (pdef2) {
							if (pdef2->name()->value() == "action") {
								GetDefNumber(act, pdef2, filename);
							}
							else if (pdef2->name()->value() == "status") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								stat = CombatAction::StatusFromName(txt);
							}
							else if (pdef2->name()->value() == "not") {
								GetDefBool(not, pdef2, filename);
							}

							else if (pdef2->name()->value() == "c1") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								c1 = GetCombatant(txt);
							}
							else if (pdef2->name()->value() == "c2") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								c2 = GetCombatant(txt);
							}
							else if (pdef2->name()->value() == "comp") {
								char txt[64];
								GetDefText(txt, pdef2, filename);
								comp = CombatActionReq::CompFromName(txt);
							}
							else if (pdef2->name()->value() == "score") {
								GetDefNumber(score, pdef2, filename);
							}
							else if (pdef2->name()->value() == "intel") {
								if (pdef2->term()->isNumber()) {
									GetDefNumber(intel, pdef2, filename);
								}
								else if (pdef2->term()->isText()) {
									char txt[64];
									GetDefText(txt, pdef2, filename);
									intel = Intel::IntelFromName(txt);
								}
							}
							else if (pdef2->name()->value() == "group_type") {
								char type_name[64];
								GetDefText(type_name, pdef2, filename);
								gtype = CombatGroup::TypeFromName(type_name);
							}
							else if (pdef2->name()->value() == "group_id") {
								GetDefNumber(gid, pdef2, filename);
							}
						}
					}

					if (act)
					action->AddRequirement(act, stat, not);

					else if (gtype)
					action->AddRequirement(c1, gtype, gid, comp, score, intel);
					
					else
					action->AddRequirement(c1, c2, comp, score);
				}
			}
		}
	}

	if (!action)
	action = new(__FILE__,__LINE__) CombatAction(id, type, subtype, team);

	if (action) {
		action->SetSource(source);
		action->SetOpposingType(opp_type);
		action->SetLocation(loc);
		action->SetSystem(system);
		action->SetRegion(region);
		action->SetFilename(file);
		action->SetImageFile(image);
		action->SetSceneFile(scene);
		action->SetCount(count);
		action->SetStartAfter(start_after);
		action->SetStartBefore(start_before);
		action->SetMinRank(min_rank);
		action->SetMaxRank(max_rank);
		action->SetDelay(delay);
		action->SetProbability(probability);

		action->SetAssetId(asset_id);
		action->SetAssetType(asset_type);
		action->SetTargetId(target_id);
		action->SetTargetIFF(target_iff);
		action->SetTargetType(target_type);
		action->SetText(text);

		actions.append(action);
	}
}

// +--------------------------------------------------------------------+

CombatGroup*
Campaign::CloneOver(CombatGroup* force, CombatGroup* clone, CombatGroup* group)
{
	CombatGroup* orig_parent = group->GetParent();

	if (orig_parent) {
		CombatGroup* clone_parent = clone->FindGroup(orig_parent->Type(), orig_parent->GetID());

		if (!clone_parent)
		clone_parent = CloneOver(force, clone, orig_parent);

		CombatGroup* new_clone = clone->FindGroup(group->Type(), group->GetID());

		if (!new_clone) {
			new_clone = group->Clone(false);
			clone_parent->AddComponent(new_clone);
		}

		return new_clone;
	}
	else {
		return clone;
	}
}

// +--------------------------------------------------------------------+

void
Campaign::LoadMissionList(DataLoader* loader)
{
	bool        ok       = true;
	BYTE*       block    = 0;
	const char* filename = "Missions.def";

	loader->UseFileSystem(true);
	loader->LoadBuffer(filename, block, true);
	loader->UseFileSystem(Starshatter::UseFileSystem());
	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

	Term*  term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "MISSIONLIST") {
			::Print("WARNING: invalid mission list file '%s'\n", filename);
			term->print(10);
			return;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "mission") {
					if (!def->term() || !def->term()->isStruct()) {
						::Print("WARNING: mission struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						int   id = 0;
						Text  name;
						Text  desc;
						char  script[256];
						char  system[256];
						char  region[256];
						int   start = 0;
						int   type = 0;

						ZeroMemory(script, sizeof(script));

						strcpy_s(system, "Unknown");
						strcpy_s(region, "Unknown");

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "id")
								GetDefNumber(id, pdef, filename);

								else if (pdef->name()->value() == "name") {
									GetDefText(name, pdef, filename);
									name = Game::GetText(name);
								}

								else if (pdef->name()->value() == "desc") {
									GetDefText(desc, pdef, filename);
									if (desc.length() > 0 && desc.length() < 32)
									desc = Game::GetText(desc);
								}

								else if (pdef->name()->value() == "start")
								GetDefTime(start, pdef, filename);

								else if (pdef->name()->value() == "system")
								GetDefText(system, pdef, filename);

								else if (pdef->name()->value() == "region")
								GetDefText(region, pdef, filename);

								else if (pdef->name()->value() == "script")
								GetDefText(script, pdef, filename);

								else if (pdef->name()->value() == "type") {
									char typestr[64];
									GetDefText(typestr, pdef, filename);
									type = Mission::TypeFromName(typestr);
								}
							}
						}

						MissionInfo* info    = new(__FILE__,__LINE__) MissionInfo;
						if (info) {
							info->id             = id;
							info->name           = name;
							info->description    = desc;
							info->system         = system;
							info->region         = region;
							info->script         = script;
							info->start          = start;
							info->type           = type;
							info->mission        = 0;

							info->script.setSensitive(false);

							missions.append(info);
						}
						else {
							ok = false;
						}
					}
				}
			}
		}
	}
	while (term);

	loader->ReleaseBuffer(block);

	if (!ok)
	Unload();
}

void
Campaign::LoadCustomMissions(DataLoader* loader)
{
	bool       ok = true;
	List<Text> files;
	loader->UseFileSystem(true);
	loader->ListFiles("*.*", files);

	for (int i = 0; i < files.size(); i++) {
		Text file = *files[i];
		file.setSensitive(false);

		if (file.contains(".def")) {
			BYTE*       block    = 0;
			const char* filename = file.data();

			loader->UseFileSystem(true);
			loader->LoadBuffer(filename, block, true);
			loader->UseFileSystem(Starshatter::UseFileSystem());

			if (strstr((const char*) block, "MISSION") == (const char*) block) {
				Text  name;
				Text  desc;
				Text  system   = "Unknown";
				Text  region   = "Unknown";
				int   start    = 0;
				int   type     = 0;
				int   msn_id   = 0;

				Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));
				Term*  term = parser.ParseTerm();

				if (!term) {
					Print("ERROR: could not parse '%s'\n", filename);
					continue;
				}
				else {
					TermText* file_type = term->isText();
					if (!file_type || file_type->value() != "MISSION") {
						Print("ERROR: invalid mission file '%s'\n", filename);
						term->print(10);
						continue;
					}
				}

				do {
					delete term; term = 0;
					term = parser.ParseTerm();
					
					if (term) {
						TermDef* def = term->isDef();
						if (def) {
							if (def->name()->value() == "name") {
								GetDefText(name, def, filename);
								name = Game::GetText(name);
							}

							else if (def->name()->value() == "type") {
								char typestr[64];
								GetDefText(typestr, def, filename);
								type = Mission::TypeFromName(typestr);
							}

							else if (def->name()->value() == "id")
							GetDefNumber(msn_id, def, filename);

							else if (def->name()->value() == "desc") {
								GetDefText(desc, def, filename);
								if (desc.length() > 0 && desc.length() < 32)
								desc = Game::GetText(desc);
							}

							else if (def->name()->value() == "system")
							GetDefText(system, def, filename);

							else if (def->name()->value() == "region")
							GetDefText(region, def, filename);

							else if (def->name()->value() == "start")
							GetDefTime(start, def, filename);
						}
					}
				}
				while (term);

				loader->ReleaseBuffer(block);

				if (strstr(filename, "custom") == filename) {
					sscanf_s(filename+6, "%d", &msn_id);

					if (msn_id <= i)
					msn_id = i+1;
				}
				else if (msn_id < 1) {
					msn_id = i+1;
				}

				MissionInfo* info = new(__FILE__,__LINE__) MissionInfo;
				if (info) {
					info->id          = msn_id;
					info->name        = name;
					info->type        = type;
					info->description = desc;
					info->system      = system;
					info->region      = region;
					info->script      = filename;
					info->start       = start;
					info->mission     = 0;

					info->script.setSensitive(false);

					missions.append(info);
				}
				else {
					ok = false;
				}
			}
			else {
				Print("Invalid Custom Mission File: '%s'\n", filename);
			}

			loader->ReleaseBuffer(block);
		}
	}

	files.destroy();

	if (!ok)
	Unload();
	else
	missions.sort();
}

void
Campaign::LoadTemplateList(DataLoader* loader)
{
	BYTE*       block    = 0;
	const char* filename = "Templates.def";

	loader->UseFileSystem(true);
	loader->LoadBuffer(filename, block, true);
	loader->UseFileSystem(Starshatter::UseFileSystem());
	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

	Term*  term = parser.ParseTerm();

	if (!term) {
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "TEMPLATELIST") {
			::Print("WARNING: invalid template list file '%s'\n", filename);
			term->print(10);
			return;
		}
	}

	do {
		delete term; term = 0;
		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "mission") {
					if (!def->term() || !def->term()->isStruct()) {
						::Print("WARNING: mission struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						char  name[256];
						char  script[256];
						char  region[256];
						int   id             = 0;
						int   msn_type       = 0;
						int   grp_type       = 0;

						int   min_rank       = 0;
						int   max_rank       = 0;
						int   action_id      = 0;
						int   action_status  = 0;
						int   exec_once      = 0;
						int   start_before   = TIME_NEVER;
						int   start_after    = 0;

						name[0]   = 0;
						script[0] = 0;
						region[0] = 0;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "id")
								GetDefNumber(id, pdef, filename);

								else if (pdef->name()->value() == "name")
								GetDefText(name, pdef, filename);

								else if (pdef->name()->value() == "script")
								GetDefText(script, pdef, filename);

								else if (pdef->name()->value() == "rgn" || pdef->name()->value() == "region")
								GetDefText(region, pdef, filename);

								else if (pdef->name()->value() == "type") {
									char typestr[64];
									GetDefText(typestr, pdef, filename);
									msn_type = Mission::TypeFromName(typestr);
								}

								else if (pdef->name()->value() == "group") {
									char typestr[64];
									GetDefText(typestr, pdef, filename);
									grp_type = CombatGroup::TypeFromName(typestr);
								}

								else if (pdef->name()->value() == "min_rank")
								GetDefNumber(min_rank, pdef, filename);

								else if (pdef->name()->value() == "max_rank")
								GetDefNumber(max_rank, pdef, filename);

								else if (pdef->name()->value() == "action_id")
								GetDefNumber(action_id, pdef, filename);

								else if (pdef->name()->value() == "action_status")
								GetDefNumber(action_status, pdef, filename);

								else if (pdef->name()->value() == "exec_once")
								GetDefNumber(exec_once, pdef, filename);

								else if (pdef->name()->value().contains("before")) {
									if (pdef->term()->isNumber()) {
										GetDefNumber(start_before, pdef, filename);
									}
									else {
										GetDefTime(start_before, pdef, filename);
										start_before -= ONE_DAY;
									}
								}
								else if (pdef->name()->value().contains("after")) {
									if (pdef->term()->isNumber()) {
										GetDefNumber(start_after, pdef, filename);
									}
									else {
										GetDefTime(start_after, pdef, filename);
										start_after -= ONE_DAY;
									}
								}
							}
						}

						MissionInfo* info    = new(__FILE__,__LINE__) MissionInfo;
						if (info) {
							info->id             = id;
							info->name           = name;
							info->script         = script;
							info->region         = region;
							info->type           = msn_type;
							info->min_rank       = min_rank;
							info->max_rank       = max_rank;
							info->action_id      = action_id;
							info->action_status  = action_status;
							info->exec_once      = exec_once;
							info->start_before   = start_before;
							info->start_after    = start_after;

							info->script.setSensitive(false);

							TemplateList* templist = GetTemplateList(msn_type, grp_type);

							if (!templist) {
								templist = new(__FILE__,__LINE__) TemplateList;
								templist->mission_type = msn_type;
								templist->group_type   = grp_type;
								templates.append(templist);
							}

							templist->missions.append(info);
						}
					}
				}
			}
		}
	}
	while (term);

	loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
Campaign::CreatePlanners()
{
	if (planners.size() > 0)
	planners.destroy();

	CampaignPlan* p;

	// PLAN EVENT MUST BE FIRST PLANNER:
	p = new(__FILE__,__LINE__) CampaignPlanEvent(this);
	if (p)
	planners.append(p);

	p = new(__FILE__,__LINE__) CampaignPlanStrategic(this);
	if (p)
	planners.append(p);

	p = new(__FILE__,__LINE__) CampaignPlanAssignment(this);
	if (p)
	planners.append(p);

	p = new(__FILE__,__LINE__) CampaignPlanMovement(this);
	if (p)
	planners.append(p);

	p = new(__FILE__,__LINE__) CampaignPlanMission(this);
	if (p)
	planners.append(p);

	if (lockout > 0 && planners.size()) {
		ListIter<CampaignPlan> plan = planners;
		while (++plan)
		plan->SetLockout(lockout);
	}
}

// +--------------------------------------------------------------------+

int
Campaign::GetPlayerIFF()
{
	int iff = 1;

	if (player_group)
	iff = player_group->GetIFF();

	return iff;
}

void
Campaign::SetPlayerGroup(CombatGroup* pg)
{
	if (player_group != pg) {
		::Print("Campaign::SetPlayerGroup(%s)\n", pg ? pg->Name().data() : "0");

		// should verify that the player group is
		// actually part of this campaign, first!

		player_group = pg;
		player_unit  = 0;

		// need to regenerate missions when changing
		// player combat group:
		if (IsDynamic()) {
			::Print("  destroying mission list...\n");
			missions.destroy();
		}
	}
}

void
Campaign::SetPlayerUnit(CombatUnit* unit)
{
	if (player_unit != unit) {
		::Print("Campaign::SetPlayerUnit(%s)\n", unit ? unit->Name().data() : "0");

		// should verify that the player unit is
		// actually part of this campaign, first!

		player_unit = unit;

		if (unit)
		player_group = unit->GetCombatGroup();

		// need to regenerate missions when changing
		// player combat unit:
		if (IsDynamic()) {
			::Print("  destroying mission list...\n");
			missions.destroy();
		}
	}
}

// +--------------------------------------------------------------------+

CombatZone*
Campaign::GetZone(const char* rgn)
{
	ListIter<CombatZone> z = zones;
	while (++z) {
		if (z->HasRegion(rgn))
		return z.value();
	}

	return 0;
}

StarSystem*
Campaign::GetSystem(const char* sys)
{
	return Galaxy::GetInstance()->GetSystem(sys);
}

Combatant*
Campaign::GetCombatant(const char* cname)
{
	ListIter<Combatant> iter = combatants;
	while (++iter) {
		Combatant* c = iter.value();
		if (!strcmp(c->Name(), cname))
		return c;
	}

	return 0;
}

// +--------------------------------------------------------------------+

Mission*
Campaign::GetMission()
{
	return GetMission(mission_id);
}

Mission*
Campaign::GetMission(int id)
{
	if (id < 0) {
		::Print("ERROR - Campaign::GetMission(%d) invalid mission id\n", id);
		return 0;
	}

	if (mission && mission->Identity() == id) {
		return mission;
	}

	MissionInfo* info = 0;
	for (int i = 0; !info && i < missions.size(); i++)
	if (missions[i]->id == id)
	info = missions[i];

	if (info) {
		if (!info->mission) {
			::Print("Campaign::GetMission(%d) loading mission...\n", id);
			info->mission = new(__FILE__,__LINE__) Mission(id, info->script, path);
			if (info->mission)
			info->mission->Load();
		}

		if (IsDynamic()) {
			if (info->mission) {
				if (!_stricmp(info->mission->Situation(), "Unknown")) {
					::Print("Campaign::GetMission(%d) generating sitrep...\n", id);
					CampaignSituationReport sitrep(this, info->mission);
					sitrep.GenerateSituationReport();
				}
			}
			else {
				::Print("Campaign::GetMission(%d) could not find/load mission.\n", id);
			}
		}

		return info->mission;
	}

	return 0;
}

Mission*
Campaign::GetMissionByFile(const char* filename)
{
	if (!filename || !*filename) {
		::Print("ERROR - Campaign::GetMissionByFile() invalid filename\n");
		return 0;
	}

	int          id    = 0;
	int          maxid = 0;
	MissionInfo* info  = 0;

	for (int i = 0; !info && i < missions.size(); i++) {
		MissionInfo* m = missions[i];

		if (m->id > maxid)
		maxid = m->id;

		if (m->script == filename)
		info = m;
	}

	if (info) {
		id = info->id;

		if (!info->mission) {
			::Print("Campaign::GetMission(%d) loading mission...\n", id);
			info->mission = new(__FILE__,__LINE__) Mission(id, info->script, path);
			if (info->mission)
			info->mission->Load();
		}

		if (IsDynamic()) {
			if (info->mission) {
				if (!_stricmp(info->mission->Situation(), "Unknown")) {
					::Print("Campaign::GetMission(%d) generating sitrep...\n", id);
					CampaignSituationReport sitrep(this, info->mission);
					sitrep.GenerateSituationReport();
				}
			}
			else {
				::Print("Campaign::GetMission(%d) could not find/load mission.\n", id);
			}
		}
	}

	else {
		info = new(__FILE__,__LINE__) MissionInfo;
		if (info) {
			info->id      = maxid+1;
			info->name    = "New Custom Mission";
			info->script  = filename;
			info->mission = new(__FILE__,__LINE__) Mission(info->id, info->script, "Mods/Missions/");
			info->mission->SetName(info->name);

			info->script.setSensitive(false);

			missions.append(info);
		}
	}

	return info->mission;
}

MissionInfo*
Campaign::CreateNewMission()
{
	int          id    = 0;
	int          maxid = 0;
	MissionInfo* info  = 0;

	if (campaign_id == MULTIPLAYER_MISSIONS)
	maxid = 10;

	for (int i = 0; !info && i < missions.size(); i++) {
		MissionInfo* m = missions[i];

		if (m->id > maxid)
		maxid = m->id;
	}

	char filename[64];
	sprintf_s(filename, "custom%03d.def", maxid+1);

	info = new(__FILE__,__LINE__) MissionInfo;
	if (info) {
		info->id      = maxid+1;
		info->name    = "New Custom Mission";
		info->script  = filename;
		info->mission = new(__FILE__,__LINE__) Mission(info->id, filename, path);
		info->mission->SetName(info->name);

		info->script.setSensitive(false);

		missions.append(info);
	}

	return info;
}

void
Campaign::DeleteMission(int id)
{
	if (id < 0) {
		::Print("ERROR - Campaign::DeleteMission(%d) invalid mission id\n", id);
		return;
	}

	MissionInfo*   m     = 0;
	int            index = -1;

	for (int i = 0; !m && i < missions.size(); i++) {
		if (missions[i]->id == id) {
			m = missions[i];
			index = i;
		}
	}

	if (m) {
		char full_path[256];

		if (path[strlen(path)-1] == '/')
			sprintf_s(full_path, "%s%s",  path, m->script);
		else
			sprintf_s(full_path, "%s/%s", path, m->script);

		DeleteFile(full_path);
		Load();
	}

	else {
		::Print("ERROR - Campaign::DeleteMission(%d) could not find mission\n", id);
	}
}

MissionInfo*
Campaign::GetMissionInfo(int id)
{
	if (id < 0) {
		::Print("ERROR - Campaign::GetMissionInfo(%d) invalid mission id\n", id);
		return 0;
	}

	MissionInfo* m = 0;
	for (int i = 0; !m && i < missions.size(); i++)
	if (missions[i]->id == id)
	m = missions[i];

	if (m) {
		if (!m->mission) {
			m->mission = new(__FILE__,__LINE__) Mission(id, m->script);
			if (m->mission)
			m->mission->Load();
		}

		return m;
	}

	else {
		::Print("ERROR - Campaign::GetMissionInfo(%d) could not find mission\n", id);
	}

	return 0;
}

void
Campaign::ReloadMission(int id)
{
	if (mission && mission == net_mission) {
		delete net_mission;
		net_mission = 0;
	}

	mission = 0;

	if (id >= 0 && id < missions.size()) {
		MissionInfo* m = missions[id];
		delete m->mission;
		m->mission = 0;
	}
}

void
Campaign::LoadNetMission(int id, const char* net_mission_script)
{
	if (mission && mission == net_mission) {
		delete net_mission;
		net_mission = 0;
	}

	mission_id = id;
	mission    = new(__FILE__,__LINE__) Mission(id);

	if (mission && mission->ParseMission(net_mission_script))
	mission->Validate();

	net_mission = mission;
}

// +--------------------------------------------------------------------+

CombatAction*
Campaign::FindAction(int action_id)
{
	ListIter<CombatAction> iter = actions;
	while (++iter) {
		CombatAction* a = iter.value();

		if (a->Identity() == action_id)
		return a;
	}

	return 0;
}

// +--------------------------------------------------------------------+

MissionInfo*
Campaign::FindMissionTemplate(int mission_type, CombatGroup* player_group)
{
	MissionInfo* info = 0;

	if (!player_group)
	return info;

	TemplateList* templates = GetTemplateList(mission_type, player_group->Type());

	if (!templates || !templates->missions.size())
	return info;

	int tries = 0;
	int msize = templates->missions.size();

	while (!info && tries < msize) {
		// get next template:
		int index = templates->index;

		if (index >= msize)
		index = 0;

		info = templates->missions[index];
		templates->index = index + 1;
		tries++;

		// validate the template:
		if (info) {
			if (info->action_id) {
				CombatAction* a = FindAction(info->action_id);

				if (a && a->Status() != info->action_status)
				info = 0;
			}

			if (info && !info->IsAvailable()) {
				info = 0;
			}
		}
	}

	return info;
}

// +--------------------------------------------------------------------+

TemplateList*
Campaign::GetTemplateList(int msn_type, int grp_type)
{
	for (int i = 0; i < templates.size(); i++) {
		if (templates[i]->mission_type == msn_type &&
				templates[i]->group_type   == grp_type)
		return templates[i];
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
Campaign::SetMissionId(int id)
{
	::Print("Campaign::SetMissionId(%d)\n", id);

	if (id > 0)
	mission_id = id;
	else
	::Print("   retaining mission id = %d\n", mission_id);
}

// +--------------------------------------------------------------------+

double
Campaign::Stardate()
{
	return StarSystem::Stardate();
}

// +--------------------------------------------------------------------+

void
Campaign::SelectDefaultPlayerGroup(CombatGroup* g, int type)
{
	if (player_group || !g) return;

	if (g->Type() == type && !g->IsReserve() && g->Value() > 0) {
		player_group = g;
		player_unit  = 0;
		return;
	}

	for (int i = 0; i < g->GetComponents().size(); i++)
	SelectDefaultPlayerGroup(g->GetComponents()[i], type);
}

// +--------------------------------------------------------------------+

void
Campaign::Prep()
{
	// if this is a new campaign,
	// create combatants from roster and template:
	if (IsDynamic() && combatants.isEmpty()) {
		DataLoader* loader   = DataLoader::GetLoader();
		loader->SetDataPath(path);
		LoadCampaign(loader, true);
	}

	StarSystem::SetBaseTime(loadTime);

	// load scripted missions:
	if (IsScripted() && actions.isEmpty()) {
		DataLoader* loader   = DataLoader::GetLoader();
		loader->SetDataPath(path);
		LoadCampaign(loader, true);

		ListIter<MissionInfo> m = missions;
		while (++m) {
			GetMission(m->id);
		}
	}

	CheckPlayerGroup();
}

void
Campaign::Start()
{
	::Print("Campaign::Start()\n");

	Prep();

	// create planners:
	CreatePlanners();
	SetStatus(CAMPAIGN_ACTIVE);
}

void
Campaign::ExecFrame()
{
	if (InCutscene())
	return;

	time = Stardate() - startTime;

	if (status < CAMPAIGN_ACTIVE)
	return;

	if (IsDynamic()) {
		bool completed = false;
		ListIter<MissionInfo> m = missions;
		while (++m) {
			if (m->mission && m->mission->IsComplete()) {
				::Print("Campaign::ExecFrame() completed mission %d '%s'\n", m->id, m->name.data());
				completed = true;
			}
		}

		if (completed) {
			::Print("Campaign::ExecFrame() destroying mission list after completion...\n");
			missions.destroy();

			if (!player_group || player_group->IsFighterGroup())
			time += 10 * 3600;
			else
			time += 20 * 3600;

			StarSystem::SetBaseTime(startTime + time - Game::GameTime()/1000.0);
		}
		else {
			m.reset();

			while (++m) {
				if (m->start < time && !m->mission->IsActive()) {
					MissionInfo* info = m.removeItem();

					if (info)
					::Print("Campaign::ExecFrame() deleting expired mission %d start: %d current: %d\n",
					info->id,
					info->start,
					(int) time);

					delete info;
				}
			}
		}

		// PLAN EVENT MUST BE FIRST PLANNER:
		if (loaded_from_savegame && planners.size() > 0) {
			CampaignPlanEvent* plan_event = (CampaignPlanEvent*) planners.first();
			plan_event->ExecScriptedEvents();

			loaded_from_savegame = false;
		}

		ListIter<CampaignPlan> plan = planners;
		while (++plan) {
			CheckPlayerGroup();
			plan->ExecFrame();
		}

		CheckPlayerGroup();

		// Auto save game AFTER planners have run!
		// This is done to ensure that campaign status
		// is properly recorded after winning or losing
		// the campaign.

		if (completed) {
			CampaignSaveGame save(this);
			save.SaveAuto();
		}
	}
	else {
		// PLAN EVENT MUST BE FIRST PLANNER:
		if (planners.size() > 0) {
			CampaignPlanEvent* plan_event = (CampaignPlanEvent*) planners.first();
			plan_event->ExecScriptedEvents();
		}
	}
}

// +--------------------------------------------------------------------+

void
Campaign::LockoutEvents(int seconds)
{
	lockout = seconds;
}

void
Campaign::CheckPlayerGroup()
{
	if (!player_group || player_group->IsReserve() || player_group->CalcValue() < 1) {
		int player_iff = GetPlayerIFF();
		player_group = 0;

		CombatGroup* force = 0;
		for (int i = 0; i < combatants.size() && !force; i++) {
			if (combatants[i]->GetIFF() == player_iff) {
				force = combatants[i]->GetForce();
			}
		}

		if (force) {
			force->CalcValue();
			SelectDefaultPlayerGroup(force, CombatGroup::WING);

			if (!player_group)
			SelectDefaultPlayerGroup(force, CombatGroup::DESTROYER_SQUADRON);
		}
	}

	if (player_unit && player_unit->GetValue() < 1)
	SetPlayerUnit(0);
}

// +--------------------------------------------------------------------+

void FPU2Extended();
void FPURestore();

void
Campaign::StartMission()
{
	Mission* m = GetMission();

	if (m) {
		::Print("\n\nCampaign Start Mission - %d. '%s'\n", m->Identity(), m->Name());

		if (!scripted) {
			FPU2Extended();

			double gtime = (double) Game::GameTime() / 1000.0;
			double base  = startTime + m->Start() - 15 - gtime;

			StarSystem::SetBaseTime(base);

			double current_time = Stardate() - startTime;

			char buffer[32];
			FormatDayTime(buffer, current_time);
			::Print("  current time:  %s\n", buffer);

			FormatDayTime(buffer, m->Start());
			::Print("  mission start: %s\n", buffer);
			::Print("\n");
		}
	}
}

void
Campaign::RollbackMission()
{
	::Print("Campaign::RollbackMission()\n");

	Mission* m = GetMission();

	if (m) {
		if (!scripted) {
			FPU2Extended();

			double gtime = (double) Game::GameTime() / 1000.0;
			double base  = startTime + m->Start() - 60 - gtime;

			StarSystem::SetBaseTime(base);

			double current_time = Stardate() - startTime;
			::Print("  mission start: %d\n", m->Start());
			::Print("  current time:  %d\n", (int) current_time);
		}

		m->SetActive(false);
		m->SetComplete(false);
	}
}

// +--------------------------------------------------------------------+

bool
Campaign::InCutscene() const
{
	Starshatter* stars = Starshatter::GetInstance();
	return stars ? stars->InCutscene() : false;
}

bool
Campaign::IsDynamic() const
{
	return campaign_id >= DYNAMIC_CAMPAIGN &&
	campaign_id <  SINGLE_MISSIONS;
}

bool
Campaign::IsTraining() const
{
	return campaign_id == TRAINING_CAMPAIGN;
}

bool
Campaign::IsScripted() const
{
	return scripted;
}

bool
Campaign::IsSequential() const
{
	return sequential;
}

// +--------------------------------------------------------------------+

static CombatGroup* FindGroup(CombatGroup* g, int type, int id)
{
	if (g->Type() == type && g->GetID() == id)
	return g;

	CombatGroup* result = 0;

	ListIter<CombatGroup> subgroup = g->GetComponents();
	while (++subgroup && !result)
	result = FindGroup(subgroup.value(), type, id);

	return result;
}

CombatGroup*
Campaign::FindGroup(int iff, int type, int id)
{
	CombatGroup*      result = 0;

	ListIter<Combatant> combatant = combatants;
	while (++combatant && !result) {
		if (combatant->GetIFF() == iff) {
			result = ::FindGroup(combatant->GetForce(), type, id);
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

static void FindGroups(CombatGroup* g, int type, CombatGroup* near_group,
List<CombatGroup>& groups)
{
	if (g->Type() == type && g->IntelLevel() > Intel::RESERVE) {
		if (!near_group || g->GetAssignedZone() == near_group->GetAssignedZone())
		groups.append(g);
	}

	ListIter<CombatGroup> subgroup = g->GetComponents();
	while (++subgroup)
	FindGroups(subgroup.value(), type, near_group, groups);
}

CombatGroup*
Campaign::FindGroup(int iff, int type, CombatGroup* near_group)
{
	CombatGroup*      result = 0;
	List<CombatGroup> groups;

	ListIter<Combatant> combatant = combatants;
	while (++combatant) {
		if (combatant->GetIFF() == iff) {
			FindGroups(combatant->GetForce(), type, near_group, groups);
		}
	}

	if (groups.size() > 0) {
		int index = (int) Random(0, groups.size());
		if (index >= groups.size()) index = groups.size() - 1;
		result = groups[index];
	}

	return result;
}

// +--------------------------------------------------------------------+

static void FindStrikeTargets(CombatGroup* g, CombatGroup* strike_group,
List<CombatGroup>& groups)
{
	if (!strike_group || !strike_group->GetAssignedZone()) return;

	if (g->IsStrikeTarget() && g->IntelLevel() > Intel::RESERVE) {
		if (strike_group->GetAssignedZone() == g->GetAssignedZone() ||
				strike_group->GetAssignedZone()->HasRegion(g->GetRegion()))
		groups.append(g);
	}

	ListIter<CombatGroup> subgroup = g->GetComponents();
	while (++subgroup)
	FindStrikeTargets(subgroup.value(), strike_group, groups);
}

CombatGroup*
Campaign::FindStrikeTarget(int iff, CombatGroup* strike_group)
{
	CombatGroup*   result   = 0;

	List<CombatGroup> groups;

	ListIter<Combatant> combatant = GetCombatants();
	while (++combatant) {
		if (combatant->GetIFF() != 0 && combatant->GetIFF() != iff) {
			FindStrikeTargets(combatant->GetForce(), strike_group, groups);
		}
	}

	if (groups.size() > 0) {
		int index = rand() % groups.size();
		result = groups[index];
	}

	return result;
}

// +--------------------------------------------------------------------+

void
Campaign::CommitExpiredActions()
{
	ListIter<CombatAction> iter = actions;
	while (++iter) {
		CombatAction* a = iter.value();

		if (a->IsAvailable())
		a->SetStatus(CombatAction::COMPLETE);
	}

	updateTime = time;
}

// +--------------------------------------------------------------------+

int
Campaign::GetPlayerTeamScore()
{
	int score_us   = 0;
	int score_them = 0;

	if (player_group) {
		int iff = player_group->GetIFF();

		ListIter<Combatant> iter = combatants;
		while (++iter) {
			Combatant* c = iter.value();

			if (iff <= 1) {
				if (c->GetIFF() <= 1)
				score_us += c->Score();
				else
				score_them += c->Score();
			}

			else {
				if (c->GetIFF() <= 1)
				score_them += c->Score();
				else
				score_us += c->Score();
			}
		}
	}

	return score_us - score_them;
}

// +--------------------------------------------------------------------+

void
Campaign::SetStatus(int s)
{
	status = s;

	// record the win in player profile:
	if (status == CAMPAIGN_SUCCESS) {
		Player* player = Player::GetCurrentPlayer();

		if (player)
		player->SetCampaignComplete(campaign_id);
	}

	if (status > CAMPAIGN_ACTIVE) {
		::Print("Campaign::SetStatus() destroying mission list at campaign end\n");
		missions.destroy();
	}
}

// +--------------------------------------------------------------------+

static void GetCombatUnits(CombatGroup* g, List<CombatUnit>& units)
{
	if (g) {
		ListIter<CombatUnit> unit = g->GetUnits();
		while (++unit) {
			CombatUnit* u = unit.value();

			if (u->Count() - u->DeadCount() > 0)
			units.append(u);
		}

		ListIter<CombatGroup> comp = g->GetComponents();
		while (++comp) {
			CombatGroup* g2 = comp.value();

			if (!g2->IsReserve())
			GetCombatUnits(g2, units);
		}
	}
}

int
Campaign::GetAllCombatUnits(int iff, List<CombatUnit>& units)
{
	units.clear();

	ListIter<Combatant> iter = combatants;
	while (++iter) {
		Combatant* c = iter.value();

		if (iff < 0 || c->GetIFF() == iff) {
			GetCombatUnits(c->GetForce(), units);
		}
	}

	return units.size();
}
