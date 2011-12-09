/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         SimEvent.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Simulation Events for mission summary
*/

#include "MemDebug.h"
#include "SimEvent.h"
#include "Sim.h"
#include "Game.h"

// +====================================================================+

List<ShipStats>   records;

// +====================================================================+

SimEvent::SimEvent(int e, const char* t, const char* i)
: event(e), count(0)
{
	Sim* sim = Sim::GetSim();
	if (sim) {
		time = (int) sim->MissionClock();
	}
	else {
		time = (int) (Game::GameTime()/1000);
	}

	SetTarget(t);
	SetInfo(i);
}

SimEvent::~SimEvent()
{
}

// +--------------------------------------------------------------------+

void
SimEvent::SetTime(int t)
{
	time = t;
}

void
SimEvent::SetTarget(const char* t)
{
	if (t && t[0])
	target = t;
}

void
SimEvent::SetInfo(const char* i)
{
	if (i && i[0])
	info = i;
}

void
SimEvent::SetCount(int c)
{
	count = c;
}

Text
SimEvent::GetEventDesc() const
{
	switch (event) {
	case LAUNCH:         return Game::GetText("sim.event.Launch");
	case DOCK:           return Game::GetText("sim.event.Dock");
	case LAND:           return Game::GetText("sim.event.Land");
	case EJECT:          return Game::GetText("sim.event.Eject");
	case CRASH:          return Game::GetText("sim.event.Crash");
	case COLLIDE:        return Game::GetText("sim.event.Collision With");
	case DESTROYED:      return Game::GetText("sim.event.Destroyed By");
	case MAKE_ORBIT:     return Game::GetText("sim.event.Make Orbit");
	case BREAK_ORBIT:    return Game::GetText("sim.event.Break Orbit");
	case QUANTUM_JUMP:   return Game::GetText("sim.event.Quantum Jump");
	case LAUNCH_SHIP:    return Game::GetText("sim.event.Launch Ship");
	case RECOVER_SHIP:   return Game::GetText("sim.event.Recover Ship");
	case FIRE_GUNS:      return Game::GetText("sim.event.Fire Guns");
	case FIRE_MISSILE:   return Game::GetText("sim.event.Fire Missile");
	case DROP_DECOY:     return Game::GetText("sim.event.Drop Decoy");
	case GUNS_KILL:      return Game::GetText("sim.event.Guns Kill");
	case MISSILE_KILL:   return Game::GetText("sim.event.Missile Kill");
	case LAUNCH_PROBE:   return Game::GetText("sim.event.Launch Probe");
	case SCAN_TARGET:    return Game::GetText("sim.event.Scan Target");
	default:             return Game::GetText("sim.event.no event");
	}
}

// +====================================================================+

ShipStats::ShipStats(const char* n, int i)
: name(n), iff(i), kill1(0), kill2(0), lost(0), coll(0), points(0),
cmd_points(0), gun_shots(0), gun_hits(0), missile_shots(0), missile_hits(0),
combat_group(0), combat_unit(0), player(false), ship_class(0), elem_index(-1)
{
	if (!n || !n[0])
	name = Game::GetText("[unknown]");
}

ShipStats::~ShipStats()
{
	events.destroy();
}

// +--------------------------------------------------------------------+

void
ShipStats::SetType(const char* t)
{
	if (t && t[0])
	type = t;
}

void
ShipStats::SetRole(const char* r)
{
	if (r && r[0])
	role = r;
}

void
ShipStats::SetRegion(const char* r)
{
	if (r && r[0])
	region = r;
}

void
ShipStats::SetCombatGroup(CombatGroup* g)
{
	combat_group = g;
}

void
ShipStats::SetCombatUnit(CombatUnit* u)
{
	combat_unit = u;
}

void
ShipStats::SetElementIndex(int n)
{
	elem_index = n;
}

void
ShipStats::SetPlayer(bool p)
{
	player = p;
}

// +--------------------------------------------------------------------+

void
ShipStats::Summarize()
{
	kill1 = 0;
	kill2 = 0;
	lost  = 0;
	coll  = 0;

	ListIter<SimEvent> iter = events;
	while (++iter) {
		SimEvent* event = iter.value();
		int       code  = event->GetEvent();

		if (code == SimEvent::GUNS_KILL)
		kill1++;

		else if (code == SimEvent::MISSILE_KILL)
		kill2++;

		else if (code == SimEvent::DESTROYED)
		lost++;

		else if (code == SimEvent::CRASH)
		coll++;

		else if (code == SimEvent::COLLIDE)
		coll++;
	}
}

// +--------------------------------------------------------------------+

SimEvent*
ShipStats::AddEvent(SimEvent* e)
{
	events.append(e);
	return e;
}

SimEvent*
ShipStats::AddEvent(int event, const char* tgt, const char* info)
{
	SimEvent* e = new(__FILE__,__LINE__) SimEvent(event, tgt, info);
	events.append(e);
	return e;
}

bool
ShipStats::HasEvent(int event)
{
	for (int i = 0; i < events.size(); i++)
	if (events[i]->GetEvent() == event)
	return true;

	return false;
}

// +--------------------------------------------------------------------+

void ShipStats::Initialize()  { records.destroy(); }
void ShipStats::Close()       { records.destroy(); }

// +--------------------------------------------------------------------+

int
ShipStats::NumStats()
{
	return records.size();
}

ShipStats*
ShipStats::GetStats(int i)
{
	if (i >= 0 && i < records.size())
	return records.at(i);

	return 0;
}

ShipStats*
ShipStats::Find(const char* name)
{
	if (name && name[0]) {
		ListIter<ShipStats> iter = records;
		while (++iter) {
			ShipStats* stats = iter.value();
			if (!strcmp(stats->GetName(), name))
			return stats;
		}

		ShipStats* stats = new(__FILE__,__LINE__) ShipStats(name);
		records.append(stats);
		return stats;
	}

	return 0;
}

