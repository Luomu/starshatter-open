/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TacticalAI.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Generic Ship Tactical Level AI class
*/

#include "MemDebug.h"
#include "TacticalAI.h"
#include "ShipAI.h"
#include "CarrierAI.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Contact.h"
#include "WeaponGroup.h"
#include "Drive.h"
#include "Hangar.h"
#include "Sim.h"
#include "Shot.h"
#include "Drone.h"
#include "StarSystem.h"

#include "Game.h"
#include "Random.h"

// +----------------------------------------------------------------------+

static int exec_time_seed = 0;

// +----------------------------------------------------------------------+

TacticalAI::TacticalAI(ShipAI* ai)
: ship(0), ship_ai(0), carrier_ai(0), navpt(0), orders(0),
action(0), threat_level(0), support_level(1),
directed_tgtid(0)
{
	if (ai) {
		ship_ai     = ai;
		ship        = ai->GetShip();

		Sim* sim    = Sim::GetSim();

		if (ship && ship->GetHangar() && ship->GetCommandAILevel() > 0 &&
				ship != sim->GetPlayerShip())
		carrier_ai = new(__FILE__,__LINE__) CarrierAI(ship, ship_ai->GetAILevel());
	}

	agression      = 0;
	roe            = FLEXIBLE;
	element_index  = 1;
	exec_time      = exec_time_seed;
	exec_time_seed += 17;
}

TacticalAI::~TacticalAI()
{
	delete carrier_ai;
}

// +--------------------------------------------------------------------+

void
TacticalAI::ExecFrame(double secs)
{
	const int exec_period = 1000;

	if (!ship || !ship_ai)
	return;

	navpt    = ship->GetNextNavPoint();
	orders   = ship->GetRadioOrders();

	if ((int) Game::GameTime() - exec_time > exec_period) {
		element_index = ship->GetElementIndex();

		CheckOrders();
		SelectTarget();
		FindThreat();
		FindSupport();

		if (element_index > 1) {
			int formation = 0;

			if (orders && orders->Formation() >= 0)
			formation = orders->Formation();

			else if (navpt)
			formation = navpt->Formation();

			FindFormationSlot(formation);
		}

		ship_ai->SetNavPoint(navpt);

		if (carrier_ai)
		carrier_ai->ExecFrame(secs);  

		exec_time += exec_period;
	}
}

// +--------------------------------------------------------------------+

void
TacticalAI::CheckOrders()
{
	directed_tgtid = 0;

	if (CheckShipOrders())
	return;

	if (CheckFlightPlan())
	return;

	if (CheckObjectives())
	return;
}

// +--------------------------------------------------------------------+

bool
TacticalAI::CheckShipOrders()
{
	return ProcessOrders();
}

// +--------------------------------------------------------------------+

bool
TacticalAI::CheckObjectives()
{
	bool     processed = false;
	Ship*    ward      = 0;
	Element* elem      = ship->GetElement();

	if (elem) {
		Instruction* obj = elem->GetTargetObjective();

		if (obj) {
			ship_ai->ClearPatrol();

			if (obj->Action()) {
				switch (obj->Action()) {
				case Instruction::INTERCEPT:
				case Instruction::STRIKE:
				case Instruction::ASSAULT:
					{
						SimObject* tgt = obj->GetTarget();
						if (tgt && tgt->Type() == SimObject::SIM_SHIP) {
							roe = DIRECTED;
							SelectTargetDirected((Ship*) tgt);
						}
					}
					break;

				case Instruction::DEFEND:
				case Instruction::ESCORT:
					{
						SimObject* tgt = obj->GetTarget();
						if (tgt && tgt->Type() == SimObject::SIM_SHIP) {
							roe = DEFENSIVE;
							ward = (Ship*) tgt;
						}
					}
					break;

				default:
					break;
				}
			}

			orders    = obj;
			processed = true;
		}
	}

	ship_ai->SetWard(ward);
	return processed;
}

// +--------------------------------------------------------------------+

bool
TacticalAI::ProcessOrders()
{
	ship_ai->ClearPatrol();

	if (orders && orders->EMCON() > 0) {
		int desired_emcon = orders->EMCON();

		if (ship_ai && (ship_ai->GetThreat() || ship_ai->GetThreatMissile()))
		desired_emcon = 3;

		if (ship->GetEMCON() != desired_emcon)
		ship->SetEMCON(desired_emcon);
	}

	if (orders && orders->Action()) {
		switch (orders->Action()) {
		case RadioMessage::ATTACK:
		case RadioMessage::BRACKET:
		case RadioMessage::IDENTIFY:
			{
				bool        tgt_ok   = false;
				SimObject*  tgt      = orders->GetTarget();

				if (tgt && tgt->Type() == SimObject::SIM_SHIP) {
					Ship* tgt_ship = (Ship*) tgt;

					if (CanTarget(tgt_ship)) {
						roe = DIRECTED;
						SelectTargetDirected((Ship*) tgt);

						ship_ai->SetBracket(orders->Action() == RadioMessage::BRACKET);
						ship_ai->SetIdentify(orders->Action() == RadioMessage::IDENTIFY);
						ship_ai->SetNavPoint(0);

						tgt_ok = true;
					}
				}

				if (!tgt_ok)
				ClearRadioOrders();
			}
			break;

		case RadioMessage::ESCORT:
		case RadioMessage::COVER_ME:
			{
				SimObject* tgt = orders->GetTarget();
				if (tgt && tgt->Type() == SimObject::SIM_SHIP) {
					roe = DEFENSIVE;
					ship_ai->SetWard((Ship*) tgt);
					ship_ai->SetNavPoint(0);
				}
				else {
					ClearRadioOrders();
				}
			}
			break;

		case RadioMessage::WEP_FREE:
			roe = AGRESSIVE;
			ship_ai->DropTarget(0.1);
			break;

		case RadioMessage::WEP_HOLD:
		case RadioMessage::FORM_UP:
			roe = NONE;
			ship_ai->DropTarget(5);
			break;

		case RadioMessage::MOVE_PATROL:
			roe = SELF_DEFENSIVE;
			ship_ai->SetPatrol(orders->Location());
			ship_ai->SetNavPoint(0);
			ship_ai->DropTarget(Random(5, 10));
			break;

		case RadioMessage::RTB:
		case RadioMessage::DOCK_WITH:
			roe = NONE;

			ship_ai->DropTarget(10);
			
			if (!ship->GetInbound()) {
				RadioMessage* msg = 0;
				Ship* controller = ship->GetController();

				if (orders->Action() == RadioMessage::DOCK_WITH && orders->GetTarget()) {
					controller = (Ship*) orders->GetTarget();
				}

				if (!controller) {
					Element* elem = ship->GetElement();
					if (elem && elem->GetCommander()) {
						Element* cmdr = elem->GetCommander();
						controller    = cmdr->GetShip(1);
					}
				}

				if (controller && controller->GetHangar() &&
						controller->GetHangar()->CanStow(ship)) {
					SimRegion*  self_rgn = ship->GetRegion();
					SimRegion*  rtb_rgn  = controller->GetRegion();

					if (self_rgn == rtb_rgn) {
						double range = Point(controller->Location() - ship->Location()).length();

						if (range < 50e3) {
							msg = new(__FILE__,__LINE__) RadioMessage(controller, ship, RadioMessage::CALL_INBOUND);
							RadioTraffic::Transmit(msg);
						}
					}
				}
				else {
					ship->ClearRadioOrders();
				}

				ship_ai->SetNavPoint(0);
			}
			break;

		case RadioMessage::QUANTUM_TO:
		case RadioMessage::FARCAST_TO:
			roe = NONE;
			ship_ai->DropTarget(10);
			break;

		}

		action = orders->Action();
		return true;
	}

	// if we had an action before, this must be a "cancel orders" 
	else if (action) {
		ClearRadioOrders();
	}

	return false;
}

void
TacticalAI::ClearRadioOrders()
{
	action = 0;
	roe = FLEXIBLE;

	if (ship_ai)
	ship_ai->DropTarget(0.1);

	if (ship)
	ship->ClearRadioOrders();

}

// +--------------------------------------------------------------------+

bool
TacticalAI::CheckFlightPlan()
{
	Ship* ward = 0;

	// Find next Instruction:
	navpt = ship->GetNextNavPoint();

	roe = FLEXIBLE;

	if (navpt) {
		switch (navpt->Action())  {
		case Instruction::LAUNCH:
		case Instruction::DOCK:
		case Instruction::RTB:     roe = NONE;
			break;

		case Instruction::VECTOR:  roe = SELF_DEFENSIVE;
			break;

		case Instruction::DEFEND:
		case Instruction::ESCORT:  roe = DEFENSIVE;
			break;

		case Instruction::INTERCEPT:
			roe = DIRECTED;
			break;

		case Instruction::RECON:
		case Instruction::STRIKE:
		case Instruction::ASSAULT: roe = DIRECTED;
			break;

		case Instruction::PATROL:
		case Instruction::SWEEP:   roe = FLEXIBLE;
			break;

		default: break;
		}

		if (roe == DEFENSIVE) {
			SimObject* tgt = navpt->GetTarget();

			if (tgt && tgt->Type() == SimObject::SIM_SHIP)
			ward = (Ship*) tgt;
		}


		if (navpt->EMCON() > 0) {
			int desired_emcon = navpt->EMCON();

			if (ship_ai && (ship_ai->GetThreat() || ship_ai->GetThreatMissile()))
			desired_emcon = 3;

			if (ship->GetEMCON() != desired_emcon)
			ship->SetEMCON(desired_emcon);
		}
	}

	if (ship_ai)
	ship_ai->SetWard(ward);

	return (navpt != 0);
}

// +--------------------------------------------------------------------+

void
TacticalAI::SelectTarget()
{
	if (!ship) {
		roe = NONE;
		return;
	}

	// unarmed vessels should never engage an enemy:
	if (ship->Weapons().size() < 1)
	roe = NONE;

	SimObject* target = ship_ai->GetTarget();
	SimObject* ward   = ship_ai->GetWard();

	// if not allowed to engage, drop and return:
	if (roe == NONE) {
		if (target)
		ship_ai->DropTarget();
		return;
	}

	// if we have abandoned our ward, drop and return:
	if (ward && roe != AGRESSIVE) {
		double d = (ward->Location() - ship->Location()).length();
		double safe_zone = 50e3;

		if (target) {
			if (ship->IsStarship())
			safe_zone = 100e3;

			if (d > safe_zone) {
				ship_ai->DropTarget();
				return;
			}
		}
		else {
			if (d > safe_zone) {
				return;
			}
		}
	}

	// already have a target, keep it:
	if (target) {
		if (target->Life()) {
			CheckTarget();

			// frigates need to be ready to abandon ship-type targets
			// in favor of drone-type targets, others should just go
			// with what they have:
			if (ship->Class() != Ship::CORVETTE && ship->Class() != Ship::FRIGATE)
			return;

			// in case the check decided to drop the target:
			target = ship_ai->GetTarget();
		}

		// if the old target is dead, forget it:
		else {
			ship_ai->DropTarget();
			target = 0;
		}
	}

	// if not allowed to acquire, forget it:
	if (ship_ai->DropTime() > 0)
	return;

	if (roe == DIRECTED) {
		if (target && target->Type() == SimObject::SIM_SHIP)
		SelectTargetDirected((Ship*) target);
		else if (navpt && navpt->GetTarget() && navpt->GetTarget()->Type() == SimObject::SIM_SHIP)
		SelectTargetDirected((Ship*) navpt->GetTarget());
		else
		SelectTargetDirected();
	}

	else {
		SelectTargetOpportunity();

		// don't switch one ship target for another...
		if (ship->Class() == Ship::CORVETTE || ship->Class() == Ship::FRIGATE) {
			SimObject* potential_target = ship_ai->GetTarget();
			if (target && potential_target && target != potential_target) {
				if (target->Type()           == SimObject::SIM_SHIP &&
						potential_target->Type() == SimObject::SIM_SHIP) {

					ship_ai->SetTarget(target);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
TacticalAI::SelectTargetDirected(Ship* tgt)
{
	Ship* potential_target = tgt;

	// try to target one of the element's objectives
	// (if it shows up in the contact list)

	if (!tgt) {
		Element* elem = ship->GetElement();

		if (elem) {
			Instruction* objective = elem->GetTargetObjective();

			if (objective) {
				SimObject* obj_sim_obj = objective->GetTarget();
				Ship*      obj_tgt     = 0;

				if (obj_sim_obj && obj_sim_obj->Type() == SimObject::SIM_SHIP)
				obj_tgt = (Ship*) obj_sim_obj;

				if (obj_tgt) {
					ListIter<Contact> contact = ship->ContactList();
					while (++contact && !potential_target) {
						Ship* test = contact->GetShip();

						if (obj_tgt == test) {
							potential_target = test;
						}
					}
				}
			}
		}
	}

	if (!CanTarget(potential_target))
	potential_target = 0;

	ship_ai->SetTarget(potential_target);

	if (tgt && tgt == ship_ai->GetTarget())
	directed_tgtid = tgt->Identity();
	else
	directed_tgtid = 0;
}

// +--------------------------------------------------------------------+

bool
TacticalAI::CanTarget(Ship* tgt)
{
	bool result = false;

	if (tgt && !tgt->InTransition()) {
		if (tgt->IsRogue() || tgt->GetIFF() != ship->GetIFF())
		result = true;
	}

	return result;
}

// +--------------------------------------------------------------------+

void
TacticalAI::SelectTargetOpportunity()
{
	// NON-COMBATANTS do not pick targets of opportunity:
	if (ship->GetIFF() == 0)
	return;

	SimObject* potential_target = 0;

	// pick the closest combatant ship with a different IFF code:
	double target_dist = ship->Design()->commit_range;

	SimObject* ward   = ship_ai->GetWard();

	// FRIGATES are primarily anti-air platforms, but may
	// also attack smaller starships:

	if (ship->Class() == Ship::CORVETTE || ship->Class() == Ship::FRIGATE) {
		Ship* current_ship_target = 0;
		Shot* current_shot_target = 0;

		// if we are escorting a larger warship, it is good to attack
		// the same target as our ward:

		if (ward) {
			Ship* s = (Ship*) ward;

			if (s->Class() > ship->Class()) {
				SimObject* obj = s->GetTarget();

				if (obj && obj->Type() == SimObject::SIM_SHIP) {
					current_ship_target = (Ship*) obj;
					target_dist = (ship->Location() - obj->Location()).length();
				}
			}
		}

		ListIter<Contact> contact = ship->ContactList();
		while (++contact) {
			Ship* c_ship = contact->GetShip();
			Shot* c_shot = contact->GetShot();

			if (!c_ship && !c_shot)
			continue;

			int   c_iff  = contact->GetIFF(ship);
			bool  rogue  = c_ship && c_ship->IsRogue();
			bool  tgt_ok = c_iff > 0               &&
			c_iff != ship->GetIFF() &&
			c_iff < 1000;

			if (rogue || tgt_ok) {
				if (c_ship && c_ship != ship && !c_ship->InTransition()) {
					if (c_ship->Class() <  Ship::DESTROYER || 
							(c_ship->Class() >= Ship::MINE && c_ship->Class() <= Ship::SWACS)) {
						// found an enemy, check distance:
						double dist = (ship->Location() - c_ship->Location()).length();

						if (dist < 0.75 * target_dist &&
								(!current_ship_target || c_ship->Class() <= current_ship_target->Class())) {
							current_ship_target = c_ship;
							target_dist = dist;
						}
					}
				}

				else if (c_shot) {
					// found an enemy shot, is there enough time to engage?
					if (c_shot->GetEta() < 3)
					continue;

					// found an enemy shot, check distance:
					double dist = (ship->Location() - c_shot->Location()).length();

					if (!current_shot_target) {
						current_shot_target = c_shot;
						target_dist = dist;
					}

					// is this shot a better target than the one we've found?
					else {
						Ship* ward = ship_ai->GetWard();

						if ((c_shot->IsTracking(ward) || c_shot->IsTracking(ship)) &&
								(!current_shot_target->IsTracking(ward) || 
									!current_shot_target->IsTracking(ship))) {
							current_shot_target = c_shot;
							target_dist = dist;
						}
						else if (dist < target_dist) {
							current_shot_target = c_shot;
							target_dist = dist;
						}
					}
				}
			}
		}

		if (current_shot_target)
		potential_target = current_shot_target;
		else
		potential_target = current_ship_target;
	}

	// ALL OTHER SHIP CLASSES ignore fighters and only engage
	// other starships:

	else {
		List<Ship> ward_threats;

		ListIter<Contact> contact = ship->ContactList();
		while (++contact) {
			Ship* c_ship = contact->GetShip();

			if (!c_ship)
			continue;

			int   c_iff  = contact->GetIFF(ship);
			bool  rogue  = c_ship->IsRogue();
			bool  tgt_ok = c_ship != ship          && 
			c_iff > 0               && 
			c_iff != ship->GetIFF() && 
			!c_ship->InTransition();

			if (rogue || tgt_ok) {
				if (c_ship->IsStarship() || c_ship->IsStatic()) {
					// found an enemy, check distance:
					double dist = (ship->Location() - c_ship->Location()).length();

					if (dist < 0.75 * target_dist) {
						potential_target = c_ship;
						target_dist = dist;
					}

					if (ward && c_ship->IsTracking(ward)) {
						ward_threats.append(c_ship);
					}
				}
			}
		}

		// if this ship is protecting a ward,
		// prefer targets that are threatening that ward:
		if (potential_target && ward_threats.size() && !ward_threats.contains((Ship*)potential_target)) {
			target_dist *= 2;
			
			ListIter<Ship> iter = ward_threats;
			while (++iter) {
				Ship* threat = iter.value();

				double dist = (ward->Location() - threat->Location()).length();

				if (dist < target_dist) {
					potential_target = threat;
					target_dist = dist;
				}
			}
		}
	}

	if (ship->Class() != Ship::CARRIER)
	ship_ai->SetTarget(potential_target);
}

// +--------------------------------------------------------------------+

void
TacticalAI::CheckTarget()
{
	SimObject* tgt = ship_ai->GetTarget();

	if (!tgt) return;

	if (tgt->GetRegion() != ship->GetRegion()) {
		ship_ai->DropTarget();
		return;
	}

	if (tgt->Type() == SimObject::SIM_SHIP) {
		Ship* target = (Ship*) tgt;

		// has the target joined our side?
		if (target->GetIFF() == ship->GetIFF() && !target->IsRogue()) {
			ship_ai->DropTarget();
			return;
		}

		// is the target already jumping/breaking/dying?
		if (target->InTransition()) {
			ship_ai->DropTarget();
			return;
		}

		// have we been ordered to pursue the target?
		if (directed_tgtid) {
			if (directed_tgtid != target->Identity()) {
				ship_ai->DropTarget();
			}

			return;
		}

		// can we catch the target?
		if (target->Design()->vlimit  <= ship->Design()->vlimit ||
				ship->Velocity().length() <= ship->Design()->vlimit)
		return;

		// is the target now out of range?
		WeaponDesign* wep_dsn = ship->GetPrimaryDesign();
		if (!wep_dsn)
		return;

		// compute the "give up" range:
		double drop_range = 3 * wep_dsn->max_range;
		if (drop_range > 0.75 * ship->Design()->commit_range)
		drop_range = 0.75 * ship->Design()->commit_range;

		double range = Point(target->Location() - ship->Location()).length();
		if (range < drop_range)
		return;

		// is the target closing or separating?
		Point  delta = (target->Location() + target->Velocity()) -
		(ship->Location()   + ship->Velocity());

		if (delta.length() < range)
		return;

		ship_ai->DropTarget();
	}

	else if (tgt->Type() == SimObject::SIM_DRONE) {
		Drone* drone = (Drone*) tgt;

		// is the target still a threat?
		if (drone->GetEta() < 1 || drone->GetTarget() == 0)
		ship_ai->DropTarget();
	}
}

// +--------------------------------------------------------------------+

void
TacticalAI::FindThreat()
{
	// pick the closest contact on Threat Warning System:
	Ship*       threat               = 0;
	Shot*       threat_missile       = 0;
	Ship*       rumor                = 0;
	double      threat_dist          = 1e9;
	const DWORD THREAT_REACTION_TIME = 1000; // 1 second

	ListIter<Contact> iter = ship->ContactList();

	while (++iter) {
		Contact* contact = iter.value();

		if (contact->Threat(ship) && 
				(Game::GameTime() - contact->AcquisitionTime()) > THREAT_REACTION_TIME) {

			if (contact->GetShot()) {
				threat_missile = contact->GetShot();
				rumor = (Ship*) threat_missile->Owner();
			}
			else {
				double rng = contact->Range(ship);

				Ship* c_ship = contact->GetShip();
				if (c_ship && !c_ship->InTransition()    &&
						c_ship->Class() != Ship::FREIGHTER &&
						c_ship->Class() != Ship::FARCASTER) {

					if (c_ship->GetTarget() == ship) {
						if (!threat || c_ship->Class() > threat->Class()) {
							threat      = c_ship;
							threat_dist = 0;
						}
					}
					else if (rng < threat_dist) {
						threat      = c_ship;
						threat_dist = rng;
					}
				}
			}
		}
	}

	if (rumor && !rumor->InTransition()) {
		iter.reset();

		while (++iter) {
			if (iter->GetShip() == rumor) {
				rumor = 0;
				ship_ai->ClearRumor();
				break;
			}
		}
	}
	else {
		rumor = 0;
		ship_ai->ClearRumor();
	}

	ship_ai->SetRumor(rumor);
	ship_ai->SetThreat(threat);
	ship_ai->SetThreatMissile(threat_missile);
}

// +--------------------------------------------------------------------+

void
TacticalAI::FindSupport()
{
	if (!ship_ai->GetThreat()) {
		ship_ai->SetSupport(0);
		return;
	}

	// pick the biggest friendly contact in the sector:
	Ship*       support        = 0;
	double      support_dist   = 1e9;

	ListIter<Contact> contact = ship->ContactList();

	while (++contact) {
		if (contact->GetShip() && contact->GetIFF(ship) == ship->GetIFF()) {
			Ship* c_ship = contact->GetShip();

			if (c_ship != ship && c_ship->Class() >= ship->Class() && !c_ship->InTransition()) {
				if (!support || c_ship->Class() > support->Class())
				support = c_ship;
			}
		}
	}

	ship_ai->SetSupport(support);
}

// +--------------------------------------------------------------------+

void
TacticalAI::FindFormationSlot(int formation)
{
	// find the formation delta:
	int s = element_index - 1;
	Point delta(10*s, 0, 10*s);

	// diamond:
	if (formation == Instruction::DIAMOND) {
		switch (element_index) {
		case 2:  delta = Point( 10,  0, -12); break;
		case 3:  delta = Point(-10,  0, -12); break;
		case 4:  delta = Point(  0,  0, -24); break;
		}
	}

	// spread:
	if (formation == Instruction::SPREAD) {
		switch (element_index) {
		case 2:  delta = Point( 15, 0, 0); break;
		case 3:  delta = Point(-15, 0, 0); break;
		case 4:  delta = Point(-30, 0, 0); break;
		}
	}

	// box:
	if (formation == Instruction::BOX) {
		switch (element_index) {
		case 2:  delta = Point(15,  0,   0); break;
		case 3:  delta = Point( 0, -1, -15); break;
		case 4:  delta = Point(15, -1, -15); break;
		}
	}

	// trail:
	if (formation == Instruction::TRAIL) {
		delta = Point(0, 0, -15*s);
	}

	ship_ai->SetFormationDelta(delta * ship->Radius() * 2);
}
