/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CombatUnit.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	A ship, station, or ground unit in the dynamic campaign.
*/

#include "MemDebug.h"
#include "CombatUnit.h"
#include "CombatGroup.h"
#include "Campaign.h"
#include "ShipDesign.h"
#include "Ship.h"

#include "Game.h"

// +----------------------------------------------------------------------+

inline double random() { return (double) rand() / (double) RAND_MAX; }

// +----------------------------------------------------------------------+

CombatUnit::CombatUnit(const char* n, const char* reg, int t, const char* d, int c, int i)
: name(n), regnum(reg), type(t), design_name(d), design(0),
count(c), iff(i), leader(false), dead_count(0), available(c),
carrier(0), plan_value(0), launch_time(-1e6), jump_time(0),
sustained_damage(0), target(0), group(0), heading(0)
{ }

CombatUnit::CombatUnit(const CombatUnit& u)
: name(u.name), regnum(u.regnum), type(u.type), design_name(u.design_name),
design(u.design), count(u.count), iff(u.iff),
dead_count(u.dead_count), available(u.available),
leader(u.leader), region(u.region), location(u.location),
carrier(u.carrier), plan_value(0), launch_time(u.launch_time),
jump_time(u.jump_time), sustained_damage(u.sustained_damage),
target(0), group(0), heading(u.heading)
{ }

// +----------------------------------------------------------------------+

const ShipDesign*
CombatUnit::GetDesign()
{
	if (!design)
	design = ShipDesign::Get(design_name);

	return design;
}

int
CombatUnit::GetShipClass() const
{
	if (design)
	return design->type;

	return type;
}

int
CombatUnit::GetValue() const
{
	return GetSingleValue() * LiveCount();
}

int
CombatUnit::GetSingleValue() const
{
	return Ship::Value(GetShipClass());
}

// +----------------------------------------------------------------------+

const char*
CombatUnit::GetDescription() const
{
	if (!design) {
		CombatUnit* pThis = (CombatUnit*) this; // cast-away const
		pThis->GetDesign();
	}

	static char desc[256];

	if (!design) {
		strcpy_s(desc, Game::GetText("[unknown]").data());
	}

	else if (count > 1) {
		sprintf_s(desc, "%dx %s %s", LiveCount(), design->abrv, design->DisplayName());
	}

	else {
		if (regnum.length() > 0)
		sprintf_s(desc, "%s-%s %s", design->abrv, (const char*) regnum, (const char*) name);
		else
		sprintf_s(desc, "%s %s", design->abrv, (const char*) name);

		if (dead_count > 0) {
			strcat_s(desc, " ");
			strcat_s(desc, Game::GetText("killed.in.action"));
		}
	}

	return desc;
}

// +----------------------------------------------------------------------+

bool
CombatUnit::CanLaunch() const
{
	bool result = false;

	switch (type) {
	case Ship::FIGHTER:
	case Ship::ATTACK:   result = (Campaign::Stardate() - launch_time) >= 300;
		break;

	case Ship::CORVETTE:
	case Ship::FRIGATE:
	case Ship::DESTROYER:
	case Ship::CRUISER:
	case Ship::CARRIER:  result = true;
		break;
	}

	return result;
}

// +----------------------------------------------------------------------+

Color
CombatUnit::MarkerColor() const
{
	return Ship::IFFColor(iff);
}

bool
CombatUnit::IsGroundUnit() const
{
	return (design && (design->type & Ship::GROUND_UNITS)) ? true : false;
}

bool
CombatUnit::IsStarship() const
{
	return (design && (design->type & Ship::STARSHIPS)) ? true : false;
}

bool
CombatUnit::IsDropship() const
{
	return (design && (design->type & Ship::DROPSHIPS)) ? true : false;
}

bool
CombatUnit::IsStatic() const
{
	return design && (design->type >= Ship::STATION);
}

// +----------------------------------------------------------------------+

double
CombatUnit::MaxRange() const
{
	return 100e3;
}

double CombatUnit::MaxEffectiveRange() const
{
	return 50e3;
}

double CombatUnit::OptimumRange() const
{
	if (type == Ship::FIGHTER || type == Ship::ATTACK)
	return 15e3;

	return 30e3;
}

// +----------------------------------------------------------------------+

bool CombatUnit::CanDefend(CombatUnit* unit) const
{
	if (unit == 0 || unit == this)
	return false;

	if (type > Ship::STATION)
	return false;

	double distance = (location - unit->location).length();

	if (type > unit->type)
	return false;

	if (distance > MaxRange())
	return false;

	return true;
}

// +----------------------------------------------------------------------+

double CombatUnit::PowerVersus(CombatUnit* tgt) const
{
	if (tgt == 0 || tgt == this || available < 1)
	return 0;

	if (type > Ship::STATION)
	return 0;

	double effectiveness = 1;
	double distance = (location - tgt->location).length();

	if (distance > MaxRange())
	return 0;

	if (distance > MaxEffectiveRange())
	effectiveness = 0.5;

	if (type == Ship::FIGHTER) {
		if (tgt->type == Ship::FIGHTER || tgt->type == Ship::ATTACK)
		return Ship::FIGHTER * 2 * available * effectiveness;
		else
		return 0;
	}
	else if (type == Ship::ATTACK) {
		if (tgt->type > Ship::ATTACK)
		return Ship::ATTACK * 3 * available * effectiveness;
		else
		return 0;
	}
	else if (type == Ship::CARRIER) {
		return 0;
	}
	else if (type == Ship::SWACS) {
		return 0;
	}
	else if (type == Ship::CRUISER) {
		if (tgt->type <= Ship::ATTACK)
		return type * effectiveness;

		else
		return 0;
	}
	else {
		if (tgt->type > Ship::ATTACK)
		return type * effectiveness;
		else
		return type * 0.1 * effectiveness;
	}

	return 0;
}

// +----------------------------------------------------------------------+

int
CombatUnit::AssignMission()
{
	int assign = count;

	if (count > 4)
	assign = 4;

	if (assign > 0) {
		available -= assign;
		launch_time = Campaign::Stardate();
		return assign;
	}

	return 0;
}

// +----------------------------------------------------------------------+

void
CombatUnit::CompleteMission()
{
	Disengage();

	if (count > 4)
	available += 4;

	else
	available += count;
}

// +----------------------------------------------------------------------+

void
CombatUnit::MoveTo(const Point& loc)
{
	if (!carrier)
	location = loc;
	else
	location = carrier->location;
}

// +----------------------------------------------------------------------+

void
CombatUnit::Engage(CombatUnit* tgt)
{
	if (!tgt)
	Disengage();

	else if (!tgt->attackers.contains(this))
	tgt->attackers.append(this);

	target = tgt;
}

void
CombatUnit::Disengage()
{
	if (target)
	target->attackers.remove(this);

	target = 0;
}

// +----------------------------------------------------------------------+

static int KillGroup(CombatGroup* group)
{
	int value_killed = 0;

	if (group) {
		ListIter<CombatUnit>  u_iter = group->GetUnits();
		while (++u_iter) {
			CombatUnit* u = u_iter.value();
			value_killed += u->Kill(u->LiveCount());
		}

		ListIter<CombatGroup> g_iter = group->GetComponents();
		while (++g_iter) {
			CombatGroup* g = g_iter.value();
			value_killed += KillGroup(g);
		}
	}

	return value_killed;
}

int
CombatUnit::Kill(int n)
{
	int killed = n;

	if (killed > LiveCount())
	killed = LiveCount();

	dead_count += killed;

	int value_killed = killed * GetSingleValue();

	if (killed) {
		// if unit could support children, kill them too:
		if (type == Ship::CARRIER ||
				type == Ship::STATION ||
				type == Ship::STARBASE) {

			if (group) {
				ListIter<CombatGroup> iter = group->GetComponents();
				while (++iter) {
					CombatGroup* g = iter.value();
					value_killed += KillGroup(g);
				}
			}
		}
	}

	return value_killed;
}

