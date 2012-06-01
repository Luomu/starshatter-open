/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2005. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         WeaponGroup.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Weapon Control Category (Group) class
*/

#include "MemDebug.h"
#include "WeaponGroup.h"
#include "Ship.h"

// +----------------------------------------------------------------------+

WeaponGroup::WeaponGroup(const char* n)
: selected(0), trigger(false), orders(Weapon::MANUAL),
control(Weapon::SINGLE_FIRE), sweep(Weapon::SWEEP_TIGHT),
mass(0.0f), resist(0.0f), name(n), ammo(0)
{ }

// +--------------------------------------------------------------------+

WeaponGroup::~WeaponGroup()
{
	weapons.destroy();
}

// +--------------------------------------------------------------------+

void
WeaponGroup::SetName(const char* n)   
{
	name = n;
}

void
WeaponGroup::SetAbbreviation(const char* a)
{
	abrv = a;
}

// +--------------------------------------------------------------------+

bool
WeaponGroup::IsPrimary() const
{
	if (weapons.size() > 0)
	return weapons[0]->IsPrimary();

	return false;
}

bool
WeaponGroup::IsDrone()   const
{
	if (weapons.size() > 0)
	return weapons[0]->IsDrone();

	return false;
}

bool
WeaponGroup::IsDecoy()   const
{
	if (weapons.size() > 0)
	return weapons[0]->IsDecoy();

	return false;
}

bool
WeaponGroup::IsProbe()   const
{
	if (weapons.size() > 0)
	return weapons[0]->IsProbe();

	return false;
}

bool
WeaponGroup::IsMissile() const
{
	if (weapons.size() > 0)
	return weapons[0]->IsMissile();

	return false;
}

bool
WeaponGroup::IsBeam()    const
{
	if (weapons.size() > 0)
	return weapons[0]->IsBeam();

	return false;
}

// +--------------------------------------------------------------------+

void
WeaponGroup::AddWeapon(Weapon* w)
{
	weapons.append(w);
}

int
WeaponGroup::NumWeapons() const
{
	return weapons.size();
}

List<Weapon>&
WeaponGroup::GetWeapons()
{
	return weapons;
}

bool
WeaponGroup::Contains(const Weapon* w) const
{
	return weapons.contains(w)?true:false;
}

// +--------------------------------------------------------------------+

void
WeaponGroup::SelectWeapon(int n)
{
	if (n >= 0 && n < weapons.size())
	selected = n;
}

void
WeaponGroup::CycleWeapon()
{
	selected++;

	if (selected >= weapons.size())
	selected = 0;
}

Weapon*
WeaponGroup::GetWeapon(int n) const
{
	if (n >= 0 && n < weapons.size())
	return weapons[n];

	return 0;
}

Weapon*
WeaponGroup::GetSelected() const
{
	return weapons[selected];
}

bool
WeaponGroup::CanTarget(DWORD tgt_class) const
{
	if (selected >= 0 && selected < weapons.size())
	return weapons[selected]->CanTarget(tgt_class);

	return false;
}

// +--------------------------------------------------------------------+

void
WeaponGroup::ExecFrame(double seconds)
{
	ammo   = 0;
	mass   = 0.0f;
	resist = 0.0f;

	ListIter<Weapon> iter = weapons;
	while (++iter) {
		Weapon* w = iter.value();
		w->ExecFrame(seconds);

		ammo   += w->Ammo();
		mass   += w->Mass();
		resist += w->Resistance();
	}
}

void
WeaponGroup::CheckAmmo()
{
	ammo   = 0;
	mass   = 0.0f;
	resist = 0.0f;

	ListIter<Weapon> iter = weapons;
	while (++iter) {
		Weapon* w = iter.value();

		ammo   += w->Ammo();
		mass   += w->Mass();
		resist += w->Resistance();
	}
}

// +--------------------------------------------------------------------+

void
WeaponGroup::SetTarget(SimObject* target, System* subtarget)
{
	ListIter<Weapon> w = weapons;
	while (++w)
	w->SetTarget(target, subtarget);
}

SimObject*
WeaponGroup::GetTarget() const
{
	SimObject* target = 0;

	if (weapons.size())
	target = weapons[0]->GetTarget();

	return target;
}

System*
WeaponGroup::GetSubTarget() const
{
	System* subtarget = 0;

	if (weapons.size())
	subtarget = weapons[0]->GetSubTarget();

	return subtarget;
}

void
WeaponGroup::DropTarget()
{
	ListIter<Weapon> w = weapons;
	while (++w)
	w->SetTarget(0, 0);
}

// +--------------------------------------------------------------------+

WeaponDesign*
WeaponGroup::GetDesign() const
{
	if (selected >= 0 && selected < weapons.size())
	return (WeaponDesign*) weapons[selected]->Design();

	return 0;
}

// +--------------------------------------------------------------------+

int
WeaponGroup::Status() const
{
	int status   = System::NOMINAL;
	int critical = true;

	ListIter<Weapon> iter = (List<Weapon>&) weapons; // cast-away const
	while (++iter) {
		Weapon* w = iter.value();

		if (w->Status() < System::NOMINAL)
		status = System::DEGRADED;

		if (w->Status() > System::CRITICAL)
		critical = false;
	}

	if (critical)
	return System::CRITICAL;

	return status;
}

// +--------------------------------------------------------------------+

void
WeaponGroup::SetFiringOrders(int o)
{
	orders = o;

	ListIter<Weapon> w = weapons;
	while (++w)
	w->SetFiringOrders(orders);
}

void
WeaponGroup::SetControlMode(int m)
{
	control = m;

	ListIter<Weapon> w = weapons;
	while (++w)
	w->SetControlMode(control);
}

void
WeaponGroup::SetSweep(int s)
{
	sweep = s;

	ListIter<Weapon> w = weapons;
	while (++w)
	w->SetSweep(sweep);
}

// +--------------------------------------------------------------------+

void
WeaponGroup::PowerOff()
{
	ListIter<Weapon> w = weapons;
	while (++w)
	w->PowerOff();
}

void
WeaponGroup::PowerOn()
{
	ListIter<Weapon> w = weapons;
	while (++w)
	w->PowerOn();
}

// +--------------------------------------------------------------------+

int
WeaponGroup::Value() const
{
	int result = 0;

	for (int i = 0; i < weapons.size(); i++) {
		const Weapon* w = weapons[i];
		result += w->Value();
	}

	return result;
}
