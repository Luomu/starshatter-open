/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright (C) 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnEditDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Editor Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnEditDlg.h"
#include "MsnElemDlg.h"
#include "MsnEventDlg.h"
#include "NavDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "MissionEvent.h"
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
#include "FormatUtil.h"
#include "Random.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(MsnEditDlg, OnAccept);
DEF_MAP_CLIENT(MsnEditDlg, OnCancel);
DEF_MAP_CLIENT(MsnEditDlg, OnTabButton);
DEF_MAP_CLIENT(MsnEditDlg, OnSystemSelect);
DEF_MAP_CLIENT(MsnEditDlg, OnElemAdd);
DEF_MAP_CLIENT(MsnEditDlg, OnElemEdit);
DEF_MAP_CLIENT(MsnEditDlg, OnElemDel);
DEF_MAP_CLIENT(MsnEditDlg, OnElemSelect);
DEF_MAP_CLIENT(MsnEditDlg, OnElemInc);
DEF_MAP_CLIENT(MsnEditDlg, OnElemDec);
DEF_MAP_CLIENT(MsnEditDlg, OnEventAdd);
DEF_MAP_CLIENT(MsnEditDlg, OnEventEdit);
DEF_MAP_CLIENT(MsnEditDlg, OnEventDel);
DEF_MAP_CLIENT(MsnEditDlg, OnEventSelect);
DEF_MAP_CLIENT(MsnEditDlg, OnEventInc);
DEF_MAP_CLIENT(MsnEditDlg, OnEventDec);

// +--------------------------------------------------------------------+

MsnEditDlg::MsnEditDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
btn_accept(0), btn_cancel(0),
btn_elem_add(0),  btn_elem_edit(0),  btn_elem_del(0),  btn_elem_inc(0),  btn_elem_dec(0),
btn_event_add(0), btn_event_edit(0), btn_event_del(0), btn_event_inc(0), btn_event_dec(0),
btn_sit(0), btn_pkg(0), btn_map(0),
txt_name(0), cmb_type(0), cmb_system(0), cmb_region(0),
txt_description(0), txt_situation(0), txt_objective(0),
lst_elem(0), lst_event(0), mission(0), mission_info(0), current_tab(0),
exit_latch(true)
{
	Init(def);
}

MsnEditDlg::~MsnEditDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::SetMission(Mission* m)
{
	mission      = m;
	current_tab  = 0;
}

void
MsnEditDlg::SetMissionInfo(MissionInfo* m)
{
	mission_info = m;
	current_tab  = 0;
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::RegisterControls()
{
	btn_accept     = (Button*) FindControl(  1);
	btn_cancel     = (Button*) FindControl(  2);
	btn_sit        = (Button*) FindControl(301);
	btn_pkg        = (Button*) FindControl(302);
	btn_map        = (Button*) FindControl(303);

	btn_elem_add   = (Button*)   FindControl(501);
	btn_elem_edit  = (Button*)   FindControl(505);
	btn_elem_del   = (Button*)   FindControl(502);
	btn_elem_inc   = (Button*)   FindControl(503);
	btn_elem_dec   = (Button*)   FindControl(504);

	btn_event_add  = (Button*)   FindControl(511);
	btn_event_edit = (Button*)   FindControl(515);
	btn_event_del  = (Button*)   FindControl(512);
	btn_event_inc  = (Button*)   FindControl(513);
	btn_event_dec  = (Button*)   FindControl(514);

	txt_name       = (EditBox*)  FindControl(201);
	cmb_type       = (ComboBox*) FindControl(202);
	cmb_system     = (ComboBox*) FindControl(203);
	cmb_region     = (ComboBox*) FindControl(204);

	txt_description= (EditBox*)  FindControl(410);
	txt_situation  = (EditBox*)  FindControl(411);
	txt_objective  = (EditBox*)  FindControl(412);

	lst_elem       = (ListBox*)  FindControl(510);
	lst_event      = (ListBox*)  FindControl(520);

	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_accept,    MsnEditDlg, OnAccept);

	if (btn_cancel)
	REGISTER_CLIENT(EID_CLICK,    btn_cancel,    MsnEditDlg, OnCancel);

	if (btn_elem_add)
	REGISTER_CLIENT(EID_CLICK,    btn_elem_add,  MsnEditDlg, OnElemAdd);

	if (btn_elem_edit)
	REGISTER_CLIENT(EID_CLICK,    btn_elem_edit, MsnEditDlg, OnElemEdit);

	if (btn_elem_del)
	REGISTER_CLIENT(EID_CLICK,    btn_elem_del,  MsnEditDlg, OnElemDel);

	if (btn_elem_inc) {
		char up_arrow[2];
		up_arrow[0] = Font::ARROW_UP;
		up_arrow[1] = 0;
		btn_elem_inc->SetText(up_arrow);
		btn_elem_inc->SetEnabled(false);
		REGISTER_CLIENT(EID_CLICK,    btn_elem_inc,  MsnEditDlg, OnElemInc);
	}

	if (btn_elem_dec) {
		char dn_arrow[2];
		dn_arrow[0] = Font::ARROW_DOWN;
		dn_arrow[1] = 0;
		btn_elem_dec->SetText(dn_arrow);
		btn_elem_dec->SetEnabled(false);
		REGISTER_CLIENT(EID_CLICK,    btn_elem_dec,  MsnEditDlg, OnElemDec);
	}

	if (btn_event_add)
	REGISTER_CLIENT(EID_CLICK,    btn_event_add,  MsnEditDlg, OnEventAdd);

	if (btn_event_edit)
	REGISTER_CLIENT(EID_CLICK,    btn_event_edit, MsnEditDlg, OnEventEdit);

	if (btn_event_del)
	REGISTER_CLIENT(EID_CLICK,    btn_event_del,  MsnEditDlg, OnEventDel);

	if (btn_event_inc) {
		char up_arrow[2];
		up_arrow[0] = Font::ARROW_UP;
		up_arrow[1] = 0;
		btn_event_inc->SetText(up_arrow);
		btn_event_inc->SetEnabled(false);
		REGISTER_CLIENT(EID_CLICK,    btn_event_inc,  MsnEditDlg, OnEventInc);
	}

	if (btn_event_dec) {
		char dn_arrow[2];
		dn_arrow[0] = Font::ARROW_DOWN;
		dn_arrow[1] = 0;
		btn_event_dec->SetText(dn_arrow);
		btn_event_dec->SetEnabled(false);
		REGISTER_CLIENT(EID_CLICK,    btn_event_dec,  MsnEditDlg, OnEventDec);
	}

	if (btn_sit)
	REGISTER_CLIENT(EID_CLICK,    btn_sit,       MsnEditDlg, OnTabButton);

	if (btn_pkg)
	REGISTER_CLIENT(EID_CLICK,    btn_pkg,       MsnEditDlg, OnTabButton);

	if (btn_map)
	REGISTER_CLIENT(EID_CLICK,    btn_map,       MsnEditDlg, OnTabButton);

	if (cmb_system)
	REGISTER_CLIENT(EID_SELECT,   cmb_system,    MsnEditDlg, OnSystemSelect);

	if (lst_elem)
	REGISTER_CLIENT(EID_SELECT,   lst_elem,      MsnEditDlg, OnElemSelect);

	if (lst_event)
	REGISTER_CLIENT(EID_SELECT,   lst_event,     MsnEditDlg, OnEventSelect);
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::Show()
{
	FormWindow::Show();

	if (!txt_name || !cmb_type) return;

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

		if (txt_description && mission_info)
		txt_description->SetText(mission_info->description);

		if (txt_situation)
		txt_situation->SetText(mission->Situation());

		if (txt_objective)
		txt_objective->SetText(mission->Objective());

		DrawPackages();
	}

	ShowTab(current_tab);

	exit_latch = true;
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::DrawPackages()
{
	bool elem_del   = false;
	bool elem_edit  = false;
	bool elem_inc   = false;
	bool elem_dec   = false;

	bool event_del  = false;
	bool event_edit = false;
	bool event_inc  = false;
	bool event_dec  = false;

	if (mission) {
		if (lst_elem) {
			bool cleared = false;
			int  index   = lst_elem->GetSelection();

			if (lst_elem->NumItems() != mission->GetElements().size()) {
				if (lst_elem->NumItems() < mission->GetElements().size())
				index = lst_elem->NumItems();

				lst_elem->ClearItems();
				cleared = true;
			}

			int i = 0;
			ListIter<MissionElement> elem = mission->GetElements();
			while (++elem) {
				char txt[256];

				sprintf_s(txt, "%d", elem->Identity());

				if (cleared) {
					lst_elem->AddItemWithData(txt, elem->ElementID());
				}
				else {
					lst_elem->SetItemText(i, txt);
					lst_elem->SetItemData(i, elem->ElementID());
				}

				sprintf_s(txt, "%d", elem->GetIFF());
				lst_elem->SetItemText(i, 1, txt);
				lst_elem->SetItemText(i, 2, elem->Name());
				lst_elem->SetItemText(i, 4, elem->RoleName());
				lst_elem->SetItemText(i, 5, elem->Region());

				const ShipDesign* design = elem->GetDesign();

				if (design) {
					if (elem->Count() > 1)
					sprintf_s(txt, "%d %s", elem->Count(), design->abrv);
					else
					sprintf_s(txt, "%s %s", design->abrv, design->name);
				}
				else {
					sprintf_s(txt, Game::GetText("MsnDlg.undefined").data());
				}

				lst_elem->SetItemText(i, 3, txt);

				i++;
			}

			int nitems = lst_elem->NumItems();
			if (nitems) {
				if (index >= nitems)
				index = nitems - 1;

				if (index >= 0) {
					lst_elem->SetSelected(index);
					elem_del  = true;
					elem_edit = true;
					elem_inc  = index > 0;
					elem_dec  = index < nitems-1;
				}
			}
		}

		if (lst_event) {
			bool cleared = false;
			int  index   = lst_event->GetSelection();

			if (lst_event->NumItems() != mission->GetEvents().size()) {
				if (lst_event->NumItems() < mission->GetEvents().size())
				index = lst_event->NumItems();

				lst_event->ClearItems();
				cleared = true;
			}

			int i = 0;
			ListIter<MissionEvent> event = mission->GetEvents();
			while (++event) {
				char txt[256];

				sprintf_s(txt, "%d", event->EventID());
				if (cleared) {
					lst_event->AddItemWithData(txt, event->EventID());
				}
				else {
					lst_event->SetItemText(i, txt);
					lst_event->SetItemData(i, event->EventID());
				}

				if (event->Time()) {
					FormatTime(txt, event->Time());
				}
				else if (event->Delay()) {
					txt[0] = '+';
					txt[1] = ' ';
					FormatTime(txt+2, event->Delay());
				}
				else {
					strcpy_s(txt, " ");
				}

				lst_event->SetItemText(i, 1, txt);
				lst_event->SetItemText(i, 2, event->EventName());
				lst_event->SetItemText(i, 3, event->EventShip());
				lst_event->SetItemText(i, 4, event->EventMessage());

				i++;
			}

			int nitems = lst_event->NumItems();
			if (nitems) {
				if (index >= nitems)
				index = nitems - 1;

				if (index >= 0) {
					lst_event->SetSelected(index);
					event_del  = true;
					event_edit = true;
					event_inc  = index > 0;
					event_dec  = index < nitems-1;
				}
			}
		}
	}

	if (btn_elem_del)    btn_elem_del->SetEnabled(elem_del);
	if (btn_elem_edit)   btn_elem_edit->SetEnabled(elem_edit);
	if (btn_elem_inc)    btn_elem_inc->SetEnabled(elem_inc);
	if (btn_elem_dec)    btn_elem_dec->SetEnabled(elem_del);

	if (btn_event_del)   btn_event_del->SetEnabled(event_del);
	if (btn_event_edit)  btn_event_edit->SetEnabled(event_edit);
	if (btn_event_inc)   btn_event_inc->SetEnabled(event_inc);
	if (btn_event_dec)   btn_event_dec->SetEnabled(event_dec);
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::ExecFrame()
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
MsnEditDlg::ScrapeForm()
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

		if (galaxy && cmb_system)
		system = galaxy->GetSystem(cmb_system->GetSelectedItem());

		if (system) {
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

		if (txt_description && mission_info) {
			mission_info->description = txt_description->GetText();
			mission->SetDescription(txt_description->GetText());
		}

		if (txt_situation)
		mission->SetSituation(txt_situation->GetText());

		if (txt_objective)
		mission->SetObjective(txt_objective->GetText());
	}
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::ShowTab(int tab)
{
	current_tab = tab;

	if (current_tab < 0 || current_tab > 2)
	current_tab = 0;

	if (btn_sit)   btn_sit->SetButtonState(tab == 0 ? 1 : 0);
	if (btn_pkg)   btn_pkg->SetButtonState(tab == 1 ? 1 : 0);
	if (btn_map)   btn_map->SetButtonState(tab == 2 ? 1 : 0);

	DWORD low  = 400 + tab*100;
	DWORD high = low + 100;

	ListIter<ActiveWindow> iter = Controls();
	while (++iter) {
		ActiveWindow* a = iter.value();

		if (a->GetID() < 400)
		a->Show();

		else if (a->GetID() >= low && a->GetID() < high)
		a->Show();

		else
		a->Hide();
	}

	if (tab == 2) {
		NavDlg* navdlg = manager->GetNavDlg();

		if (navdlg) {
			navdlg->SetMission(mission);
			navdlg->SetEditorMode(true);
		}

		manager->ShowNavDlg();
	}
	else {
		manager->HideNavDlg();
	}
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::OnTabButton(AWEvent* event)
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
MsnEditDlg::OnSystemSelect(AWEvent* event)
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

	NavDlg* navdlg = manager->GetNavDlg();

	if (navdlg)
	navdlg->SetMission(mission);
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::OnElemSelect(AWEvent* event)
{
	static DWORD click_time = 0;

	if (lst_elem && mission) {
		int selection = lst_elem->GetSelection();

		if (btn_elem_edit)
		btn_elem_edit->SetEnabled(selection >= 0);

		if (btn_elem_del)
		btn_elem_del->SetEnabled(selection >= 0);

		if (btn_elem_inc)
		btn_elem_inc->SetEnabled(selection > 0);

		if (btn_elem_dec)
		btn_elem_dec->SetEnabled(selection >= 0 && selection < lst_elem->NumItems() - 1);

		// double-click:
		if (Game::RealTime() - click_time < 350) {
			if (lst_elem->GetSelCount() == 1) {
				int               index        = lst_elem->GetSelection();
				MissionElement*   elem         = mission->GetElements().at(index);
				MsnElemDlg*       msn_elem_dlg = manager->GetMsnElemDlg();

				if (elem && msn_elem_dlg) {
					ScrapeForm();
					msn_elem_dlg->SetMission(mission);
					msn_elem_dlg->SetMissionElement(elem);
					manager->ShowMsnElemDlg();
				}
			}
		}
	}

	click_time = Game::RealTime();
}

void
MsnEditDlg::OnElemInc(AWEvent* event)
{
	int index = lst_elem->GetSelection();
	mission->IncreaseElemPriority(index--);

	DrawPackages();
	lst_elem->SetSelected(index);
	btn_elem_edit->SetEnabled(true);
	btn_elem_del->SetEnabled(true);
	btn_elem_inc->SetEnabled(index > 0);
	btn_elem_dec->SetEnabled(index >= 0 && index < lst_elem->NumItems()-1);
}

void
MsnEditDlg::OnElemDec(AWEvent* event)
{
	int index = lst_elem->GetSelection();
	mission->DecreaseElemPriority(index++);

	DrawPackages();
	lst_elem->SetSelected(index);
	btn_elem_edit->SetEnabled(true);
	btn_elem_del->SetEnabled(true);
	btn_elem_inc->SetEnabled(index > 0);
	btn_elem_dec->SetEnabled(index >= 0 && index < lst_elem->NumItems()-1);
}


// +--------------------------------------------------------------------+

void
MsnEditDlg::OnElemAdd(AWEvent* event)
{
	if (lst_elem && mission) {
		List<MissionElement>& elements = mission->GetElements();
		MissionElement*       elem     = new(__FILE__,__LINE__) MissionElement;

		if (elements.size() > 0)
		elem->SetLocation(RandomPoint());

		if (cmb_region)
		elem->SetRegion(cmb_region->GetSelectedItem());

		elements.append(elem);
		DrawPackages();

		MsnElemDlg* msn_elem_dlg = manager->GetMsnElemDlg();

		if (msn_elem_dlg) {
			ScrapeForm();
			msn_elem_dlg->SetMission(mission);
			msn_elem_dlg->SetMissionElement(elem);
			manager->ShowMsnElemDlg();
		}
	}
}

void
MsnEditDlg::OnElemDel(AWEvent* event)
{
	if (lst_elem && mission) {
		List<MissionElement>& elements = mission->GetElements();
		delete elements.removeIndex(lst_elem->GetSelection());
		DrawPackages();
	}
}

void
MsnEditDlg::OnElemEdit(AWEvent* event)
{
	if (lst_elem && mission && lst_elem->GetSelCount() == 1) {
		int               index        = lst_elem->GetSelection();
		MissionElement*   elem         = mission->GetElements().at(index);
		MsnElemDlg*       msn_elem_dlg = manager->GetMsnElemDlg();

		if (elem && msn_elem_dlg) {
			ScrapeForm();
			msn_elem_dlg->SetMission(mission);
			msn_elem_dlg->SetMissionElement(elem);
			manager->ShowMsnElemDlg();
		}
	}
}


// +--------------------------------------------------------------------+

void
MsnEditDlg::OnEventSelect(AWEvent* event)
{
	static DWORD click_time = 0;

	if (lst_event && mission) {
		int selection = lst_event->GetSelection();

		if (btn_event_edit)
		btn_event_edit->SetEnabled(selection >= 0);

		if (btn_event_del)
		btn_event_del->SetEnabled(selection >= 0);

		if (btn_event_inc)
		btn_event_inc->SetEnabled(selection > 0);

		if (btn_event_dec)
		btn_event_dec->SetEnabled(selection >= 0 && selection < lst_event->NumItems() - 1);

		// double-click:
		if (Game::RealTime() - click_time < 350) {
			if (lst_event->GetSelCount() == 1) {
				int               index         = lst_event->GetSelection();
				MissionEvent*     event         = mission->GetEvents().at(index);
				MsnEventDlg*      msn_event_dlg = manager->GetMsnEventDlg();

				if (event && msn_event_dlg) {
					ScrapeForm();
					msn_event_dlg->SetMission(mission);
					msn_event_dlg->SetMissionEvent(event);
					manager->ShowMsnEventDlg();
				}
			}
		}
	}

	click_time = Game::RealTime();
}

void
MsnEditDlg::OnEventInc(AWEvent* event)
{
	int index = lst_event->GetSelection();
	mission->IncreaseEventPriority(index--);

	DrawPackages();
	lst_event->SetSelected(index);
	btn_event_edit->SetEnabled(true);
	btn_event_del->SetEnabled(true);
	btn_event_inc->SetEnabled(index > 0);
	btn_event_dec->SetEnabled(index >= 0 && index < lst_event->NumItems()-1);
}

void
MsnEditDlg::OnEventDec(AWEvent* event)
{
	int index = lst_event->GetSelection();
	mission->DecreaseEventPriority(index++);

	DrawPackages();
	lst_event->SetSelected(index);
	btn_event_edit->SetEnabled(true);
	btn_event_del->SetEnabled(true);
	btn_event_inc->SetEnabled(index > 0);
	btn_event_dec->SetEnabled(index >= 0 && index < lst_event->NumItems()-1);
}


// +--------------------------------------------------------------------+

void
MsnEditDlg::OnEventAdd(AWEvent* event)
{
	if (lst_event && mission) {
		List<MissionEvent>& events = mission->GetEvents();
		MissionEvent*       event     = new(__FILE__,__LINE__) MissionEvent;

		int id = 1;
		for (int i = 0; i < events.size(); i++) {
			MissionEvent* e = events[i];
			if (e->EventID() >= id)
			id = e->EventID() + 1;
		}

		event->id = id;

		events.append(event);
		DrawPackages();

		MsnEventDlg* msn_event_dlg = manager->GetMsnEventDlg();

		if (msn_event_dlg) {
			ScrapeForm();
			msn_event_dlg->SetMission(mission);
			msn_event_dlg->SetMissionEvent(event);
			manager->ShowMsnEventDlg();
		}
	}
}

void
MsnEditDlg::OnEventDel(AWEvent* event)
{
	if (lst_event && mission) {
		List<MissionEvent>& events = mission->GetEvents();
		delete events.removeIndex(lst_event->GetSelection());
		DrawPackages();
	}
}

void
MsnEditDlg::OnEventEdit(AWEvent* event)
{
	if (lst_event && mission && lst_event->GetSelCount() == 1) {
		int               index         = lst_event->GetSelection();
		MissionEvent*     event         = mission->GetEvents().at(index);
		MsnEventDlg*      msn_event_dlg = manager->GetMsnEventDlg();

		if (event && msn_event_dlg) {
			ScrapeForm();
			msn_event_dlg->SetMission(mission);
			msn_event_dlg->SetMissionEvent(event);
			manager->ShowMsnEventDlg();
		}
	}
}

// +--------------------------------------------------------------------+

void
MsnEditDlg::OnAccept(AWEvent* event)
{
	if (mission) {
		ScrapeForm();
		mission_info->name = mission->Name();
		mission->Save();
	}

	manager->ShowMsnSelectDlg();
}

void
MsnEditDlg::OnCancel(AWEvent* event)
{
	if (mission)
	mission->Load();

	manager->ShowMsnSelectDlg();
}
