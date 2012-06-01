/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Mission.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Template classes
*/

#include "MemDebug.h"
#include "MissionTemplate.h"
#include "MissionEvent.h"
#include "StarSystem.h"
#include "Galaxy.h"
#include "Callsign.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "Starshatter.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Instruction.h"
#include "Random.h"

#include "Game.h"
#include "DataLoader.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+

MissionTemplate::MissionTemplate(int identity, const char* fname, const char* pname)
: Mission(identity, fname, pname)
{
}

MissionTemplate::~MissionTemplate()
{
	callsigns.destroy();
	aliases.destroy();
}

// +--------------------------------------------------------------------+

void
MissionTemplate::AddElement(MissionElement* elem)
{
	if (elem) {
		elements.append(elem);
		aliases.append(new(__FILE__,__LINE__) MissionAlias(elem->Name(), elem));
	}
}

bool
MissionTemplate::MapElement(MissionElement* elem)
{
	bool result = false;

	if (elem && !elem->GetCombatUnit()) {
		if (elem->IsDropship()) {
			Text callsign = MapCallsign(elem->Name(), elem->GetIFF());

			if (callsign.length())
			elem->SetName(callsign);
		}

		ListIter<Instruction> obj = elem->Objectives();
		while (++obj) {
			Instruction* i = obj.value();
			if (strlen(i->TargetName())) {
				// find a callsign, only if one already exists:
				Text callsign = MapCallsign(i->TargetName(), -1);
				if (callsign.length())
				i->SetTarget(callsign.data());
			}
		}

		ListIter<Instruction> nav = elem->NavList();
		while (++nav) {
			Instruction* i = nav.value();
			if (strlen(i->TargetName())) {
				// find a callsign, only if one already exists:
				Text callsign = MapCallsign(i->TargetName(), -1);
				if (callsign.length())
				i->SetTarget(callsign.data());
			}
		}

		CombatGroup* g = FindCombatGroup(elem->GetIFF(), elem->GetDesign());

		if (g) {
			CombatUnit* u = g->GetNextUnit();

			if (u) {
				elem->SetCombatGroup(g);
				elem->SetCombatUnit(u);
			}
		}

		if (elem->GetCombatUnit()) {
			MissionElement* cmdr = FindElement(elem->Commander());
			if (cmdr)
			elem->SetCommander(cmdr->Name());

			MissionElement* sqdr = FindElement(elem->Squadron());
			if (sqdr)
			elem->SetSquadron(sqdr->Name());

			if (!elem->IsDropship()) {
				aliases.append(new(__FILE__,__LINE__) MissionAlias(elem->Name(), elem));
				elem->SetName(elem->GetCombatUnit()->Name());
			}

			result = true;
		}
	}

	return result;
}

Text
MissionTemplate::MapShip(Text name)
{
	Text result = name;
	int  len    = name.length();

	if (len) {
		MissionElement* elem = 0;

		// single ship from an element (e.g. "Alpha 1")?
		if (isdigit(name[len-1]) && isspace(name[len-2])) {
			Text elem_name = name.substring(0, len-2);

			elem = FindElement(elem_name);
			if (elem)
			result = elem->Name() + name.substring(len-2, 2);
		}

		// full element name
		// (also used to try again if single-ship search fails)
		if (!elem) {
			elem = FindElement(name);

			if (elem)
			result = elem->Name();
		}
	}

	return result;
}

// +--------------------------------------------------------------------+

bool
MissionTemplate::MapEvent(MissionEvent* event)
{
	bool result = false;

	if (event) {
		event->event_ship       = MapShip(event->event_ship);
		event->event_source     = MapShip(event->event_source);
		event->event_target     = MapShip(event->event_target);
		event->trigger_ship     = MapShip(event->trigger_ship);
		event->trigger_target   = MapShip(event->trigger_target);

		result = true;
	}

	return result;
}

// +--------------------------------------------------------------------+

Text
MissionTemplate::MapCallsign(const char* name, int iff)
{
	for (int i = 0; i < callsigns.size(); i++) {
		if (callsigns[i]->Name() == name)
		return callsigns[i]->Callsign();
	}

	if (iff >= 0) {
		const char* callsign = Callsign::GetCallsign(iff);
		MissionCallsign* mc = new(__FILE__,__LINE__) MissionCallsign(callsign, name);
		callsigns.append(mc);

		return mc->Callsign();
	}

	return name;
}

// +--------------------------------------------------------------------+

static void SelectCombatGroups(CombatGroup* g, const ShipDesign* d, List<CombatGroup>& list)
{
	if (g->IntelLevel() <= Intel::RESERVE)
	return;

	if (g->GetUnits().size() > 0) {
		for (int i = 0; i < g->GetUnits().size(); i++) {
			CombatUnit* u = g->GetUnits().at(i);
			if (u->GetDesign() == d && u->Count() - u->DeadCount() > 0) {
				list.append(g);
			}
		}
	}

	ListIter<CombatGroup> subgroup = g->GetComponents();
	while (++subgroup)
	SelectCombatGroups(subgroup.value(), d, list);
}

CombatGroup*
MissionTemplate::FindCombatGroup(int iff, const ShipDesign* d)
{
	CombatGroup*      result   = 0;
	Campaign*         campaign = Campaign::GetCampaign();
	List<CombatGroup> group_list;
	static int        combat_group_index = 0;

	if (campaign) {
		ListIter<Combatant> combatant = campaign->GetCombatants();
		while (++combatant && !result) {
			if (combatant->GetIFF() == iff) {
				::SelectCombatGroups(combatant->GetForce(), d, group_list);
			}
		}

		if (group_list.size() > 0)
		result = group_list[combat_group_index++ % group_list.size()];
	}

	return result;
}

// +--------------------------------------------------------------------+

MissionElement*
MissionTemplate::FindElement(const char* n)
{
	Text name = n;

	ListIter<MissionCallsign> c_iter = callsigns;
	while (++c_iter) {
		MissionCallsign* c = c_iter.value();
		if (c->Name() == name) {
			name = c->Callsign();
			break;
		}
	}

	ListIter<MissionAlias> a_iter = aliases;
	while (++a_iter) {
		MissionAlias* a = a_iter.value();
		if (a->Name() == name)
		return a->Element();
	}

	ListIter<MissionElement> e_iter = elements;
	while (++e_iter) {
		MissionElement* elem = e_iter.value();
		if (elem->Name() == name)
		return elem;
	}

	return 0;
}

// +--------------------------------------------------------------------+

bool
MissionTemplate::Load(const char* fname, const char* pname)
{
	ok = false;

	if (fname)
	strcpy_s(filename, fname);

	if (pname)
	strcpy_s(path, pname);

	if (!filename[0]) {
		Print("\nCan't Load Mission Template, script unspecified.\n");
		return ok;
	}

	Print("\nLoad Mission Template: '%s'\n", filename);

	int max_ships = (int) 1e6;

	DataLoader* loader = DataLoader::GetLoader();
	bool        old_fs = loader->IsFileSystemEnabled();
	BYTE*       block  = 0;

	loader->UseFileSystem(true);
	loader->SetDataPath(path);
	loader->LoadBuffer(filename, block, true);
	loader->SetDataPath(0);
	loader->UseFileSystem(old_fs);

	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse '%s'\n", filename);
		return ok;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "MISSION_TEMPLATE") {
			Print("ERROR: invalid MISSION TEMPLATE file '%s'\n", filename);
			term->print(10);
			return ok;
		}
	}

	ok = true;

	char  target_name[256];
	char  ward_name[256];

	target_name[0] = 0;
	ward_name[0]   = 0;

	do {
		delete term; term = 0;
		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				Text defname = def->name()->value();

				if (defname == "name")
				GetDefText(name, def, filename);

				else if (defname == "type") {
					char typestr[64];
					GetDefText(typestr, def, filename);
					type = TypeFromName(typestr);
				}

				else if (defname == "system") {
					char  sysname[64];
					GetDefText(sysname, def, filename);

					Campaign* campaign = Campaign::GetCampaign();

					if (campaign) {
						Galaxy* galaxy = Galaxy::GetInstance();

						if (galaxy) {
							star_system = galaxy->GetSystem(sysname);
						}
					}
				}

				else if (defname == "degrees")
				GetDefBool(degrees, def, filename);

				else if (defname == "region")
				GetDefText(region, def, filename);

				else if (defname == "objective")
				GetDefText(objective, def, filename);

				else if (defname == "sitrep")
				GetDefText(sitrep, def, filename);

				else if (defname == "start")
				GetDefTime(start, def, filename);

				else if (defname == "team")
				GetDefNumber(team, def, filename);

				else if (defname == "target")
				GetDefText(target_name, def, filename);

				else if (defname == "ward")
				GetDefText(ward_name, def, filename);

				else if ((defname == "alias")) {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: alias struct missing in '%s'\n", filename);
						ok = false;
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseAlias(val);
					}
				}

				else if ((defname == "callsign")) {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: callsign struct missing in '%s'\n", filename);
						ok = false;
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseCallsign(val);
					}
				}

				else if (defname == "optional") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: optional group struct missing in '%s'\n", filename);
						ok = false;
					}
					else {
						TermStruct* val = def->term()->isStruct();
						ParseOptional(val);
					}
				}

				else if (defname == "element") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: element struct missing in '%s'\n", filename);
						ok = false;
					}
					else {
						TermStruct* val = def->term()->isStruct();
						MissionElement* elem = ParseElement(val);
						if (MapElement(elem)) {
							AddElement(elem);
						}
						else {
							Print("WARNING: failed to map element %s '%s' in '%s'\n",
							elem->GetDesign() ? elem->GetDesign()->name : "NO DSN",
							elem->Name().data(),
							filename);
							val->print();
							Print("\n");
							delete elem;
							ok = false;
						}
					}
				}

				else if (defname == "event") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: event struct missing in '%s'\n", filename);
						ok = false;
					}
					else {
						TermStruct* val = def->term()->isStruct();
						MissionEvent* event = ParseEvent(val);

						if (MapEvent(event))
						AddEvent(event);
					}
				}
			}     // def
		}        // term
	}
	while (term);

	loader->ReleaseBuffer(block);

	if (ok) {
		CheckObjectives();

		if (target_name[0])
		target = FindElement(target_name);

		if (ward_name[0])
		ward = FindElement(ward_name);

		Print("Mission Template Loaded.\n\n");
	}

	return ok;
}

// +--------------------------------------------------------------------+

void
MissionTemplate::ParseAlias(TermStruct* val)
{
	Text  name;
	Text  design;
	Text  code;
	Text  elem_name;
	int   iff      = -1;
	int   player   = 0;
	RLoc* rloc     = 0;
	bool  use_loc  = false;
	Vec3  loc;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name")
			GetDefText(name, pdef, filename);

			else if (defname == "elem")
			GetDefText(elem_name, pdef, filename);

			else if (defname == "code")
			GetDefText(code, pdef, filename);

			else if (defname == "design")
			GetDefText(design, pdef, filename);

			else if (defname == "iff")
			GetDefNumber(iff, pdef, filename);

			else if (defname == "loc") {
				loc;
				GetDefVec(loc, pdef, filename);
				use_loc = true;
			}

			else if (defname == "rloc") {
				if (pdef->term()->isStruct()) {
					rloc = ParseRLoc(pdef->term()->isStruct());
				}
			}

			else if (defname == "player") {
				GetDefNumber(player, pdef, filename);

				if (player && !code.length())
				code = "player";
			}
		}
	}

	MissionElement* elem = 0;

	// now find element and create alias:
	if (name.length()) {
		for (int i = 0; i < aliases.size(); i++)
		if (aliases[i]->Name() == name)
		return;

		// by element name?
		if (elem_name.length()) {
			elem = FindElement(elem_name);
		}

		// by special code?
		else if (code.length()) {
			code.toLower();
			Campaign* campaign = Campaign::GetCampaign();

			if (code == "player") {
				for (int i = 0; !elem && i < elements.size(); i++) {
					MissionElement* e = elements[i];
					if (e->Player() > 0) {
						elem = e;
					}
				}
			}

			else if (campaign && code == "player_carrier") {
				CombatGroup* player_group = campaign->GetPlayerGroup();

				if (player_group) {
					CombatGroup* carrier = player_group->FindCarrier();

					if (carrier) {
						elem = FindElement(carrier->Name());
					}
				}
			}

			else if (campaign && code == "player_squadron") {
				CombatGroup* player_group = player_squadron;
				
				if (!player_group)
				player_group = campaign->GetPlayerGroup();

				if (player_group &&
						(player_group->Type() == CombatGroup::INTERCEPT_SQUADRON ||
							player_group->Type() == CombatGroup::FIGHTER_SQUADRON   ||
							player_group->Type() == CombatGroup::ATTACK_SQUADRON)) {
					elem = FindElement(player_group->Name());
				}
			}

			else if (campaign && code == "strike_target") {
				CombatGroup* player_group  = campaign->GetPlayerGroup();

				if (player_group) {
					CombatGroup* strike_target = campaign->FindStrikeTarget(player_group->GetIFF(), player_group);

					if (strike_target) {
						elem = FindElement(strike_target->Name());
					}
				}
			}
		}

		// by design and team?
		else {
			MissionElement* first_match = 0;

			for (int i = 0; !elem && i < elements.size(); i++) {
				MissionElement* e = elements[i];
				if (e->GetIFF() == iff && design == e->GetDesign()->name) {
					// do we already have an alias for this element?
					bool found = false;
					for (int a = 0; !found && a < aliases.size(); a++)
					if (aliases[a]->Element() == e)
					found = true;

					if (!found)
					elem = e;

					else if (!first_match)
					first_match = e;
				}
			}

			if (first_match && !elem)
			elem = first_match;
		}

		if (elem) {
			if (rloc)         elem->SetRLoc(*rloc);
			else if (use_loc) elem->SetLocation(loc);

			delete rloc;

			aliases.append(new(__FILE__,__LINE__) MissionAlias(name, elem));
		}
		else {
			::Print("WARNING: Could not resolve mission alias '%s'\n", (const char*) name);
			ok = false;
		}
	}

	if (!elem || !ok) return;

	// re-parse the struct, dealing with stuff
	// that needs to be attached to the element:
	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "objective") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: order struct missing for element '%s' in '%s'\n", (const char*) elem->Name(), filename);
					ok = false;
				}
				else {
					TermStruct*  val = pdef->term()->isStruct();
					Instruction* obj = ParseInstruction(val, elem);
					elem->Objectives().append(obj);
				}
			}

			else if (defname == "instr") {
				Text* obj = new(__FILE__,__LINE__) Text;
				if (GetDefText(*obj, pdef, filename))
				elem->Instructions().append(obj);
			}

			else if (defname == "order" || defname == "navpt") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: order struct missing for element '%s' in '%s'\n", (const char*) elem->Name(), filename);
					ok = false;
				}
				else {
					TermStruct*  val = pdef->term()->isStruct();
					Instruction* npt = ParseInstruction(val, elem);
					elem->NavList().append(npt);
				}
			}

			else if (defname == "loadout") {
				if (!pdef->term() || !pdef->term()->isStruct()) {
					Print("WARNING: loadout struct missing for element '%s' in '%s'\n", (const char*) elem->Name(), filename);
					ok = false;
				}
				else {
					TermStruct* val = pdef->term()->isStruct();
					ParseLoadout(val, elem);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MissionTemplate::ParseCallsign(TermStruct* val)
{
	Text  name;
	int   iff      = -1;

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "name")
			GetDefText(name, pdef, filename);

			else if (defname == "iff")
			GetDefNumber(iff, pdef, filename);
		}
	}

	if (name.length() > 0 && iff >= 0)
	MapCallsign(name, iff);
}

// +--------------------------------------------------------------------+

bool
MissionTemplate::ParseOptional(TermStruct* val)
{
	int   n     = 0;
	int   min   = 0;
	int   max   = 1000;
	int   skip  = 0;
	int   total = val->elements()->size();

	for (int i = 0; i < val->elements()->size(); i++) {
		TermDef* pdef = val->elements()->at(i)->isDef();
		if (pdef) {
			Text defname = pdef->name()->value();
			defname.setSensitive(false);

			if (defname == "min") {
				GetDefNumber(min, pdef, filename);
				total--;
			}

			else if (defname == "max") {
				GetDefNumber(max, pdef, filename);
				total--;
			}


			else if ((defname == "optional")) {
				bool select;

				if (n >= max)
				select = false;
				else if (total - n - skip <= min)
				select = true;
				else
				select = RandomChance();

				if (select) {
					if (!pdef->term() || !pdef->term()->isStruct()) {
						Print("WARNING: optional group struct missing in '%s'\n", filename);
						ok = false;
						skip++;
					}
					else {
						TermStruct* val = pdef->term()->isStruct();
						if (ParseOptional(val))
						n++;
						else
						skip++;
					}
				}
				else {
					skip++;
				}
			}

			else if (defname == "element") {
				bool select;

				if (n >= max)
				select = false;
				else if (total - n - skip <= min)
				select = true;
				else
				select = RandomChance();

				if (select) {
					if (!pdef->term() || !pdef->term()->isStruct()) {
						Print("WARNING: element struct missing in '%s'\n", filename);
						ok = false;
						skip++;
					}
					else {
						TermStruct* es = pdef->term()->isStruct();
						MissionElement* elem = ParseElement(es);
						if (MapElement(elem)) {
							AddElement(elem);
							n++;
						}
						else {
							delete elem;
							skip++;
						}
					}
				}
				else {
					skip++;
				}
			}
		}
	}

	return n > 0 && n >= min;
}

// +--------------------------------------------------------------------+

void
MissionTemplate::CheckObjectives()
{
	ListIter<MissionElement> iter = elements;
	while (++iter) {
		MissionElement* elem = iter.value();

		ListIter<Instruction> obj = elem->Objectives();
		while (++obj) {
			Instruction* o = obj.value();
			Text tgt = o->TargetName();

			MissionElement* tgt_elem = 0;

			if (tgt.length()) {
				tgt_elem = FindElement(tgt);

				if (!tgt_elem)
				obj.removeItem();
				else
				o->SetTarget(tgt_elem->Name());
			}
		}
	}
}
