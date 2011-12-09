/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnPkgDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Briefing Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnPkgDlg.h"
#include "PlanScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"

#include "Game.h"
#include "Mouse.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "Keyboard.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(MsnPkgDlg, OnPackage);
DEF_MAP_CLIENT(MsnPkgDlg, OnCommit);
DEF_MAP_CLIENT(MsnPkgDlg, OnCancel);
DEF_MAP_CLIENT(MsnPkgDlg, OnTabButton);

// +--------------------------------------------------------------------+

MsnPkgDlg::MsnPkgDlg(Screen* s, FormDef& def, PlanScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), MsnDlg(mgr)
{
	campaign = Campaign::GetCampaign();

	if (campaign)
	mission = campaign->GetMission();

	Init(def);
}

MsnPkgDlg::~MsnPkgDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::RegisterControls()
{
	pkg_list                   = (ListBox*) FindControl(320);
	nav_list                   = (ListBox*) FindControl(330);

	for (int i = 0; i < 5; i++)
	threat[i]         = FindControl(251 + i);

	RegisterMsnControls(this);

	if (pkg_list)
	REGISTER_CLIENT(EID_SELECT, pkg_list, MsnPkgDlg, OnPackage);

	if (commit)
	REGISTER_CLIENT(EID_CLICK, commit, MsnPkgDlg, OnCommit);

	if (cancel)
	REGISTER_CLIENT(EID_CLICK, cancel, MsnPkgDlg, OnCancel);

	if (sit_button)
	REGISTER_CLIENT(EID_CLICK, sit_button, MsnPkgDlg, OnTabButton);

	if (pkg_button)
	REGISTER_CLIENT(EID_CLICK, pkg_button, MsnPkgDlg, OnTabButton);

	if (nav_button)
	REGISTER_CLIENT(EID_CLICK, nav_button, MsnPkgDlg, OnTabButton);

	if (wep_button)
	REGISTER_CLIENT(EID_CLICK, wep_button, MsnPkgDlg, OnTabButton);
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::Show()
{
	FormWindow::Show();
	ShowMsnDlg();

	DrawPackages();
	DrawNavPlan();
	DrawThreats();
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::DrawPackages()
{
	if (mission) {
		if (pkg_list) {
			pkg_list->ClearItems();

			int i = 0;
			int elem_index = 0;
			ListIter<MissionElement> elem = mission->GetElements();
			while (++elem) {
				// display this element?
				if (elem->GetIFF() == mission->Team()        && 
						!elem->IsSquadron()                      &&
						elem->Region() == mission->GetRegion()   &&
						elem->GetDesign()->type < Ship::STATION) {

					char txt[256];

					if (elem->Player() > 0) {
						sprintf_s(txt, "==>");
						if (pkg_index < 0)
						pkg_index = elem_index;
					}
					else {
						strcpy_s(txt, " ");
					}

					pkg_list->AddItemWithData(txt, elem->ElementID());
					pkg_list->SetItemText(i, 1, elem->Name());
					pkg_list->SetItemText(i, 2, elem->RoleName());

					const ShipDesign* design = elem->GetDesign();

					if (elem->Count() > 1)
					sprintf_s(txt, "%d %s", elem->Count(), design->abrv);
					else
					sprintf_s(txt, "%s %s", design->abrv, design->name);
					pkg_list->SetItemText(i, 3, txt);

					i++;
				}

				elem_index++;
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::DrawNavPlan()
{
	if (mission) {
		if (pkg_index < 0 || pkg_index >= mission->GetElements().size())
		pkg_index = 0;

		MissionElement* element = mission->GetElements()[pkg_index];
		if (nav_list && element) {
			nav_list->ClearItems();

			Point loc          = element->Location();
			int   i            = 0;

			ListIter<Instruction> navpt = element->NavList();
			while (++navpt) {
				char txt[256];
				sprintf_s(txt, "%d", i + 1);

				nav_list->AddItem(txt);
				nav_list->SetItemText(i, 1, Instruction::ActionName(navpt->Action()));
				nav_list->SetItemText(i, 2, navpt->RegionName());

				double dist = Point(loc - navpt->Location()).length();
				FormatNumber(txt, dist);
				nav_list->SetItemText(i, 3, txt);

				sprintf_s(txt, "%d", navpt->Speed());
				nav_list->SetItemText(i, 4, txt);

				loc = navpt->Location();
				i++;
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::DrawThreats()
{
	for (int i = 0; i < 5; i++)
	if (threat[i])
	threat[i]->SetText("");

	if (!mission) return;

	MissionElement* player = mission->GetPlayer();
	Text            rgn0   = player->Region();
	Text            rgn1;
	int             iff    = player->GetIFF();

	if (!player)
	return;

	ListIter<Instruction> nav = player->NavList();
	while (++nav) {
		if (rgn0 != nav->RegionName())
		rgn1 = nav->RegionName();
	}

	if (threat[0]) {
		Point base_loc = mission->GetElements()[0]->Location();

		int i = 0;
		ListIter<MissionElement> iter = mission->GetElements();
		while (++iter) {
			MissionElement* elem = iter.value();

			if (elem->GetIFF() == 0 || elem->GetIFF() == iff || elem->IntelLevel() <= Intel::SECRET)
			continue;

			if (elem->IsSquadron())
			continue;

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
					mission_role >= Mission::TRANSPORT)
			continue;

			char   rng[32];
			char   role[32];
			char   txt[256];

			if (mission_role == Mission::SWEEP     ||
					mission_role == Mission::INTERCEPT ||
					mission_role == Mission::FLEET     ||
					mission_role == Mission::BOMBARDMENT)
			strcpy_s(role, Game::GetText("MsnDlg.ATTACK").data());
			else
			strcpy_s(role, Game::GetText("MsnDlg.PATROL").data());

			double dist = Point(base_loc - elem->Location()).length();
			FormatNumber(rng, dist);

			sprintf_s(txt, "%s - %d %s - %s",  role, 
			elem->Count(), 
			elem->GetDesign()->abrv,
			rng);
			if (threat[i])
			threat[i]->SetText(txt);

			i++;

			if (i >= 5)
			break;
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		OnCommit(0);
	}
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::OnPackage(AWEvent* event)
{
	if (!pkg_list || !mission)
	return;

	int seln = pkg_list->GetListIndex();
	int pkg  = pkg_list->GetItemData(seln);

	int i = 0;
	ListIter<MissionElement> elem = mission->GetElements();
	while (++elem) {
		if (elem->ElementID() == pkg) {
			pkg_index = i;
			//mission->SetPlayer(elem.value());
		}

		i++;
	}

	//DrawPackages();
	DrawNavPlan();
}

// +--------------------------------------------------------------------+

void
MsnPkgDlg::OnCommit(AWEvent* event)
{
	MsnDlg::OnCommit(event);
}

void
MsnPkgDlg::OnCancel(AWEvent* event)
{
	MsnDlg::OnCancel(event);
}

void
MsnPkgDlg::OnTabButton(AWEvent* event)
{
	MsnDlg::OnTabButton(event);
}
