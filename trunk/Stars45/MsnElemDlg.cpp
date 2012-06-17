/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnElemDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mod Config Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnElemDlg.h"
#include "MsnEditDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"
#include "Galaxy.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "EditBox.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "Skin.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(MsnElemDlg, OnAccept);
DEF_MAP_CLIENT(MsnElemDlg, OnCancel);
DEF_MAP_CLIENT(MsnElemDlg, OnClassSelect);
DEF_MAP_CLIENT(MsnElemDlg, OnDesignSelect);
DEF_MAP_CLIENT(MsnElemDlg, OnObjectiveSelect);
DEF_MAP_CLIENT(MsnElemDlg, OnIFFChange);

// +--------------------------------------------------------------------+

MsnElemDlg::MsnElemDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
btn_accept(0), btn_cancel(0), edt_name(0), cmb_class(0), cmb_design(0),
edt_size(0), edt_iff(0), cmb_role(0), cmb_region(0), cmb_skin(0),
edt_loc_x(0), edt_loc_y(0), edt_loc_z(0), cmb_heading(0), edt_hold_time(0),
btn_player(0), btn_playable(0), btn_alert(0), btn_command_ai(0),
edt_respawns(0), cmb_commander(0), cmb_carrier(0), cmb_squadron(0),
cmb_intel(0), cmb_loadout(0), cmb_objective(0), cmb_target(0),
mission(0), elem(0), exit_latch(true)
{
	Init(def);
}

MsnElemDlg::~MsnElemDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::RegisterControls()
{
	btn_accept     = (Button*) FindControl(  1);
	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_accept,    MsnElemDlg, OnAccept);

	btn_cancel     = (Button*) FindControl(  2);
	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_cancel,    MsnElemDlg, OnCancel);

	edt_name       = (EditBox*)   FindControl(201);
	cmb_class      = (ComboBox*)  FindControl(202);
	cmb_design     = (ComboBox*)  FindControl(203);
	cmb_skin       = (ComboBox*)  FindControl(213);
	edt_size       = (EditBox*)   FindControl(204);
	edt_iff        = (EditBox*)   FindControl(205);
	cmb_role       = (ComboBox*)  FindControl(206);
	cmb_region     = (ComboBox*)  FindControl(207);
	edt_loc_x      = (EditBox*)   FindControl(208);
	edt_loc_y      = (EditBox*)   FindControl(209);
	edt_loc_z      = (EditBox*)   FindControl(210);
	cmb_heading    = (ComboBox*)  FindControl(211);
	edt_hold_time  = (EditBox*)   FindControl(212);

	btn_player     = (Button*)    FindControl(221);
	btn_alert      = (Button*)    FindControl(222);
	btn_playable   = (Button*)    FindControl(223);
	btn_command_ai = (Button*)    FindControl(224);
	edt_respawns   = (EditBox*)   FindControl(225);
	cmb_commander  = (ComboBox*)  FindControl(226);
	cmb_carrier    = (ComboBox*)  FindControl(227);
	cmb_squadron   = (ComboBox*)  FindControl(228);
	cmb_intel      = (ComboBox*)  FindControl(229);
	cmb_loadout    = (ComboBox*)  FindControl(230);
	cmb_objective  = (ComboBox*)  FindControl(231);
	cmb_target     = (ComboBox*)  FindControl(232);

	if (cmb_class)
	REGISTER_CLIENT(EID_SELECT,   cmb_class,     MsnElemDlg, OnClassSelect);

	if (cmb_design)
	REGISTER_CLIENT(EID_SELECT,   cmb_design,    MsnElemDlg, OnDesignSelect);

	if (cmb_objective)
	REGISTER_CLIENT(EID_SELECT,   cmb_objective, MsnElemDlg, OnObjectiveSelect);

	if (edt_iff)
	REGISTER_CLIENT(EID_KILL_FOCUS, edt_iff,     MsnElemDlg, OnIFFChange);
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::Show()
{
	FormWindow::Show();

	if (!elem) return;

	int current_class = 0;

	if (cmb_class) {
		cmb_class->ClearItems();

		cmb_class->AddItem(Ship::ClassName(Ship::DRONE));
		cmb_class->AddItem(Ship::ClassName(Ship::FIGHTER));
		cmb_class->AddItem(Ship::ClassName(Ship::ATTACK));

		cmb_class->AddItem(Ship::ClassName(Ship::LCA));
		cmb_class->AddItem(Ship::ClassName(Ship::COURIER));
		cmb_class->AddItem(Ship::ClassName(Ship::CARGO));
		cmb_class->AddItem(Ship::ClassName(Ship::CORVETTE));
		cmb_class->AddItem(Ship::ClassName(Ship::FREIGHTER));
		cmb_class->AddItem(Ship::ClassName(Ship::FRIGATE));
		cmb_class->AddItem(Ship::ClassName(Ship::DESTROYER));
		cmb_class->AddItem(Ship::ClassName(Ship::CRUISER));
		cmb_class->AddItem(Ship::ClassName(Ship::BATTLESHIP));
		cmb_class->AddItem(Ship::ClassName(Ship::CARRIER));
		cmb_class->AddItem(Ship::ClassName(Ship::SWACS));
		cmb_class->AddItem(Ship::ClassName(Ship::DREADNAUGHT));
		cmb_class->AddItem(Ship::ClassName(Ship::STATION));
		cmb_class->AddItem(Ship::ClassName(Ship::FARCASTER));

		cmb_class->AddItem(Ship::ClassName(Ship::MINE));
		cmb_class->AddItem(Ship::ClassName(Ship::COMSAT));
		cmb_class->AddItem(Ship::ClassName(Ship::DEFSAT));

		cmb_class->AddItem(Ship::ClassName(Ship::BUILDING));
		cmb_class->AddItem(Ship::ClassName(Ship::FACTORY));
		cmb_class->AddItem(Ship::ClassName(Ship::SAM));
		cmb_class->AddItem(Ship::ClassName(Ship::EWR));
		cmb_class->AddItem(Ship::ClassName(Ship::C3I));
		cmb_class->AddItem(Ship::ClassName(Ship::STARBASE));

		const ShipDesign* design = elem->GetDesign();

		for (int i = 0; i < cmb_class->NumItems(); i++) {
			const char* cname   = cmb_class->GetItem(i);
			int         classid = Ship::ClassForName(cname);

			if (design && classid == design->type) {
				cmb_class->SetSelection(i);
				current_class = classid;
				break;
			}
		}
	}

	if (cmb_design) {
		OnClassSelect(0);
		OnDesignSelect(0);
	}

	if (cmb_role) {
		cmb_role->ClearItems();

		for (int i = Mission::PATROL; i <= Mission::OTHER; i++) {
			cmb_role->AddItem(Mission::RoleName(i));

			if (i == 0)
			cmb_role->SetSelection(0);

			else if (elem->MissionRole() == i)
			cmb_role->SetSelection(cmb_role->NumItems()-1);
		}
	}

	if (cmb_region) {
		cmb_region->ClearItems();

		if (mission) {
			StarSystem* sys = mission->GetStarSystem();
			if (sys) {
				List<OrbitalRegion> regions;
				regions.append(sys->AllRegions());
				regions.sort();

				ListIter<OrbitalRegion> iter = regions;
				while (++iter) {
					OrbitalRegion* region = iter.value();
					cmb_region->AddItem(region->Name());

					if (!strcmp(elem->Region(), region->Name()))
					cmb_region->SetSelection(cmb_region->NumItems()-1);
				}
			}
		}
	}

	char buf[64];

	if (edt_name)  edt_name->SetText(elem->Name());

	sprintf_s(buf, "%d", elem->Count());
	if (edt_size)  edt_size->SetText(buf);

	sprintf_s(buf, "%d", elem->GetIFF());
	if (edt_iff)   edt_iff->SetText(buf);

	sprintf_s(buf, "%d", (int) elem->Location().x / 1000);
	if (edt_loc_x) edt_loc_x->SetText(buf);

	sprintf_s(buf, "%d", (int) elem->Location().y / 1000);
	if (edt_loc_y) edt_loc_y->SetText(buf);

	sprintf_s(buf, "%d", (int) elem->Location().z / 1000);
	if (edt_loc_z) edt_loc_z->SetText(buf);

	sprintf_s(buf, "%d", elem->RespawnCount());
	if (edt_respawns) edt_respawns->SetText(buf);

	sprintf_s(buf, "%d", elem->HoldTime());
	if (edt_hold_time) edt_hold_time->SetText(buf);

	if (btn_player)      btn_player->SetButtonState(elem->Player() > 0 ? 1 : 0);
	if (btn_playable)    btn_playable->SetButtonState(elem->IsPlayable() ? 1 : 0);
	if (btn_alert)       btn_alert->SetButtonState(elem->IsAlert() ? 1 : 0);
	if (btn_command_ai)  btn_command_ai->SetButtonState(elem->CommandAI());

	UpdateTeamInfo();

	if (cmb_intel) {
		cmb_intel->ClearItems();

		for (int i = Intel::RESERVE; i < Intel::TRACKED; i++) {
			cmb_intel->AddItem(Intel::NameFromIntel(i));

			if (i == 0)
			cmb_intel->SetSelection(0);

			else if (elem->IntelLevel() == i)
			cmb_intel->SetSelection(cmb_intel->NumItems()-1);
		}
	}

	Instruction* instr = 0;
	if (elem->Objectives().size() > 0)
	instr = elem->Objectives().at(0);

	if (cmb_objective) {
		cmb_objective->ClearItems();
		cmb_objective->AddItem("");
		cmb_objective->SetSelection(0);

		for (int i = 0; i < Instruction::NUM_ACTIONS; i++) {
			cmb_objective->AddItem(Instruction::ActionName(i));

			if (instr && instr->Action() == i)
			cmb_objective->SetSelection(i+1);
		}
	}

	if (cmb_target) {
		OnObjectiveSelect(0);
	}

	if (cmb_heading) {
		double heading = elem->Heading();

		while (heading > 2*PI)
		heading -= 2*PI;

		while (heading < 0)
		heading += 2*PI;

		if (heading >= PI/4 && heading < 3*PI/4)
		cmb_heading->SetSelection(1);

		else if (heading >= 3*PI/4 && heading < 5*PI/4)
		cmb_heading->SetSelection(2);

		else if (heading >= 5*PI/4 && heading < 7*PI/4)
		cmb_heading->SetSelection(3);

		else
		cmb_heading->SetSelection(0);
	}

	exit_latch = true;
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		if (!exit_latch && btn_accept && btn_accept->IsEnabled())
		OnAccept(0);
	}

	else if (Keyboard::KeyDown(VK_ESCAPE)) {
		if (!exit_latch)
		OnCancel(0);
	}

	else {
		exit_latch = false;
	}
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::SetMission(Mission* m)
{
	mission = m;
}

void
MsnElemDlg::SetMissionElement(MissionElement* e)
{
	elem = e;
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::UpdateTeamInfo()
{
	if (cmb_commander) {
		cmb_commander->ClearItems();
		cmb_commander->AddItem("");
		cmb_commander->SetSelection(0);

		if (mission && elem) {
			ListIter<MissionElement> iter = mission->GetElements();
			while (++iter) {
				MissionElement* e = iter.value();

				if (CanCommand(e, elem)) {
					cmb_commander->AddItem(e->Name());

					if (elem->Commander() == e->Name())
					cmb_commander->SetSelection(cmb_commander->NumItems()-1);
				}
			}
		}
	}

	if (cmb_squadron) {
		cmb_squadron->ClearItems();
		cmb_squadron->AddItem("");
		cmb_squadron->SetSelection(0);

		if (mission && elem) {
			ListIter<MissionElement> iter = mission->GetElements();
			while (++iter) {
				MissionElement* e = iter.value();

				if (e->GetIFF() == elem->GetIFF() && e != elem && e->IsSquadron()) {
					cmb_squadron->AddItem(e->Name());

					if (elem->Squadron() == e->Name())
					cmb_squadron->SetSelection(cmb_squadron->NumItems()-1);
				}
			}
		}
	}

	if (cmb_carrier) {
		cmb_carrier->ClearItems();
		cmb_carrier->AddItem("");
		cmb_carrier->SetSelection(0);

		if (mission && elem) {
			ListIter<MissionElement> iter = mission->GetElements();
			while (++iter) {
				MissionElement* e = iter.value();

				if (e->GetIFF() == elem->GetIFF() && e != elem && e->GetDesign() && e->GetDesign()->flight_decks.size()) {
					cmb_carrier->AddItem(e->Name());

					if (elem->Carrier() == e->Name())
					cmb_carrier->SetSelection(cmb_carrier->NumItems()-1);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::OnClassSelect(AWEvent* event)
{
	if (!cmb_class || !cmb_design) return;

	const char* cname   = cmb_class->GetSelectedItem();
	int         classid = Ship::ClassForName(cname);

	cmb_design->ClearItems();

	List<Text> designs;
	ShipDesign::GetDesignList(classid, designs);

	if (designs.size() > 0) {
		const ShipDesign* design = elem->GetDesign();
		bool              found  = false;

		for (int i = 0; i < designs.size(); i++) {
			const char* dsn = designs[i]->data();
			cmb_design->AddItem(dsn);

			if (design && !_stricmp(dsn, design->name)) {
				cmb_design->SetSelection(i);
				found = true;
			}
		}

		if (!found)
		cmb_design->SetSelection(0);
	}
	else {
		cmb_design->AddItem("");
		cmb_design->SetSelection(0);
	}

	OnDesignSelect(0);
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::OnDesignSelect(AWEvent* event)
{
	if (!cmb_design) return;

	ShipDesign* design = 0;

	const char* dname = cmb_design->GetSelectedItem();
	int         load_index = 0;

	if (dname)
	design = ShipDesign::Get(dname);

	if (cmb_loadout) {
		cmb_loadout->ClearItems();

		if (design) {
			MissionLoad* mload = 0;
			
			if (elem && elem->Loadouts().size() > 0)
			mload = elem->Loadouts().at(0);

			const List<ShipLoad>& loadouts = design->loadouts;

			if (loadouts.size() > 0) {
				for (int i = 0; i < loadouts.size(); i++) {
					const ShipLoad* load = loadouts[i];
					if (load->name[0]) {
						cmb_loadout->AddItem(load->name);

						if (mload && mload->GetName() == load->name) {
							load_index = cmb_loadout->NumItems()-1;
						}
					}
				}
			}
		}

		if (cmb_loadout->NumItems() < 1)
		cmb_loadout->AddItem("");

		cmb_loadout->SetSelection(load_index);
	}

	if (cmb_skin) {
		cmb_skin->ClearItems();

		if (design) {
			cmb_skin->AddItem(Game::GetText("MsnDlg.default"));
			cmb_skin->SetSelection(0);

			ListIter<Skin> iter = design->skins;

			while (++iter) {
				Skin* s = iter.value();
				cmb_skin->AddItem(s->Name());

				if (elem && elem->GetSkin() && !strcmp(s->Name(), elem->GetSkin()->Name())) {
					cmb_skin->SetSelection(cmb_skin->NumItems()-1);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::OnObjectiveSelect(AWEvent* event)
{
	if (!cmb_objective || !cmb_target) return;

	Instruction* instr = 0;
	if (elem->Objectives().size() > 0)
	instr = elem->Objectives().at(0);

	int objid = cmb_objective->GetSelectedIndex() - 1;

	cmb_target->ClearItems();
	cmb_target->AddItem("");

	if (mission) {
		ListIter<MissionElement> iter = mission->GetElements();
		while (++iter) {
			MissionElement* e = iter.value();

			if (e != elem) {
				bool add = false;

				if (objid < Instruction::PATROL)
				add = e->GetIFF() == 0 || e->GetIFF() == elem->GetIFF();
				else
				add = e->GetIFF() != elem->GetIFF();

				if (add) {
					cmb_target->AddItem(e->Name());

					if (instr && !_stricmp(instr->TargetName(), e->Name()))
					cmb_target->SetSelection(cmb_target->NumItems()-1);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::OnIFFChange(AWEvent* event)
{
	if (edt_iff && elem) {
		int elem_iff = 0;
		sscanf_s(edt_iff->GetText().data(), "%d", &elem_iff);

		if (elem->GetIFF() == elem_iff)
		return;

		elem->SetIFF(elem_iff);
	}

	UpdateTeamInfo();

	if (cmb_target)
	OnObjectiveSelect(0);
}

// +--------------------------------------------------------------------+

void
MsnElemDlg::OnAccept(AWEvent* event)
{
	if (mission && elem) {
		char buf[64];
		int  val;

		if (edt_name) {
			elem->SetName(edt_name->GetText());
		}

		if (edt_size) {
			strcpy_s(buf, edt_size->GetText());

			if (isdigit(*buf))
			sscanf_s(buf, "%d", &val);
			else
			val = 1;

			elem->SetCount(val);
		}

		if (edt_iff) {
			strcpy_s(buf, edt_iff->GetText());

			if (isdigit(*buf))
			sscanf_s(buf, "%d", &val);
			else
			val = 1;

			elem->SetIFF(val);
		}

		if (edt_loc_x && edt_loc_y && edt_loc_z) {
			Point loc;

			strcpy_s(buf, edt_loc_x->GetText());

			if (isdigit(*buf) || *buf == '-')
			sscanf_s(buf, "%d", &val);
			else
			val = 0;

			loc.x = val * 1000;

			strcpy_s(buf, edt_loc_y->GetText());

			if (isdigit(*buf) || *buf == '-')
			sscanf_s(buf, "%d", &val);
			else
			val = 0;

			loc.y = val * 1000;

			strcpy_s(buf, edt_loc_z->GetText());

			if (isdigit(*buf) || *buf == '-')
			sscanf_s(buf, "%d", &val);
			else
			val = 0;

			loc.z = val * 1000;

			elem->SetLocation(loc);
		}

		if (edt_respawns) {
			strcpy_s(buf, edt_respawns->GetText());

			if (isdigit(*buf))
			sscanf_s(buf, "%d", &val);
			else
			val = 0;

			elem->SetRespawnCount(val);
		}

		if (edt_hold_time) {
			strcpy_s(buf, edt_hold_time->GetText());

			if (isdigit(*buf))
			sscanf_s(buf, "%d", &val);
			else
			val = 0;

			elem->SetHoldTime(val);
		}

		if (btn_player) {
			if (btn_player->GetButtonState() > 0) {
				mission->SetPlayer(elem);
			}
			else {
				elem->SetPlayer(0);
			}
		}

		if (btn_playable)
		elem->SetPlayable(btn_playable->GetButtonState() ? true : false);

		if (btn_alert)
		elem->SetAlert(btn_alert->GetButtonState() ? true : false);

		if (btn_command_ai)
		elem->SetCommandAI(btn_command_ai->GetButtonState() ? 1 : 0);

		if (cmb_design) {
			ShipDesign* d = ShipDesign::Get(cmb_design->GetSelectedItem());

			if (d) {
				elem->SetDesign(d);

				if (cmb_skin) {
					const char* skin_name = cmb_skin->GetSelectedItem();

					if (strcmp(skin_name, Game::GetText("MsnDlg.default").data())) {
						elem->SetSkin(d->FindSkin(skin_name));
					}
					else {
						elem->SetSkin(0);
					}
				}
			}
		}

		if (cmb_role) {
			elem->SetMissionRole(cmb_role->GetSelectedIndex());
		}

		if (cmb_region) {
			elem->SetRegion(cmb_region->GetSelectedItem());

			if (elem->Player() > 0) {
				mission->SetRegion(cmb_region->GetSelectedItem());
			}
		}

		if (cmb_heading) {
			switch (cmb_heading->GetSelectedIndex()) {
			default:
			case 0:  elem->SetHeading(0);       break;
			case 1:  elem->SetHeading(PI/2);    break;
			case 2:  elem->SetHeading(PI);      break;
			case 3:  elem->SetHeading(3*PI/2);  break;
			}
		}

		if (cmb_commander) {
			elem->SetCommander(cmb_commander->GetSelectedItem());
		}

		if (cmb_squadron) {
			elem->SetSquadron(cmb_squadron->GetSelectedItem());
		}

		if (cmb_carrier) {
			elem->SetCarrier(cmb_carrier->GetSelectedItem());
		}

		if (cmb_intel) {
			elem->SetIntelLevel(Intel::IntelFromName(cmb_intel->GetSelectedItem()));
		}

		if (cmb_loadout && cmb_loadout->NumItems()) {
			elem->Loadouts().destroy();

			const char* loadname = cmb_loadout->GetSelectedItem();
			if (loadname && *loadname) {
				MissionLoad* mload = new(__FILE__,__LINE__) MissionLoad(-1, cmb_loadout->GetSelectedItem());
				elem->Loadouts().append(mload);
			}
		}

		if (cmb_objective && cmb_target) {
			List<Instruction>& objectives = elem->Objectives();
			objectives.destroy();

			int         action = cmb_objective->GetSelectedIndex() - 1;
			const char* target = cmb_target->GetSelectedItem();

			if (action >= Instruction::VECTOR) {
				Instruction* obj   = new(__FILE__,__LINE__) Instruction(action, target);
				objectives.append(obj);
			}
		}

		if (elem->Player()) {
			mission->SetTeam(elem->GetIFF());
		}
	}

	manager->ShowMsnEditDlg();
}

void
MsnElemDlg::OnCancel(AWEvent* event)
{
	manager->ShowMsnEditDlg();
}

bool
MsnElemDlg::CanCommand(const MissionElement* commander,
const MissionElement* subordinate) const
{
	if (mission && commander && subordinate && commander != subordinate) {
		if (commander->GetIFF() != subordinate->GetIFF())
		return false;

		if (commander->IsSquadron())
		return false;

		if (commander->Commander().length() == 0)
		return true;

		if (subordinate->Name() == commander->Commander())
		return false;

		MissionElement* elem = mission->FindElement(commander->Commander());

		if (elem)
		return CanCommand(elem, subordinate);
	}

	return false;
}