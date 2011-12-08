/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FlightDeck.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Everything needed to launch and recover space craft...
*/

#include "MemDebug.h"
#include "FlightDeck.h"
#include "Ship.h"
#include "ShipCtrl.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Mission.h"
#include "MissionEvent.h"
#include "Drive.h"
#include "Sim.h"
#include "Instruction.h"
#include "Hoop.h"
#include "LandingGear.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "SimEvent.h"
#include "AudioConfig.h"
#include "CameraDirector.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"

#include "NetData.h"
#include "NetUtil.h"

#include "Game.h"
#include "Solid.h"
#include "Light.h"
#include "Sound.h"
#include "DataLoader.h"

static Sound* tire_sound      = 0;
static Sound* catapult_sound  = 0;

// +======================================================================+

class FlightDeckSlot
{
public:
   FlightDeckSlot() : ship(0), state(0), sequence(0), time(0), filter(0xf) { }
   int operator == (const FlightDeckSlot& that) const { return this == &that; }
   
   Ship*             ship;
   Point             spot_rel;
   Point             spot_loc;

   int               state;
   int               sequence;
   double            time;
   double            clearance;
   DWORD             filter;
};

// +======================================================================+

InboundSlot::InboundSlot(Ship* s, FlightDeck* d, int squad, int index) 
      : ship(s), deck(d), squadron(squad), slot(index), cleared(0), final(0), approach(0)
{
   if (ship)
      Observe(ship);

   if (deck && deck->GetCarrier())
      Observe((SimObject*) deck->GetCarrier());
}

int InboundSlot::operator < (const InboundSlot& that) const
{
   double dthis = 0;
   double dthat = 0;

   if (ship == that.ship) {
      return false;
   }

   if (cleared && !that.cleared) {
      return true;
   }

   if (!cleared && that.cleared) {
      return false;
   }

   if (ship && deck && that.ship) {
      dthis = Point(ship->Location()      - deck->MountLocation()).length();
      dthat = Point(that.ship->Location() - deck->MountLocation()).length();
   }

   if (dthis == dthat) {
      return (DWORD) this < (DWORD) &that;
   }

   return dthis < dthat;
}

int InboundSlot::operator <= (const InboundSlot& that) const
{
   double dthis = 0;
   double dthat = 0;

   if (ship == that.ship)
      return true;

   if (cleared && !that.cleared)
      return true;

   if (!cleared && that.cleared)
      return false;

   if (ship && deck && that.ship) {
      dthis = Point(ship->Location()      - deck->MountLocation()).length();
      dthat = Point(that.ship->Location() - deck->MountLocation()).length();
   }

   return dthis <= dthat;
}

int InboundSlot::operator == (const InboundSlot& that) const
{
   return this == &that;
}

void InboundSlot::Clear(bool c)
{
   if (ship)
      cleared = c;
}

double InboundSlot::Distance()
{
   if (ship && deck) {
      return Point(ship->Location() - deck->MountLocation()).length();
   }

   return 1e9;
}

// +----------------------------------------------------------------------+

bool
InboundSlot::Update(SimObject* obj)
{
   if (obj->Type() == SimObject::SIM_SHIP) {
      Ship* s = (Ship*) obj;

      if (s == ship) {
         ship = 0;
      }

      // Actually, this can't happen.  The flight deck
      // owns the slot.  When the carrier is destroyed,
      // the flight decks and slots are destroyed before
      // the carrier updates the observers.

      // I'm leaving this block in, just in case.

      else if (deck && s == deck->GetCarrier()) {
         ship = 0;
         deck = 0;
      }
   }

   return SimObserver::Update(obj);
}

const char*
InboundSlot::GetObserverName() const
{
   return "InboundSlot";
}

// +======================================================================+

FlightDeck::FlightDeck()
   : System(FLIGHT_DECK, FLIGHT_DECK_LAUNCH, "Flight Deck", 1, 1),
     carrier(0), index(0), num_slots(0), slots(0), cycle_time(5), num_hoops(0), hoops(0),
     azimuth(0), light(0), num_catsounds(0), num_approach_pts(0)
{
   name = Game::GetText("sys.flight-deck");
   abrv = Game::GetText("sys.flight-deck.abrv");
}

// +----------------------------------------------------------------------+

FlightDeck::FlightDeck(const FlightDeck& s)
   : System(s), 
     carrier(0), index(0), start_rel(s.start_rel),
     end_rel(s.end_rel), cam_rel(s.cam_rel),
     cycle_time(s.cycle_time),
     num_slots(s.num_slots), slots(0),
     num_hoops(0), hoops(0), azimuth(s.azimuth), light(0),
     num_catsounds(0), num_approach_pts(s.num_approach_pts)
{
   Mount(s);

   slots = new(__FILE__,__LINE__) FlightDeckSlot[num_slots];
   for (int i = 0; i < num_slots; i++) {
      slots[i].spot_rel = s.slots[i].spot_rel;
      slots[i].filter   = s.slots[i].filter;
   }

   for (int i = 0; i < NUM_APPROACH_PTS; i++)
      approach_rel[i] = s.approach_rel[i];

   for (int i = 0; i < 2; i++)
      runway_rel[i] = s.runway_rel[i];

   if (s.light) {
      light = new(__FILE__,__LINE__) Light(*s.light);
   }
}

// +--------------------------------------------------------------------+

FlightDeck::~FlightDeck()
{
   if (hoops && num_hoops) {
      for (int i = 0; i < num_hoops; i++) {
         Hoop*  h     = &hoops[i];
         Scene* scene = h->GetScene();
         if (scene)
            scene->DelGraphic(h);
      }
   }

   delete [] slots;
   delete [] hoops;

   LIGHT_DESTROY(light);

   recovery_queue.destroy();
}

// +--------------------------------------------------------------------+

void
FlightDeck::Initialize()
{
   static int initialized = 0;
   if (initialized) return;

   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath("Sounds/");

   const int SOUND_FLAGS = Sound::LOCALIZED |
                           Sound::LOC_3D;

   loader->LoadSound("Tires.wav",      tire_sound,       SOUND_FLAGS);
   loader->LoadSound("Catapult.wav",   catapult_sound,   SOUND_FLAGS);
   loader->SetDataPath(0);

   if (tire_sound)
      tire_sound->SetMaxDistance(2.5e3f);

   if (catapult_sound)
      catapult_sound->SetMaxDistance(0.5e3f);

   initialized = 1;
}

// +--------------------------------------------------------------------+

void
FlightDeck::Close()
{
   delete tire_sound;
   delete catapult_sound;

   tire_sound     = 0;
   catapult_sound = 0;
}

// +--------------------------------------------------------------------+

void
FlightDeck::ExecFrame(double seconds)
{
   System::ExecFrame(seconds);

   bool advance_queue = false;
   long max_vol       = AudioConfig::EfxVolume();
   long volume        = -1000;
   Sim* sim           = Sim::GetSim();

   if (volume > max_vol)
      volume = max_vol;

   // update ship status:
   for (int i = 0; i < num_slots; i++) {
      FlightDeckSlot* slot = &slots[i];
      Ship*           slot_ship = 0;

      if (slot->ship == 0) {
         slot->state = CLEAR;
      }
      else {
         slot_ship = slot->ship;
         slot_ship->SetThrottle(0);
      }

      switch (slot->state) {
      case CLEAR:
         if (slot->time > 0) {
            slot->time -= seconds;
         }
         else if (IsRecoveryDeck()) {
            GrantClearance();
         }
         break;

      case READY: {
            Camera c;
            c.Clone(carrier->Cam());
            c.Yaw(azimuth);

            if (slot_ship) {
               slot_ship->CloneCam(c);
               slot_ship->MoveTo(slot->spot_loc);
               slot_ship->TranslateBy(carrier->Cam().vup() * slot->clearance);
               slot_ship->SetVelocity(carrier->Velocity());
            }
            slot->time = 0;
         }
         break;

      case QUEUED:
         if (slot->time > 0) {
            Camera c;
            c.Clone(carrier->Cam());
            c.Yaw(azimuth);

            slot->time -= seconds;
            if (slot_ship) {
               slot_ship->CloneCam(c);
               slot_ship->MoveTo(slot->spot_loc);
               slot_ship->TranslateBy(carrier->Cam().vup() * slot->clearance);
               slot_ship->SetFlightPhase(Ship::ALERT);
            }
         }

         if (slot->sequence == 1 && slot->time <= 0) {
            bool clear_for_launch = true;
            for (int i = 0; i < num_slots; i++)
               if (slots[i].state == LOCKED)
                  clear_for_launch = false;

            if (clear_for_launch) {
               slot->sequence = 0;
               slot->state    = LOCKED;
               slot->time     = cycle_time;
               if (slot_ship)
                  slot_ship->SetFlightPhase(Ship::LOCKED);
               num_catsounds     = 0;

               advance_queue  = true;
            }
         }
         break;

      case LOCKED:
         if (slot->time > 0) {
            slot->time -= seconds;

            if (slot_ship) {
               double ct4 = cycle_time/4;

               double dx   = start_rel.x - slot->spot_rel.x;
               double dy   = start_rel.z - slot->spot_rel.z;
               double dyaw = atan2(dx,dy) - azimuth;

               Camera c;
               c.Clone(carrier->Cam());
               c.Yaw(azimuth);

               // rotate:
               if (slot->time > 3*ct4) {
                  double step = 1 - (slot->time - 3*ct4) / ct4;
                  c.Yaw(dyaw * step);
                  slot_ship->CloneCam(c);
                  slot_ship->MoveTo(slot->spot_loc);
                  slot_ship->TranslateBy(carrier->Cam().vup() * slot->clearance);

                  if (carrier->IsGroundUnit()) {
                     slot_ship->SetThrottle(25);
                  }

                  else if (num_catsounds < 1) {
                     if (catapult_sound) {
                        Sound* sound = catapult_sound->Duplicate();
                        sound->SetLocation(slot_ship->Location());
                        sound->SetVolume(volume);
                        sound->Play();
                     }
                     num_catsounds = 1;
                  }
               }

               // translate:
               else if (slot->time > 2*ct4) {
                  double step = (slot->time - 2*ct4) / ct4;

                  Point  loc  = start_point +
                                (slot->spot_loc - start_point) * step;

                  c.Yaw(dyaw);
                  slot_ship->CloneCam(c);
                  slot_ship->MoveTo(loc);
                  slot_ship->TranslateBy(carrier->Cam().vup() * slot->clearance);

                  if (carrier->IsGroundUnit()) {
                     slot_ship->SetThrottle(25);
                  }

                  else if (num_catsounds < 2) {
                     if (catapult_sound) {
                        Sound* sound = catapult_sound->Duplicate();
                        sound->SetLocation(slot_ship->Location());
                        sound->SetVolume(volume);
                        sound->Play();
                     }
                     num_catsounds = 2;
                  }
               }

               // rotate:
               else if (slot->time > ct4) {
                  double step = (slot->time - ct4) / ct4;
                  c.Yaw(dyaw*step);
                  slot_ship->CloneCam(c);
                  slot_ship->MoveTo(start_point);
                  slot_ship->TranslateBy(carrier->Cam().vup() * slot->clearance);

                  if (carrier->IsGroundUnit()) {
                     slot_ship->SetThrottle(25);
                  }

                  else if (num_catsounds < 3) {
                     if (catapult_sound) {
                        Sound* sound = catapult_sound->Duplicate();
                        sound->SetLocation(slot_ship->Location());
                        sound->SetVolume(volume);
                        sound->Play();
                     }
                     num_catsounds = 3;
                  }
               }

               // wait:
               else {
                  slot_ship->SetThrottle(100);
                  slot_ship->CloneCam(c);
                  slot_ship->MoveTo(start_point);
                  slot_ship->TranslateBy(carrier->Cam().vup() * slot->clearance);
               }

               slot_ship->SetFlightPhase(Ship::LOCKED);
            }
         }
         else {
            slot->state    = LAUNCH;
            slot->time     = 0;

         }
         break;

      case LAUNCH:
         LaunchShip(slot_ship);
         break;

      case DOCKING:
         if (slot_ship && !slot_ship->IsAirborne()) {
            // do arresting gear stuff:
            if (slot_ship->GetFlightModel() == Ship::FM_ARCADE)
               slot_ship->ArcadeStop();

            slot_ship->SetVelocity(carrier->Velocity());
         }

         if (slot->time > 0) {
            slot->time -= seconds;
         }
         else {
            if (slot_ship) {
               slot_ship->SetFlightPhase(Ship::DOCKED);
               slot_ship->Stow();

               NetUtil::SendObjKill(slot_ship, carrier, NetObjKill::KILL_DOCK, index);
            }

            Clear(i);
         }
         break;

      default:
         break;
      }
   }
   
   if (advance_queue) {
      for (int i = 0; i < num_slots; i++) {
         FlightDeckSlot* slot = &slots[i];
         if (slot->state == QUEUED && slot->sequence > 1)
            slot->sequence--;
      }
   }
}

bool
FlightDeck::LaunchShip(Ship* slot_ship)
{
   FlightDeckSlot* slot = 0;
   Sim*            sim  = Sim::GetSim();

   if (slot_ship) {
      for (int i = 0; i < num_slots; i++) {
         if (slots[i].ship == slot_ship)
            slot = &(slots[i]);
      }

      // only suggest a launch point if no flight plan has been filed...
      if (slot_ship->GetElement()->FlightPlanLength() == 0) {
         Point departure = end_point;
         departure = departure.OtherHand();

         Instruction* launch_point = new(__FILE__,__LINE__)
               Instruction(carrier->GetRegion(), departure, Instruction::LAUNCH);
         launch_point->SetSpeed(350);

         slot_ship->SetLaunchPoint(launch_point);
      }

      if (!slot_ship->IsAirborne()) {
         Point cat;
         if (fabs(azimuth) < 5*DEGREES) {
            cat = carrier->Heading() * 300;
         }
         else {
            Camera c;
            c.Clone(carrier->Cam());
            c.Yaw(azimuth);
            cat = c.vpn() * 300;
         }

         slot_ship->SetVelocity(carrier->Velocity() + cat);
         slot_ship->SetFlightPhase(Ship::LAUNCH);
      }
      else {
         slot_ship->SetFlightPhase(Ship::TAKEOFF);
      }

      Director* dir = slot_ship->GetDirector();
      if (dir && dir->Type() == ShipCtrl::DIR_TYPE) {
         ShipCtrl* ctrl = (ShipCtrl*) dir;
         ctrl->Launch();
      }

      ShipStats* c = ShipStats::Find(carrier->Name());
      if (c) c->AddEvent(SimEvent::LAUNCH_SHIP, slot_ship->Name());

      ShipStats* stats = ShipStats::Find(slot_ship->Name());
      if (stats) {
         stats->SetRegion(carrier->GetRegion()->Name());
         stats->SetType(slot_ship->Design()->name);

         if (slot_ship->GetElement()) {
            Element* elem = slot_ship->GetElement();
            stats->SetRole(Mission::RoleName(elem->Type()));
            stats->SetCombatGroup(elem->GetCombatGroup());
            stats->SetCombatUnit(elem->GetCombatUnit());
            stats->SetElementIndex(slot_ship->GetElementIndex());
         }

         stats->SetIFF(slot_ship->GetIFF());
         stats->AddEvent(SimEvent::LAUNCH, carrier->Name());

         if (slot_ship == sim->GetPlayerShip())
            stats->SetPlayer(true);
      }

      sim->ProcessEventTrigger(MissionEvent::TRIGGER_LAUNCH, 0, slot_ship->Name());

      if (slot) {
         slot->ship     = 0;
         slot->state    = CLEAR;
         slot->sequence = 0;
         slot->time     = 0;
      }

      return true;
   }

   return false;
}

// +----------------------------------------------------------------------+

void
FlightDeck::SetLight(double l)
{
   LIGHT_DESTROY(light);
   light = new(__FILE__,__LINE__) Light((float) l);
}

void
FlightDeck::SetApproachPoint(int i, Point loc)
{
   if (i >= 0 && i < NUM_APPROACH_PTS) {
      approach_rel[i] = loc;

      if (i >= num_approach_pts)
         num_approach_pts = i+1;
   }
}

void
FlightDeck::SetRunwayPoint(int i, Point loc)
{
   if (i >= 0 && i < 2)
      runway_rel[i] = loc;
}

void
FlightDeck::SetStartPoint(Point loc)
{
   start_rel = loc;
}

void
FlightDeck::SetEndPoint(Point loc)
{
   end_rel = loc;
}

void
FlightDeck::SetCamLoc(Point loc)
{
   cam_rel = loc;
}

// +----------------------------------------------------------------------+

void
FlightDeck::AddSlot(const Point& spot, DWORD filter)
{
   if (!slots)
      slots = new(__FILE__,__LINE__) FlightDeckSlot[10];

   slots[num_slots].spot_rel = spot;
   slots[num_slots].filter   = filter;
   num_slots++;
}

// +----------------------------------------------------------------------+

void
FlightDeck::SetCycleTime(double t)
{
   cycle_time = t;
}

// +----------------------------------------------------------------------+

void
FlightDeck::Orient(const Physical* rep)
{
   System::Orient(rep);

   Matrix        orientation = rep->Cam().Orientation();
   Point         loc         = rep->Location();

   start_point = (start_rel   * orientation) + loc;
   end_point   = (end_rel     * orientation) + loc;
   cam_loc     = (cam_rel     * orientation) + loc;

   for (int i = 0; i < num_approach_pts; i++)
      approach_point[i] = (approach_rel[i] * orientation) + loc;

   for (int i = 0; i < num_slots; i++)
      slots[i].spot_loc = (slots[i].spot_rel * orientation) + loc;

   if (IsRecoveryDeck()) {
      if (carrier->IsAirborne()) {
         runway_point[0] = (runway_rel[0] * orientation) + loc;
         runway_point[1] = (runway_rel[1] * orientation) + loc;
      }

      if (num_hoops < 1) {
         num_hoops = 4;
         hoops  = new(__FILE__,__LINE__) Hoop[num_hoops];
      }

      Point  hoop_vec = start_point - end_point;
      double hoop_d   = hoop_vec.Normalize() / num_hoops;

      orientation.Yaw(azimuth);

      for (int i = 0; i < num_hoops; i++) {
         hoops[i].MoveTo(end_point + hoop_vec * (i+1) * hoop_d);
         hoops[i].SetOrientation(orientation);
      }
   }

   if (light)
      light->MoveTo(mount_loc);
}

// +----------------------------------------------------------------------+

int
FlightDeck::SpaceLeft(int type) const
{
   int space_left = 0;

   for (int i = 0; i < num_slots; i++)
      if (slots[i].ship == 0 && (slots[i].filter & type))
         space_left++;

   return space_left;
}

// +----------------------------------------------------------------------+

bool
FlightDeck::Spot(Ship* s, int& index)
{
   if (!s)
      return false;

   if (index < 0) {
      for (int i = 0; i < num_slots; i++) {
         if (slots[i].ship == 0 && (slots[i].filter & s->Class())) {
            index = i;
            break;
         }
      }
   }

   if (index >= 0 && index < num_slots && slots[index].ship == 0) {
      slots[index].state     = READY;
      slots[index].ship      = s;
      slots[index].clearance = 0;

      if (s->GetGear())
         slots[index].clearance = s->GetGear()->GetClearance();

      if (IsRecoveryDeck() && !s->IsAirborne()) {
         s->SetVelocity(s->Velocity() * 0.01);   // hit the brakes!
      }

      if (!IsRecoveryDeck()) {
         Camera work;
         work.Clone(carrier->Cam());
         work.Yaw(azimuth);

         s->CloneCam(work);
         s->MoveTo(slots[index].spot_loc);

         LandingGear* g = s->GetGear();
         if (g) {
            g->SetState(LandingGear::GEAR_DOWN);
            g->ExecFrame(0);
            s->TranslateBy(carrier->Cam().vup() * slots[index].clearance);
         }

         s->SetFlightPhase(Ship::ALERT);
      }

      s->SetCarrier(carrier, this);
      Observe(s);

      return true;
   }

   return false;
}

bool
FlightDeck::Clear(int index)
{
   if (index >= 0 && index < num_slots && slots[index].ship != 0) {
      Ship* s = slots[index].ship;

      slots[index].ship  = 0;
      slots[index].time  = cycle_time;
      slots[index].state = CLEAR;

      ListIter<InboundSlot> iter = recovery_queue;
      while (++iter) {
         if (iter->GetShip() == s) {
            delete iter.removeItem(); // ??? SHOULD DELETE HERE ???
            break;
         }
      }

      return true;
   }

   return false;
}

// +----------------------------------------------------------------------+

bool
FlightDeck::Launch(int index)
{
   if (subtype == FLIGHT_DECK_LAUNCH && index >= 0 && index < num_slots) {
      FlightDeckSlot* slot = &slots[index];

      if (slot->ship && slot->state == READY) {
         int last = 0;
         FlightDeckSlot* last_slot = 0;
         FlightDeckSlot* lock_slot = 0;

         for (int i = 0; i < num_slots; i++) {
            FlightDeckSlot* s = &slots[i];

            if (s->state == QUEUED && s->sequence > last) {
               last        = s->sequence;
               last_slot   = s;
            }

            else if (s->state == LOCKED) {
               lock_slot   = s;
            }
         }

         // queue the slot for launch:
         slot->state    = QUEUED;
         slot->sequence = last + 1;
         slot->time     = 0;

         if (last_slot)
            slot->time  = last_slot->time + cycle_time;

         else if (lock_slot)
            slot->time  = lock_slot->time;

         return true;
      }
   }

   return false;
}

// +----------------------------------------------------------------------+

bool
FlightDeck::Recover(Ship* s)
{
   if (s && subtype == FLIGHT_DECK_RECOVERY) {
      if (OverThreshold(s)) {
         if (s->GetFlightPhase() < Ship::RECOVERY) {
            s->SetFlightPhase(Ship::RECOVERY);
            s->SetCarrier(carrier, this); // let ship know which flight deck it's in (needed for dock cam)
         }

         // are we there yet?
         if (s->GetFlightPhase() >= Ship::ACTIVE && s->GetFlightPhase() < Ship::DOCKING) {
            if (slots[0].ship == 0) { // only need to ask this on approach
               double dock_distance = (s->Location() - MountLocation()).length();

               if (s->IsAirborne()) {
                  double altitude = s->Location().y - MountLocation().y;
                  if (dock_distance < Radius()*3 && altitude < s->Radius())
                     Dock(s);
               }
               else {
                  if (dock_distance < s->Radius())
                     Dock(s);
               }
            }
         }

         return true;
      }
      else {
         if (s->GetFlightPhase() == Ship::RECOVERY)
            s->SetFlightPhase(Ship::ACTIVE);
      }
   }

   return false;
}

bool
FlightDeck::Dock(Ship* s)
{
   if (s && subtype == FLIGHT_DECK_RECOVERY && slots[0].time <= 0) {
      int index = 0;
      if (Spot(s, index)) {
         s->SetFlightPhase(Ship::DOCKING);
         s->SetCarrier(carrier, this);

         // hard landings?
         if (Ship::GetLandingModel() == 0) {
            double base_damage = s->Design()->integrity;

            // did player do something stupid?
            if (s->GetGear() && !s->IsGearDown()) {
               Print("FlightDeck::Dock(%s) Belly landing!\n", s->Name());
               s->InflictDamage(0.5 * base_damage);
            }

            double docking_deflection = fabs(carrier->Cam().vup().y - s->Cam().vup().y);

            if (docking_deflection > 0.35) {
               Print("Landing upside down? y = %.3f\n", docking_deflection);
               s->InflictDamage(0.8 * base_damage);
            }

            // did incoming ship exceed safe landing parameters?
            if (s->IsAirborne()) {
               if (s->Velocity().y < -20) {
                  Print("FlightDeck::Dock(%s) Slammed it!\n", s->Name());
                  s->InflictDamage(0.1 * base_damage);
               }
            }

            // did incoming ship exceed safe docking speed?
            else {
               Point delta_v = s->Velocity() - carrier->Velocity();
               double excess = (delta_v.length() - 100);

               if (excess > 0)
                  s->InflictDamage(excess);
            }
         }

         if (s->IsAirborne()) {
            if (s == Sim::GetSim()->GetPlayerShip() && tire_sound) {
               Sound* sound = tire_sound->Duplicate();
               sound->Play();
            }
         }

         if (s->GetDrive())
            s->GetDrive()->PowerOff();

         slots[index].state = DOCKING;
         slots[index].time  = 5;

         if (s->IsAirborne())
            slots[index].time = 7.5;
         return true;
      }
   }

   return false;
}

int
FlightDeck::Inbound(InboundSlot*& s)
{
   if (s && s->GetShip()) {
      Ship* inbound = s->GetShip();

      if (recovery_queue.contains(s)) {
         InboundSlot* orig = s;
         s = recovery_queue.find(s);
         delete orig;
      }
      else {
         recovery_queue.append(s);
         Observe(s->GetShip());
      }

      inbound->SetInbound(s);

      // find the best initial approach point for this ship:
      double current_distance = 1e9;
      for (int i = 0; i < num_approach_pts; i++) {
         double distance = Point(inbound->Location() - approach_point[i]).length();
         if (distance < current_distance) {
            current_distance = distance;
            s->SetApproach(i);
         }
      }

      Point offset(rand()-16000, rand()-16000, rand()-16000);
      offset.Normalize();
      offset *= 200;

      s->SetOffset(offset);

      // *** DEBUG ***
      // PrintQueue();

      // if the new guy is first in line, and the deck is almost ready,
      // go ahead and clear him for approach now
      recovery_queue.sort();
      if (recovery_queue[0] == s && !s->Cleared() && slots[0].time <= 3)
         s->Clear();

      return recovery_queue.index(s) + 1;
   }

   return 0;
}

void
FlightDeck::GrantClearance()
{
   if (recovery_queue.size() > 0) {
      if (recovery_queue[0]->Cleared() && recovery_queue[0]->Distance() > 45e3) {
         recovery_queue[0]->Clear(false);
      }

      if (!recovery_queue[0]->Cleared()) {
         recovery_queue.sort();

         if (recovery_queue[0]->Distance() < 35e3) {
            recovery_queue[0]->Clear();

            Ship* dst = recovery_queue[0]->GetShip();

            RadioMessage* clearance = new(__FILE__,__LINE__) RadioMessage(dst, carrier, RadioMessage::CALL_CLEARANCE);
            clearance->SetInfo(Text("for final approach to ") + Name());
            RadioTraffic::Transmit(clearance);
         }
      }
   }
}

// +----------------------------------------------------------------------+

void
FlightDeck::PrintQueue()
{
   Print("\nRecovery Queue for %s\n", Name());
   if (recovery_queue.size() < 1) {
      Print("  (empty)\n\n");
      return;
   }

   for (int i = 0; i < recovery_queue.size(); i++) {
      InboundSlot* s = recovery_queue.at(i);

      if (!s) {
         Print("  %2d. null\n", i);
      }
      else if (!s->GetShip()) {
         Print("  %2d. ship is null\n", i);
      }
      else {
         double d = Point(s->GetShip()->Location() - MountLocation()).length();
         Print("  %2d. %c %-20s %8d km\n", i, s->Cleared()?'*':' ', s->GetShip()->Name(), (int) (d/1000));
      }
   }

   Print("\n");
}

// +----------------------------------------------------------------------+

Ship*
FlightDeck::GetShip(int index) const
{
   if (index >= 0 && index < num_slots)
      return slots[index].ship;

   return 0;
}

double
FlightDeck::TimeRemaining(int index) const
{
   if (index >= 0 && index < num_slots)
      return slots[index].time;

   return 0;
}


int
FlightDeck::State(int index) const
{
   if (index >= 0 && index < num_slots)
      return slots[index].state;

   return 0;
}

int
FlightDeck::Sequence(int index) const
{
   if (index >= 0 && index < num_slots)
      return slots[index].sequence;

   return 0;
}

// +----------------------------------------------------------------------+

bool
FlightDeck::Update(SimObject* obj)
{
   if (obj->Type() == SimObject::SIM_SHIP) {
      Ship* s = (Ship*) obj;

      ListIter<InboundSlot> iter = recovery_queue;
      while (++iter) {
         InboundSlot* recovery_slot = iter.value();

         if (recovery_slot->GetShip() == s || recovery_slot->GetShip() == 0) {
            delete iter.removeItem();
         }
      }

      for (int i = 0; i < num_slots; i++) {
         FlightDeckSlot* slot = &slots[i];

         if (slot->ship == s) {
            slot->ship = 0;
            slot->state = 0;
            slot->sequence = 0;
            slot->time = 0;
            break;
         }
      }
   }

   return SimObserver::Update(obj);
}

const char*
FlightDeck::GetObserverName() const
{
   return Name();
}

// +----------------------------------------------------------------------+

bool
FlightDeck::OverThreshold(Ship* s) const
{
   if (carrier->IsAirborne()) {
      if (s->AltitudeAGL() > s->Radius() * 4)
         return false;

      const Point& sloc = s->Location();

      // is ship between the markers?
      double distance = 1e9;

      Point  d0      = runway_point[0] - sloc;
      Point  d1      = runway_point[1] - sloc;
      double d       = d0 * d1;
      bool   between = (d0 * d1) < 0;

      if (between) {
         // distance from point to line:
         Point src   = runway_point[0];
         Point dir   = runway_point[1] - src;
         Point w     = (sloc - src).cross(dir);

         distance = w.length() / dir.length();
      }

      return distance < Radius();
   }

   else {
      return (s->Location() - MountLocation()).length() < (s->Radius() + Radius());
   }
}

// +----------------------------------------------------------------------+

bool
FlightDeck::ContainsPoint(const Point& p) const
{
   return (p - MountLocation()).length() < Radius();
}
