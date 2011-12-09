/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         HardPoint.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	HardPoint class
*/

#include "MemDebug.h"
#include "HardPoint.h"
#include "Weapon.h"
#include "WeaponDesign.h"
#include "Shot.h"
#include "Ship.h"
#include "Sim.h"

// +----------------------------------------------------------------------+

HardPoint::HardPoint(Vec3 muzzle_loc, double az, double el)
: aim_azimuth((float) az), aim_elevation((float) el), muzzle(muzzle_loc)
{
	ZeroMemory(designs, sizeof(designs));
}

// +----------------------------------------------------------------------+

HardPoint::HardPoint(const HardPoint& h)
: aim_azimuth(h.aim_azimuth), aim_elevation(h.aim_elevation), muzzle(h.muzzle),
mount_rel(h.mount_rel), radius(h.radius), hull_factor(h.hull_factor)
{
	CopyMemory(designs, h.designs, sizeof(designs));
}

// +--------------------------------------------------------------------+

HardPoint::~HardPoint()
{
}

// +--------------------------------------------------------------------+

void
HardPoint::Mount(Point loc, float rad, float hull)
{
	mount_rel   = loc;
	radius      = rad;
	hull_factor = hull;
}

// +--------------------------------------------------------------------+

void
HardPoint::AddDesign(WeaponDesign* d)
{
	for (int i = 0; i < MAX_DESIGNS; i++) {
		if (!designs[i]) {
			designs[i] = d;
			return;
		}
	}
}

// +--------------------------------------------------------------------+

Weapon*
HardPoint::CreateWeapon(int type_index)
{
	if (type_index >= 0 && type_index < MAX_DESIGNS && designs[type_index]) {
		Vec3  zero_pt   = Vec3(0.0f, 0.0f, 0.0f);
		Vec3* muzzle_pt = &zero_pt;
		
		if (designs[type_index]->turret.length() == 0)
		muzzle_pt = &muzzle;

		Weapon* missile =  new(__FILE__,__LINE__) Weapon(designs[type_index], 
		1,
		muzzle_pt,
		aim_azimuth,
		aim_elevation);
		missile->SetAbbreviation(GetAbbreviation());
		missile->Mount(mount_rel, radius, hull_factor);
		return missile;
	}

	return 0;
}

// +--------------------------------------------------------------------+

double
HardPoint::GetCarryMass(int type_index)
{
	if (type_index >= 0 && type_index < MAX_DESIGNS && designs[type_index])
	return designs[type_index]->carry_mass;

	return 0;
}

