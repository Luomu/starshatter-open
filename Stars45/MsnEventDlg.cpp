/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MsnEventDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mod Config Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnEventDlg.h"
#include "MsnEditDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "MissionEvent.h"
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

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(MsnEventDlg, OnEventSelect);
DEF_MAP_CLIENT(MsnEventDlg, OnAccept);
DEF_MAP_CLIENT(MsnEventDlg, OnCancel);

// +--------------------------------------------------------------------+

MsnEventDlg::MsnEventDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
btn_accept(0), btn_cancel(0), mission(0), event(0)
{
	Init(def);
}

MsnEventDlg::~MsnEventDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::RegisterControls()
{
	btn_accept     = (Button*) FindControl(  1);
	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_accept,    MsnEventDlg, OnAccept);

	btn_cancel     = (Button*) FindControl(  2);
	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_cancel,    MsnEventDlg, OnCancel);

	lbl_id             =              FindControl(201);
	edt_time           = (EditBox*)   FindControl(202);
	edt_delay          = (EditBox*)   FindControl(203);
	cmb_event          = (ComboBox*)  FindControl(204);
	cmb_event_ship     = (ComboBox*)  FindControl(205);
	cmb_event_source   = (ComboBox*)  FindControl(206);
	cmb_event_target   = (ComboBox*)  FindControl(207);
	edt_event_param    = (EditBox*)   FindControl(208);
	edt_event_chance   = (EditBox*)   FindControl(220);
	edt_event_sound    = (EditBox*)   FindControl(209);
	edt_event_message  = (EditBox*)   FindControl(210);

	cmb_trigger        = (ComboBox*)  FindControl(221);
	cmb_trigger_ship   = (ComboBox*)  FindControl(222);
	cmb_trigger_target = (ComboBox*)  FindControl(223);
	edt_trigger_param  = (EditBox*)   FindControl(224);

	if (cmb_event)
	REGISTER_CLIENT(EID_SELECT, cmb_event, MsnEventDlg, OnEventSelect);
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::Show()
{
	FormWindow::Show();

	if (!event) return;

	FillShipList(cmb_event_ship,     event->EventShip());
	FillShipList(cmb_event_source,   event->EventSource());

	if (event->Event() == MissionEvent::JUMP)
	FillRgnList(cmb_event_target, event->EventTarget());
	else
	FillShipList(cmb_event_target,event->EventTarget());

	FillShipList(cmb_trigger_ship,   event->TriggerShip());
	FillShipList(cmb_trigger_target, event->TriggerTarget());

	char buf[64];

	sprintf(buf, "%d", event->EventID());
	if (lbl_id) lbl_id->SetText(buf);

	if (edt_time) {
		sprintf(buf, "%.1f", event->Time());
		edt_time->SetText(buf);
	}

	if (edt_delay) {
		sprintf(buf, "%.1f", event->Delay());
		edt_delay->SetText(buf);
	}

	if (edt_event_chance) {
		sprintf(buf, "%d", event->EventChance());
		edt_event_chance->SetText(buf);
	}

	sprintf(buf, "%d", event->EventParam());
	if (edt_event_param)  edt_event_param->SetText(buf);

	if (edt_trigger_param) 
	edt_trigger_param->SetText(event->TriggerParamStr());

	if (edt_event_message)
	edt_event_message->SetText(event->EventMessage());

	if (edt_event_sound)
	edt_event_sound->SetText(event->EventSound());

	if (cmb_event) {
		cmb_event->ClearItems();

		for (int i = 0; i < MissionEvent::NUM_EVENTS; i++) {
			cmb_event->AddItem(MissionEvent::EventName(i));
		}

		cmb_event->SetSelection(event->Event());
	}

	if (cmb_trigger) {
		cmb_trigger->ClearItems();

		for (int i = 0; i < MissionEvent::NUM_TRIGGERS; i++) {
			cmb_trigger->AddItem(MissionEvent::TriggerName(i));
		}

		cmb_trigger->SetSelection(event->Trigger());
	}
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::FillShipList(ComboBox* cmb, const char* seln)
{
	if (!cmb) return;
	cmb->ClearItems();

	if (!mission) return;

	int index          = 1;
	int selected_index = 0;
	cmb->AddItem("");

	List<MissionElement>& list = mission->GetElements();
	for (int i = 0; i < list.size(); i++) {
		MissionElement* elem = list[i];

		if (elem->IsSquadron())
		continue;

		if (elem->Count() == 1) {
			cmb->AddItem(elem->Name());

			if (elem->Name() == seln)
			selected_index = index;

			index++;
		}
		else {
			char ship_name[256];

			for (int n = 0; n < elem->Count(); n++) {
				sprintf(ship_name, "%s %d", elem->Name().data(), n+1);
				cmb->AddItem(ship_name);

				if (!stricmp(ship_name, seln))
				selected_index = index;

				index++;
			}
		}
	}

	cmb->SetSelection(selected_index);
}

void
MsnEventDlg::FillRgnList(ComboBox* cmb, const char* seln)
{
	if (!cmb) return;
	cmb->ClearItems();

	if (!mission) return;

	int selected_index = 0;
	int i              = 0;

	ListIter<StarSystem> iter = mission->GetSystemList();
	while (++iter) {
		StarSystem* sys = iter.value();

		ListIter<OrbitalRegion> iter2 = sys->AllRegions();
		while (++iter2) {
			OrbitalRegion* region = iter2.value();

			if (!strcmp(region->Name(), seln))
			selected_index = i;

			cmb->AddItem(region->Name());
			i++;
		}
	}

	cmb->SetSelection(selected_index);
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		if (btn_accept && btn_accept->IsEnabled())
		OnAccept(0);
	}
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::SetMission(Mission* m)
{
	mission = m;
}

void
MsnEventDlg::SetMissionEvent(MissionEvent* e)
{
	event = e;
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::OnEventSelect(AWEvent* e)
{
	if (cmb_event->GetSelectedIndex() == MissionEvent::JUMP)
	FillRgnList(cmb_event_target, event->EventTarget());

	else
	FillShipList(cmb_event_target, event->EventTarget());
}

// +--------------------------------------------------------------------+

void
MsnEventDlg::OnAccept(AWEvent* e)
{
	if (mission && event) {
		char buf[64];
		int  val;

		if (edt_time) {
			strcpy(buf, edt_time->GetText());

			float t = 0;
			sscanf(buf, "%f", &t);

			event->time = t;
		}

		if (edt_delay) {
			strcpy(buf, edt_delay->GetText());

			float t = 0;
			sscanf(buf, "%f", &t);

			event->delay = t;
		}

		if (edt_event_param) {
			strcpy(buf, edt_event_param->GetText());

			if (isdigit(*buf)) {
				sscanf(buf, "%d", &val);
				event->event_param[0] = val;
				event->event_nparams = 1;
			}
			else if (*buf == '(') {
				Parser parser(new(__FILE__,__LINE__) BlockReader(buf));
				Term*  term = parser.ParseTerm();

				if (term && term->isArray()) {
					TermArray* val = term->isArray();
					if (val) {
						int nelem = val->elements()->size();

						if (nelem > 10)
						nelem = 10;

						for (int i = 0; i < nelem; i++)
						event->event_param[i] = (int) (val->elements()->at(i)->isNumber()->value());

						event->event_nparams = nelem;
					}
				}
			}
		}

		if (edt_event_chance) {
			strcpy(buf, edt_event_chance->GetText());

			if (isdigit(*buf)) {
				sscanf(buf, "%d", &val);
			}
			else {
				val = 0;
			}

			event->event_chance = val;
		}

		if (edt_event_message)
		event->event_message = edt_event_message->GetText();

		if (edt_event_sound)
		event->event_sound = edt_event_sound->GetText();

		if (edt_trigger_param) {
			strcpy(buf, edt_trigger_param->GetText());

			ZeroMemory(event->trigger_param, sizeof(event->trigger_param));

			if (isdigit(*buf)) {
				sscanf(buf, "%d", &val);
				event->trigger_param[0] = val;
				event->trigger_nparams = 1;
			}

			else if (*buf == '(') {
				Parser parser(new(__FILE__,__LINE__) BlockReader(buf));
				Term*  term = parser.ParseTerm();

				if (term && term->isArray()) {
					TermArray* val = term->isArray();
					if (val) {
						int nelem = val->elements()->size();

						if (nelem > 10)
						nelem = 10;

						for (int i = 0; i < nelem; i++)
						event->trigger_param[i] = (int) (val->elements()->at(i)->isNumber()->value());

						event->trigger_nparams = nelem;
					}
				}
			}

		}

		if (cmb_event)
		event->event = cmb_event->GetSelectedIndex();

		if (cmb_event_ship)
		event->event_ship = cmb_event_ship->GetSelectedItem();

		if (cmb_event_source)
		event->event_source = cmb_event_source->GetSelectedItem();

		if (cmb_event_target)
		event->event_target = cmb_event_target->GetSelectedItem();

		if (cmb_trigger)
		event->trigger = cmb_trigger->GetSelectedIndex();

		if (cmb_trigger_ship)
		event->trigger_ship = cmb_trigger_ship->GetSelectedItem();

		if (cmb_trigger_target)
		event->trigger_target = cmb_trigger_target->GetSelectedItem();
	}

	manager->ShowMsnEditDlg();
}

void
MsnEventDlg::OnCancel(AWEvent* event)
{
	manager->ShowMsnEditDlg();
}
