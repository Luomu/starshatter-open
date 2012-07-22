/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         StarshipTacticalAI.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Starship-specific mid-level (tactical) AI
*/

#include "MemDebug.h"
#include "StarshipTacticalAI.h"
#include "ShipAI.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shot.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Contact.h"
#include "WeaponGroup.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "Random.h"

#include "Game.h"

const double STARSHIP_TACTICAL_DROP_TIME = 15;

// +----------------------------------------------------------------------+

StarshipTacticalAI::StarshipTacticalAI(ShipAI* ai)
: TacticalAI(ai), drop_time(1.0e9), bugout(false), ai_level(0), initial_integrity(0)
{
	if (ai && ai->GetShip()) {
		ai_level          = ai->GetAILevel();
		initial_integrity = ai->GetShip()->Integrity();
	}

	switch (ai_level) {
	default:
	case 2:  THREAT_REACTION_TIME =   500; break;
	case 1:  THREAT_REACTION_TIME =  1000; break;
	case 0:  THREAT_REACTION_TIME =  2500; 
		drop_time            =  STARSHIP_TACTICAL_DROP_TIME +
		Random(0, STARSHIP_TACTICAL_DROP_TIME);
		break;
	}
}


// +--------------------------------------------------------------------+

StarshipTacticalAI::~StarshipTacticalAI()
{ }

// +--------------------------------------------------------------------+

void
StarshipTacticalAI::ExecFrame(double seconds)
{
	TacticalAI::ExecFrame(seconds);

	drop_time -= seconds;

	if (drop_time <= 0) {
		drop_time = STARSHIP_TACTICAL_DROP_TIME;

		ship_ai->DropTarget(STARSHIP_TACTICAL_DROP_TIME/4);
	}
}

// +--------------------------------------------------------------------+

void
StarshipTacticalAI::FindThreat()
{
	// pick the closest contact on Threat Warning System:
	Ship*       threat_ship          = 0;
	Shot*       threat_missile       = 0;
	Ship*       rumor                = 0;
	double      threat_dist          = 1e9;
	double      CELL_SIZE            = 20e3;

	threat_level   = 0;
	support_level  = ship->AIValue() / CELL_SIZE;

	ListIter<Contact> iter = ship->ContactList();

	while (++iter) {
		Contact* contact = iter.value();
		Ship*    c_ship  = contact->GetShip();
		Shot*    c_shot  = contact->GetShot();

		if (!c_ship && !c_shot)
		continue;

		if (c_ship && c_ship != ship) {
			double basis    = max(contact->Range(ship), CELL_SIZE);
			double ai_value = c_ship->AIValue() / basis;

			if (c_ship->GetIFF() == ship->GetIFF()) {
				support_level += ai_value;
			}
			else if (ship->GetIFF() > 0 && c_ship->GetIFF() > 0) {
				threat_level  += ai_value;
			}
			else if (c_ship->GetIFF() > 1) { // neutrals should not be afraid of alliance
				threat_level  += ai_value;
			}
		}

		if (contact->Threat(ship) && 
				(Game::GameTime() - contact->AcquisitionTime()) > THREAT_REACTION_TIME) {

			if (c_shot) {
				threat_missile = c_shot;
				rumor = (Ship* ) threat_missile->Owner();
			}
			else {
				double rng = contact->Range(ship);

				if (c_ship && 
						c_ship->Class() != Ship::FREIGHTER &&
						c_ship->Class() != Ship::FARCASTER) {


					if (c_ship->GetTarget() == ship) {
						if (!threat_ship || c_ship->Class() > threat_ship->Class()) {
							threat_ship = c_ship;
							threat_dist = 0;
						}
					}
					else if (rng < threat_dist) {
						threat_ship = c_ship;
						threat_dist = rng;
					}

					CheckBugOut(c_ship, rng);
				}
			}
		}
	}

	if (rumor) {
		iter.reset();

		while (++iter) {
			if (iter->GetShip() == rumor) {
				rumor = 0;
				ship_ai->ClearRumor();
				break;
			}
		}
	}

	ship_ai->SetRumor(rumor);
	ship_ai->SetThreat(threat_ship);
	ship_ai->SetThreatMissile(threat_missile);
}

// +--------------------------------------------------------------------+

void
StarshipTacticalAI::FindSupport()
{
	if (threat_level < 0.01) {
		ship_ai->SetSupport(0);
		return;
	}

	// pick the biggest friendly contact in the sector:
	Ship*       support        = 0;
	double      support_dist   = 1e9;

	ListIter<Contact> c_iter = ship->ContactList();

	while (++c_iter) {
		Contact* contact = c_iter.value();
		if (contact->GetShip() && contact->GetIFF(ship) == ship->GetIFF()) {
			Ship* c_ship = contact->GetShip();

			if (c_ship != ship && c_ship->Class() >= ship->Class()) {
				if (!support || c_ship->Class() > support->Class())
				support = c_ship;
			}
		}
	}

	ship_ai->SetSupport(support);
}

void
StarshipTacticalAI::CheckBugOut(Ship* c_ship, double rng)
{
	// see if carrier should bug out...
	if (!ship || !c_ship || ship->Class() != Ship::CARRIER && ship->Class() != Ship::SWACS)
	return;

	if (bugout)
	return;

	if (ship->GetElement() && ship->GetElement()->GetZoneLock())
	return;

	if (c_ship->Class() < Ship::DESTROYER || c_ship->Class() > Ship::STATION)
	return;

	Starshatter* stars = Starshatter::GetInstance();
	if (stars && stars->InCutscene())
	return;

	double sustained_damage = initial_integrity - ship->Integrity();
	double allowable_damage = ship->Design()->integrity * 0.25;

	if (rng > 50e3 && sustained_damage < allowable_damage)
	return;

	// still here?  we must need to bug out!

	Sim*        sim = Sim::GetSim();
	SimRegion*  dst = 0;

	List<SimRegion>& regions = sim->GetRegions();

	if (regions.size() > 1) {
		int tries = 10;
		while (!dst && tries--) {
			int n = RandomIndex() % regions.size();
			dst = regions[n];

			if (dst == ship->GetRegion() || dst->IsAirSpace())
			dst = 0;
		}
	}

	if (dst) {
		// bug out!
		QuantumDrive* quantum = ship->GetQuantumDrive();
		if (quantum) {
			quantum->SetDestination(dst, Point(0,0,0));
			quantum->Engage();
		}

		// ask highest ranking escort to go with you:
		Element* escort = 0;

		ListIter<Element> iter = sim->GetElements();
		while (++iter) {
			Element* elem = iter.value();

			if (!escort || elem->GetShipClass() > escort->GetShipClass()) {
				if (ship->GetElement()->CanCommand(elem))
				escort = elem;
			}
		}

		if (escort) {
			RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(escort, ship, RadioMessage::QUANTUM_TO);
			if (msg) {
				msg->SetInfo(dst->Name());
				RadioTraffic::Transmit(msg);
			}
		}

		bugout = true;
	}
}
