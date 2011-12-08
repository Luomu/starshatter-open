/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FlightPlanner.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Flight Planning class for creating navpoint routes for fighter elements.
    Used both by the CarrierAI class and the Flight Dialog.
*/

#include "MemDebug.h"
#include "FlightPlanner.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "Mission.h"
#include "Contact.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Callsign.h"

#include "Game.h"
#include "Random.h"

// +----------------------------------------------------------------------+

FlightPlanner::FlightPlanner(Ship* s)
   : sim(0), ship(s)
{
   sim      = Sim::GetSim();

   patrol_range = (float) (250e3 + 10e3 * (int) Random(0, 8.9));
}

FlightPlanner::~FlightPlanner()
{ }

// +--------------------------------------------------------------------+

void
FlightPlanner::CreatePatrolRoute(Element* elem, int index)
{
   RLoc           rloc;
   Vec3           dummy(0,0,0);
   Point          loc   = ship->Location();
   double         zone  = ship->CompassHeading();
   Instruction*   instr = 0;

   if (ship->IsAirborne())
      loc.y += 8e3;
   else
      loc.y += 1e3;

   loc = loc.OtherHand();

   if (index > 2)
      zone += 170*DEGREES;
   else if (index > 1)
      zone += -90*DEGREES;
   else if (index > 0)
      zone +=  90*DEGREES;

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   rloc.SetDistance(30e3);
   rloc.SetDistanceVar(0);
   rloc.SetAzimuth(-10*DEGREES + zone);
   rloc.SetAzimuthVar(0);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::VECTOR);
   instr->SetSpeed(750);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   if (ship->IsAirborne())
      rloc.SetDistance(140e3);
   else
      rloc.SetDistance(220e3);
   rloc.SetDistanceVar(50e3);
   rloc.SetAzimuth(-20*DEGREES + zone);
   rloc.SetAzimuthVar(15*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::PATROL);
   instr->SetSpeed(500);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);

   rloc.SetReferenceLoc(&instr->GetRLoc());
   rloc.SetDistance(120e3);
   rloc.SetDistanceVar(30e3);
   rloc.SetAzimuth(60*DEGREES + zone);
   rloc.SetAzimuthVar(20*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::PATROL);
   instr->SetSpeed(350);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);

   rloc.SetReferenceLoc(&instr->GetRLoc());
   rloc.SetDistance(120e3);
   rloc.SetDistanceVar(30e3);
   rloc.SetAzimuth(120*DEGREES + zone);
   rloc.SetAzimuthVar(20*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::PATROL);
   instr->SetSpeed(350);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   rloc.SetDistance(40e3);
   rloc.SetDistanceVar(0);
   rloc.SetAzimuth(180*DEGREES + ship->CompassHeading());
   rloc.SetAzimuthVar(0*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::RTB);
   instr->SetSpeed(500);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);
}

// +--------------------------------------------------------------------+

void
FlightPlanner::CreateStrikeRoute(Element* elem, Element* target)
{
   if (!elem) return;

   RLoc           rloc;
   Vec3           dummy(0,0,0);
   Point          loc      = ship->Location();
   double         head     = ship->CompassHeading() + 15*DEGREES;
   double         dist     = 30e3;
   Instruction*   instr    = 0;
   Ship*          tgt_ship = 0;

   if (ship->IsAirborne())
      loc += ship->Cam().vup() * 8e3;
   else
      loc += ship->Cam().vup() * 1e3;

   loc = loc.OtherHand();

   if (target)
      tgt_ship = target->GetShip(1);

   if (tgt_ship) {
      double range = Point(tgt_ship->Location() - ship->Location()).length();

      if (range < 100e3)
         dist = 20e3;
   }

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   rloc.SetDistance(dist);
   rloc.SetDistanceVar(0);
   rloc.SetAzimuth(head);
   rloc.SetAzimuthVar(2*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::VECTOR);
   instr->SetSpeed(750);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);

   if (tgt_ship) {
      Ship*    tgt_ship = target->GetShip(1);
      Point    tgt      = tgt_ship->Location() + tgt_ship->Velocity() * 10;
      Point    mid      = ship->Location() + (tgt - ship->Location()) * 0.5;
      double   beam     = tgt_ship->CompassHeading() + 90*DEGREES;

      if (tgt_ship->IsAirborne())
         tgt += tgt_ship->Cam().vup() * 8e3;
      else
         tgt += tgt_ship->Cam().vup() * 1e3;

      tgt = tgt.OtherHand();
      mid = mid.OtherHand();

      if (tgt_ship && tgt_ship->IsStarship()) {
         rloc.SetReferenceLoc(0);
         rloc.SetBaseLocation(tgt);
         rloc.SetDistance(60e3);
         rloc.SetDistanceVar(5e3);
         rloc.SetAzimuth(beam);
         rloc.SetAzimuthVar(5*DEGREES);

         instr = new(__FILE__,__LINE__) Instruction(tgt_ship->GetRegion(), dummy, Instruction::ASSAULT);
         instr->SetSpeed(750);
         instr->GetRLoc() = rloc;
         instr->SetTarget(target->Name());
         instr->SetFormation(Instruction::TRAIL);

         elem->AddNavPoint(instr);
      }

      if (tgt_ship && tgt_ship->IsStatic()) {
         rloc.SetReferenceLoc(0);
         rloc.SetBaseLocation(mid);
         rloc.SetDistance(60e3);
         rloc.SetDistanceVar(5e3);
         rloc.SetAzimuth(beam);
         rloc.SetAzimuthVar(15*DEGREES);

         instr = new(__FILE__,__LINE__) Instruction(tgt_ship->GetRegion(), dummy, Instruction::VECTOR);
         instr->SetSpeed(750);
         instr->GetRLoc() = rloc;

         elem->AddNavPoint(instr);

         rloc.SetReferenceLoc(0);
         rloc.SetBaseLocation(tgt);
         rloc.SetDistance(40e3);
         rloc.SetDistanceVar(5e3);
         rloc.SetAzimuth(beam);
         rloc.SetAzimuthVar(5*DEGREES);

         int action = Instruction::ASSAULT;
         
         if (tgt_ship->IsGroundUnit())
            action = Instruction::STRIKE;

         instr = new(__FILE__,__LINE__) Instruction(tgt_ship->GetRegion(), dummy, action);
         instr->SetSpeed(750);
         instr->GetRLoc() = rloc;
         instr->SetTarget(target->Name());
         instr->SetFormation(Instruction::TRAIL);

         elem->AddNavPoint(instr);
      }

      else if (tgt_ship && tgt_ship->IsDropship()) {
         rloc.SetReferenceLoc(0);
         rloc.SetBaseLocation(tgt);
         rloc.SetDistance(60e3);
         rloc.SetDistanceVar(5e3);
         rloc.SetAzimuth(tgt_ship->CompassHeading());
         rloc.SetAzimuthVar(20*DEGREES);

         instr = new(__FILE__,__LINE__) Instruction(tgt_ship->GetRegion(), dummy, Instruction::INTERCEPT);
         instr->SetSpeed(750);
         instr->GetRLoc() = rloc;
         instr->SetTarget(target->Name());
         instr->SetFormation(Instruction::SPREAD);

         elem->AddNavPoint(instr);
      }
   }

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   rloc.SetDistance(40e3);
   rloc.SetDistanceVar(0);
   rloc.SetAzimuth(180*DEGREES + ship->CompassHeading());
   rloc.SetAzimuthVar(0*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::RTB);
   instr->SetSpeed(500);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);
}

// +--------------------------------------------------------------------+

void
FlightPlanner::CreateEscortRoute(Element* elem, Element* ward)
{
   if (!elem) return;

   RLoc           rloc;
   Vec3           dummy(0,0,0);
   Point          loc   = ship->Location();
   double         head  = ship->CompassHeading();
   Instruction*   instr = 0;

   if (ship->IsAirborne())
      loc += ship->Cam().vup() * 8e3;
   else
      loc += ship->Cam().vup() * 1e3;

   loc = loc.OtherHand();

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   rloc.SetDistance(30e3);
   rloc.SetDistanceVar(0);
   rloc.SetAzimuth(head);
   rloc.SetAzimuthVar(0);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::VECTOR);
   instr->SetSpeed(750);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);

   if (ward && ward->GetShip(1)) {
      // follow ward's flight plan:
      if (ward->GetFlightPlan().size()) {
         ListIter<Instruction> iter = ward->GetFlightPlan();

         while (++iter) {
            Instruction* ward_instr = iter.value();

            if (ward_instr->Action() != Instruction::RTB) {
               rloc.SetReferenceLoc(&ward_instr->GetRLoc());
               rloc.SetDistance(25e3);
               rloc.SetDistanceVar(5e3);
               rloc.SetAzimuth(0);
               rloc.SetAzimuthVar(90*DEGREES);

               instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::ESCORT);
               instr->SetSpeed(350);
               instr->GetRLoc() = rloc;
               instr->SetTarget(ward->Name());

               elem->AddNavPoint(instr);
            }
         }
      }

      // if ward has no flight plan, just go to a point nearby:
      else {
         rloc.SetReferenceLoc(0);
         rloc.SetBaseLocation(ward->GetShip(1)->Location());
         rloc.SetDistance(25e3);
         rloc.SetDistanceVar(5e3);
         rloc.SetAzimuth(0);
         rloc.SetAzimuthVar(90*DEGREES);

         instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::DEFEND);
         instr->SetSpeed(500);
         instr->GetRLoc() = rloc;
         instr->SetTarget(ward->Name());
         instr->SetHoldTime(15 * 60); // fifteen minutes

         elem->AddNavPoint(instr);
      }
   }

   rloc.SetReferenceLoc(0);
   rloc.SetBaseLocation(loc);
   rloc.SetDistance(40e3);
   rloc.SetDistanceVar(0);
   rloc.SetAzimuth(180*DEGREES + ship->CompassHeading());
   rloc.SetAzimuthVar(0*DEGREES);

   instr = new(__FILE__,__LINE__) Instruction(ship->GetRegion(), dummy, Instruction::RTB);
   instr->SetSpeed(500);
   instr->GetRLoc() = rloc;

   elem->AddNavPoint(instr);
}
