/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MapView.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Star Map class
*/

#include "MemDebug.h"
#include "MapView.h"

#include "Galaxy.h"
#include "StarSystem.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Instruction.h"
#include "Element.h"
#include "NavAI.h"
#include "Weapon.h"
#include "Sim.h"
#include "Mission.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "Contact.h"
#include "MenuView.h"

#include "NetLobby.h"
#include "NetUtil.h"

#include "Game.h"
#include "DataLoader.h"
#include "EventDispatch.h"
#include "Video.h"
#include "Button.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "Mouse.h"
#include "FormatUtil.h"
#include "Menu.h"

// +--------------------------------------------------------------------+

// Supported Selection Modes:

const int SELECT_NONE      = -1;
const int SELECT_SYSTEM    =  0;
const int SELECT_PLANET    =  1;
const int SELECT_REGION    =  2;
const int SELECT_STATION   =  3;
const int SELECT_STARSHIP  =  4;
const int SELECT_FIGHTER   =  5;
const int SELECT_NAVPT     =  6;

const int VIEW_GALAXY      =  0;
const int VIEW_SYSTEM      =  1;
const int VIEW_REGION      =  2;

// +--------------------------------------------------------------------+

MapView::MapView(Window* win)
: View(win)
, system(0), zoom(1.1), offset_x(0), offset_y(0), ship(0), campaign(0)
, captured(false), dragging(false), adding_navpt(false)
, moving_navpt(false), moving_elem(false)
, view_mode(VIEW_SYSTEM), seln_mode(SELECT_REGION), ship_filter(0xffffffff)
, current_star(0), current_planet(0), current_region(0)
, current_ship(0), current_elem(0), current_navpt(0), mission(0)
, scrolling(0), scroll_x(0), scroll_y(0), click_x(0), click_y(0)
, active_menu(0), map_menu(0), map_system_menu(0), map_sector_menu(0)
, ship_menu(0), editor(false)
, nav_menu(0), action_menu(0), objective_menu(0), formation_menu(0), speed_menu(0)
, hold_menu(0), farcast_menu(0), menu_view(0)
{
	for (int i = 0; i < 3; i++) {
		view_zoom[i]      = zoom;
		view_offset_x[i]  = offset_x;
		view_offset_y[i]  = offset_y;
	}

	menu_view = new(__FILE__,__LINE__) MenuView(window);

	title_font  = FontMgr::Find("Limerick12");
	font        = FontMgr::Find("Verdana");

	active_window = (ActiveWindow*) window;

	if (active_window)
	active_window->AddView(this);
}

// +--------------------------------------------------------------------+

MapView::~MapView()
{
	ClearMenu();
	galaxy_image.ClearImage();

	delete menu_view;
}

// +--------------------------------------------------------------------+

void
MapView::OnWindowMove()
{
	if (menu_view)
	menu_view->OnWindowMove();
}

// +--------------------------------------------------------------------+

void
MapView::SetGalaxy(List<StarSystem>& g)
{
	system_list.clear();
	system_list.append(g);

	if (system_list.size() > 0) {
		SetSystem(system_list[0]);
	}
}

void
MapView::SetSystem(StarSystem* s)
{
	if (system != s) {
		system = s;

		// forget invalid selection:
		current_star   = 0;
		current_planet = 0;
		current_region = 0;
		current_ship   = 0;
		current_elem   = 0;
		current_navpt  = 0;

		// flush old object pointers:
		stars.clear();
		planets.clear();
		regions.clear();

		// insert objects from star system:
		if (system) {
			ListIter<OrbitalBody> star = system->Bodies();
			while (++star) {
				switch (star->Type()) {
				case Orbital::STAR:       stars.append(star.value());
					break;
				case Orbital::PLANET:
				case Orbital::MOON:       planets.append(star.value());
					break;
				}
			}

			ListIter<OrbitalBody> planet = star->Satellites();
			while (++planet) {
				planets.append(planet.value());
				
				ListIter<OrbitalBody> moon = planet->Satellites();
				while (++moon) {
					planets.append(moon.value());
				}
			}

			ListIter<OrbitalRegion> rgn = system->AllRegions();
			while (++rgn)
			regions.append(rgn.value());

			// sort region list by distance from the star:
			regions.sort();
		}

		BuildMenu();
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetShip(Ship* s)
{
	if (ship != s) {
		ship = s;

		// forget invalid selection:
		current_star   = 0;
		current_planet = 0;
		current_region = 0;
		current_ship   = 0;
		current_elem   = 0;
		current_navpt  = 0;

		if (ship && system_list.size() > 0) {
			SimRegion* rgn = ship->GetRegion();

			if (rgn && rgn->System()) {
				system = 0;
				SetSystem(rgn->System());
			}
		}

		BuildMenu();
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetMission(Mission* m)
{
	if (mission != m) {
		mission = m;

		// forget invalid selection:
		current_star   = 0;
		current_planet = 0;
		current_region = 0;
		current_ship   = 0;
		current_elem   = 0;
		current_navpt  = 0;

		if (mission && system_list.size() > 0) {
			system = 0;
			SetSystem(mission->GetStarSystem());
		}

		BuildMenu();
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetCampaign(Campaign* c)
{
	if (campaign != c) {
		campaign       = c;

		// forget invalid selection:
		current_star   = 0;
		current_planet = 0;
		current_region = 0;
		current_ship   = 0;
		current_elem   = 0;
		current_navpt  = 0;

		if (campaign)
		SetGalaxy(campaign->GetSystemList());
	}
}

// +--------------------------------------------------------------------+

enum MapView_MENU {
	MAP_SYSTEM     = 1000,
	MAP_SECTOR     = 2000,
	MAP_SHIP       = 3000,
	MAP_NAV        = 4000,
	MAP_ADDNAV     = 4001,
	MAP_DELETE     = 4002,
	MAP_CLEAR      = 4003,
	MAP_ACTION     = 5000,
	MAP_FORMATION  = 6000,
	MAP_SPEED      = 7000,
	MAP_HOLD       = 8000,
	MAP_FARCAST    = 8500,
	MAP_OBJECTIVE  = 9000
};

void
MapView::BuildMenu()
{
	ClearMenu();

	map_system_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.STARSYSTEM"));

	if (system_list.size() > 0) {
		int i = 0;
		ListIter<StarSystem> iter = system_list;
		while (++iter) {
			StarSystem* s = iter.value();
			map_system_menu->AddItem(s->Name(), MAP_SYSTEM + i);
			i++;
		}
	}

	else if (system) {
		map_system_menu->AddItem(system->Name(), MAP_SYSTEM);
	}

	map_sector_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.SECTOR"));
	for (int i = 0; i < regions.size(); i++) {
		Orbital* rgn = regions[i];
		map_sector_menu->AddItem(rgn->Name(), MAP_SECTOR + i);
	}

	map_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.MAP"));
	map_menu->AddMenu("System", map_system_menu);
	map_menu->AddMenu("Sector", map_sector_menu);

	if (ship || mission) {
		ship_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.SHIP"));
		ship_menu->AddMenu(Game::GetText("MapView.item.Starsystem"), map_system_menu);
		ship_menu->AddMenu(Game::GetText("MapView.item.Sector"), map_sector_menu);

		ship_menu->AddItem("",       0);
		ship_menu->AddItem(Game::GetText("MapView.item.Add-Nav"),   MAP_ADDNAV);
		ship_menu->AddItem(Game::GetText("MapView.item.Clear-All"), MAP_CLEAR);

		action_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.ACTION"));
		for (int i = 0; i < Instruction::NUM_ACTIONS; i++) {
			action_menu->AddItem(Game::GetText(Text("MapView.item.") + Instruction::ActionName(i)), MAP_ACTION + i);
		}

		formation_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.FORMATION"));
		for (int i = 0; i < Instruction::NUM_FORMATIONS; i++) {
			formation_menu->AddItem(Game::GetText(Text("MapView.item.") + Instruction::FormationName(i)), MAP_FORMATION + i);
		}

		speed_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.SPEED"));
		speed_menu->AddItem("250",  MAP_SPEED + 0);
		speed_menu->AddItem("500",  MAP_SPEED + 1);
		speed_menu->AddItem("750",  MAP_SPEED + 2);
		speed_menu->AddItem("1000", MAP_SPEED + 3);

		hold_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.HOLD"));
		hold_menu->AddItem(Game::GetText("MapView.item.None"),       MAP_HOLD + 0);
		hold_menu->AddItem(Game::GetText("MapView.item.1-Minute"),   MAP_HOLD + 1);
		hold_menu->AddItem(Game::GetText("MapView.item.5-Minutes"),  MAP_HOLD + 2);
		hold_menu->AddItem(Game::GetText("MapView.item.10-Minutes"), MAP_HOLD + 3);
		hold_menu->AddItem(Game::GetText("MapView.item.15-Minutes"), MAP_HOLD + 4);

		farcast_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.FARCAST"));
		farcast_menu->AddItem(Game::GetText("MapView.item.Use-Quantum"),  MAP_FARCAST + 0);
		farcast_menu->AddItem(Game::GetText("MapView.item.Use-Farcast"),  MAP_FARCAST + 1);

		objective_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.OBJECTIVE"));

		nav_menu = new(__FILE__,__LINE__) Menu(Game::GetText("MapView.menu.NAVPT"));
		nav_menu->AddMenu(Game::GetText("MapView.item.Action"),    action_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Objective"), objective_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Formation"), formation_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Speed"),     speed_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Hold"),      hold_menu);
		nav_menu->AddMenu(Game::GetText("MapView.item.Farcast"),   farcast_menu);
		nav_menu->AddItem("",          0);
		nav_menu->AddItem(Game::GetText("MapView.item.Add-Nav"),   MAP_ADDNAV);
		nav_menu->AddItem(Game::GetText("MapView.item.Del-Nav"),   MAP_DELETE);
	}

	else if (campaign) {
		ship_menu      = 0;
		speed_menu     = 0;
		hold_menu      = 0;
		farcast_menu   = 0;
		objective_menu = 0;
		formation_menu = 0;
		nav_menu       = 0;
	}

	active_menu = map_menu;
}

// +--------------------------------------------------------------------+

void
MapView::ClearMenu()
{
	delete map_menu;
	delete map_system_menu;
	delete map_sector_menu;
	delete ship_menu;
	delete nav_menu;
	delete action_menu;
	delete objective_menu;
	delete formation_menu;
	delete speed_menu;
	delete hold_menu;
	delete farcast_menu;

	map_menu          = 0;
	map_system_menu   = 0;
	map_sector_menu   = 0;
	ship_menu         = 0;
	nav_menu          = 0;
	action_menu       = 0;
	objective_menu    = 0;
	formation_menu    = 0;
	speed_menu        = 0;
	hold_menu         = 0;
	farcast_menu      = 0;
}

// +--------------------------------------------------------------------+

void
MapView::ProcessMenuItem(int action)
{
	bool send_nav_data = false;
	bool can_command   = true;

	if (ship && current_ship && ship != current_ship) {
		if (ship->GetElement() && current_ship->GetElement()) {
			if (!ship->GetElement()->CanCommand(current_ship->GetElement())) {
				can_command = false;
			}
		}
	}

	else if (current_elem && NetLobby::GetInstance()) {
		can_command = false;
	}

	if (action >= MAP_OBJECTIVE) {
		int index = action - MAP_OBJECTIVE;
		
		if (current_navpt && can_command) {
			current_navpt->SetTarget(objective_menu->GetItem(index)->GetText());
			send_nav_data = true;
		}
	}

	else if (action >= MAP_FARCAST) {
		if (current_navpt && can_command) {
			current_navpt->SetFarcast(action - MAP_FARCAST);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_HOLD) {
		int hold_time = 0;
		switch (action) {
		default:
		case MAP_HOLD + 0:  hold_time = 0;    break;
		case MAP_HOLD + 1:  hold_time = 60;   break;
		case MAP_HOLD + 2:  hold_time = 300;  break;
		case MAP_HOLD + 3:  hold_time = 600;  break;
		case MAP_HOLD + 4:  hold_time = 900;  break;
		}

		if (current_navpt && can_command) {
			current_navpt->SetHoldTime(hold_time);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_SPEED) {
		if (current_navpt && can_command) {
			current_navpt->SetSpeed((action - MAP_SPEED + 1) * 250);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_FORMATION) {
		if (current_navpt && can_command) {
			current_navpt->SetFormation(action - MAP_FORMATION);
			send_nav_data = true;
		}
	}

	else if (action >= MAP_ACTION) {
		if (current_navpt && can_command) {
			current_navpt->SetAction(action - MAP_ACTION);
			SelectNavpt(current_navpt);
			send_nav_data = true;
		}
	}

	else if (action == MAP_ADDNAV) {
		Text           rgn_name   = regions[current_region]->Name();
		Instruction*   prior      = current_navpt;
		Instruction*   n          = 0;

		if (current_ship && can_command) {
			Sim*        sim        = Sim::GetSim();
			SimRegion*  rgn        = sim->FindRegion(rgn_name);
			Point       init_pt;

			if (rgn) {
				if (rgn->IsAirSpace())
				init_pt.z = 10e3;

				n = new(__FILE__,__LINE__) Instruction(rgn, init_pt);
			}
			else {
				n = new(__FILE__,__LINE__) Instruction(rgn_name, init_pt);
			}

			n->SetSpeed(500);

			if (prior) {
				n->SetAction(prior->Action());
				n->SetFormation(prior->Formation());
				n->SetSpeed(prior->Speed());
				n->SetTarget(prior->GetTarget());
			}

			current_ship->AddNavPoint(n, prior);
		}

		else if (current_elem && can_command) {
			Point       init_pt;

			if (regions[current_region]->Type() == Orbital::TERRAIN)
			init_pt.z = 10e3;

			n = new(__FILE__,__LINE__) Instruction(rgn_name, init_pt);
			n->SetSpeed(500);

			if (prior) {
				n->SetAction(prior->Action());
				n->SetFormation(prior->Formation());
				n->SetSpeed(prior->Speed());
				n->SetTarget(prior->GetTarget());
			}

			current_elem->AddNavPoint(n, prior);
		}

		if (can_command) {
			current_navpt  = n;
			current_status = Instruction::PENDING;
			adding_navpt   = true;
			captured       = SetCapture();
		}
	}

	else if (action == MAP_DELETE) {
		if (current_navpt && can_command) {
			if (current_ship)
			current_ship->DelNavPoint(current_navpt);
			else if (current_elem && can_command)
			current_elem->DelNavPoint(current_navpt);

			SelectNavpt(0);
		}
	}

	else if (action == MAP_CLEAR) {
		if (current_ship && can_command)
		current_ship->ClearFlightPlan();
		else if (current_elem && can_command)
		current_elem->ClearFlightPlan();

		SelectNavpt(0);
	}

	else if (action >= MAP_NAV) {
	}

	else if (action >= MAP_SHIP) {
	}

	else if (action >= MAP_SECTOR) {
		int index = action - MAP_SECTOR;

		if (index < regions.size())
		current_region = index;

		if (view_mode == VIEW_SYSTEM) {
			Orbital* s = regions[current_region];
			SetupScroll(s);
		}
	}

	else if (system_list.size() > 0 && action >= MAP_SYSTEM) {
		int index = action - MAP_SYSTEM;

		if (index < system_list.size())
		SetSystem(system_list[index]);
	}

	else {
	}

	Sim* sim = Sim::GetSim();
	if (send_nav_data && sim) {
		Ship* s = current_ship;

		if (s && s->GetElement()) {
			Element* elem  = s->GetElement();
			int      index = elem->GetNavIndex(current_navpt);

			if (index >= 0)
			NetUtil::SendNavData(false, elem, index-1, current_navpt);
		}
	}
}

// +--------------------------------------------------------------------+

bool
MapView::SetCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	return dispatch->CaptureMouse(this) ? true : false;

	return 0;
}

// +--------------------------------------------------------------------+

bool
MapView::ReleaseCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	return dispatch->ReleaseMouse(this) ? true : false;

	return 0;
}

// +--------------------------------------------------------------------+

void
MapView::SetViewMode(int mode)
{
	if (mode >= 0 && mode < 3) {
		// save state:
		view_zoom[view_mode]     = zoom;
		view_offset_x[view_mode] = offset_x;
		view_offset_y[view_mode] = offset_y;

		// switch mode:
		view_mode = mode;
		
		// restore state:

		if (view_mode == VIEW_GALAXY) {
			zoom     = 1;
			offset_x = 0;
			offset_y = 0;
		}
		else {
			zoom     = view_zoom[view_mode];
			offset_x = view_offset_x[view_mode];
			offset_y = view_offset_y[view_mode];
		}

		scrolling = 0;
		scroll_x  = 0;
		scroll_y  = 0;
	}
}

// +--------------------------------------------------------------------+

bool
MapView::Update(SimObject* obj)
{
	if (obj == current_ship) {
		current_ship = 0;
		active_menu  = map_menu;
	}

	return SimObserver::Update(obj);   
}

// +--------------------------------------------------------------------+

void
MapView::SelectShip(Ship* selship)
{
	if (selship != current_ship) {
		current_ship = selship;   

		if (current_ship) {
			if (current_ship->Life() == 0 || current_ship->IsDying() || current_ship->IsDead()) {
				current_ship = 0;
			}
			else {
				Observe(current_ship);
			}
		}
	}

	SelectNavpt(0);
}

// +--------------------------------------------------------------------+

void
MapView::SelectElem(MissionElement* elem)
{
	if (elem != current_elem) {
		current_elem = elem;
		
		if (current_elem) {
			if (current_elem->IsStarship()) {
				ship_menu->GetItem(3)->SetEnabled(true);
			}

			else if (current_elem->IsDropship()) {
				ship_menu->GetItem(3)->SetEnabled(true);
			}

			else {
				ship_menu->GetItem(3)->SetEnabled(false);
			}
		}
	}

	SelectNavpt(0);
}

// +--------------------------------------------------------------------+

void
MapView::SelectNavpt(Instruction* navpt)
{
	current_navpt = navpt;

	if (current_navpt) {
		current_status = current_navpt->Status();

		List<Text> ships;
		objective_menu->ClearItems();

		switch (current_navpt->Action()) {
		case Instruction::VECTOR:
		case Instruction::LAUNCH:
		case Instruction::PATROL:
		case Instruction::SWEEP:
		case Instruction::RECON:
			objective_menu->AddItem(Game::GetText("MapView.item.not-available"), 0);
			objective_menu->GetItem(0)->SetEnabled(false);
			break;

		case Instruction::DOCK:
			FindShips(true, true, true, false, ships);
			break;

		case Instruction::DEFEND:
			FindShips(true, true, true, false, ships);
			break;

		case Instruction::ESCORT:
			FindShips(true, false, true, true, ships);
			break;

		case Instruction::INTERCEPT:
			FindShips(false, false, false, true, ships);
			break;

		case Instruction::ASSAULT:
			FindShips(false, false, true, false, ships);
			break;

		case Instruction::STRIKE:
			FindShips(false, true, false, false, ships);
			break;
		}

		for (int i = 0; i < ships.size(); i++)
		objective_menu->AddItem(ships[i]->data(), MAP_OBJECTIVE + i);

		ships.destroy();
	}
	else {
		objective_menu->ClearItems();
		objective_menu->AddItem(Game::GetText("MapView.item.not-available"), 0);
		objective_menu->GetItem(0)->SetEnabled(false);
	}
}

// +--------------------------------------------------------------------+

void
MapView::FindShips(bool friendly, bool station, bool starship, bool dropship,
List<Text>& result)
{
	if (mission) {
		for (int i = 0; i < mission->GetElements().size(); i++) {
			MissionElement* elem = mission->GetElements().at(i);

			if (elem->IsSquadron())                continue;
			if (!station  && elem->IsStatic())     continue;
			if (!starship && elem->IsStarship())   continue;
			if (!dropship && elem->IsDropship())   continue;

			if (!editor && friendly && elem->GetIFF() > 0 && elem->GetIFF() != mission->Team())
			continue;

			if (!editor && !friendly && (elem->GetIFF() == 0 || elem->GetIFF() == mission->Team()))
			continue;

			result.append(new(__FILE__,__LINE__) Text(elem->Name()));
		}
	}

	else if (ship) {
		Sim* sim = Sim::GetSim();

		if (sim) {
			for (int r = 0; r < sim->GetRegions().size(); r++) {
				SimRegion* rgn = sim->GetRegions().at(r);

				for (int i = 0; i < rgn->Ships().size(); i++) {
					Ship* s = rgn->Ships().at(i);

					if (!station  && s->IsStatic())     continue;
					if (!starship && s->IsStarship())   continue;
					if (!dropship && s->IsDropship())   continue;

					if (friendly && s->GetIFF() > 0 && s->GetIFF() != ship->GetIFF())
					continue;

					if (!friendly && (s->GetIFF() == 0 || s->GetIFF() == ship->GetIFF()))
					continue;

					result.append(new(__FILE__,__LINE__) Text(s->Name()));
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetSelectionMode(int mode)
{
	if (mode <= SELECT_NONE) {
		seln_mode = SELECT_NONE;
		return;
	}

	if (mode != seln_mode && mode <= SELECT_FIGHTER) {
		seln_mode = mode;
		
		// when changing mode, 
		// select the item closest to the current center:
		if (system && view_mode == VIEW_SYSTEM)
		SelectAt(rect.x + rect.w/2, 
		rect.y + rect.h/2);
	}
}

void
MapView::SetSelection(int index)
{
	if (scrolling) return;
	Orbital* s = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:
		if (index < system_list.size())
		SetSystem(system_list[index]);
		s = stars[current_star];
		break;

	default:
	case SELECT_PLANET:
		if (index < planets.size())
		current_planet = index;
		s = planets[current_planet];
		break;

	case SELECT_REGION:
		if (index < regions.size())
		current_region = index;
		s = regions[current_region];
		break;

	case SELECT_STATION:
		{
			if (mission) {
				MissionElement* selected_elem = 0;

				ListIter<MissionElement> elem = mission->GetElements();
				while (++elem) {
					if (elem->IsStatic()) {
						if (elem->Identity() == index) {
							selected_elem = elem.value();
							break;
						}
					}
				}

				SelectElem(selected_elem);

				if (selected_elem && regions.size()) {
					ListIter<Orbital> rgn = regions;
					while (++rgn) {
						if (!stricmp(selected_elem->Region(), rgn->Name())) {
							Orbital* elem_region = rgn.value();
							current_region = regions.index(elem_region);
						}
					}
				}
			}

			else {
				Ship*       selship = 0;

				if (ship) {
					SimRegion*  simrgn  = ship->GetRegion();
					if (simrgn) {
						ListIter<Ship> s = simrgn->Ships();
						while (++s) {
							if (s->IsStatic()) {
								if (s->Identity() == index) {
									selship = s.value();
									break;
								}
							}
						}
					}
				}

				SelectShip(selship);

				if (selship) {
					s = selship->GetRegion()->GetOrbitalRegion();
					current_region = regions.index(s);
				}
			}
		}
		break;

	case SELECT_STARSHIP:
		{
			if (mission) {
				MissionElement* selected_elem = 0;

				ListIter<MissionElement> elem = mission->GetElements();
				while (++elem) {
					if (elem->IsStarship()) {
						if (elem->Identity() == index) {
							selected_elem = elem.value();
							break;
						}
					}
				}

				SelectElem(selected_elem);

				if (selected_elem && regions.size()) {
					ListIter<Orbital> rgn = regions;
					while (++rgn) {
						if (!stricmp(selected_elem->Region(), rgn->Name())) {
							Orbital* elem_region = rgn.value();
							current_region = regions.index(elem_region);
						}
					}
				}
			}

			else {
				Ship*       selship = 0;

				if (ship) {
					SimRegion*  simrgn  = ship->GetRegion();
					if (simrgn) {
						ListIter<Ship> s = simrgn->Ships();
						while (++s) {
							if (s->IsStarship()) {
								if (s->Identity() == index) {
									selship = s.value();
									break;
								}
							}
						}
					}
				}

				SelectShip(selship);

				if (selship) {
					s = selship->GetRegion()->GetOrbitalRegion();
					current_region = regions.index(s);
				}
			}
		}
		break;

	case SELECT_FIGHTER:
		{
			if (mission) {
				MissionElement* selected_elem = 0;

				ListIter<MissionElement> elem = mission->GetElements();
				while (++elem) {
					if (elem->IsDropship() && !elem->IsSquadron()) {
						if (elem->Identity() == index) {
							selected_elem = elem.value();
							break;
						}
					}
				}

				SelectElem(selected_elem);

				if (selected_elem && regions.size()) {
					ListIter<Orbital> rgn = regions;
					while (++rgn) {
						if (!stricmp(selected_elem->Region(), rgn->Name())) {
							Orbital* elem_region = rgn.value();
							current_region = regions.index(elem_region);
						}
					}
				}
			}

			else {
				Ship*       selship = 0;

				if (ship) {
					SimRegion*  simrgn  = ship->GetRegion();
					if (simrgn) {
						ListIter<Ship> s = simrgn->Ships();
						while (++s) {
							if (s->IsDropship()) {
								if (s->Identity() == index) {
									selship = s.value();
									break;
								}
							}
						}
					}
				}

				SelectShip(selship);

				if (selship) {
					s = selship->GetRegion()->GetOrbitalRegion();
					current_region = regions.index(s);
				}
			}
		}
		break;
	}

	SetupScroll(s);
}

void
MapView::SetSelectedShip(Ship* ship)
{
	if (scrolling) return;
	Orbital* s       = 0;
	Ship*    selship = 0;


	switch (seln_mode) {
	case SELECT_SYSTEM:
	case SELECT_PLANET:
	case SELECT_REGION:
	default:
		break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
		{
			if (ship) {
				SimRegion*  simrgn = ship->GetRegion();
				
				if (simrgn && simrgn->NumShips()) {
					selship = simrgn->Ships().find(ship);
				}
			}
			
			SelectShip(selship);
		}
		break;
	}

	if (selship)
	SetupScroll(s);
}

void
MapView::SetSelectedElem(MissionElement* elem)
{
	if (scrolling) return;
	Orbital* s = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:
	case SELECT_PLANET:
	case SELECT_REGION:
	default:
		break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
		{
			SelectElem(elem);
		}
		break;
	}

	if (current_elem)
	SetupScroll(s);
}

void
MapView::SetupScroll(Orbital* s)
{
	switch (view_mode) {
	case VIEW_GALAXY:
		zoom        = 1;
		offset_x    = 0;
		offset_y    = 0;
		scrolling   = 0;
		break;

	case VIEW_SYSTEM:
		if (s == 0) {
			offset_x = 0;
			offset_y = 0;
			scrolling = 0;
		}
		else {
			scroll_x = (offset_x + s->Location().x) / 5.0;
			scroll_y = (offset_y + s->Location().y) / 5.0;
			scrolling = 5;
		}
		break;

	case VIEW_REGION:
		if (current_navpt) {
			// don't move the map
			scrolling = 0;
		}
		else if (current_ship) {
			Point sloc = current_ship->Location().OtherHand();

			if (!IsVisible(sloc)) {
				scroll_x  = (offset_x + sloc.x) / 5.0;
				scroll_y  = (offset_y + sloc.y) / 5.0;
				scrolling = 5;
			}
			else {
				scroll_x  = 0;
				scroll_y  = 0;
				scrolling = 0;
			}
		}
		else if (current_elem) {
			Point sloc = current_elem->Location();

			if (!IsVisible(sloc)) {
				scroll_x  = (offset_x + sloc.x) / 5.0;
				scroll_y  = (offset_y + sloc.y) / 5.0;
				scrolling = 5;
			}
			else {
				scroll_x  = 0;
				scroll_y  = 0;
				scrolling = 0;
			}
		}
		else {
			offset_x = 0;
			offset_y = 0;
			scrolling = 0;
		}
		break;
	}
}

bool
MapView::IsVisible(const Point& loc)
{
	if (view_mode == VIEW_REGION) {
		double scale   = c/r;
		double ox      = offset_x * scale;
		double oy      = offset_y * scale;
		double sx      = loc.x * scale;
		double sy      = loc.y * scale;
		double cx      = rect.w/2;
		double cy      = rect.h/2;

		int    test_x  = (int) (cx + sx + ox);
		int    test_y  = (int) (cy + sy + oy);

		bool   visible = test_x >= 0 && test_x < rect.w &&
		test_y >= 0 && test_y < rect.h;

		return visible;
	}

	return false;
}

// +--------------------------------------------------------------------+

void
MapView::SetRegion(OrbitalRegion* rgn)
{
	if (scrolling || rgn == 0) return;

	int index = regions.index(rgn);

	if (index < 0 || index == current_region || index >= regions.size())
	return;

	current_region = index;
	Orbital* s = regions[current_region];

	if (!s)
	return;

	switch (view_mode) {
	case VIEW_GALAXY:
	case VIEW_SYSTEM:
		scroll_x = (offset_x + s->Location().x) / 5.0;
		scroll_y = (offset_y + s->Location().y) / 5.0;
		scrolling = 5;
		break;

	case VIEW_REGION:
		offset_x = 0;
		offset_y = 0;
		scrolling = 0;
		break;
	}
}

// +--------------------------------------------------------------------+

void
MapView::SetRegionByName(const char* rgn_name)
{
	OrbitalRegion* rgn = 0;

	for (int i = 0; i < regions.size(); i++) {
		Orbital* r = regions[i];
		if (!strcmp(rgn_name, r->Name())) {
			rgn = (OrbitalRegion*) r;
			break;
		}
	}

	SetRegion(rgn);
}

// +--------------------------------------------------------------------+

void
MapView::SelectAt(int x, int y)
{
	if (scrolling) return;
	if (c == 0) return;
	if (seln_mode < 0) return;

	Orbital* s = 0;

	double scale   = r/c;
	double cx      = rect.w/2;
	double cy      = rect.h/2;
	double test_x  = (x - rect.x - cx) * scale - offset_x;
	double test_y  = (y - rect.y - cy) * scale - offset_y;
	double dist    = 1.0e20;
	int    closest = 0;

	if (view_mode == VIEW_GALAXY) {
		c  = (cx>cy) ? cx : cy;
		r  = 10;

		Galaxy* g = Galaxy::GetInstance();
		if (g)
		r = g->Radius();

		StarSystem* closest_system = 0;

		// draw the list of systems, and their connections:
		ListIter<StarSystem> iter = system_list;
		while (++iter) {
			StarSystem* s = iter.value();

			double dx = (s->Location().x - test_x);
			double dy = (s->Location().y - test_y);
			double d  = sqrt(dx*dx + dy*dy);

			if (d < dist) {
				dist = d;
				closest_system = s;
			}
		}

		if (closest_system)
		SetSystem(closest_system);
	}

	else if (view_mode == VIEW_SYSTEM) {
		switch (seln_mode) {
		case SELECT_SYSTEM: {
				if (stars.isEmpty()) return;
				int index = 0;
				ListIter<Orbital> star = stars;
				while (++star) {
					double dx = (star->Location().x - test_x);
					double dy = (star->Location().y - test_y);
					double d  = sqrt(dx*dx + dy*dy);

					if (d < dist) {
						dist = d;
						closest = index;
					}

					index++;
				}
				
				current_star = closest;
			}
			s = stars[current_star];
			break;

		case SELECT_PLANET: {
				if (planets.isEmpty()) return;
				int index = 0;
				ListIter<Orbital> planet = planets;
				while (++planet) {
					double dx = (planet->Location().x - test_x);
					double dy = (planet->Location().y - test_y);
					double d  = sqrt(dx*dx + dy*dy);
					
					if (d < dist) {
						dist = d;
						closest = index;
					}

					index++;
				}
				
				current_planet = closest;
			}
			s = planets[current_planet];
			break;

		default:
		case SELECT_REGION: {
				if (regions.isEmpty()) return;
				int index = 0;
				ListIter<Orbital> region = regions;
				while (++region) {
					double dx = (region->Location().x - test_x);
					double dy = (region->Location().y - test_y);
					double d  = sqrt(dx*dx + dy*dy);
					
					if (d < dist) {
						dist = d;
						closest = index;
					}

					index++;
				}
				
				current_region = closest;
			}
			s = regions[current_region];
			break;
		}
	}

	else if (view_mode == VIEW_REGION) {
		dist = 5.0e3;

		if (mission) {
			Orbital*          rgn      = regions[current_region];
			MissionElement*   sel_elem = 0;
			Instruction*      sel_nav  = 0;

			if (!rgn) return;

			// check nav points:
			ListIter<MissionElement> elem = mission->GetElements();
			while (++elem) {
				MissionElement* e = elem.value();

				if (!e->IsSquadron() && (editor || e->GetIFF() == mission->Team())) {
					ListIter<Instruction> navpt = e->NavList();
					while (++navpt) {
						Instruction* n = navpt.value();

						if (!stricmp(n->RegionName(), rgn->Name())) {
							Point nloc = n->Location();
							double dx = nloc.x - test_x;
							double dy = nloc.y - test_y;
							double d  = sqrt(dx*dx + dy*dy);

							if (d < dist) {
								dist = d;
								sel_nav = n;
								sel_elem = e;
							}
						}
					}
				}
			}

			if (sel_nav) {
				SelectElem(sel_elem);
				SelectNavpt(sel_nav);
			}

			// check elements:
			else {
				elem.reset();
				while (++elem) {
					MissionElement* e = elem.value();

					if (e->Region() == rgn->Name() && !e->IsSquadron()) {
						Point sloc = e->Location();
						double dx = sloc.x - test_x;
						double dy = sloc.y - test_y;
						double d  = sqrt(dx*dx + dy*dy);

						if (d < dist) {
							dist = d;
							sel_elem = e;
						}
					}
				}

				SelectElem(sel_elem);

				if (sel_elem)
				s = rgn;
			}
		}
		else if (ship) {
			Sim*           sim      = Sim::GetSim();
			Orbital*       rgn      = regions[current_region];
			SimRegion*     simrgn   = 0;
			Ship*          sel_ship = 0;
			Instruction*   sel_nav  = 0;

			if (sim && rgn)
			simrgn = sim->FindRegion(rgn->Name());

			// check nav points:
			if (simrgn) {
				for (int r = 0; r < sim->GetRegions().size(); r++) {
					SimRegion* simrgn = sim->GetRegions().at(r);

					for (int i = 0; i < simrgn->Ships().size(); i++) {
						Ship* s = simrgn->Ships().at(i);

						if (s->GetIFF() == ship->GetIFF() && s->GetElementIndex() == 1) {
							ListIter<Instruction> navpt = s->GetFlightPlan();
							while (++navpt) {
								Instruction* n = navpt.value();

								if (!stricmp(n->RegionName(), rgn->Name())) {
									Point nloc = n->Location();
									double dx = nloc.x - test_x;
									double dy = nloc.y - test_y;
									double d  = sqrt(dx*dx + dy*dy);

									if (d < dist) {
										dist     = d;
										sel_nav  = n;
										sel_ship = s;
									}
								}
							}
						}
					}
				}
			}

			if (sel_nav) {
				SelectShip(sel_ship);
				SelectNavpt(sel_nav);
			}

			// check ships:
			else if (simrgn->NumShips()) {
				ListIter<Ship> ship = simrgn->Ships();
				while (++ship) {
					Ship* s = ship.value();

					if (!IsClutter(*s)) {
						Point sloc = s->Location().OtherHand();
						double dx = sloc.x - test_x;
						double dy = sloc.y - test_y;
						double d  = sqrt(dx*dx + dy*dy);

						if (d < dist) {
							dist = d;
							sel_ship = s;
						}
					}
				}

				SelectShip(sel_ship);
			}
			else {
				SelectShip(0);
				SelectNavpt(0);
			}
		}
	}

	if (s)
	SetupScroll(s);
}

// +--------------------------------------------------------------------+

Orbital*
MapView::GetSelection()
{
	Orbital* s = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:
		if (current_star < stars.size())
		s = stars[current_star];
		break;

	default:
	case SELECT_PLANET:
		if (current_planet < planets.size())
		s = planets[current_planet];
		break;

	case SELECT_REGION:
		if (current_region < regions.size())
		s = regions[current_region];
		break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
		break;
	}

	return s;
}

Ship*
MapView::GetSelectedShip()
{
	return current_ship;
}

MissionElement*
MapView::GetSelectedElem()
{
	return current_elem;
}

// +--------------------------------------------------------------------+

int
MapView::GetSelectionIndex()
{
	int s = 0;

	switch (seln_mode) {
	case SELECT_SYSTEM:  s = current_star;   break;
	default:
	case SELECT_PLANET:  s = current_planet; break;
	case SELECT_REGION:  s = current_region; break;

	case SELECT_STATION:
	case SELECT_STARSHIP:
	case SELECT_FIGHTER:
		{
			s = -1;
			Sim*        sim    = Sim::GetSim();
			Orbital*    rgn    = regions[current_region];
			SimRegion*  simrgn = 0;

			if (sim && rgn)
			simrgn = sim->FindRegion(rgn->Name());
			
			if (simrgn) {
				if (current_ship && simrgn->NumShips()) {
					s = simrgn->Ships().index(current_ship);
				}
			}
		}
		break;
	}

	return s;
}

void
MapView::DrawTabbedText(Font* font, const char* text)
{
	if (font && text && *text) {
		Rect label_rect;

		label_rect.w = rect.w;
		label_rect.h = rect.h;

		label_rect.Inset(8,8,8,8);

		DWORD text_flags = DT_WORDBREAK | DT_LEFT;

		active_window->SetFont(font);
		active_window->DrawText(text, 0, label_rect, text_flags);
	}
}

// +--------------------------------------------------------------------+

void
MapView::Refresh()
{
	rect = window->GetRect();

	if (!system) {
		DrawGrid();
		DrawTabbedText(title_font, Game::GetText("MapView.item.no-system"));
		return;
	}

	if (font)
	font->SetColor(Color::White);

	if (scrolling) {
		offset_x -= scroll_x;
		offset_y -= scroll_y;
		scrolling--;
	}

	rect.w -= 2;
	rect.h -= 2;

	switch (view_mode) {
	case VIEW_GALAXY: DrawGalaxy(); break;
	case VIEW_SYSTEM: DrawSystem(); break;
	case VIEW_REGION: DrawRegion(); break;
	default:          DrawGrid();   break;
	}

	rect.w += 2;
	rect.h += 2;

	if (menu_view) {
		if (current_navpt) {
			active_menu = nav_menu;
		}
		else if (current_ship) {
			if (current_ship->GetIFF() == ship->GetIFF())
			active_menu = ship_menu;
			else
			active_menu = map_menu;
		}
		else if (current_elem) {
			if (editor || current_elem->GetIFF() == mission->Team())
			active_menu = ship_menu;
			else
			active_menu = map_menu;
		}
		else {
			active_menu = map_menu;
		}

		menu_view->SetBackColor(Color::Gray);
		menu_view->SetTextColor(Color::White);
		menu_view->SetMenu(active_menu);
		menu_view->DoMouseFrame();

		if (menu_view->GetAction()) {
			ProcessMenuItem(menu_view->GetAction());
		}

		menu_view->Refresh();
	}

	DrawTitle();
}

// +--------------------------------------------------------------------+

void
MapView::DrawTitle()
{
	title_font->SetColor(active_window->GetForeColor());
	DrawTabbedText(title_font, title);
}

// +--------------------------------------------------------------------+

void
MapView::DrawGalaxy()
{
	title = Game::GetText("MapView.title.Galaxy");
	DrawGrid();

	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx>cy) ? cx : cy;
	r  = 10; // * zoom;

	Galaxy* g = Galaxy::GetInstance();
	if (g)
	r = g->Radius(); // * zoom;

	double scale = c/r;
	double ox    = 0;
	double oy    = 0;

	// compute offset:
	ListIter<StarSystem> iter = system_list;
	while (++iter) {
		StarSystem* s = iter.value();

		if (system == s) {
			if (fabs(s->Location().x) > 10 || fabs(s->Location().y) > 10) {
				int sx = (int) s->Location().x;
				int sy = (int) s->Location().y;

				sx -= sx % 10;
				sy -= sy % 10;

				ox = sx * -scale;
				oy = sy * -scale;
			}
		}
	}

	// draw the list of systems, and their connections:
	iter.reset();
	while (++iter) {
		StarSystem* s = iter.value();

		int sx = (int) (cx + ox + s->Location().x * scale);
		int sy = (int) (cy + oy + s->Location().y * scale);

		if (sx < 4 || sx > rect.w-4 || sy < 4 || sy > rect.h-4)
		continue;

		window->DrawEllipse(sx-7, sy-7, sx+7, sy+7, Ship::IFFColor(s->Affiliation()));

		if (s == system) {
			window->DrawLine(0, sy, rect.w, sy, Color::Gray, Video::BLEND_ADDITIVE);
			window->DrawLine(sx, 0, sx, rect.h, Color::Gray, Video::BLEND_ADDITIVE);
		}

		ListIter<StarSystem> iter2 = system_list;
		while (++iter2) {
			StarSystem* s2 = iter2.value();

			if (s != s2 && s->HasLinkTo(s2)) {
				int ax  = sx;
				int ay  = sy;

				int bx = (int) (cx + ox + s2->Location().x * scale);
				int by = (int) (cy + oy + s2->Location().y * scale);

				if (ax == bx) {
					if (ay < by) {
						ay += 8; by -= 8;
					}
					else {
						ay -= 8; by += 8;
					}
				}

				else if (ay == by) {
					if (ax < bx) {
						ax += 8; bx -= 8;
					}
					else {
						ax -= 8; bx += 8;
					}
				}

				else {
					Point d = Point(bx, by, 0) - Point(ax, ay, 0);
					d.Normalize();

					ax += (int) (8 * d.x);
					ay += (int) (8 * d.y);

					bx -= (int) (8 * d.x);
					by -= (int) (8 * d.y);
				}

				window->DrawLine(ax, ay, bx, by, Color(120,120,120), Video::BLEND_ADDITIVE);
			}
		}
	}

	// finally draw all the stars in the galaxy:
	if (g) {
		ListIter<Star> iter = g->Stars();
		while (++iter) {
			Star* s = iter.value();

			int sx = (int) (cx + ox + s->Location().x * scale);
			int sy = (int) (cy + oy + s->Location().y * scale);
			int sr = s->GetSize();

			if (sx < 4 || sx > rect.w-4 || sy < 4 || sy > rect.h-4)
			continue;

			window->FillEllipse(sx-sr, sy-sr, sx+sr, sy+sr, s->GetColor());

			if (!strncmp(s->Name(), "GSC", 3))
			font->SetColor(Color(100,100,100));
			else
			font->SetColor(Color::White);

			Rect name_rect(sx-60, sy+8, 120, 20);
			active_window->SetFont(font);
			active_window->DrawText(s->Name(), 0, name_rect, DT_SINGLELINE | DT_CENTER);
		}
	}

	font->SetColor(Color::White);
}

// +--------------------------------------------------------------------+

void
MapView::DrawSystem()
{
	Text caption = Game::GetText("MapView.title.Starsystem");
	caption += " ";
	caption += system->Name();

	if (current_ship) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_ship->Name();
	}
	else if (current_elem) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_elem->Name();
	}

	title = caption;

	ListIter<OrbitalBody> star = system->Bodies();
	while (++star) {
		int p_orb = 1;

		ListIter<OrbitalBody> planet = star->Satellites();
		while (++planet) {
			DrawOrbital(*planet, p_orb++);

			int m_orb = 1;

			ListIter<OrbitalBody> moon = planet->Satellites();
			while (++moon) {
				DrawOrbital(*moon, m_orb++);

				ListIter<OrbitalRegion> region = moon->Regions();
				while (++region) {
					DrawOrbital(*region, 1);
				}
			}

			ListIter<OrbitalRegion> region = planet->Regions();
			while (++region) {
				DrawOrbital(*region, 1);
			}
		}
		
		ListIter<OrbitalRegion> region = star->Regions();
		while (++region) {
			DrawOrbital(*region, 1);
		}

		DrawOrbital(*star, 0);
	}

	char r_txt[32];
	FormatNumber(r_txt, system->Radius() * zoom);
	char resolution[64];
	sprintf(resolution, "%s: %s", Game::GetText("MapView.info.Resolution").data(), r_txt);

	active_window->SetFont(font);
	active_window->DrawText(resolution, -1, Rect(4, 4, rect.w-8, 24), DT_SINGLELINE|DT_RIGHT);
}

// +--------------------------------------------------------------------+

void
MapView::DrawRegion()
{
	OrbitalRegion* rgn = (OrbitalRegion*) regions[current_region];

	Text caption = Game::GetText("MapView.title.Sector");
	caption += " ";
	caption += rgn->Name();

	if (current_ship) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_ship->Name();
	}
	else if (current_elem) {
		caption += "\n";
		caption += Game::GetText("MapView.title.Ship");
		caption += " ";
		caption += current_elem->Name();
	}

	title = caption;

	double cx = rect.w/2;
	double cy = rect.h/2;

	int size = (int) rgn->Radius();
	int step = (int) rgn->GridSpace();

	c  = (cx<cy) ? cx : cy;
	r  = rgn->Radius() * zoom;
	double scale = c/r;

	double ox = offset_x * scale;
	double oy = offset_y * scale;

	int left    = (int) (-size * scale + ox + cx);
	int right   = (int) ( size * scale + ox + cx);
	int top     = (int) (-size * scale + oy + cy);
	int bottom  = (int) ( size * scale + oy + cy);

	Color major(48,48,48);
	Color minor(24,24,24);

	int x,y;
	int tick = 0;

	for (x = 0; x <= size; x += step) {
		int lx = (int) (x * scale + ox + cx);
		if (!tick)
		window->DrawLine(lx, top, lx, bottom, major, Video::BLEND_ADDITIVE);
		else
		window->DrawLine(lx, top, lx, bottom, minor, Video::BLEND_ADDITIVE);

		lx = (int) (-x * scale + ox + cx);
		if (!tick)
		window->DrawLine(lx, top, lx, bottom, major, Video::BLEND_ADDITIVE);
		else
		window->DrawLine(lx, top, lx, bottom, minor, Video::BLEND_ADDITIVE);

		if (++tick > 3) tick = 0;
	}

	tick = 0;

	for (y = 0; y <= size; y += step) {
		int ly = (int) (y * scale + oy + cy);
		if (!tick)
		window->DrawLine(left, ly, right, ly, major, Video::BLEND_ADDITIVE);
		else
		window->DrawLine(left, ly, right, ly, minor, Video::BLEND_ADDITIVE);

		ly = (int) (-y * scale + oy + cy);
		if (!tick)
		window->DrawLine(left, ly, right, ly, major, Video::BLEND_ADDITIVE);
		else
		window->DrawLine(left, ly, right, ly, minor, Video::BLEND_ADDITIVE);

		if (++tick > 3) tick = 0;
	}

	int rep = 3;
	if (r > 70e3)  rep = 2;
	if (r > 250e3) rep = 1;

	if (campaign && rgn) {
		// draw the combatants in this region:
		ListIter<Combatant> iter = campaign->GetCombatants();
		while (++iter) {
			Combatant* combatant = iter.value();
			DrawCombatGroup(combatant->GetForce(), rep);
		}
	}

	else if (mission && rgn) {
		// draw the elements in this region:
		ListIter<MissionElement> elem = mission->GetElements();
		while (++elem)
		if (!elem->IsSquadron())
		DrawElem(*elem, (elem.value() == current_elem), rep);
	}

	else if (ship) {
		// draw the ships in this region:
		Sim*        sim    = Sim::GetSim();
		SimRegion*  simrgn = 0;

		if (sim && rgn)
		simrgn = sim->FindRegion(rgn->Name());

		// this algorithm uses the allied track list for the region,
		// even if this ship is in a different region.  a previous
		// version used the ship's own contact list, which only shows
		// ships in the player's region.  this way is less "realistic"
		// but more fun for managing large battles.

		if (simrgn) {
			ListIter<Contact> c = simrgn->TrackList(ship->GetIFF());
			while (++c) {
				Contact* contact = c.value();
				Ship*    s       = contact->GetShip();

				if (s && (s->Class() & ship_filter) && !IsClutter(*s) && s != ship)
				DrawShip(*s, (s == current_ship), rep);
			}

			ListIter<Ship> s_iter = simrgn->Ships();
			while (++s_iter) {
				Ship* s = s_iter.value();

				if (s && (s->IsStatic()) && !IsClutter(*s) &&
						(s->GetIFF() == ship->GetIFF() || s->GetIFF() == 0))
				DrawShip(*s, (s == current_ship), rep);
			}

			// draw nav routes for allied ships not in the region:
			ListIter<SimRegion> r_iter = sim->GetRegions();
			while (++r_iter) {
				SimRegion* r = r_iter.value();

				if (r != simrgn) {
					ListIter<Ship> s_iter = r->Ships();
					while (++s_iter) {
						Ship* s = s_iter.value();

						if (s && !s->IsStatic() && !IsClutter(*s) &&
								(s->GetIFF() == ship->GetIFF() || s->GetIFF() == 0)) {
							DrawNavRoute(simrgn->GetOrbitalRegion(),
							s->GetFlightPlan(),
							s->MarkerColor(),
							s, 0);
						}
					}
				}
			}
		}

		// draw our own ship:
		DrawShip(*ship, (ship == current_ship), rep);
	}

	char r_txt[32];
	FormatNumber(r_txt, r*2);
	char resolution[64];
	sprintf(resolution, "%s: %s", Game::GetText("MapView.info.Resolution").data(), r_txt);

	active_window->SetFont(font);
	active_window->DrawText(resolution, -1, Rect(4, 4, rect.w-8, 24), DT_SINGLELINE|DT_RIGHT);
}

// +--------------------------------------------------------------------+

void
MapView::DrawGrid()
{
	int grid_step = rect.w/8;
	int cx        = rect.w/2;
	int cy        = rect.h/2;

	Color c(32,32,32);

	window->DrawLine(0, cy, rect.w, cy, c, Video::BLEND_ADDITIVE);
	window->DrawLine(cx, 0, cx, rect.h, c, Video::BLEND_ADDITIVE);

	for (int i = 1; i < 4; i++) {
		window->DrawLine(0, cy + (i*grid_step), rect.w, cy + (i*grid_step), c, Video::BLEND_ADDITIVE);
		window->DrawLine(0, cy - (i*grid_step), rect.w, cy - (i*grid_step), c, Video::BLEND_ADDITIVE);
		window->DrawLine(cx + (i*grid_step), 0, cx + (i*grid_step), rect.h, c, Video::BLEND_ADDITIVE);
		window->DrawLine(cx - (i*grid_step), 0, cx - (i*grid_step), rect.h, c, Video::BLEND_ADDITIVE);
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawOrbital(Orbital& body, int index)
{
	int type = body.Type();

	if (type == Orbital::NOTHING)
	return;

	int x1, y1, x2, y2;
	Rect label_rect;
	int  label_w = 64;
	int  label_h = 18;

	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r  = system->Radius() * zoom;

	if ((r > 300e9) && (type > Orbital::PLANET))
	return;

	double xscale = cx / r;
	double yscale = cy / r * 0.75;

	double ox = offset_x * xscale;
	double oy = offset_y * yscale;

	double bo_x = body.Orbit()    * xscale;
	double bo_y = body.Orbit()    * yscale;
	double br = body.Radius()     * yscale;
	double bx = body.Location().x * xscale;
	double by = body.Location().y * yscale;

	double px = 0;
	double py = 0;

	if (body.Primary()) {
		double min_pr = GetMinRadius(body.Primary()->Type());

		if (index) {
			if (min_pr < 4)
			min_pr = 4;

			min_pr *= (index+1);
		}

		double min_x = min_pr * xscale / yscale;
		double min_y = min_pr;

		if (bo_x < min_x)
		bo_x = min_x;

		if (bo_y < min_y)
		bo_y = min_y;

		px = body.Primary()->Location().x * xscale;
		py = body.Primary()->Location().y * yscale;
	}

	if (type == Orbital::TERRAIN)
	bo_x = bo_y;

	int ipx = (int) (cx + px + ox);
	int ipy = (int) (cy + py + oy);
	int ibo_x = (int) bo_x;
	int ibo_y = (int) bo_y;

	x1 = ipx - ibo_x;
	y1 = ipy - ibo_y;
	x2 = ipx + ibo_x;
	y2 = ipy + ibo_y;

	if (type != Orbital::TERRAIN) {
		double a = x2-x1;
		double b = rect.w*32;

		if (a < b)
		window->DrawEllipse(x1, y1, x2, y2, Color(64,64,64), Video::BLEND_ADDITIVE);
	}

	// show body's location on possibly magnified orbit
	bx = px + bo_x * cos(body.Phase());
	by = py + bo_y * sin(body.Phase());

	double min_br = GetMinRadius(type);
	if (br < min_br) br = min_br;

	Color color;

	switch (type) {
	case Orbital::STAR:     color = Color(248, 248, 128); break;
	case Orbital::PLANET:   color = Color( 64,  64, 192); break;
	case Orbital::MOON:     color = Color( 32, 192,  96); break;
	case Orbital::REGION:   color = Color(255, 255, 255); break;
	case Orbital::TERRAIN:  color = Color( 16, 128,  48); break;
	}

	int icx = (int) (cx + bx + ox);
	int icy = (int) (cy + by + oy);
	int ibr = (int) br;

	x1 = icx - ibr;
	y1 = icy - ibr;
	x2 = icx + ibr;
	y2 = icy + ibr;

	if (type < Orbital::REGION) {
		if (body.GetMapIcon().Width() > 64) {
			Bitmap* map_icon = (Bitmap*) &body.GetMapIcon();

			if (type == Orbital::STAR)
			window->DrawBitmap(x1,y1,x2,y2, map_icon, Video::BLEND_ADDITIVE);
			else
			window->DrawBitmap(x1,y1,x2,y2, map_icon, Video::BLEND_ALPHA);
		}
		else {
			window->FillEllipse(x1, y1, x2, y2, color);
		}
	}
	else {
		window->DrawRect(x1, y1, x2, y2, color);

		if (campaign) {
			ListIter<Combatant> iter = campaign->GetCombatants();
			while (++iter) {
				Combatant* combatant = iter.value();

				if (ibr >= 4 || combatant->GetIFF() == 1)
				DrawCombatantSystem(combatant, &body, icx, icy, ibr);
			}
		}
	}

	if (type == Orbital::STAR || bo_y > label_h) {
		label_rect.x = x1 - label_w + (int) br;
		label_rect.y = y1 - label_h;
		label_rect.w = label_w * 2;
		label_rect.h = label_h;

		active_window->SetFont(font);
		active_window->DrawText(body.Name(), -1, label_rect, DT_SINGLELINE|DT_CENTER);
	}
}

// +--------------------------------------------------------------------+

double
MapView::GetMinRadius(int type)
{
	switch (type) {
	case Orbital::STAR:     return 8;
	case Orbital::PLANET:   return 4;
	case Orbital::MOON:     return 2;
	case Orbital::REGION:   return 2;
	}

	return 0;
}

// +--------------------------------------------------------------------+

static void 
ColorizeBitmap(Bitmap& img, Color color)
{
	int w = img.Width();
	int h = img.Height();

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			Color c = img.GetColor(x, y);

			if (c != Color::Black && c != Color::White) {
				img.SetColor(x,y,color.ShadeColor(c.Blue()/2));
			}
		}
	}

	img.AutoMask();
}

static POINT shipshape1[] = { {6,0}, {-6,4}, {-6,-4} };
static POINT shipshape2[] = { {8,0}, { 4,4}, {-8,4}, {-8,-4}, {4,-4} };
static POINT shipshape3[] = { {8,8}, {-8,8}, {-8,-8}, {8,-8} };

void
MapView::DrawShip(Ship& s, bool current, int rep)
{
	OrbitalRegion* rgn = (OrbitalRegion*) regions[current_region];
	if (!rgn) return;

	int x1, y1, x2, y2;
	POINT shiploc;
	Point sloc = s.Location().OtherHand();

	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	double scale = c/r;

	double ox = offset_x * scale;
	double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	int sprite_width = 10;

	window->SetFont(font);

	// draw ship icon:
	if (rep && view_mode == VIEW_REGION && rgn == s.GetRegion()->GetOrbitalRegion()) {
		double sx = (sloc.x + rlx) * scale;
		double sy = (sloc.y + rly) * scale;

		shiploc.x = (int) (cx + sx + ox);
		shiploc.y = (int) (cy + sy + oy);

		bool ship_visible = shiploc.x >= 0 && shiploc.x < rect.w &&
		shiploc.y >= 0 && shiploc.y < rect.h;

		if (ship_visible) {
			if (rep < 3) {
				window->FillRect(shiploc.x-2, shiploc.y-2, shiploc.x+2, shiploc.y+2, s.MarkerColor());
				sprite_width = 2;

				if (&s == ship || !IsCrowded(s))
				window->Print(shiploc.x-sprite_width, shiploc.y+sprite_width+2, s.Name());
			}
			else {
				Point heading = s.Heading().OtherHand();
				heading.z = 0;
				heading.Normalize();
				
				double theta = 0;
				
				if (heading.y > 0)
				theta = acos(heading.x);
				else
				theta = -acos(heading.x);

				const double THETA_SLICE  = 4 / PI;
				const double THETA_OFFSET = PI + THETA_SLICE/2;

				int sprite_index = (int) ((theta + THETA_OFFSET) * THETA_SLICE);
				int nsprites     = s.Design()->map_sprites.size();

				if (nsprites) {
					if (sprite_index < 0 || sprite_index >= nsprites)
					sprite_index = sprite_index % nsprites;

					Bitmap* map_sprite = s.Design()->map_sprites[sprite_index];

					Bitmap bmp;
					bmp.CopyBitmap(*map_sprite);
					ColorizeBitmap(bmp, s.MarkerColor());
					sprite_width = bmp.Width()/2;
					int h        = bmp.Height()/2;

					window->DrawBitmap(shiploc.x-sprite_width,
					shiploc.y-h,
					shiploc.x+sprite_width,
					shiploc.y+h,
					&bmp,
					Video::BLEND_ALPHA);
				}

				else {
					theta -= PI/2;

					if (s.IsStatic()) {
						window->FillRect(shiploc.x-6, shiploc.y-6, shiploc.x+6, shiploc.y+6, s.MarkerColor());
						window->DrawRect(shiploc.x-6, shiploc.y-6, shiploc.x+6, shiploc.y+6, Color::White);
					}
					else if (s.IsStarship()) {
						window->FillRect(shiploc.x-4, shiploc.y-4, shiploc.x+4, shiploc.y+4, s.MarkerColor());
						window->DrawRect(shiploc.x-4, shiploc.y-4, shiploc.x+4, shiploc.y+4, Color::White);
					}
					else {
						window->FillRect(shiploc.x-3, shiploc.y-3, shiploc.x+3, shiploc.y+3, s.MarkerColor());
						window->DrawRect(shiploc.x-3, shiploc.y-3, shiploc.x+3, shiploc.y+3, Color::White);
					}
				}

				window->Print(shiploc.x-sprite_width, shiploc.y+sprite_width+2, s.Name());
			}
		}
	}

	// draw nav route:
	// draw current ship marker:
	if (current && Text(s.GetRegion()->Name()) == regions[current_region]->Name()) {
		x1 = (int) (shiploc.x - sprite_width - 1);
		x2 = (int) (shiploc.x + sprite_width + 1);
		y1 = (int) (shiploc.y - sprite_width - 1);
		y2 = (int) (shiploc.y + sprite_width + 1);

		window->DrawRect(x1, y1, x2, y2, Color::White);
	}

	// only see routes for your own team:
	if (s.GetIFF() == 0 || ship && s.GetIFF() == ship->GetIFF()) {
		DrawNavRoute(rgn, s.GetFlightPlan(), s.MarkerColor(), &s, 0);
	}
}

void
MapView::DrawElem(MissionElement& s, bool current, int rep)
{
	if (!mission) return;

	bool visible = editor                        ||
	s.GetIFF() == 0               ||
	s.GetIFF() == mission->Team() ||
	s.IntelLevel() > Intel::KNOWN;

	if (!visible) return;

	OrbitalRegion* rgn = (OrbitalRegion*) regions[current_region];

	if (!rgn) return;

	int x1, y1, x2, y2;
	POINT shiploc;

	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	double scale = c/r;

	double ox = offset_x * scale;
	double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	int sprite_width = 10;

	window->SetFont(font);

	// draw ship icon:
	if (!stricmp(s.Region(), rgn->Name())) {
		double sx = (s.Location().x + rlx) * scale;
		double sy = (s.Location().y + rly) * scale;

		shiploc.x = (int) (cx + sx + ox);
		shiploc.y = (int) (cy + sy + oy);

		bool ship_visible = shiploc.x >= 0 && shiploc.x < rect.w &&
		shiploc.y >= 0 && shiploc.y < rect.h;

		if (ship_visible) {
			if (rep < 3) {
				window->FillRect(shiploc.x-2, shiploc.y-2, shiploc.x+2, shiploc.y+2, s.MarkerColor());
				sprite_width = 2;

				if (!IsCrowded(s))
				window->Print(shiploc.x-sprite_width, shiploc.y+sprite_width+2, s.Name());
			}
			else {
				double theta = s.Heading();

				const double THETA_SLICE  = 4 / PI;
				const double THETA_OFFSET = PI / 2;

				int sprite_index = (int) ((theta + THETA_OFFSET) * THETA_SLICE);
				int nsprites     = 0;
				
				if (s.GetDesign())
				nsprites = s.GetDesign()->map_sprites.size();

				if (nsprites > 0) {
					if (sprite_index < 0 || sprite_index >= nsprites)
					sprite_index = sprite_index % nsprites;

					Bitmap* map_sprite = s.GetDesign()->map_sprites[sprite_index];

					Bitmap bmp;
					bmp.CopyBitmap(*map_sprite);
					ColorizeBitmap(bmp, s.MarkerColor());
					sprite_width = bmp.Width()/2;
					int h        = bmp.Height()/2;

					window->DrawBitmap(shiploc.x-sprite_width,
					shiploc.y-h,
					shiploc.x+sprite_width,
					shiploc.y+h,
					&bmp,
					Video::BLEND_ALPHA);
				}

				else {
					theta -= PI/2;

					if (s.IsStatic()) {
						window->FillRect(shiploc.x-6, shiploc.y-6, shiploc.x+6, shiploc.y+6, s.MarkerColor());
						window->DrawRect(shiploc.x-6, shiploc.y-6, shiploc.x+6, shiploc.y+6, Color::White);
					}
					else if (s.IsStarship()) {
						window->FillRect(shiploc.x-4, shiploc.y-4, shiploc.x+4, shiploc.y+4, s.MarkerColor());
						window->DrawRect(shiploc.x-4, shiploc.y-4, shiploc.x+4, shiploc.y+4, Color::White);
					}
					else {
						window->FillRect(shiploc.x-3, shiploc.y-3, shiploc.x+3, shiploc.y+3, s.MarkerColor());
						window->DrawRect(shiploc.x-3, shiploc.y-3, shiploc.x+3, shiploc.y+3, Color::White);
					}
				}

				char label[64];

				if (s.Count() > 1)
				sprintf(label, "%s x %d", (const char*) s.Name(), s.Count());
				else
				strcpy(label, (const char*) s.Name());

				window->Print(shiploc.x-sprite_width, shiploc.y+sprite_width+2, label);
			}
		}
	}

	// draw nav route:
	// draw current ship marker:
	if (current && s.Region() == regions[current_region]->Name()) {
		x1 = (int) (shiploc.x - sprite_width - 1);
		x2 = (int) (shiploc.x + sprite_width + 1);
		y1 = (int) (shiploc.y - sprite_width - 1);
		y2 = (int) (shiploc.y + sprite_width + 1);

		window->DrawRect(x1, y1, x2, y2, Color::White);
	}

	// only see routes for your own team:
	if (editor || s.GetIFF() == 0 || mission && s.GetIFF() == mission->Team()) {
		DrawNavRoute(rgn, s.NavList(), s.MarkerColor(), 0, &s);
	}
}

void
MapView::DrawNavRoute(OrbitalRegion*       rgn,
List<Instruction>&   s_route,
Color                s_marker,
Ship*                ship,
MissionElement*      elem)
{
	int x1, y1, x2, y2;
	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r  = system->Radius() * zoom;

	if (view_mode == VIEW_REGION) {
		if (!rgn) return;
		r = rgn->Radius() * zoom;
	}

	double scale = c/r;
	double ox = offset_x * scale;
	double oy = offset_y * scale;

	Point  old_loc;
	double old_x  = 0;
	double old_y  = 0;
	bool   old_in = false;

	Point  first_loc;
	int    first_x  = 0;
	int    first_y  = 0;
	bool   first_in = false;

	bool   draw_route = true;
	bool   draw_bold  = false;

	if (ship && ship->GetElementIndex() > 1)
	draw_route = false;

	if (ship && ship == current_ship) {
		s_marker  = s_marker * 1.5;
		draw_bold = true;
	}

	else if (elem && elem == current_elem) {
		s_marker  = s_marker * 1.5;
		draw_bold =  true;
	}

	for (int i = 0; i < s_route.size(); i++) {
		Instruction* navpt = s_route[i];

		if (!stricmp(navpt->RegionName(), rgn->Name())) {
			double nav_x = navpt->Location().x * scale;
			double nav_y = navpt->Location().y * scale;

			int isx = (int) (cx + nav_x + ox);
			int isy = (int) (cy + nav_y + oy);

			if (old_in && draw_route) {
				int iox = (int) (cx + old_x + ox);
				int ioy = (int) (cy + old_y + oy);
				window->DrawLine(iox, ioy, isx, isy, s_marker);

				int x1 = (iox-isx);
				int y1 = (ioy-isy);

				if (draw_bold) {
					if (x1 > y1) {
						window->DrawLine(iox, ioy+1, isx, isy+1, s_marker);
					}
					else {
						window->DrawLine(iox+1, ioy, isx+1, isy, s_marker);
					}
				}

				if ((x1*x1 + y1*y1) > 2000) {
					double dist = Point(navpt->Location() - old_loc).length();

					int imx = (int) (cx + (old_x+nav_x)/2 + ox);
					int imy = (int) (cy + (old_y+nav_y)/2 + oy);

					char dist_txt[32];
					FormatNumber(dist_txt, dist);
					font->SetColor(Color::Gray);
					window->SetFont(font);
					window->Print(imx-20, imy-6, dist_txt);
					font->SetColor(Color::White);
				}
			}

			x1 = isx - 3;
			y1 = isy - 3;
			x2 = isx + 3;
			y2 = isy + 3;

			Color c = Color::White;
			if (navpt->Status() > Instruction::ACTIVE) {
				c = Color::Gray;
			}
			else if (!first_in) {
				first_in  = true;
				first_loc = navpt->Location();
				first_x   = isx;
				first_y   = isy;
			}

			if (draw_route) {
				window->DrawLine(x1, y1, x2, y2, c);
				window->DrawLine(x1, y2, x2, y1, c);

				if (navpt == current_navpt)
				window->DrawRect(x1-2, y1-2, x2+2, y2+2, c);

				char buf[256];
				sprintf(buf, "%d", i+1);
				window->SetFont(font);
				window->Print(x2+3, y1, buf);

				if (navpt == current_navpt) {
					if (navpt->TargetName() && strlen(navpt->TargetName())) {
						sprintf(buf, "%s %s", Game::GetText(Text("MapView.item.") + Instruction::ActionName(navpt->Action())).data(), navpt->TargetName());
						window->Print(x2+3, y1+10, buf);
					}
					else {
						sprintf(buf, "%s", Game::GetText(Text("MapView.item.") + Instruction::ActionName(navpt->Action())).data());
						window->Print(x2+3, y1+10, buf);
					}

					sprintf(buf, "%s", Game::GetText(Text("MapView.item.") + Instruction::FormationName(navpt->Formation())).data());
					window->Print(x2+3, y1+20, buf);

					sprintf(buf, "%d", navpt->Speed());
					window->Print(x2+3, y1+30, buf);

					if (navpt->HoldTime()) {
						char hold_time[32];
						FormatTime(hold_time, navpt->HoldTime());

						sprintf(buf, "%s %s", Game::GetText("MapView.item.Hold").data(), hold_time);
						window->Print(x2+3, y1+40, buf);
					}
				}
			}

			old_loc = navpt->Location();
			old_x   = nav_x;
			old_y   = nav_y;
			old_in  = true;
		}

		else {
			old_loc = navpt->Location();
			old_x   = 0;
			old_y   = 0;
			old_in  = false;
		}
	}

	// if the ship and the first active navpoint are both in the region,
	// draw a line from the ship to the first active navpoint:

	if (first_in) {
		old_in = false;

		if (ship && ship->GetRegion()) {
			old_in = (ship->GetRegion()->GetOrbitalRegion() == rgn);

			if (old_in) {
				old_loc = ship->Location().OtherHand();
				old_x = old_loc.x * scale;
				old_y = old_loc.y * scale;
			}
		}

		else if (elem) {
			old_in = (elem->Region() == rgn->Name()) ? true : false;

			if (old_in) {
				old_loc = elem->Location();
				old_x = old_loc.x * scale;
				old_y = old_loc.y * scale;
			}
		}

		if (old_in) {
			int iox = (int) (cx + old_x + ox);
			int ioy = (int) (cy + old_y + oy);
			window->DrawLine(iox, ioy, first_x, first_y, s_marker);

			int x1 = (iox-first_x);
			int y1 = (ioy-first_y);

			if (draw_bold) {
				if (x1 > y1) {
					window->DrawLine(iox, ioy+1, first_x, first_y+1, s_marker);
				}
				else {
					window->DrawLine(iox+1, ioy, first_x+1, first_y, s_marker);
				}
			}

			if ((x1*x1 + y1*y1) > 2000) {
				double dist = Point(first_loc - old_loc).length();
				double nav_x = first_loc.x * scale;
				double nav_y = first_loc.y * scale;

				int imx = (int) (cx + (old_x+nav_x)/2 + ox);
				int imy = (int) (cy + (old_y+nav_y)/2 + oy);

				char dist_txt[32];
				FormatNumber(dist_txt, dist);
				font->SetColor(Color::Gray);
				window->SetFont(font);
				window->Print(imx-20, imy-6, dist_txt);
				font->SetColor(Color::White);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawCombatantSystem(Combatant* c, Orbital* rgn, int x, int y, int r)
{
	int team = c->GetIFF();
	int x1   = 0;
	int x2   = 0;
	int y1   = y - r;
	int a    = 0;

	switch (team) {
	case 0:  x1 = x - 64;
		x2 = x + 64;
		y1 = y + r + 4;
		a  = DT_CENTER;
		break;

	case 1:  x1 = x - 200;
		x2 = x - r - 4;
		a  = DT_RIGHT;
		break;

	default: x1 = x + r + 4;
		x2 = x + 200;
		a  = DT_LEFT;
		break;
	}

	DrawCombatGroupSystem(c->GetForce(), rgn, x1, x2, y1, a);
}

// +--------------------------------------------------------------------+

void
MapView::DrawCombatGroupSystem(CombatGroup* group, Orbital* rgn, int x1, int x2, int& y, int a)
{
	if (!group || group->IsReserve() || group->CalcValue() < 1)
	return;

	char txt[80];

	if (group->GetRegion() == rgn->Name()) {
		switch (group->Type()) {
		case CombatGroup::CARRIER_GROUP:
		case CombatGroup::BATTLE_GROUP:
		case CombatGroup::DESTROYER_SQUADRON:
			sprintf(txt, "%s '%s'", group->GetShortDescription(), group->Name().data());
			active_window->SetFont(font);
			active_window->DrawText(txt, 0, Rect(x1, y, x2-x1, 12), a);
			y += 10;
			break;

		case CombatGroup::BATTALION:
		case CombatGroup::STATION:
		case CombatGroup::STARBASE:

		case CombatGroup::MINEFIELD:
		case CombatGroup::BATTERY:
		case CombatGroup::MISSILE:

			active_window->SetFont(font);
			active_window->DrawText(group->GetShortDescription(), 0, Rect(x1, y, x2-x1, 12), a);
			y += 10;
			break;

		default:
			break;
		}
	}

	ListIter<CombatGroup> iter = group->GetComponents();
	while (++iter) {
		CombatGroup* g = iter.value();
		DrawCombatGroupSystem(g, rgn, x1, x2, y, a);
	}
}

// +--------------------------------------------------------------------+

void
MapView::DrawCombatGroup(CombatGroup* group, int rep)
{
	// does group even exist yet?
	if (!group || group->IsReserve() || group->CalcValue() < 1)
	return;

	// is group a squadron?  don't draw squadrons on map:
	if (group->Type() >= CombatGroup::WING && group->Type() < CombatGroup::FLEET)
	return;

	// has group been discovered yet?
	CombatGroup* player_group = campaign->GetPlayerGroup();
	if (group->GetIFF() && player_group && player_group->GetIFF() != group->GetIFF())
	if (group->IntelLevel() <= Intel::KNOWN)
	return;

	// has group been destroyed already?
	if (group->CalcValue() < 1)
	return;

	OrbitalRegion* rgn = (OrbitalRegion*) regions[current_region];
	if (!rgn) return;

	POINT shiploc;

	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r = rgn->Radius() * zoom;

	double scale = c/r;

	double ox = offset_x * scale;
	double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	int sprite_width = 10;

	if (group->GetUnits().size() > 0) {
		CombatUnit* unit = 0;
		
		for (int i = 0; i < group->GetUnits().size(); i++) {
			unit = group->GetUnits().at(i);

			if (unit->Count() - unit->DeadCount() > 0)
			break;
		}

		// draw unit icon:
		if (unit->GetRegion() == rgn->Name() && unit->Type() > Ship::LCA && unit->Count() > 0) {
			double sx = (unit->Location().x + rlx) * scale;
			double sy = (unit->Location().y + rly) * scale;

			shiploc.x = (int) (cx + sx + ox);
			shiploc.y = (int) (cy + sy + oy);

			bool ship_visible = shiploc.x >= 0 && shiploc.x < rect.w &&
			shiploc.y >= 0 && shiploc.y < rect.h;

			if (ship_visible) {
				if (rep < 3) {
					window->FillRect(shiploc.x-2, shiploc.y-2, shiploc.x+2, shiploc.y+2, unit->MarkerColor());
					sprite_width = 2;

					char buf[256];
					sprintf(buf, "%s", unit->Name().data());
					window->SetFont(font);
					window->Print(shiploc.x-sprite_width, shiploc.y+sprite_width+2, buf);
				}
				else {
					int sprite_index = 2;
					int nsprites     = 0;
					
					if (unit->GetDesign())
					nsprites = unit->GetDesign()->map_sprites.size();

					if (nsprites) {
						if (sprite_index < 0 || sprite_index >= nsprites)
						sprite_index = sprite_index % nsprites;

						Bitmap* map_sprite = unit->GetDesign()->map_sprites[sprite_index];

						Bitmap bmp;
						bmp.CopyBitmap(*map_sprite);
						ColorizeBitmap(bmp, unit->MarkerColor());
						sprite_width = bmp.Width()/2;
						int h        = bmp.Height()/2;

						window->DrawBitmap(shiploc.x-sprite_width,
						shiploc.y-h,
						shiploc.x+sprite_width,
						shiploc.y+h,
						&bmp,
						Video::BLEND_ALPHA);
					}

					else {
						if (unit->IsStatic()) {
							window->FillRect(shiploc.x-6, shiploc.y-6, shiploc.x+6, shiploc.y+6, unit->MarkerColor());
							window->DrawRect(shiploc.x-6, shiploc.y-6, shiploc.x+6, shiploc.y+6, Color::White);
						}
						else if (unit->IsStarship()) {
							window->FillRect(shiploc.x-4, shiploc.y-4, shiploc.x+4, shiploc.y+4, unit->MarkerColor());
							window->DrawRect(shiploc.x-4, shiploc.y-4, shiploc.x+4, shiploc.y+4, Color::White);
						}
						else {
							window->FillRect(shiploc.x-3, shiploc.y-3, shiploc.x+3, shiploc.y+3, unit->MarkerColor());
							window->DrawRect(shiploc.x-3, shiploc.y-3, shiploc.x+3, shiploc.y+3, Color::White);
						}
					}

					char label[128];
					strcpy(label, unit->GetDescription());
					window->SetFont(font);
					window->Print(shiploc.x-sprite_width, shiploc.y+sprite_width+2, label);
				}
			}
		}
	}

	// recurse
	ListIter<CombatGroup> iter = group->GetComponents();
	while (++iter) {
		CombatGroup* g = iter.value();
		DrawCombatGroup(g, rep);
	}
}

// +--------------------------------------------------------------------+

bool
MapView::IsClutter(Ship& test)
{
	// get leader:
	Ship* lead = test.GetLeader();

	if (lead == &test)   // this is the leader:
	return false;

	// too close?
	if (lead) {
		POINT testloc, leadloc;

		GetShipLoc(test, testloc);
		GetShipLoc(*lead, leadloc);

		double dx = testloc.x - leadloc.x;
		double dy = testloc.y - leadloc.y;
		double d  = dx*dx + dy*dy;

		if (d <= 64)
		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

bool
MapView::IsCrowded(Ship& test)
{
	POINT       testloc, refloc;
	Sim*        sim    = Sim::GetSim();
	Orbital*    rgn    = regions[current_region];
	SimRegion*  simrgn = sim->FindRegion(rgn->Name());

	if (simrgn) {
		GetShipLoc(test, testloc);

		ListIter<Ship> s = simrgn->Ships();
		while (++s) {
			Ship* ref = s.value();

			// too close?
			if (ref && ref != &test) {
				GetShipLoc(*ref, refloc);

				double dx = testloc.x - refloc.x;
				double dy = testloc.y - refloc.y;
				double d  = dx*dx + dy*dy;

				if (d <= 64)
				return true;
			}
		}
	}

	return false;
}

void
MapView::GetShipLoc(Ship& s, POINT& shiploc)
{
	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r  = system->Radius() * zoom;

	OrbitalRegion* rgn = (OrbitalRegion*) regions[current_region];

	if (view_mode == VIEW_REGION) {
		if (!rgn) return;
		r = rgn->Radius() * zoom;
	}

	double scale = c/r;

	double ox = offset_x * scale;
	double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	if (view_mode == VIEW_SYSTEM) {
		rgn = system->ActiveRegion();

		if (rgn) {
			rlx = rgn->Location().x;
			rly = rgn->Location().y;
		}
	}

	if (view_mode == VIEW_SYSTEM || 
			(view_mode == VIEW_REGION && rgn == s.GetRegion()->GetOrbitalRegion())) {
		double sx = (s.Location().x + rlx) * scale;
		double sy = (s.Location().y + rly) * scale;
		
		shiploc.x = (int) (cx + sx + ox);
		shiploc.y = (int) (cy + sy + oy);
	}
	else {
		shiploc.x = -1;
		shiploc.y = -1;
	}
}

// +--------------------------------------------------------------------+

bool
MapView::IsCrowded(MissionElement& test)
{
	POINT       testloc, refloc;
	Sim*        sim    = Sim::GetSim();
	Orbital*    rgn    = regions[current_region];

	if (mission) {
		GetElemLoc(test, testloc);

		ListIter<MissionElement> s = mission->GetElements();
		while (++s) {
			MissionElement* ref = s.value();

			if (ref && ref != &test && !stricmp(ref->Region(), rgn->Name())) {
				GetElemLoc(*ref, refloc);

				double dx = testloc.x - refloc.x;
				double dy = testloc.y - refloc.y;
				double d  = dx*dx + dy*dy;

				if (d <= 64)
				return true;
			}
		}
	}

	return false;
}

void
MapView::GetElemLoc(MissionElement& s, POINT& shiploc)
{
	double cx = rect.w/2;
	double cy = rect.h/2;

	c  = (cx<cy) ? cx : cy;
	r  = system->Radius() * zoom;

	OrbitalRegion* rgn = (OrbitalRegion*) regions[current_region];

	if (view_mode == VIEW_REGION) {
		if (!rgn) return;
		r = rgn->Radius() * zoom;
	}

	double scale = c/r;

	double ox = offset_x * scale;
	double oy = offset_y * scale;

	double rlx = 0;
	double rly = 0;

	if (view_mode == VIEW_SYSTEM) {
		rgn = system->ActiveRegion();

		if (rgn) {
			rlx = rgn->Location().x;
			rly = rgn->Location().y;
		}
	}

	if (view_mode == VIEW_SYSTEM || 
			(view_mode == VIEW_REGION && !stricmp(s.Region(), rgn->Name()))) {
		double sx = (s.Location().x + rlx) * scale;
		double sy = (s.Location().y + rly) * scale;
		
		shiploc.x = (int) (cx + sx + ox);
		shiploc.y = (int) (cy + sy + oy);
	}
	else {
		shiploc.x = -1;
		shiploc.y = -1;
	}
}

// +--------------------------------------------------------------------+

OrbitalRegion*
MapView::GetRegion() const
{ 
	OrbitalRegion* result = 0;

	if (current_region < regions.size())
	result = (OrbitalRegion*) regions[current_region];

	return result;
}

// +--------------------------------------------------------------------+

void
MapView::ZoomIn()
{
	zoom *= 0.9;

	if (view_mode == VIEW_SYSTEM) {
		if (system && zoom * system->Radius() < 2e6) {
			zoom = 2e6 / system->Radius();
		}
	}
	else if (view_mode == VIEW_REGION) {
		OrbitalRegion* rgn = GetRegion();
		if (rgn && zoom * rgn->Radius() < 1e3) {
			zoom = 1e3 / rgn->Radius();
		}
	}
}

void
MapView::ZoomOut()
{
	zoom *= 1.1;

	if (view_mode == VIEW_SYSTEM) {
		if (system && zoom * system->Radius() > 500e9) {
			zoom = 500e9 / system->Radius();
		}
	}
	else if (view_mode == VIEW_REGION) {
		OrbitalRegion* rgn = GetRegion();
		if (rgn && zoom * rgn->Radius() > 1e6) {
			zoom = 1e6 / rgn->Radius();
		}
	}
}

// +--------------------------------------------------------------------+

void
MapView::OnShow()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	dispatch->Register(this);
}

void
MapView::OnHide()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	dispatch->Unregister(this);

	if (captured) {
		ReleaseCapture();
		captured = false;
		Mouse::Show(true);
	}

	dragging = false;
}

// +--------------------------------------------------------------------+

bool
MapView::IsEnabled() const
{
	if (active_window)
	return active_window->IsEnabled();

	return false;
}

bool
MapView::IsVisible() const
{
	if (active_window)
	return active_window->IsVisible();

	return false;
}

bool
MapView::IsFormActive() const
{
	if (active_window)
	return active_window->IsFormActive();

	return false;
}

Rect
MapView::TargetRect() const
{
	if (active_window)
	return active_window->TargetRect();

	return Rect();
}

// +--------------------------------------------------------------------+

int
MapView::OnMouseMove(int x, int y)
{
	if (captured) {
		EventTarget*   test     = 0;
		EventDispatch* dispatch = EventDispatch::GetInstance();
		if (dispatch)
		test = dispatch->GetCapture();

		if (test != this) {
			captured = false;
			Mouse::Show(true);
		}

		else {
			if (dragging) {
				int delta_x = x - mouse_x;
				int delta_y = y - mouse_y;
				
				offset_x += delta_x * r / c;
				offset_y += delta_y * r / c;

				Mouse::SetCursorPos(mouse_x, mouse_y);
			}

			else if (view_mode == VIEW_REGION) {
				double scale = r/c;
				click_x = (x - rect.x - rect.w/2) * scale - offset_x;
				click_y = (y - rect.y - rect.h/2) * scale - offset_y;

				if ((adding_navpt || moving_navpt) && current_navpt) {
					Point loc = current_navpt->Location();
					loc.x = click_x;
					loc.y = click_y;
					current_navpt->SetLocation(loc);
					current_navpt->SetStatus(current_status);
				}

				else if (editor && moving_elem && current_elem) {
					Point loc = current_elem->Location();
					loc.x = click_x;
					loc.y = click_y;
					current_elem->SetLocation(loc);
				}
			}
		}
	}

	return active_window->OnMouseMove(x,y);
}

// +--------------------------------------------------------------------+

int
MapView::OnRButtonDown(int x, int y)
{
	if (!captured)
	captured = SetCapture();

	if (captured) {
		dragging = true;
		mouse_x = x;
		mouse_y = y;
		Mouse::Show(false);
	}

	return active_window->OnRButtonDown(x, y);
}

// +--------------------------------------------------------------------+

int
MapView::OnRButtonUp(int x, int y)
{
	if (captured) {
		ReleaseCapture();
		captured = false;
		Mouse::Show(true);
	}

	dragging = false;

	return active_window->OnRButtonUp(x, y);
}

// +--------------------------------------------------------------------+

int MapView::OnClick()
{
	return active_window->OnClick();
}

int MapView::OnLButtonDown(int x, int y)
{
	if (menu_view && menu_view->IsShown()) {
		// ignore this event...
	}
	else {
		if (!captured)
		captured = SetCapture();

		if (view_mode == VIEW_REGION) {
			double scale = r/c;
			click_x = (x - rect.x - rect.w/2) * scale - offset_x;
			click_y = (y - rect.y - rect.h/2) * scale - offset_y;

			if (current_navpt) {
				Point nloc = current_navpt->Location();
				double dx = nloc.x - click_x;
				double dy = nloc.y - click_y;
				double d  = sqrt(dx*dx + dy*dy);

				if (d < 5e3) {
					moving_navpt = true;

					if (ship && current_ship && ship != current_ship) {
						if (ship->GetElement() && current_ship->GetElement()) {
							if (!ship->GetElement()->CanCommand(current_ship->GetElement())) {
								moving_navpt = false;
							}
						}
					}
					else if (current_elem && NetLobby::GetInstance()) {
						moving_navpt = false;
					}
				}
			}

			else if (editor && current_elem) {
				Point nloc = current_elem->Location();
				double dx = nloc.x - click_x;
				double dy = nloc.y - click_y;
				double d  = sqrt(dx*dx + dy*dy);

				if (d < 5e3) {
					moving_elem = true;

					if (current_elem && NetLobby::GetInstance()) {
						moving_elem = false;
					}
				}
			}
		}
	}

	return active_window->OnLButtonDown(x,y);
}

int MapView::OnLButtonUp(int x, int y)
{
	bool process_event = false;

	if (captured) {
		process_event = true;
		ReleaseCapture();
		captured = false;
		Mouse::Show(true);
	}

	if (process_event && !adding_navpt) {
		if (!moving_navpt && !moving_elem) {
			Button::PlaySound();
		}

		if (view_mode == VIEW_REGION) {
			double scale = r/c;
			click_x = (x - rect.x - rect.w/2) * scale - offset_x;
			click_y = (y - rect.y - rect.h/2) * scale - offset_y;

			if (!scrolling)
			SelectAt(x,y);

			active_window->ClientEvent(EID_MAP_CLICK, x, y);
		}

		else if (!scrolling) {
			SelectAt(x,y);

			active_window->ClientEvent(EID_MAP_CLICK, x, y);
		}
	}

	if ((adding_navpt || moving_navpt) && current_navpt) {
		current_navpt->SetStatus(current_status);

		Sim* sim = Sim::GetSim();
		if (sim) {
			Ship* s = current_ship;

			if (s && s->GetElement()) {
				Element* elem  = s->GetElement();
				int      index = elem->GetNavIndex(current_navpt);

				if (index >= 0)
				NetUtil::SendNavData(false, elem, index-1, current_navpt);
			}
		}
	}

	adding_navpt = false;
	moving_navpt = false;
	moving_elem  = false;

	return active_window->OnLButtonUp(x,y);
}


