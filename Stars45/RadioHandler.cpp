/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         RadioHandler.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Radio message handler class implementation
*/

#include "MemDebug.h"
#include "RadioHandler.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Instruction.h"

#include "Contact.h"
#include "Element.h"
#include "Mission.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Power.h"
#include "Drive.h"
#include "Shield.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "WeaponGroup.h"
#include "SteerAI.h"

#include "Text.h"
#include "Game.h"

// +----------------------------------------------------------------------+

RadioHandler::RadioHandler()
{ }

RadioHandler::~RadioHandler()
{ }

// +----------------------------------------------------------------------+

bool
RadioHandler::ProcessMessage(RadioMessage* msg, Ship* s)
{
   if (!s || !msg || !msg->Sender())
      return false;

   if (s->Class() >= Ship::FARCASTER && s->Class() <= Ship::C3I)
      return false;

   if (msg->Sender()->IsRogue()) {
      Ship* sender = (Ship*) msg->Sender();  // cast-away const
      RadioMessage* nak = new(__FILE__,__LINE__) RadioMessage(sender, s, RadioMessage::NACK);
      RadioTraffic::Transmit(nak);
      return false;
   }

   bool respond = (s != msg->Sender());

   // SPECIAL CASE:
   // skip navpoint must be processed by elem leader,
   // even if the elem leader sent the message:

   if (msg->Action() == RadioMessage::SKIP_NAVPOINT && !respond)
      ProcessMessageAction(msg, s);

   if (ProcessMessageOrders(msg, s))
      respond = respond && true;

   else
      respond = respond && ProcessMessageAction(msg, s);

   return respond;
}

// +----------------------------------------------------------------------+

bool
RadioHandler::IsOrder(int action)
{
   bool result = false;

   switch (action) {
   default:
   case RadioMessage::NONE:
   case RadioMessage::ACK:
   case RadioMessage::NACK:            result = false; break;

// target mgt:
   case RadioMessage::ATTACK:
   case RadioMessage::ESCORT:
   case RadioMessage::BRACKET:
   case RadioMessage::IDENTIFY:        result = true;  break;

// combat mgt:
   case RadioMessage::COVER_ME:
   case RadioMessage::WEP_HOLD:
   case RadioMessage::FORM_UP:         result = true;  break;

   case RadioMessage::WEP_FREE:
   case RadioMessage::SAY_POSITION:
   case RadioMessage::LAUNCH_PROBE:    result = false; break;

// formation mgt:
   case RadioMessage::GO_DIAMOND:
   case RadioMessage::GO_SPREAD:
   case RadioMessage::GO_BOX:
   case RadioMessage::GO_TRAIL:        result = true;  break;

// mission mgt:
   case RadioMessage::MOVE_PATROL:     result = true;  break;
   case RadioMessage::SKIP_NAVPOINT:   result = false; break;
   case RadioMessage::RESUME_MISSION:  result = true;  break;

   case RadioMessage::RTB:
   case RadioMessage::DOCK_WITH:
   case RadioMessage::QUANTUM_TO:
   case RadioMessage::FARCAST_TO:      result = true;  break;

// sensor mgt:
   case RadioMessage::GO_EMCON1:
   case RadioMessage::GO_EMCON2:
   case RadioMessage::GO_EMCON3:       result = true;  break;

// support:
   case RadioMessage::REQUEST_PICTURE:
   case RadioMessage::REQUEST_SUPPORT:
   case RadioMessage::PICTURE:         result = false; break;

// traffic control:
   case RadioMessage::CALL_INBOUND:
   case RadioMessage::CALL_APPROACH:
   case RadioMessage::CALL_CLEARANCE:
   case RadioMessage::CALL_FINALS:
   case RadioMessage::CALL_WAVE_OFF:   result = false; break;
   }

   return result;
}

// +----------------------------------------------------------------------+

bool
RadioHandler::ProcessMessageOrders(RadioMessage* msg, Ship* ship)
{
   Instruction* instruction = ship->GetRadioOrders();
   int          action      = 0;

   if (msg && msg->Action() == RadioMessage::RESUME_MISSION) {
      instruction->SetAction(RadioMessage::NONE);
      instruction->SetFormation(-1);
      instruction->SetWeaponsFree(true);
      if (instruction->GetTarget()) {
         instruction->ClearTarget();
         ship->DropTarget();
      }
      return true;
   }

   if (msg && IsOrder(msg->Action())) {
      int posture_only = false;

      action = msg->Action();

      if (action == RadioMessage::FORM_UP)
         action = RadioMessage::WEP_HOLD;

      // target orders => drop current target:
      if (action >= RadioMessage::ATTACK    &&
          action <= RadioMessage::COVER_ME  ||
          action == RadioMessage::WEP_HOLD  ||
          action >= RadioMessage::DOCK_WITH &&
          action <= RadioMessage::FARCAST_TO) {

         if (ship != msg->Sender())
            ship->DropTarget();

         Director* dir = ship->GetDirector();
         if (dir && dir->Type() >= SteerAI::SEEKER && dir->Type() <= SteerAI::GROUND) {
            SteerAI* ai = (SteerAI*) dir;
            ai->SetTarget(0);
         }

         // farcast and quantum jump radio messages:
         if (action >= RadioMessage::QUANTUM_TO) {
            Sim* sim = Sim::GetSim();
            
            if (sim) {
               SimRegion* rgn = sim->FindRegion(msg->Info());

               if (rgn) {
                  instruction->SetAction(action);
                  instruction->SetLocation(Point(0,0,0));
                  instruction->SetRegion(rgn);
                  instruction->SetFarcast(action == RadioMessage::FARCAST_TO);
                  instruction->SetWeaponsFree(false);
                  return true;
               }
            }
         }
      }

      // formation orders => set formation:
      if (action >= RadioMessage::GO_DIAMOND &&
          action <= RadioMessage::GO_TRAIL) {

         switch (action) {
         case RadioMessage::GO_DIAMOND:   instruction->SetFormation(Instruction::DIAMOND); break;
         case RadioMessage::GO_SPREAD:    instruction->SetFormation(Instruction::SPREAD);  break;
         case RadioMessage::GO_BOX:       instruction->SetFormation(Instruction::BOX);     break;
         case RadioMessage::GO_TRAIL:     instruction->SetFormation(Instruction::TRAIL);   break;
         }

         posture_only = true;
      }

      // emcon orders => set emcon:
      if (action >= RadioMessage::GO_EMCON1 &&
          action <= RadioMessage::GO_EMCON3) {

         switch (msg->Action()) {
         case RadioMessage::GO_EMCON1:    instruction->SetEMCON(1);  break;
         case RadioMessage::GO_EMCON2:    instruction->SetEMCON(2);  break;
         case RadioMessage::GO_EMCON3:    instruction->SetEMCON(3);  break;
         }

         posture_only = true;
      }

      if (!posture_only) {
         instruction->SetAction(action);
         instruction->ClearTarget();

         if (msg->TargetList().size() > 0) {
            SimObject* msg_tgt = msg->TargetList().at(0);
            instruction->SetTarget(msg_tgt);
            instruction->SetLocation(msg_tgt->Location());
         }

         else if (action == RadioMessage::COVER_ME) {
            instruction->SetTarget((Ship*) msg->Sender());
            instruction->SetLocation(msg->Sender()->Location());
         }

         else if (action == RadioMessage::MOVE_PATROL) {
            instruction->SetLocation(msg->Location());
         }

         // handle element engagement:
         if (action == RadioMessage::ATTACK && msg->TargetList().size() > 0) {
            Element* elem = msg->DestinationElem();

            if (!elem && msg->DestinationShip())
               elem = msg->DestinationShip()->GetElement();

            if (elem) {
               SimObject* msg_tgt = msg->TargetList().at(0);
               if (msg_tgt && msg_tgt->Type() == SimObject::SIM_SHIP) {
                  Element* tgt  = ((Ship*) msg_tgt)->GetElement();
                  elem->SetAssignment(tgt);

                  if (msg->TargetList().size() > 1)
                     instruction->SetTarget(tgt->Name().data());
                  else
                     instruction->SetTarget(msg_tgt);
               }
               else {
                  elem->ResumeAssignment();
               }
            }
         }

         else if (action == RadioMessage::RESUME_MISSION) {
            Element* elem = msg->DestinationElem();

            if (!elem && msg->DestinationShip())
               elem = msg->DestinationShip()->GetElement();

            if (elem) {
               elem->ResumeAssignment();
            }
         }
      }

      instruction->SetWeaponsFree(action <= RadioMessage::WEP_FREE);
      return true;
   }

   return false;
}

// +----------------------------------------------------------------------+

bool
RadioHandler::ProcessMessageAction(RadioMessage* msg, Ship* ship)
{
   if (!msg) return false;

   if (msg->Action() == RadioMessage::CALL_INBOUND)
      return Inbound(msg, ship);

   if (msg->Action() == RadioMessage::CALL_FINALS)
      return true;   // acknowledge

   if (msg->Action() == RadioMessage::REQUEST_PICTURE)
      return Picture(msg, ship);

   if (msg->Action() == RadioMessage::REQUEST_SUPPORT)
      return Support(msg, ship);

   if (msg->Action() == RadioMessage::SKIP_NAVPOINT)
      return SkipNavpoint(msg, ship);

   if (msg->Action() == RadioMessage::LAUNCH_PROBE)
      return LaunchProbe(msg, ship);

   return false;
}

bool
RadioHandler::SkipNavpoint(RadioMessage* msg, Ship* ship)
{
   // Find next Instruction:
   Instruction* navpt      = ship->GetNextNavPoint();
   int          elem_index = ship->GetElementIndex();

   if (navpt && elem_index < 2) {
      ship->SetNavptStatus(navpt, Instruction::SKIPPED);
   }

   return true;
}

bool
RadioHandler::LaunchProbe(RadioMessage* msg, Ship* ship)
{
   if (ship && ship->GetProbeLauncher()) {
      ship->LaunchProbe();
      return ship->GetProbe() != 0;
   }

   return false;
}

bool
RadioHandler::Inbound(RadioMessage* msg, Ship* ship)
{
   Ship*       inbound  = (Ship*) msg->Sender();
   Hangar*     hangar   = ship->GetHangar();
   FlightDeck* deck     = 0;
   int         squadron = -1;
   int         slot     = -1;
   bool        same_rgn = false;

   if (inbound && inbound->GetRegion() == ship->GetRegion())
      same_rgn = true;

   // is the sender already inbound to us?
   if (inbound->GetInbound() &&
       inbound->GetInbound()->GetDeck() &&
       inbound->GetInbound()->GetDeck()->GetCarrier() == ship) {
      InboundSlot* islot = inbound->GetInbound();
      deck      = islot->GetDeck();
      squadron  = islot->Squadron();
      slot      = islot->Index();
   }

   // otherwise, find space for sender:
   else {
      if (hangar && same_rgn) {
         if (hangar->FindSlot(inbound, squadron, slot)) {
            int shortest_queue = 1000;

            for (int i = 0; i < ship->NumFlightDecks(); i++) {
               FlightDeck* d = ship->GetFlightDeck(i);
               if (d->IsRecoveryDeck()) {
                  int nwaiting = d->GetRecoveryQueue().size();

                  if (nwaiting < shortest_queue) {
                     deck = d;
                     shortest_queue = nwaiting;
                  }
               }
            }
         }
      }
   }

   // if no space (or not a carrier!) wave sender off:
   if (!deck || !same_rgn || squadron < 0 || slot < 0) {
      RadioMessage* wave_off = new(__FILE__,__LINE__) RadioMessage(inbound, ship, RadioMessage::NACK);
      if (!hangar)
         wave_off->SetInfo(Game::GetText("RadioHandler.no-hangar"));

      else if (!same_rgn) {
         char info[256];
         sprintf(info, Game::GetText("RadioHandler.too-far-away").data(), ship->GetRegion()->Name());
         wave_off->SetInfo(info);
      }

      else
         wave_off->SetInfo(Game::GetText("RadioHandler.all-full"));

      RadioTraffic::Transmit(wave_off);
      return false;
   }

   // put sender in recovery queue, if not already there:
   InboundSlot* inbound_slot = inbound->GetInbound();
   int          sequence     = 0;
   
   if (!inbound_slot) {
      inbound_slot = new(__FILE__,__LINE__) InboundSlot(inbound, deck, squadron, slot);
      sequence = deck->Inbound(inbound_slot);
   }
   else {
      sequence = inbound_slot->Index();
   }

   // inform sender of status:
   RadioMessage* approach = new(__FILE__,__LINE__) RadioMessage(inbound, ship, RadioMessage::CALL_APPROACH);

   if (inbound_slot->Cleared()) {
      char info[256];
      sprintf(info, Game::GetText("RadioHandler.cleared").data(), deck->Name());
      approach->SetInfo(info);
   }
   else if (sequence) {
      char info[256];
      sprintf(info, Game::GetText("RadioHandler.sequenced").data(), sequence, deck->Name());
      approach->SetInfo(info);
   }

   RadioTraffic::Transmit(approach);

   return false;
}

bool
RadioHandler::Picture(RadioMessage* msg, Ship* ship)
{
   if (!ship) return false;

   // try to find some enemy fighters in the area:
   Ship*       tgt   = 0;
   double      range = 1e9;

   ListIter<Contact> iter = ship->ContactList();
   while (++iter) {
      Contact* c = iter.value();
      int      iff = c->GetIFF(ship);
      Ship*    s = c->GetShip();

      if (s && s->IsDropship() && s->IsHostileTo(ship)) {
         double s_range = Point(msg->Sender()->Location() - s->Location()).length();
         if (!tgt || s_range < range) {
            tgt   = s;
            range = s_range;
         }
      }
   }

   // found some:
   if (tgt) {
      Element*       sender   = msg->Sender()->GetElement();
      Element*       tgt_elem = tgt->GetElement();
      RadioMessage*  response = new(__FILE__,__LINE__) RadioMessage(sender, ship, RadioMessage::ATTACK);

      if (tgt_elem) {
         for (int i = 1; i <= tgt_elem->NumShips(); i++)
            response->AddTarget(tgt_elem->GetShip(i));
      }
      else {
         response->AddTarget(tgt);
      }

      RadioTraffic::Transmit(response);
   }

   // nobody worth killin':
   else {
      Ship* sender = (Ship*) msg->Sender();  // cast-away const
      RadioMessage* response = new(__FILE__,__LINE__) RadioMessage(sender, ship, RadioMessage::PICTURE);
      RadioTraffic::Transmit(response);
   }

   return false;
}

bool
RadioHandler::Support(RadioMessage* msg, Ship* ship)
{
   if (!ship) return false;

   // try to find some fighters with time on their hands...
   Element*    help = 0;
   Element*    cmdr = ship->GetElement();
   Element*    baby = msg->Sender()->GetElement();
   SimRegion*  rgn  = msg->Sender()->GetRegion();

   for (int i = 0; i < rgn->Ships().size(); i++) {
      Ship*    s = rgn->Ships().at(i);
      Element* e = s->GetElement();

      if (e && s->IsDropship()               &&
               e->Type() == Mission::PATROL  &&
               e != baby                     && 
               cmdr->CanCommand(e)           && 
               s->GetRadioOrders()->Action() == RadioMessage::NONE) {
         help = e;
         break;
      }
   }

   // found some:
   if (help) {
      RadioMessage* escort = new(__FILE__,__LINE__) RadioMessage(help, ship, RadioMessage::ESCORT);
      escort->TargetList().append(msg->Sender());
      RadioTraffic::Transmit(escort);

      Text           ok       = Game::GetText("RadioHandler.help-enroute");
      Ship*          sender   = (Ship*) msg->Sender();  // cast-away const
      RadioMessage*  response = new(__FILE__,__LINE__) RadioMessage(sender, ship, RadioMessage::ACK);
      response->SetInfo(ok);
      RadioTraffic::Transmit(response);
   }

   // no help in sight:
   else {
      Text           nope     = Game::GetText("RadioHandler.no-help-for-you");
      Ship*          sender   = (Ship*) msg->Sender();  // cast-away const
      RadioMessage*  response = new(__FILE__,__LINE__) RadioMessage(sender, ship, RadioMessage::NACK);
      response->SetInfo(nope);
      RadioTraffic::Transmit(response);
   }

   return false;
}

// +----------------------------------------------------------------------+

void
RadioHandler::AcknowledgeMessage(RadioMessage* msg, Ship* s)
{
   if (s && msg && msg->Sender() && msg->Action()) {
      if (msg->Action() >= RadioMessage::ACK && msg->Action() <= RadioMessage::NACK)
         return;  // nothing to say here

      Ship* sender = (Ship*) msg->Sender();  // cast-away const
      RadioMessage* ack = new(__FILE__,__LINE__) RadioMessage(sender, s, RadioMessage::ACK);
      RadioTraffic::Transmit(ack);
   }
}

