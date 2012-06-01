/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Instruction.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Navigation Point class implementation
*/

#include "MemDebug.h"
#include "Instruction.h"
#include "Element.h"
#include "RadioMessage.h"
#include "Ship.h"
#include "Sim.h"

#include "Game.h"
#include "Text.h"

// +----------------------------------------------------------------------+

Instruction::Instruction(int act, const char* tgt)
: region(0), action(act), formation(0), tgt_name(tgt),
status(PENDING), speed(0), target(0), emcon(0), wep_free(0),
priority(PRIMARY), farcast(0), hold_time(0)
{ }

Instruction::Instruction(const char* rgn, Point loc, int act)
: region(0), action(act), formation(0),
status(PENDING), speed(0), target(0), emcon(0), wep_free(0),
priority(PRIMARY), farcast(0), hold_time(0)
{ 
	rgn_name = rgn;
	rloc.SetBaseLocation(loc);
	rloc.SetDistance(0);
}

Instruction::Instruction(SimRegion* rgn, Point loc, int act)
: region(rgn), action(act), formation(0),
status(PENDING), speed(0), target(0), emcon(0), wep_free(0),
priority(PRIMARY), farcast(0), hold_time(0)
{ 
	rgn_name = region->Name();
	rloc.SetBaseLocation(loc);
	rloc.SetDistance(0);
}

Instruction::Instruction(const Instruction& instr)
: region(instr.region), rgn_name(instr.rgn_name),
rloc(instr.rloc), action(instr.action),
formation(instr.formation), status(instr.status), speed(instr.speed),
target(0), tgt_name(instr.tgt_name), tgt_desc(instr.tgt_desc),
emcon(instr.emcon), wep_free(instr.wep_free),
priority(instr.priority), farcast(instr.farcast),
hold_time(instr.hold_time)
{
	SetTarget(instr.target);
}

Instruction::~Instruction()
{ }

// +--------------------------------------------------------------------+

Instruction&
Instruction::operator = (const Instruction& n)
{
	rgn_name    = n.rgn_name;
	region      = n.region;
	rloc        = n.rloc;
	action      = n.action;
	formation   = n.formation;
	status      = n.status;
	speed       = n.speed;

	tgt_name    = n.tgt_name;
	tgt_desc    = n.tgt_desc;
	target      = 0;
	emcon       = n.emcon;
	wep_free    = n.wep_free;
	priority    = n.priority;
	farcast     = n.farcast;
	hold_time   = n.hold_time;

	SetTarget(n.target);
	return *this;
}

// +--------------------------------------------------------------------+

Point
Instruction::Location() const
{
	Instruction* pThis = (Instruction*) this;
	return pThis->rloc.Location();
}

void
Instruction::SetLocation(const Point& l)
{
	rloc.SetBaseLocation(l);
	rloc.SetReferenceLoc(0);
	rloc.SetDistance(0);
}

// +----------------------------------------------------------------------+

SimObject*
Instruction::GetTarget()
{
	if (!target && tgt_name.length() > 0) {
		Sim*  sim = Sim::GetSim();

		if (sim) {
			Ship* s = sim->FindShip(tgt_name, rgn_name);

			if (s) {
				target = s;
				Observe(target);
			}
		}
	}

	return target;
}

void
Instruction::SetTarget(const char* n)
{
	if (n && *n && tgt_name != n) {
		tgt_name = n;
		tgt_desc = n;

		if (target)
		target = 0;
	}
}

void
Instruction::SetTarget(SimObject* s)
{
	if (s && target != s) {
		tgt_name = s->Name();
		target   = s;
		Observe(target);
	}
}

void
Instruction::SetTargetDesc(const char* d)
{
	if (d && *d)
	tgt_desc = d;
}

void
Instruction::ClearTarget()
{
	if (target) {
		target = 0;
		tgt_name = "";
		tgt_desc = "";
	}
}

// +----------------------------------------------------------------------+

bool
Instruction::Update(SimObject* obj)
{
	if (target == obj)
	target = 0;

	return SimObserver::Update(obj);
}

const char*
Instruction::GetObserverName() const
{
	return "Instruction";
}

// +----------------------------------------------------------------------+

void
Instruction::Evaluate(Ship* ship)
{
	Sim* sim = Sim::GetSim();

	switch (action) {
	case VECTOR:
		break;

	case LAUNCH:
		if (ship->GetFlightPhase() == Ship::ACTIVE)
		SetStatus(COMPLETE);
		break;

	case DOCK:
	case RTB:
		if (sim->GetPlayerShip() == ship &&
				(ship->GetFlightPhase() == Ship::DOCKING ||
					ship->GetFlightPhase() == Ship::DOCKED))
		SetStatus(COMPLETE);
		else if (ship->Integrity() < 1)
		SetStatus(FAILED);
		break;

	case DEFEND:
	case ESCORT:
		{
			bool found = false;
			bool safe  = true;

			ListIter<Element> iter = sim->GetElements();
			while (++iter && !found) {
				Element* e = iter.value();

				if (e->IsFinished() || e->IsSquadron())
				continue;

				if (e->Name() == tgt_name || 
						(e->GetCommander() && e->GetCommander()->Name() == tgt_name)) {

					found = true;

					for (int i = 0; i < e->NumShips(); i++) {
						Ship* s = e->GetShip(i+1);

						if (s && s->Integrity() < 1)
						SetStatus(FAILED);
					}

					if (status == PENDING) {
						// if the element had a flight plan, and all nav points
						// have been addressed, then the element is safe
						if (e->FlightPlanLength() > 0) {
							if (e->GetNextNavPoint() == 0)
							SetStatus(COMPLETE);
							else
							safe = false;
						}
					}
				}
			}

			if (status == PENDING && safe &&
					sim->GetPlayerShip() == ship &&
					(ship->GetFlightPhase() == Ship::DOCKING ||
						ship->GetFlightPhase() == Ship::DOCKED)) {
				SetStatus(COMPLETE);
			}
		}
		break;

	case PATROL:
	case SWEEP:
		{
			Sim* sim   = Sim::GetSim();
			bool alive = false;

			ListIter<Element> iter = sim->GetElements();
			while (++iter) {
				Element* e = iter.value();

				if (e->IsFinished() || e->IsSquadron())
				continue;

				if (e->GetIFF() && e->GetIFF() != ship->GetIFF()) {
					for (int i = 0; i < e->NumShips(); i++) {
						Ship* s = e->GetShip(i+1);

						if (s && s->Integrity() >= 1)
						alive = true;
					}
				}
			}

			if (status == PENDING && !alive) {
				SetStatus(COMPLETE);
			}
		}
		break;

	case INTERCEPT:
	case STRIKE:
	case ASSAULT:
		{
			Sim* sim   = Sim::GetSim();
			bool alive = false;

			ListIter<Element> iter = sim->GetElements();
			while (++iter) {
				Element* e = iter.value();

				if (e->IsFinished() || e->IsSquadron())
				continue;

				if (e->Name() == tgt_name) {
					for (int i = 0; i < e->NumShips(); i++) {
						Ship* s = e->GetShip(i+1);

						if (s && s->Integrity() >= 1)
						alive = true;
					}
				}
			}

			if (status == PENDING && !alive) {
				SetStatus(COMPLETE);
			}
		}
		break;

	case RECON:
		break;

	default:
		break;
	}
}

void
Instruction::SetStatus(int s)
{
	status = s;
}

// +----------------------------------------------------------------------+

const char*
Instruction::GetShortDescription() const
{
	static char desc[256];

	switch (action) {
	case VECTOR:
		if (farcast)
		sprintf_s(desc, Game::GetText("instr.short.farcast").data(), rgn_name.data());
		else
		sprintf_s(desc, Game::GetText("instr.short.vector").data(), rgn_name.data());
		break;

	case LAUNCH:
		sprintf_s(desc, Game::GetText("instr.short.launch").data(), tgt_name.data());
		break;

	case DOCK:
		sprintf_s(desc, Game::GetText("instr.short.dock").data(), tgt_name.data());
		break;

	case RTB:
		sprintf_s(desc, Game::GetText("instr.short.return-to-base").data());
		break;

	case DEFEND:
		if (priority == PRIMARY) {
			sprintf_s(desc, Game::GetText("instr.short.defend").data(), ActionName(action), tgt_desc.data());
		}
		else {
			sprintf_s(desc, Game::GetText("instr.short.protect").data(), tgt_desc.data());
		}
		break;

	case ESCORT:
		if (priority == PRIMARY) {
			sprintf_s(desc, Game::GetText("instr.short.escort").data(), ActionName(action), tgt_desc.data());
		}
		else {
			sprintf_s(desc, Game::GetText("instr.short.protect").data(), tgt_desc.data());
		}
		break;

	case PATROL:
		sprintf_s(desc, Game::GetText("instr.short.patrol").data(),
		tgt_desc.data(),
		rgn_name.data());
		break;

	case SWEEP:
		sprintf_s(desc, Game::GetText("instr.short.sweep").data(),
		tgt_desc.data(),
		rgn_name.data());
		break;

	case INTERCEPT:
		sprintf_s(desc, Game::GetText("instr.short.intercept").data(), tgt_desc.data());
		break;

	case STRIKE:
		sprintf_s(desc, Game::GetText("instr.short.strike").data(), tgt_desc.data());
		break;

	case ASSAULT:
		sprintf_s(desc, Game::GetText("instr.short.assault").data(), tgt_desc.data());
		break;

	case RECON:
		sprintf_s(desc, Game::GetText("instr.short.recon").data(), tgt_desc.data());
		break;

	default:
		sprintf_s(desc, "%s", ActionName(action));
		break;
	}

	if (status != PENDING) {
		strcat_s(desc, " - ");
		strcat_s(desc, Game::GetText(StatusName(status)));
	}

	return desc;
}

// +----------------------------------------------------------------------+

const char*
Instruction::GetDescription() const
{
	static char desc[1024];

	switch (action) {
	case VECTOR:
		if (farcast)
		sprintf_s(desc, Game::GetText("instr.long.farcast").data(), rgn_name.data());
		else
		sprintf_s(desc, Game::GetText("instr.long.vector").data(), rgn_name.data());
		break;

	case LAUNCH:
		sprintf_s(desc, Game::GetText("instr.long.launch").data(), tgt_name.data());
		break;

	case DOCK:
		sprintf_s(desc, Game::GetText("instr.long.dock").data(), tgt_name.data());
		break;

	case RTB:
		sprintf_s(desc, Game::GetText("instr.long.return-to-base").data());
		break;

	case DEFEND:
		if (priority == PRIMARY) {
			sprintf_s(desc, Game::GetText("instr.long.defend").data(), ActionName(action), tgt_desc.data());
		}
		else {
			sprintf_s(desc, Game::GetText("instr.long.protect").data(), tgt_desc.data());
		}
		break;

	case ESCORT:
		if (priority == PRIMARY) {
			sprintf_s(desc, Game::GetText("instr.long.escort").data(), ActionName(action), tgt_desc.data());
		}
		else {
			sprintf_s(desc, Game::GetText("instr.long.protect").data(), tgt_desc.data());
		}
		break;

	case PATROL:
		sprintf_s(desc, Game::GetText("instr.long.patrol").data(),
		tgt_desc.data(),
		rgn_name.data());
		break;

	case SWEEP:
		sprintf_s(desc, Game::GetText("instr.long.sweep").data(),
		tgt_desc.data(),
		rgn_name.data());
		break;

	case INTERCEPT:
		sprintf_s(desc, Game::GetText("instr.long.intercept").data(), tgt_desc.data());
		break;

	case STRIKE:
		sprintf_s(desc, Game::GetText("instr.long.strike").data(), tgt_desc.data());
		break;

	case ASSAULT:
		sprintf_s(desc, Game::GetText("instr.long.assault").data(), tgt_desc.data());
		break;

	case RECON:
		sprintf_s(desc, Game::GetText("instr.long.recon").data(), tgt_desc.data());
		break;

	default:
		sprintf_s(desc, "%s", ActionName(action));
		break;
	}

	if (status != PENDING) {
		strcat_s(desc, " - ");
		strcat_s(desc, Game::GetText(StatusName(status)));
	}

	return desc;
}

// +----------------------------------------------------------------------+

const char*
Instruction::ActionName(int a)
{
	switch (a) {
	case VECTOR:      return "Vector";
	case LAUNCH:      return "Launch";
	case DOCK:        return "Dock";
	case RTB:         return "RTB";

	case DEFEND:      return "Defend";
	case ESCORT:      return "Escort";
	case PATROL:      return "Patrol";
	case SWEEP:       return "Sweep";
	case INTERCEPT:   return "Intercept";
	case STRIKE:      return "Strike";
	case ASSAULT:     return "Assault";
	case RECON:       return "Recon";

	default:          return "Unknown";
	}
}

const char*
Instruction::StatusName(int s)
{
	switch (s) {
	case PENDING:     return "Pending";
	case ACTIVE:      return "Active";
	case SKIPPED:     return "Skipped";
	case ABORTED:     return "Aborted";
	case FAILED:      return "Failed";
	case COMPLETE:    return "Complete";

	default:          return "Unknown";
	}
}

const char*
Instruction::FormationName(int f)
{
	switch (f) {
	case DIAMOND:     return "Diamond";
	case SPREAD:      return "Spread";
	case BOX:         return "Box";
	case TRAIL:       return "Trail";

	default:          return "Unknown";
	}
}

const char*
Instruction::PriorityName(int p)
{
	switch (p) {
	case PRIMARY:     return "Primary";
	case SECONDARY:   return "Secondary";
	case BONUS:       return "Bonus";

	default:          return "Unknown";
	}
}

