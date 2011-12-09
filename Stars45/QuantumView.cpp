/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         QuantumView.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	View class for Quantum Destination HUD Overlay
*/

#include "MemDebug.h"
#include "QuantumView.h"
#include "QuantumDrive.h"
#include "HUDView.h"
#include "Ship.h"
#include "Element.h"
#include "Sim.h"
#include "StarSystem.h"
#include "FormatUtil.h"

#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Scene.h"
#include "Font.h"
#include "FontMgr.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"
#include "Menu.h"

static Color  hud_color = Color::Black;

// +====================================================================+
//
// QUANTUM DRIVE DESTINATION MENU:
//

static Menu*         quantum_menu   = 0;
static bool          show_menu      = false;

void
QuantumView::Initialize()
{
	static int initialized = 0;
	if (initialized) return;

	quantum_menu = new(__FILE__,__LINE__) Menu(Game::GetText("QuantumView.menu"));

	initialized = 1;
}

void
QuantumView::Close()
{
	delete quantum_menu;
}

// +====================================================================+

QuantumView* QuantumView::quantum_view = 0;

QuantumView::QuantumView(Window* c)
: View(c), sim(0), ship(0), font(0)
{
	quantum_view  = this;
	sim         = Sim::GetSim();

	width       = window->Width();
	height      = window->Height();
	xcenter     = (width  / 2.0) - 0.5;
	ycenter     = (height / 2.0) + 0.5;
	font        = FontMgr::Find("HUD");

	HUDView* hud = HUDView::GetInstance();
	if (hud)
	SetColor(hud->GetTextColor());
}

QuantumView::~QuantumView()
{
	quantum_view = 0;
}

void
QuantumView::OnWindowMove()
{
	width       = window->Width();
	height      = window->Height();
	xcenter     = (width  / 2.0) - 0.5;
	ycenter     = (height / 2.0) + 0.5;
}

// +--------------------------------------------------------------------+

bool
QuantumView::Update(SimObject* obj)
{
	if (obj == ship)
	ship = 0;

	return SimObserver::Update(obj);
}

const char*
QuantumView::GetObserverName() const
{
	return "QuantumView";
}

// +--------------------------------------------------------------------+

void
QuantumView::Refresh()
{
	sim = Sim::GetSim();

	if (sim && ship != sim->GetPlayerShip()) {
		ship = sim->GetPlayerShip();

		if (ship) {
			if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
				ship = 0;
			}
			else {
				Observe(ship);
			}
		}
	}

	if (IsMenuShown()) {
		Rect menu_rect(width-115, 10, 115, 12);

		font->SetColor(hud_color);
		font->SetAlpha(1);
		font->DrawText(quantum_menu->GetTitle(), 0, menu_rect, DT_CENTER);

		menu_rect.y += 15;

		ListIter<MenuItem> item = quantum_menu->GetItems();
		while (++item) {
			item->SetEnabled(true);

			font->DrawText(item->GetText(), 0, menu_rect, DT_LEFT);
			menu_rect.y += 10;
		}
	}
}

// +--------------------------------------------------------------------+

void
QuantumView::ExecFrame()
{
	HUDView* hud = HUDView::GetInstance();
	if (hud) {
		if (hud_color != hud->GetTextColor()) {
			hud_color = hud->GetTextColor();
			SetColor(hud_color);
		}
	}

	static double time_til_change = 0;

	if (time_til_change > 0)
	time_til_change -= Game::GUITime();

	if (time_til_change <= 0) {
		time_til_change = 0;

		if (show_menu) {
			QuantumDrive* quantum_drive = 0;

			if (ship)
			quantum_drive = ship->GetQuantumDrive();

			if (quantum_drive && quantum_drive->ActiveState() != QuantumDrive::ACTIVE_READY) {
				show_menu = false;
				return;
			}

			int max_items = quantum_menu->NumItems();

			for (int i = 0; i < max_items; i++) {
				if (Keyboard::KeyDown('1' + i)) {
					MenuItem* item = quantum_menu->GetItem(i);
					if (item && item->GetEnabled()) {

						SimRegion*  rgn = (SimRegion*) item->GetData();

						if (rgn) {
							quantum_drive->SetDestination(rgn, Point(0,0,0));
							quantum_drive->Engage();
						}

						show_menu = false;
						time_til_change = 0.3;
						break;
					}
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
QuantumView::SetColor(Color c)
{
	hud_color = c;
}

// +--------------------------------------------------------------------+

bool
QuantumView::IsMenuShown()
{
	return show_menu;
}

void
QuantumView::ShowMenu()
{
	if (!ship) return;

	if (!show_menu) {
		if (ship->IsStarship() && ship->GetQuantumDrive()) {
			GetQuantumMenu(ship);
			show_menu = true;
		}

		for (int i = 0; i < 10; i++) {
			if (Keyboard::KeyDown('1' + i)) {
				// just need to clear the key down flag
				// so we don't process old keystrokes
				// as valid menu picks...
			}
		}
	}
}

void
QuantumView::CloseMenu()
{
	show_menu = false;
}

// +--------------------------------------------------------------------+

Menu*
QuantumView::GetQuantumMenu(Ship* s)
{
	if (s && sim) {
		if (s->IsStarship()) {
			quantum_menu->ClearItems();

			SimRegion*  current_region = ship->GetRegion();

			if (!current_region) return 0;

			StarSystem* current_system = current_region->System();

			List<SimRegion> rgn_list;

			ListIter<SimRegion> iter = sim->GetRegions();
			while (++iter) {
				SimRegion*  rgn = iter.value();
				StarSystem* rgn_system = rgn->System();

				if (rgn != ship->GetRegion() && !rgn->IsAirSpace() &&
						rgn_system == current_system) {
					rgn_list.append(rgn);
				}
			}

			// sort local regions by distance from star:
			rgn_list.sort();

			// now add regions in other star systems:
			iter.reset();
			while (++iter) {
				SimRegion*  rgn = iter.value();
				StarSystem* rgn_system = rgn->System();

				if (rgn != ship->GetRegion() && rgn->Type() != SimRegion::AIR_SPACE && 
						rgn_system != current_system && current_region->Links().contains(rgn)) {
					rgn_list.append(rgn);
				}
			}

			int n = 1;
			iter.attach(rgn_list);
			while (++iter) {
				SimRegion*  rgn = iter.value();
				StarSystem* rgn_system = rgn->System();
				char text[64];

				if (rgn_system != current_system)
				sprintf(text, "%d. %s/%s", n++, rgn_system->Name(), rgn->Name());
				else
				sprintf(text, "%d. %s", n++, rgn->Name());

				quantum_menu->AddItem(text, (DWORD) rgn);
			}

			return quantum_menu;
		}
	}

	return 0;
}
