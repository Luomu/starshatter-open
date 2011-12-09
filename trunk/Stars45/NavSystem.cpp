/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NavSystem.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Navigation System class implementation
*/

#include "MemDebug.h"
#include "NavSystem.h"
#include "Ship.h"
#include "Sim.h"
#include "HUDSounds.h"
#include "Button.h"
#include "Game.h"

// +----------------------------------------------------------------------+

NavSystem::NavSystem()
: System(COMPUTER, 2, "Auto Nav System", 1, 1,1,1),
autonav(0)
{
	name = Game::GetText("sys.nav-system");
	abrv = Game::GetText("sys.nav-system.abrv");

	power_flags = POWER_WATTS | POWER_CRITICAL;
}

// +----------------------------------------------------------------------+

NavSystem::NavSystem(const NavSystem& s)
: System(s), autonav(0)
{
	Mount(s);

	power_flags = POWER_WATTS | POWER_CRITICAL;
}

// +--------------------------------------------------------------------+

NavSystem::~NavSystem()
{ }

// +--------------------------------------------------------------------+

void
NavSystem::ExecFrame(double seconds)
{
	if (autonav && ship && !ship->GetNextNavPoint())
	autonav = false;

	energy = 0.0f;
	System::ExecFrame(seconds);
}

// +----------------------------------------------------------------------+

bool
NavSystem::AutoNavEngaged()
{
	return ship && autonav && IsPowerOn();
}

void
NavSystem::EngageAutoNav()
{
	if (IsPowerOn() && !autonav) {
		if (!ship->GetNextNavPoint()) {
			Button::PlaySound(Button::SND_REJECT);
		}
		else {
			HUDSounds::PlaySound(HUDSounds::SND_NAV_MODE);
			autonav = true;
		}
	}
}

void
NavSystem::DisengageAutoNav()
{
	if (autonav)
	HUDSounds::PlaySound(HUDSounds::SND_NAV_MODE);

	autonav = false;
}

// +--------------------------------------------------------------------+

void
NavSystem::Distribute(double delivered_energy, double seconds)
{
	if (IsPowerOn()) {
		// convert Joules to Watts:
		energy = (float) (delivered_energy/seconds);

		// brown out:
		if (energy < capacity*0.75f)
		power_on = false;

		// spike:
		else if (energy > capacity*1.5f) {
			power_on = false;
			ApplyDamage(50);
		}
	}
}

