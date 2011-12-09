/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Component.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Generic ship system sub-component class 
*/

#include "MemDebug.h"
#include "Component.h"
#include "System.h"
#include "Game.h"

// +----------------------------------------------------------------------+

ComponentDesign::ComponentDesign()
: repair_time(0.0f), replace_time(0.0f), spares(0), affects(0)
{ }

// +----------------------------------------------------------------------+

ComponentDesign::~ComponentDesign()
{ }

// +----------------------------------------------------------------------+

Component::Component(ComponentDesign* d, System* s)
: design(d), system(s),
status(NOMINAL), availability(100.0f), time_remaining(0.0f),
spares(0), jerried(0)
{
	if (design)
	spares = design->spares;
}

// +----------------------------------------------------------------------+

Component::Component(const Component& c)
: design(c.design), system(c.system),
status(c.status), availability(c.availability), time_remaining(c.time_remaining),
spares(c.spares), jerried(c.jerried)
{
}

// +--------------------------------------------------------------------+

Component::~Component()
{ }

// +--------------------------------------------------------------------+

void
Component::ExecMaintFrame(double seconds)
{
	if (status > NOMINAL) {
		time_remaining -= (float) seconds;

		// when repairs are complete:
		if (time_remaining <= 0) {
			if (status == REPAIR) {
				// did we just jerry-rig a failed component?
				if (availability < 50)
				jerried++;

				if (jerried < 5)
				availability += 50.0f - 10 * jerried;
				if (availability > 100) availability = 100.0f;
			}
			else {
				availability = 100.0f;
			}

			if (availability > 99)
			status = NOMINAL;
			else if (availability > 49)
			status = DEGRADED;
			else
			status = CRITICAL;

			time_remaining = 0.0f;

			if (system)
			system->CalcStatus();
		}
	}
}

// +--------------------------------------------------------------------+

void
Component::ApplyDamage(double damage)
{
	availability -= (float) damage;
	if (availability < 1) availability = 0.0f;

	if (status < REPLACE) {
		if (availability > 99)
		status = NOMINAL;
		else if (availability > 49)
		status = DEGRADED;
		else
		status = CRITICAL;
	}

	if (system)
	system->CalcStatus();
}

// +--------------------------------------------------------------------+

void
Component::Repair()
{
	if (status < NOMINAL) {
		status = REPAIR;
		time_remaining = design->repair_time;

		if (system)
		system->CalcStatus();
	}
}

// +--------------------------------------------------------------------+

void
Component::Replace()
{
	if (status <= NOMINAL) {
		status = REPLACE;
		spares--;
		time_remaining = design->replace_time;

		if (system)
		system->CalcStatus();
	}
}

// +--------------------------------------------------------------------+

float
Component::Availability() const
{
	if (status > NOMINAL && availability > 50)
	return 50.0f;

	return availability;
}

float
Component::TimeRemaining() const
{
	return (float) time_remaining;
}

int
Component::SpareCount() const
{
	return spares;
}

bool
Component::IsJerried() const
{
	return jerried?true:false;
}

int
Component::NumJerried() const
{
	return jerried;
}