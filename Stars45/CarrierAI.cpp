/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CarrierAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    "Air Boss" AI class for managing carrier fighter squadrons
*/

#include "MemDebug.h"
#include "CarrierAI.h"
#include "ShipAI.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "FlightPlanner.h"
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
#include "NetUtil.h"

#include "Game.h"
#include "Random.h"

// +----------------------------------------------------------------------+

CarrierAI::CarrierAI(Ship* s, int level)
   : sim(0), ship(s), hangar(0), exec_time(0), flight_planner(0),
     hold_time(0), ai_level(level)
{
   if (ship) {
      sim      = Sim::GetSim();
      hangar = ship->GetHangar();

      for (int i = 0; i < 4; i++)
         patrol_elem[i] = 0;

      if (ship)
         flight_planner = new(__FILE__,__LINE__) FlightPlanner(ship);

      hold_time = (int) Game::GameTime();
   }
}

CarrierAI::~CarrierAI()
{
   delete flight_planner;
}

// +--------------------------------------------------------------------+

void
CarrierAI::ExecFrame(double secs)
{
   const int INIT_HOLD   = 15000;
   const int EXEC_PERIOD =  3000;

   if (!sim || !ship || !hangar)
      return;

   if (((int) Game::GameTime() - hold_time >= INIT_HOLD) && 
       ((int) Game::GameTime() - exec_time >  EXEC_PERIOD)) {

      CheckHostileElements();
      CheckPatrolCoverage();

      exec_time = (int) Game::GameTime();
   }
}

// +--------------------------------------------------------------------+

bool
CarrierAI::CheckPatrolCoverage()
{
   const DWORD PATROL_PERIOD = 900 * 1000;

   // pick up existing patrol elements:

   ListIter<Element> iter = sim->GetElements();
   while (++iter) {
      Element* elem = iter.value();

      if (elem->GetCarrier()   == ship                   &&
         (elem->Type()         == Mission::PATROL        ||
          elem->Type()         == Mission::SWEEP         ||
          elem->Type()         == Mission::AIR_PATROL    ||
          elem->Type()         == Mission::AIR_SWEEP)    &&
         !elem->IsSquadron()                             &&
         !elem->IsFinished()) {

         bool found = false;
         int  open  = -1;

         for (int i = 0; i < 4; i++) {
            if (patrol_elem[i] == elem)
               found = true;

            else if (patrol_elem[i] == 0 && open < 0)
               open = i;
         }

         if (!found && open >= 0) {
            patrol_elem[open] = elem;
         }
      }
   }

   // manage the four screening patrols:

   for (int i = 0; i < 4; i++) {
      Element* elem = patrol_elem[i];

      if (elem) {
         if (elem->IsFinished()) {
            patrol_elem[i] = 0;
         }

         else {
            LaunchElement(elem);
         }
      }

      else if (Game::GameTime() - hangar->GetLastPatrolLaunch() > PATROL_PERIOD ||
               hangar->GetLastPatrolLaunch() == 0) {
         Element* patrol = CreatePackage(0, 2, Mission::PATROL, 0, "ACM Medium Range");
         if (patrol) {
            patrol_elem[i] = patrol;

            if (flight_planner)
               flight_planner->CreatePatrolRoute(patrol, i);

            hangar->SetLastPatrolLaunch(Game::GameTime());
            return true;
         }
      }
   }

   return false;
}

// +--------------------------------------------------------------------+

bool
CarrierAI::CheckHostileElements()
{
   List<Element>     assigned;
   ListIter<Element> iter = sim->GetElements();
   while (++iter) {
      Element* elem = iter.value();

      // if this element is hostile to us
      // or if the element is a target objective
      // of the carrier, or is hostile to any
      // of our squadrons...

      bool hostile = false;

      if (elem->IsHostileTo(ship) || elem->IsObjectiveTargetOf(ship)) {
         hostile = true;
      }
      else {
         for (int i = 0; i < hangar->NumSquadrons() && !hostile; i++) {
            int squadron_iff = hangar->SquadronIFF(i);

            if (elem->IsHostileTo(squadron_iff))
               hostile = true;
         }
      }

      if (hostile) {
         sim->GetAssignedElements(elem, assigned);

         // is one of our fighter elements already assigned to this target?
         bool found = false;
         ListIter<Element> a_iter = assigned;
         while (++a_iter && !found) {
            Element* a = a_iter.value();

            if (a->GetCarrier() == ship)
               found = true;
         }

         // nobody is assigned yet, create an attack package
         if (!found && CreateStrike(elem)) {
            hold_time = (int) Game::GameTime() + 30000;
            return true;
         }
      }
   }

   return false;
}

bool
CarrierAI::CreateStrike(Element* elem)
{
   Element* strike = 0;
   Ship*    target = elem->GetShip(1);

   if (target && !target->IsGroundUnit()) {
      Contact* contact = ship->FindContact(target);
      if (contact && contact->GetIFF(ship) > 0) {

         // fighter intercept
         if (target->IsDropship()) {
            int squadron = 0;
            if (hangar->NumShipsReady(1) >= hangar->NumShipsReady(0))
               squadron = 1;

            int count = 2;

            if (count < elem->NumShips())
               count = elem->NumShips();

            strike = CreatePackage(squadron, count, Mission::INTERCEPT, elem->Name(), "ACM Medium Range");

            if (strike) {
               strike->SetAssignment(elem);

               if (flight_planner)
                  flight_planner->CreateStrikeRoute(strike, elem);
            }
         }

         // starship or station assault
         else {
            int squadron = 0;
            if (hangar->NumSquadrons() > 1)
               squadron = 1;
            if (hangar->NumSquadrons() > 2)
               squadron = 2;

            int count = 2;

            if (target->Class() > Ship::FRIGATE) {
               count = 4;
               strike = CreatePackage(squadron, count, Mission::ASSAULT, elem->Name(), "Hvy Ship Strike");
            }
            else {
               count = 2;
               strike = CreatePackage(squadron, count, Mission::ASSAULT, elem->Name(), "Ship Strike");
            }

            if (strike) {
               strike->SetAssignment(elem);
               
               if (flight_planner)
                  flight_planner->CreateStrikeRoute(strike, elem);

               // strike escort if target has fighter protection:
               if (target->GetHangar()) {
                  if (squadron > 1) squadron--;
                  Element* escort = CreatePackage(squadron, 2, Mission::ESCORT_STRIKE, strike->Name(), "ACM Short Range");

                  if (escort && flight_planner)
                     flight_planner->CreateEscortRoute(escort, strike);
               }
            }
         }
      }
   }

   return strike != 0;
}

// +--------------------------------------------------------------------+

Element*
CarrierAI::CreatePackage(int squadron, int size, int code, const char* target, const char* loadname)
{
   if (squadron < 0 || size < 1 || code < Mission::PATROL || hangar->NumShipsReady(squadron) < size)
      return 0;

   Sim*        sim    = Sim::GetSim();
   const char* call   = sim->FindAvailCallsign(ship->GetIFF());
   Element*    elem   = sim->CreateElement(call, ship->GetIFF(), code);
   FlightDeck* deck   = 0;
   int         queue  = 1000;
   int*        load   = 0;
   const ShipDesign* 
               design = hangar->SquadronDesign(squadron);

   elem->SetSquadron(hangar->SquadronName(squadron));
   elem->SetCarrier(ship);

   if (target) {
      int i_code = 0;

      switch (code) {
      case Mission::ASSAULT:     i_code = Instruction::ASSAULT;   break;
      case Mission::STRIKE:      i_code = Instruction::STRIKE;    break;

      case Mission::AIR_INTERCEPT:
      case Mission::INTERCEPT:   i_code = Instruction::INTERCEPT; break;

      case Mission::ESCORT:
      case Mission::ESCORT_STRIKE:
      case Mission::ESCORT_FREIGHT:
                                 i_code = Instruction::ESCORT;    break;

      case Mission::DEFEND:      i_code = Instruction::DEFEND;    break;
      }

      Instruction* objective = new(__FILE__,__LINE__) Instruction(i_code, target);
      if (objective)
         elem->AddObjective(objective);
   }

   if (design && loadname) {
      Text name = loadname;
      name.setSensitive(false);

      ListIter<ShipLoad> sl = (List<ShipLoad>&) design->loadouts;
      while (++sl) {
         if (name == sl->name) {
            load = sl->load;
            elem->SetLoadout(load);
         }
      }
   }

   for (int i = 0; i < ship->NumFlightDecks(); i++) {
      FlightDeck* d = ship->GetFlightDeck(i);

      if (d && d->IsLaunchDeck()) {
         int dq = hangar->PreflightQueue(d);

         if (dq < queue) {
            queue = dq;
            deck  = d;
         }
      }
   }

   int npackage = 0;
   int slots[4];

   for (int i = 0; i < 4; i++)
      slots[i] = -1;

   for (int slot = 0; slot < hangar->SquadronSize(squadron); slot++) {
      const HangarSlot* s = hangar->GetSlot(squadron, slot);

      if (hangar->GetState(s) == Hangar::STORAGE) {
         if (npackage < 4)
            slots[npackage] = slot;

         hangar->GotoAlert(squadron, slot, deck, elem, load, code > Mission::SWEEP);
         npackage++;

         if (npackage >= size)
            break;
      }
   }

   NetUtil::SendElemCreate(elem, squadron, slots, code <= Mission::SWEEP);

   return elem;
}

// +--------------------------------------------------------------------+

bool
CarrierAI::LaunchElement(Element* elem)
{
   bool result = false;

   if (!elem)
      return result;

   for (int squadron = 0; squadron < hangar->NumSquadrons(); squadron++) {
      for (int slot = 0; slot < hangar->SquadronSize(squadron); slot++) {
         const HangarSlot* s = hangar->GetSlot(squadron, slot);

         if (hangar->GetState(s) == Hangar::ALERT &&
             hangar->GetPackageElement(s) == elem) {

            hangar->Launch(squadron, slot);
            NetUtil::SendShipLaunch(ship, squadron, slot);

            result = true;
         }
      }
   }

   return result;
}
