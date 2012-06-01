/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         DropShipAI.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Drop Ship (orbit/surface and surface/orbit) AI class
*/

#include "MemDebug.h"
#include "DropShipAI.h"
#include "TacticalAI.h"
#include "Ship.h"
#include "ShipCtrl.h"
#include "Drive.h"
#include "Sim.h"
#include "StarSystem.h"
#include "KeyMap.h"

#include "Game.h"

// +----------------------------------------------------------------------+

DropShipAI::DropShipAI(Ship* s)
: ShipAI(s)
{
	seek_gain = 20;
	seek_damp = 0.5;

	delete tactical;
	tactical = 0;
}

DropShipAI::~DropShipAI()
{
}

// +--------------------------------------------------------------------+

void
DropShipAI::FindObjective()
{
	distance = 0;

	if (!ship)  return;

	Sim*        sim = Sim::GetSim();
	SimRegion*  self_rgn = ship->GetRegion();

	// if making orbit, go up:
	if (self_rgn->Type() == Sim::AIR_SPACE) {
		obj_w = self->Location() + Point(0, 1e3, 0);
	}

	// if breaking orbit, head for terrain region:
	else {
		SimRegion* dst_rgn = sim->FindNearestTerrainRegion(ship);
		Point      dst     = dst_rgn->GetOrbitalRegion()->Location() -
		self_rgn->GetOrbitalRegion()->Location() +
		Point(0, 0, -1e6);
		
		obj_w =    dst.OtherHand();
	}

	// distance from self to navpt:
	distance = Point(obj_w - self->Location()).length();

	// transform into camera coords:
	objective = Transform(obj_w);
	objective.Normalize();
}

// +--------------------------------------------------------------------+

void
DropShipAI::Navigator()
{
	accumulator.Clear();
	magnitude = 0;

	if (other)
	ship->SetFLCSMode(Ship::FLCS_AUTO);
	else
	ship->SetFLCSMode(Ship::FLCS_MANUAL);

	Accumulate(AvoidCollision());
	Accumulate(Seek(objective));

	// are we being asked to flee?
	if (fabs(accumulator.yaw) == 1.0 && accumulator.pitch == 0.0) {
		accumulator.pitch  = -0.7f;
		accumulator.yaw   *= 0.25f;
	}

	self->ApplyRoll((float) (accumulator.yaw * -0.4));
	self->ApplyYaw((float) (accumulator.yaw * 0.2));

	if (fabs(accumulator.yaw) > 0.5 && fabs(accumulator.pitch) < 0.1)
	accumulator.pitch -= 0.1f;

	if (accumulator.pitch != 0)
	self->ApplyPitch((float) accumulator.pitch);

	// if not turning, roll to orient with world coords:
	if (fabs(accumulator.yaw) < 0.1) {
		Point vrt = ((Camera*) &(self->Cam()))->vrt();
		double deflection = vrt.y;
		if (deflection != 0) {
			double theta = asin(deflection/vrt.length());
			self->ApplyRoll(-theta);
		}
	}

	ship->SetThrottle(100);
	ship->ExecFLCSFrame();
}

