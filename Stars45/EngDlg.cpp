/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         EngDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Engineering (Power/Maint) Dialog Active Window class
*/

#include "MemDebug.h"
#include "EngDlg.h"
#include "GameScreen.h"
#include "Ship.h"
#include "Power.h"
#include "Component.h"
#include "Sim.h"

#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Game.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(EngDlg, OnSource);
DEF_MAP_CLIENT(EngDlg, OnClient);
DEF_MAP_CLIENT(EngDlg, OnRouteStart);
DEF_MAP_CLIENT(EngDlg, OnRouteComplete);
DEF_MAP_CLIENT(EngDlg, OnPowerOff);
DEF_MAP_CLIENT(EngDlg, OnPowerOn);
DEF_MAP_CLIENT(EngDlg, OnOverride);
DEF_MAP_CLIENT(EngDlg, OnPowerLevel);
DEF_MAP_CLIENT(EngDlg, OnComponent);
DEF_MAP_CLIENT(EngDlg, OnAutoRepair);
DEF_MAP_CLIENT(EngDlg, OnRepair);
DEF_MAP_CLIENT(EngDlg, OnReplace);
DEF_MAP_CLIENT(EngDlg, OnQueue);
DEF_MAP_CLIENT(EngDlg, OnPriorityIncrease);
DEF_MAP_CLIENT(EngDlg, OnPriorityDecrease);
DEF_MAP_CLIENT(EngDlg, OnClose);

// +--------------------------------------------------------------------+

EngDlg::EngDlg(Screen* s, FormDef& def, GameScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
ship(0), route_source(0), selected_source(0),
selected_repair(0), selected_component(0)
{
	Init(def);
}

EngDlg::~EngDlg()
{
}

// +--------------------------------------------------------------------+

void
EngDlg::RegisterControls()
{
	for (int i = 0; i < 4; i++) {
		sources[i] = (Button*)  FindControl(201 + i);
		REGISTER_CLIENT(EID_CLICK, sources[i], EngDlg, OnSource);

		source_levels[i] = (Slider*)  FindControl(211 + i);

		clients[i] = (ListBox*) FindControl(301 + i);

		if (clients[i]) {
			clients[i]->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
			clients[i]->SetSortColumn(0);

			REGISTER_CLIENT(EID_SELECT,     clients[i], EngDlg, OnClient);
			REGISTER_CLIENT(EID_DRAG_START, clients[i], EngDlg, OnRouteStart);
			REGISTER_CLIENT(EID_DRAG_DROP,  clients[i], EngDlg, OnRouteComplete);
		}
	}

	close_btn     = (Button*)       FindControl(1);
	selected_name = (ActiveWindow*) FindControl(401);
	power_off     = (Button*)       FindControl(402);
	power_on      = (Button*)       FindControl(403);
	override      = (Button*)       FindControl(410);
	power_level   = (Slider*)       FindControl(404);
	capacity      = (Slider*)       FindControl(405);

	if (close_btn)
	REGISTER_CLIENT(EID_CLICK, close_btn, EngDlg, OnClose);

	if (power_off)
	REGISTER_CLIENT(EID_CLICK, power_off, EngDlg, OnPowerOff);

	if (power_on)
	REGISTER_CLIENT(EID_CLICK, power_on,  EngDlg, OnPowerOn);

	if (override)
	REGISTER_CLIENT(EID_CLICK, override,  EngDlg, OnOverride);

	if (power_level)
	REGISTER_CLIENT(EID_CLICK, power_level, EngDlg, OnPowerLevel);

	components    = (ListBox*)      FindControl(501);

	if (components) {
		components->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
		components->SetSortColumn(0);
		REGISTER_CLIENT(EID_SELECT, components, EngDlg, OnComponent);
	}

	auto_repair       = (Button*) FindControl(700);
	repair            = (Button*) FindControl(502);
	replace           = (Button*) FindControl(503);
	repair_time       = FindControl(512);
	replace_time      = FindControl(513);
	priority_increase = (Button*) FindControl(602);
	priority_decrease = (Button*) FindControl(603);

	if (auto_repair)
	REGISTER_CLIENT(EID_CLICK, auto_repair, EngDlg, OnAutoRepair);

	if (repair)
	REGISTER_CLIENT(EID_CLICK, repair, EngDlg, OnRepair);

	if (replace)
	REGISTER_CLIENT(EID_CLICK, replace, EngDlg, OnReplace);

	if (repair_time)
	repair_time->Hide();

	if (replace_time)
	replace_time->Hide();

	if (priority_increase) {
		char up_arrow[2];
		up_arrow[0] = Font::ARROW_UP;
		up_arrow[1] = 0;
		priority_increase->SetText(up_arrow);

		REGISTER_CLIENT(EID_CLICK, priority_increase, EngDlg, OnPriorityIncrease);
	}

	if (priority_decrease) {
		char dn_arrow[2];
		dn_arrow[0] = Font::ARROW_DOWN;
		dn_arrow[1] = 0;
		priority_decrease->SetText(dn_arrow);

		REGISTER_CLIENT(EID_CLICK, priority_decrease, EngDlg, OnPriorityDecrease);
	}

	repair_queue    = (ListBox*)      FindControl(601);

	if (repair_queue) {
		repair_queue->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
		REGISTER_CLIENT(EID_SELECT, repair_queue, EngDlg, OnQueue);
	}
}

// +--------------------------------------------------------------------+

void
EngDlg::Show()
{
	FormWindow::Show();

	if (ship) {
		int nsources = ship->Reactors().size();

		for (int i = 0; i < 4; i++) {
			if (i >= nsources) {
				sources[i]->Hide();
				source_levels[i]->Hide();
				clients[i]->Hide();
			}
		}
	}
}

void
EngDlg::Hide()
{
	FormWindow::Hide();
}

// +--------------------------------------------------------------------+

void
EngDlg::SetShip(Ship* s)
{
	if (IsShown() && ship != s) {
		selected_source    = 0;
		selected_repair    = 0;
		selected_component = 0;
		selected_clients.clear();

		ship = s;

		UpdateRouteTables();
		ExecFrame();
	}

	// make sure we clear the ship, even if not shown:
	else if (!s) {
		ship = 0;
	}
}

void
EngDlg::UpdateRouteTables()
{
	for (int i = 0; i < 4; i++)
	clients[i]->ClearSelection();

	if (components)
	components->ClearItems();

	if (priority_increase)
	priority_increase->SetEnabled(false);

	if (priority_decrease)
	priority_decrease->SetEnabled(false);

	if (ship) {
		for (int i = 0; i < 4; i++) {
			if (sources[i])
			sources[i]->Hide();

			if (source_levels[i])
			source_levels[i]->Hide();

			if (clients[i]) {
				clients[i]->ClearItems();
				clients[i]->Hide();
			}
		}

		int                     reactor_index = 0;
		ListIter<PowerSource>   reactor_iter  = ship->Reactors();

		while (++reactor_iter) {
			PowerSource* reactor = reactor_iter.value();

			if (sources[reactor_index] && clients[reactor_index]) {
				sources[reactor_index]->SetText(Game::GetText(reactor->Abbreviation()));
				sources[reactor_index]->Show();

				source_levels[reactor_index]->Show();
				source_levels[reactor_index]->SetValue((int) reactor->GetPowerLevel());

				clients[reactor_index]->Show();

				int               index  = 0;
				ListIter<System>  client = reactor->Clients();
				while (++client) {
					char abrv[64], num[20];
					FormatNumber(num, client->GetPowerLevel());
					strcpy_s(abrv, Game::GetText(client->Name()));

					clients[reactor_index]->AddItemWithData(Game::GetText(abrv), index);
					clients[reactor_index]->SetItemText(index, 1, num);
					clients[reactor_index]->SetItemData(index, 1, (DWORD) client->GetPowerLevel());

					index++;
				}

				clients[reactor_index]->SortItems();
			}

			reactor->RouteScanned();
			reactor_index++;
		}
	}
}

// +--------------------------------------------------------------------+

void
EngDlg::ExecFrame()
{
	if (IsShown())
	UpdateSelection();
}

void
EngDlg::UpdateSelection()
{
	if (!ship) return;

	char num[20];
	int nsources = ship->Reactors().size();

	// update the route tables:
	for (int source_index = 0; source_index < nsources; source_index++) {
		PowerSource* reac = ship->Reactors()[source_index];

		if (reac->RouteChanged())
		UpdateRouteTables();

		Color c(62,106,151);

		if (reac->IsPowerOn()) {
			switch (reac->Status()) {
			default:
			case System::NOMINAL:                              break;
			case System::DEGRADED:     c = Color::Yellow;      break;
			case System::CRITICAL:     c = Color::BrightRed;   break;
			case System::DESTROYED:    c = Color::DarkRed;     break;
			}
		}
		else {
			c = Color::Gray;
		}

		sources[source_index]->SetBackColor(c);
		source_levels[source_index]->SetEnabled(reac->IsPowerOn());
		source_levels[source_index]->SetValue((int) reac->GetPowerLevel());

		ListBox*     client_list = clients[source_index];

		for (int i = 0; i < client_list->NumItems(); i++) {
			int index = client_list->GetItemData(i);

			System* client = reac->Clients()[index];
			FormatNumber(num, client->GetPowerLevel());
			client_list->SetItemText(i, 1, num);
			client_list->SetItemData(i, 1, (DWORD) client->GetPowerLevel());

			if (client->IsPowerOn()) {
				Color c;

				switch (client->Status()) {
				default:
				case System::NOMINAL:      c = Color::White;       break;
				case System::DEGRADED:     c = Color::Yellow;      break;
				case System::CRITICAL:     c = Color::BrightRed;   break;
				case System::DESTROYED:    c = Color::DarkRed;     break;
				}

				client_list->SetItemColor(i, c);
			}
			else {
				client_list->SetItemColor(i, Color::Gray);
			}

		}
	}

	// update the detail info:
	if (selected_source) {
		selected_name->SetText(Game::GetText(selected_source->Name()));
		power_off->SetEnabled(true);
		power_on->SetEnabled(true);
		override->SetEnabled(true);
		if (override->GetButtonState() != 2)
		override->SetButtonState(selected_source->GetPowerLevel() > 100 ? 1 : 0);
		power_level->SetEnabled(selected_source->IsPowerOn());
		power_level->SetValue((int) selected_source->GetPowerLevel());

		if (selected_source->Safety() < 100) {
			power_level->SetMarker((int) selected_source->Safety(), 0);
			power_level->SetMarker((int) selected_source->Safety(), 1);
		}
		else {
			power_level->SetMarker(-1, 0);
			power_level->SetMarker(-1, 1);
		}

		capacity->SetEnabled(true);
		capacity->SetValue((int) selected_source->Charge());

		if (selected_source->IsPowerOn()) {
			if (power_on->GetButtonState() != 2)
			power_on->SetButtonState(1);
			if (power_off->GetButtonState() != 2)
			power_off->SetButtonState(0);
		}
		else {
			if (power_on->GetButtonState() != 2)
			power_on->SetButtonState(0);
			if (power_off->GetButtonState() != 2)
			power_off->SetButtonState(1);
		}

		if (components) {
			for (int i = 0; i < components->NumItems(); i++) {
				int index = components->GetItemData(i);

				Component* comp = selected_source->GetComponents()[index];

				Text stat = "OK";
				Color c;

				switch (comp->Status()) {
				case Component::DESTROYED:
				case Component::CRITICAL:  stat = "FAIL";    c = Color::BrightRed; break;
				case Component::DEGRADED:  stat = "WARN";    c = Color::Yellow;    break;
				case Component::NOMINAL:   stat = "OK";      c = Color::White;     break;
				case Component::REPLACE:
				case Component::REPAIR:    stat = "MAINT";   c = Color::Cyan;      break;
				}

				stat = Game::GetText(Text("EngDlg.") + stat);
				components->SetItemText(i, 1, stat);
				components->SetItemData(i, 1, (int) comp->Status());

				FormatNumber(num, comp->SpareCount());
				components->SetItemText(i, 2, num);
				components->SetItemData(i, 2, comp->SpareCount());
				components->SetItemColor(i, c);
			}
		}
	}

	else if (selected_clients.size() == 1) {
		System* sink = selected_clients[0];

		selected_name->SetText(Game::GetText(sink->Name()));
		power_off->SetEnabled(true);
		power_on->SetEnabled(true);
		override->SetEnabled(true);
		if (override->GetButtonState() != 2)
		override->SetButtonState(sink->GetPowerLevel() > 100 ? 1 : 0);
		power_level->SetEnabled(sink->IsPowerOn());
		power_level->SetValue((int) sink->GetPowerLevel());

		if (sink->Safety() < 100) {
			power_level->SetMarker((int) sink->Safety(), 0);
			power_level->SetMarker((int) sink->Safety(), 1);
		}
		else {
			power_level->SetMarker(-1, 0);
			power_level->SetMarker(-1, 1);
		}

		capacity->SetEnabled(true);
		capacity->SetValue((int) sink->Charge());

		if (sink->IsPowerOn()) {
			if (power_on->GetButtonState() != 2)
			power_on->SetButtonState(1);
			if (power_off->GetButtonState() != 2)
			power_off->SetButtonState(0);
		}
		else {
			if (power_on->GetButtonState() != 2)
			power_on->SetButtonState(0);
			if (power_off->GetButtonState() != 2)
			power_off->SetButtonState(1);
		}

		if (components) {
			for (int i = 0; i < components->NumItems(); i++) {
				int index = components->GetItemData(i);

				Component* comp = sink->GetComponents()[index];

				Text stat = "OK";
				Color c;

				switch (comp->Status()) {
				case Component::DESTROYED:
				case Component::CRITICAL:  stat = "FAIL";    c = Color::BrightRed; break;
				case Component::DEGRADED:  stat = "WARN";    c = Color::Yellow;    break;
				case Component::NOMINAL:   stat = "OK";      c = Color::White;     break;
				case Component::REPLACE:
				case Component::REPAIR:    stat = "MAINT";   c = Color::Cyan;      break;
				}

				stat = Game::GetText(Text("EngDlg.") + stat);
				components->SetItemText(i, 1, stat);
				components->SetItemData(i, 1, (int) comp->Status());

				FormatNumber(num, comp->SpareCount());
				components->SetItemText(i, 2, num);
				components->SetItemData(i, 2, comp->SpareCount());
				components->SetItemColor(i, c);
			}
		}
	}

	else if (selected_clients.size() > 1) {
		System* sink = selected_clients[0];

		selected_name->SetText(Game::GetText("[Multiple]"));
		power_off->SetEnabled(true);
		power_on->SetEnabled(true);
		override->SetEnabled(true);
		if (override->GetButtonState() != 2)
		override->SetButtonState(sink->GetPowerLevel() > 100 ? 1 : 0);
		power_level->SetEnabled(true);
		power_level->SetValue((int) sink->GetPowerLevel());

		if (sink->Safety() < 100) {
			power_level->SetMarker((int) sink->Safety(), 0);
			power_level->SetMarker((int) sink->Safety(), 1);
		}
		else {
			power_level->SetMarker(-1, 0);
			power_level->SetMarker(-1, 1);
		}

		capacity->SetEnabled(true);
		capacity->SetValue((int) sink->Charge());

		if (sink->IsPowerOn()) {
			if (power_on->GetButtonState() != 2)
			power_on->SetButtonState(1);
			if (power_off->GetButtonState() != 2)
			power_off->SetButtonState(0);
		}
		else {
			if (power_on->GetButtonState() != 2)
			power_on->SetButtonState(0);
			if (power_off->GetButtonState() != 2)
			power_off->SetButtonState(1);
		}
	}

	else {
		selected_name->SetText(Game::GetText("No Selection"));
		power_off->SetEnabled(false);
		power_off->SetButtonState(0);
		power_on->SetEnabled(false);
		power_on->SetButtonState(0);
		override->SetEnabled(false);
		override->SetButtonState(0);
		power_level->SetEnabled(false);
		power_level->SetValue(0);
		power_level->SetMarker(-1, 0);
		power_level->SetMarker(-1, 1);
		capacity->SetEnabled(false);
		capacity->SetValue(0);
	}

	// display the repair queue:
	if (repair_queue) {
		// if adding to the queue, dump and reload:
		if (repair_queue->NumItems() < ship->RepairQueue().size()) {
			repair_queue->ClearItems();

			int i = 0;
			ListIter<System> iter = ship->RepairQueue();
			while (++iter) {
				double time_remaining = 0;
				char   etr[20];

				System* sys = iter.value();

				ListIter<Component> comp = sys->GetComponents();
				while (++comp) {
					Component* c = comp.value();
					if (c->TimeRemaining() > time_remaining)
					time_remaining = c->TimeRemaining();
				}

				FormatTime(etr, (int) (time_remaining / ship->RepairSpeed()));
				repair_queue->AddItem(Game::GetText(sys->Name()));
				repair_queue->SetItemText(i, 1, etr);
				i++;
			}
		}

		// otherwise, update in place:
		else {
			while (repair_queue->NumItems() > ship->RepairQueue().size())
			repair_queue->RemoveItem(0);

			bool found = false;

			for (int i = 0; i < repair_queue->NumItems(); i++) {
				double time_remaining = 0;
				char   etr[20];

				System* sys = ship->RepairQueue().at(i);

				ListIter<Component> comp = sys->GetComponents();
				while (++comp) {
					Component* c = comp.value();
					if (c->TimeRemaining() > time_remaining)
					time_remaining = c->TimeRemaining();
				}

				FormatTime(etr, (int) time_remaining);
				repair_queue->SetItemText(i, sys->Name());
				repair_queue->SetItemText(i, 1, etr);

				if (sys == selected_repair) {
					found = true;

					if (!Mouse::LButton())
					repair_queue->SetSelected(i);
				}
			}

			if (!found)
			selected_repair = 0;
		}
	}

	if (auto_repair && auto_repair->GetButtonState() != 2)
	auto_repair->SetButtonState(ship->AutoRepair() ? 1 : 0);
}

// +--------------------------------------------------------------------+

void
EngDlg::OnSource(AWEvent* event)
{
	selected_source = 0;
	selected_clients.clear();
	selected_component = 0;

	if (!ship) return;

	int source_index = -1;

	for (int i = 0; i < 4; i++)
	if (event->window == sources[i])
	source_index = i;

	// found the source list:
	if (source_index >= 0) {
		selected_source = ship->Reactors()[source_index];
	}

	for (int i = 0; i < 4; i++) {
		clients[i]->ClearSelection();
	}

	if (components) {
		components->ClearItems();

		if (repair)       repair->SetEnabled(false);
		if (replace)      replace->SetEnabled(false);
		if (repair_time)  repair_time->Hide();
		if (replace_time) replace_time->Hide();

		if (selected_source && selected_source->GetComponents().size()) {
			int index = 0;
			ListIter<Component> comp = selected_source->GetComponents();
			while (++comp)
			components->AddItemWithData(Game::GetText(comp->Abbreviation()), index++);
		}
	}
}

void
EngDlg::OnClient(AWEvent* event)
{
	selected_source = 0;
	selected_clients.clear();
	selected_component = 0;

	if (!ship) return;

	int source_index = -1;

	for (int i = 0; i < 4; i++) {
		if (event->window == clients[i])
		source_index = i;
		else
		clients[i]->ClearSelection();
	}

	// found the source list:
	if (source_index >= 0) {
		// find the power source:
		PowerSource* src = ship->Reactors()[source_index];

		// build a list of the clients to be manipulated:
		List<System>& client_list = src->Clients();
		for (int i = 0; i < clients[source_index]->NumItems(); i++) {
			if (clients[source_index]->IsSelected(i)) {
				int index = clients[source_index]->GetItemData(i);
				selected_clients.append(client_list[index]);
			}
		}
	}

	if (components) {
		components->ClearItems();

		if (repair)       repair->SetEnabled(false);
		if (replace)      replace->SetEnabled(false);
		if (repair_time)  repair_time->Hide();
		if (replace_time) replace_time->Hide();

		if (selected_clients.size() == 1) {
			System* sink = selected_clients[0];

			if (sink->GetComponents().size()) {
				int index = 0;
				ListIter<Component> comp = sink->GetComponents();
				while (++comp)
				components->AddItemWithData(Game::GetText(comp->Abbreviation()), index++);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
EngDlg::OnRouteStart(AWEvent* event)
{
	if (!ship) return;

	int source_index = -1;

	for (int i = 0; i < 4; i++)
	if (event->window == clients[i])
	source_index = i;

	// found the source list:
	if (source_index >= 0) {
		// remember the power source:
		route_source = ship->Reactors()[source_index];
		route_list.clear();

		// build a list of the clients to be moved:
		List<System>& rsc = route_source->Clients();
		for (int i = 0; i < clients[source_index]->NumItems(); i++) {
			if (clients[source_index]->IsSelected(i)) {
				int index = clients[source_index]->GetItemData(i);
				route_list.append(rsc[index]);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
EngDlg::OnRouteComplete(AWEvent* event)
{
	if (!ship || !route_source) return;

	int dest_index = -1;

	for (int i = 0; i < 4; i++)
	if (event->window == clients[i])
	dest_index = i;

	// found the destination list, copy the clients over:
	if (dest_index >= 0) {
		PowerSource* route_dest = ship->Reactors()[dest_index];

		if (!route_dest)
		return;

		ListIter<System> iter = route_list;
		while (++iter) {
			System* client = iter.value();

			route_source->RemoveClient(client);
			route_dest->AddClient(client);
		}
	}
}

// +--------------------------------------------------------------------+

void
EngDlg::OnPowerOff(AWEvent* event)
{
	if (selected_source) {
		selected_source->PowerOff();

		power_off->SetButtonState(1);
		power_on->SetButtonState(0);
		override->SetButtonState(0);
	}

	else if (selected_clients.size() > 0) {
		ListIter<System> iter = selected_clients;
		while (++iter)
		iter->PowerOff();

		power_off->SetButtonState(1);
		power_on->SetButtonState(0);
		override->SetButtonState(0);
	}
}

void
EngDlg::OnPowerOn(AWEvent* event)
{
	if (selected_source) {
		selected_source->PowerOn();

		power_off->SetButtonState(0);
		power_on->SetButtonState(1);
		override->SetButtonState(0);
	}

	else if (selected_clients.size() > 0) {
		ListIter<System> iter = selected_clients;
		while (++iter)
		iter->PowerOn();

		power_off->SetButtonState(0);
		power_on->SetButtonState(1);
		override->SetButtonState(0);
	}
}

void
EngDlg::OnOverride(AWEvent* event)
{
	bool over = false;

	if (override->GetButtonState() > 0)
	over = true;

	if (selected_source) {
		selected_source->SetOverride(over);
	}

	else if (selected_clients.size() > 0) {
		ListIter<System> iter = selected_clients;
		while (++iter)
		iter->SetOverride(over);
	}

	if (over) {
		power_off->SetButtonState(0);
		power_on->SetButtonState(1);
	}
}

void
EngDlg::OnPowerLevel(AWEvent* event)
{
	int level = power_level->GetValue();

	if (level < 0)
	level = 0;
	else if (level > 100)
	level = 100;

	if (selected_source)
	selected_source->SetPowerLevel(level);

	else if (selected_clients.size() > 0) {
		ListIter<System> iter = selected_clients;
		while (++iter)
		iter->SetPowerLevel(level);
	}

	if (override)
	override->SetButtonState(0);
}

// +--------------------------------------------------------------------+

void
EngDlg::OnComponent(AWEvent* event)
{
	selected_component = 0;

	if (repair)       repair->SetEnabled(false);
	if (replace)      replace->SetEnabled(false);
	if (repair_time)  repair_time->Hide();
	if (replace_time) replace_time->Hide();

	// find index of selected component:
	int component_index = -1;

	if (components) {
		int n = components->GetSelection();
		if (n >= 0)
		component_index = components->GetItemData(n);
	}

	// now examine the component info:
	if (component_index >= 0) {
		repair->SetEnabled(true);

		if (selected_source) {
			List<Component>& comps = selected_source->GetComponents();
			selected_component = comps[component_index];

			if (repair_time) {
				char t[32];
				int  s = (int) (selected_component->RepairTime() /
				ship->RepairSpeed());
				FormatTime(t, s);
				repair_time->SetText(t);
				repair_time->Show();
			}

			if (selected_component->SpareCount() > 0) {
				if (replace) replace->SetEnabled(true);

				if (replace_time) {
					char t[32];
					int  s = (int) (selected_component->ReplaceTime() /
					ship->RepairSpeed());
					FormatTime(t, s);
					replace_time->SetText(t);
					replace_time->Show();
				}
			}
		}
		else if (selected_clients.size() > 0) {
			List<Component>& comps = selected_clients[0]->GetComponents();
			selected_component = comps[component_index];

			if (repair_time) {
				char t[32];
				int  s = (int) (selected_component->RepairTime() /
				ship->RepairSpeed());
				FormatTime(t, s);
				repair_time->SetText(t);
				repair_time->Show();
			}

			if (selected_component->SpareCount() > 0) {
				if (replace) replace->SetEnabled(true);

				if (replace_time) {
					char t[32];
					int  s = (int) (selected_component->ReplaceTime() /
					ship->RepairSpeed());
					FormatTime(t, s);
					replace_time->SetText(t);
					replace_time->Show();
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
EngDlg::OnAutoRepair(AWEvent* event)
{
	if (ship)
	ship->EnableRepair(auto_repair->GetButtonState() > 0);
}

// +--------------------------------------------------------------------+

void
EngDlg::OnRepair(AWEvent* event)
{
	if (selected_component) {
		selected_component->Repair();

		if (ship)
		ship->RepairSystem(selected_component->GetSystem());
	}
}

void
EngDlg::OnReplace(AWEvent* event)
{
	if (selected_component) {
		selected_component->Replace();

		if (ship)
		ship->RepairSystem(selected_component->GetSystem());
	}
}

void
EngDlg::OnQueue(AWEvent* event)
{
	selected_repair = 0;

	if (priority_increase) priority_increase->SetEnabled(false);
	if (priority_decrease) priority_decrease->SetEnabled(false);

	if (repair_queue) {
		int n = repair_queue->GetSelection();

		if (n >= 0)
		selected_repair = ship->RepairQueue().at(n);
	}

	if (!selected_repair)
	return;

	selected_clients.clear();
	selected_clients.append(selected_repair);

	if (components) {
		components->ClearItems();

		if (repair)       repair->SetEnabled(false);
		if (replace)      replace->SetEnabled(false);
		if (repair_time)  repair_time->Hide();
		if (replace_time) replace_time->Hide();

		if (selected_repair->GetComponents().size()) {
			int index = 0;
			ListIter<Component> comp = selected_repair->GetComponents();
			while (++comp)
			components->AddItemWithData(Game::GetText(comp->Abbreviation()), index++);
		}
	}

	if (priority_increase) priority_increase->SetEnabled(true);
	if (priority_decrease) priority_decrease->SetEnabled(true);
}

void
EngDlg::OnPriorityIncrease(AWEvent* event)
{
	if (ship && repair_queue)
	ship->IncreaseRepairPriority(repair_queue->GetSelection());
}

void
EngDlg::OnPriorityDecrease(AWEvent* event)
{
	if (ship && repair_queue)
	ship->DecreaseRepairPriority(repair_queue->GetSelection());
}

// +--------------------------------------------------------------------+

void
EngDlg::OnClose(AWEvent* event)
{
	if (manager)
	manager->CloseTopmost();
}



