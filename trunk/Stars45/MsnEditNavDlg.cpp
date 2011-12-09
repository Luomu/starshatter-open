/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnEditNavDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Briefing Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnEditNavDlg.h"
#include "MsnEditDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Galaxy.h"
#include "Instruction.h"
#include "Mission.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(MsnEditNavDlg, OnCommit);
DEF_MAP_CLIENT(MsnEditNavDlg, OnCancel);
DEF_MAP_CLIENT(MsnEditNavDlg, OnTabButton);
DEF_MAP_CLIENT(MsnEditNavDlg, OnSystemSelect);

// +--------------------------------------------------------------------+

MsnEditNavDlg::MsnEditNavDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: NavDlg(s, def, mgr), menu_screen(mgr), mission_info(0),
btn_accept(0), btn_cancel(0), btn_sit(0), btn_pkg(0), btn_map(0)
{
	RegisterControls();
}

MsnEditNavDlg::~MsnEditNavDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnEditNavDlg::RegisterControls()
{
	btn_accept     = (Button*) FindControl(  1);
	btn_cancel     = (Button*) FindControl(  2);
	btn_sit        = (Button*) FindControl(301);
	btn_pkg        = (Button*) FindControl(302);
	btn_map        = (Button*) FindControl(303);

	txt_name       = (EditBox*)  FindControl(201);
	cmb_type       = (ComboBox*) FindControl(202);
	cmb_system     = (ComboBox*) FindControl(203);
	cmb_region     = (ComboBox*) FindControl(204);

	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_accept, MsnEditNavDlg, OnCommit);

	if (btn_cancel)
	REGISTER_CLIENT(EID_CLICK,    btn_cancel, MsnEditNavDlg, OnCancel);

	if (btn_sit)
	REGISTER_CLIENT(EID_CLICK,    btn_sit,    MsnEditNavDlg, OnTabButton);

	if (btn_pkg)
	REGISTER_CLIENT(EID_CLICK,    btn_pkg,    MsnEditNavDlg, OnTabButton);

	if (btn_map)
	REGISTER_CLIENT(EID_CLICK,    btn_map,    MsnEditNavDlg, OnTabButton);

	if (cmb_system)
	REGISTER_CLIENT(EID_SELECT,   cmb_system, MsnEditNavDlg, OnSystemSelect);
}

// +--------------------------------------------------------------------+

void
MsnEditNavDlg::Show()
{
	bool need_tab_update = !shown;

	NavDlg::Show();

	if (txt_name && cmb_type) {
		txt_name->SetText("");

		if (cmb_system) {
			cmb_system->ClearItems();

			Galaxy* galaxy = Galaxy::GetInstance();
			ListIter<StarSystem> iter = galaxy->GetSystemList();
			while (++iter) {
				cmb_system->AddItem(iter->Name());
			}
		}

		if (mission) {
			int i;

			txt_name->SetText(mission->Name());
			cmb_type->SetSelection(mission->Type());

			StarSystem* sys = mission->GetStarSystem();
			if (sys && cmb_system && cmb_region) {
				for (i = 0; i < cmb_system->NumItems(); i++) {
					if (!strcmp(cmb_system->GetItem(i), sys->Name())) {
						cmb_system->SetSelection(i);
						break;
					}
				}

				cmb_region->ClearItems();
				int sel_rgn = 0;

				List<OrbitalRegion> regions;
				regions.append(sys->AllRegions());
				regions.sort();

				i = 0;
				ListIter<OrbitalRegion> iter = regions;
				while (++iter) {
					OrbitalRegion* region = iter.value();
					cmb_region->AddItem(region->Name());

					if (!strcmp(mission->GetRegion(), region->Name())) {
						sel_rgn = i;
					}

					i++;
				}

				cmb_region->SetSelection(sel_rgn);
			}
		}
	}

	if (need_tab_update) {
		ShowTab(2);
	}

	exit_latch = true;
}

void
MsnEditNavDlg::SetMissionInfo(MissionInfo* m)
{
	mission_info = m;
}

// +--------------------------------------------------------------------+

void
MsnEditNavDlg::ScrapeForm()
{
	if (mission) {
		if (txt_name) {
			mission->SetName(txt_name->GetText());
		}

		if (cmb_type) {
			mission->SetType(cmb_type->GetSelectedIndex());

			if (mission_info)
			mission_info->type = cmb_type->GetSelectedIndex();
		}

		Galaxy*     galaxy = Galaxy::GetInstance();
		StarSystem* system = 0;

		if (galaxy)
		system = galaxy->GetSystem(cmb_system->GetSelectedItem());

		if (cmb_system && system) {
			mission->ClearSystemList();
			mission->SetStarSystem(system);

			if (mission_info)
			mission_info->system = system->Name();
		}

		if (cmb_region) {
			mission->SetRegion(cmb_region->GetSelectedItem());

			if (mission_info)
			mission_info->region = cmb_region->GetSelectedItem();
		}

		SetSystem(system);
	}
}

// +--------------------------------------------------------------------+

void
MsnEditNavDlg::ShowTab(int tab)
{
	if (tab < 0 || tab > 2)
	tab = 0;

	if (btn_sit)   btn_sit->SetButtonState(tab == 0 ? 1 : 0);
	if (btn_pkg)   btn_pkg->SetButtonState(tab == 1 ? 1 : 0);
	if (btn_map)   btn_map->SetButtonState(tab == 2 ? 1 : 0);

	if (tab != 2) {
		MsnEditDlg* msnEditDlg = menu_screen->GetMsnEditDlg();

		if (msnEditDlg)
		msnEditDlg->ShowTab(tab);

		menu_screen->ShowMsnEditDlg();
	}
}

// +--------------------------------------------------------------------+

void
MsnEditNavDlg::OnTabButton(AWEvent* event)
{
	if (!event) return;

	if (event->window == btn_sit)
	ShowTab(0);

	else if (event->window == btn_pkg)
	ShowTab(1);

	else if (event->window == btn_map)
	ShowTab(2);
}

void
MsnEditNavDlg::OnSystemSelect(AWEvent* event)
{
	StarSystem* sys = 0;

	if (cmb_system) {
		const char* name = cmb_system->GetSelectedItem();

		Galaxy* galaxy = Galaxy::GetInstance();
		ListIter<StarSystem> iter = galaxy->GetSystemList();
		while (++iter) {
			StarSystem* s = iter.value();

			if (!strcmp(s->Name(), name)) {
				sys = s;
				break;
			}
		}
	}

	if (sys && cmb_region) {
		cmb_region->ClearItems();

		List<OrbitalRegion> regions;
		regions.append(sys->AllRegions());
		regions.sort();

		ListIter<OrbitalRegion> iter = regions;
		while (++iter) {
			OrbitalRegion* region = iter.value();
			cmb_region->AddItem(region->Name());
		}
	}

	ScrapeForm();
}


// +--------------------------------------------------------------------+

void
MsnEditNavDlg::OnCommit(AWEvent* event)
{
	if (mission) {
		ScrapeForm();

		if (mission_info)
		mission_info->name = mission->Name();

		mission->Save();
	}

	menu_screen->ShowMsnSelectDlg();
}

void
MsnEditNavDlg::OnCancel(AWEvent* event)
{
	if (mission)
	mission->Load();

	menu_screen->ShowMsnSelectDlg();
}
