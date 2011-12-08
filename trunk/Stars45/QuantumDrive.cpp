/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         QuantumDrive.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Quantum Drive class
*/

#include "MemDebug.h"
#include "QuantumDrive.h"
#include "Ship.h"
#include "Explosion.h"
#include "Drive.h"
#include "Sim.h"
#include "SimEvent.h"
#include "StarSystem.h"

#include "Game.h"
#include "Random.h"

// +----------------------------------------------------------------------+

static int     drive_value       = 3;

// +----------------------------------------------------------------------+

QuantumDrive::QuantumDrive(SUBTYPE s, double cap, double rate)
   : System(DRIVE, s, "Quantum", drive_value, (float) cap, (float) cap, (float) rate),
     dst_rgn(0), active_state(ACTIVE_READY), warp_fov(1), jump_time(0), countdown(5)
{
   name = Game::GetText("sys.quantum");
   abrv = Game::GetText("sys.quantum.abrv");

   emcon_power[0] = 0;
   emcon_power[1] = 0;
   emcon_power[2] = 100;
}

// +----------------------------------------------------------------------+

QuantumDrive::QuantumDrive(const QuantumDrive& d)
   : System(d),
     dst_rgn(0), active_state(ACTIVE_READY), warp_fov(1), jump_time(0),
     countdown(d.countdown)
{
   Mount(d);
   SetAbbreviation(d.Abbreviation());

   energy = capacity;
}

// +--------------------------------------------------------------------+

QuantumDrive::~QuantumDrive()
{ }

// +--------------------------------------------------------------------+

void
QuantumDrive::SetDestination(SimRegion* rgn, const Point& loc)
{
   dst_rgn = rgn;
   dst_loc = loc;
}

// +--------------------------------------------------------------------+

bool
QuantumDrive::Engage(bool immediate)
{
   if (active_state == ACTIVE_READY && ship != 0 &&
       IsPowerOn() && Status() == NOMINAL && energy == capacity) {

      active_state = ACTIVE_COUNTDOWN;
      if (immediate) {
         jump_time = 1;
         return true;
      }

      jump_time    = countdown;

      SimRegion* rgn = ship->GetRegion();

      ListIter<Ship> s_iter = rgn->Ships();
      while (++s_iter) {
         Ship* s = s_iter.value();

         if (s != ship) {
            double dist = Point(s->Location() - ship->Location()).length();

            if (dist < 25e3)
               jump_time += 5;

            else if (dist < 50e3)
               jump_time += 2;

            else if (dist < 100e3)
               jump_time += 1;

            else if (dist < 200e3)
               jump_time += 0.5;
         }
      }

      return true;
   }

   return false;
}

void
QuantumDrive::PowerOff()
{
   System::PowerOff();
   AbortJump();
}

void
QuantumDrive::AbortJump()
{
   active_state = ACTIVE_READY;
   jump_time    = 0;
   energy       = 0;
   warp_fov     = 1;

   ship->SetWarp(warp_fov);

   SimRegion*     r        = ship->GetRegion();
   ListIter<Ship> neighbor = r->Ships();

   while (++neighbor) {
      if (neighbor->IsDropship()) {
         Ship* s     = neighbor.value();
         Point delta = s->Location() - ship->Location();

         if (delta.length() < 5e3)
            s->SetWarp(warp_fov);
      }
   }
}

// +--------------------------------------------------------------------+

void
QuantumDrive::ExecFrame(double seconds)
{
   System::ExecFrame(seconds);
   
   if (active_state == ACTIVE_READY)
      return;

   if (ship) {
      bool warping = false;

      if (active_state == ACTIVE_COUNTDOWN) {
         if (jump_time > 0) {
            jump_time -= seconds;
         }

         else {
            jump_time = 0;
            active_state = ACTIVE_PREWARP;
         }
      }

      else if (active_state == ACTIVE_PREWARP) {
         if (warp_fov < 5000) {
            warp_fov *= 1.5;
         }
         else {
            Jump();
            energy = 0.0f;
         }

         warping = true;
      }

      else if (active_state == ACTIVE_POSTWARP) {
         if (warp_fov > 1) {
            warp_fov *= 0.75;
         }
         else {
            warp_fov     = 1;
            active_state = ACTIVE_READY;
         }

         warping = true;
      }

      if (warping) {
         ship->SetWarp(warp_fov);

         SimRegion*     r        = ship->GetRegion();
         ListIter<Ship> neighbor = r->Ships();

         while (++neighbor) {
            if (neighbor->IsDropship()) {
               Ship* s     = neighbor.value();
               Point delta = s->Location() - ship->Location();

               if (delta.length() < 5e3)
                  s->SetWarp(warp_fov);
            }
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
QuantumDrive::Jump()
{
   Sim* sim = Sim::GetSim();

   if (ship && sim) {
      double      dist    = 150e3 + Random(0, 60e3);
      Point       esc_vec = dst_rgn->GetOrbitalRegion()->Location() -
                            dst_rgn->GetOrbitalRegion()->Primary()->Location();

      esc_vec.Normalize();
      esc_vec *= dist;
      esc_vec += RandomDirection() * Random(15e3, 22e3);

      if (subtype == HYPER)
         sim->CreateExplosion(ship->Location(), Point(0,0,0), Explosion::HYPER_FLASH,   1, 1, ship->GetRegion());
      else
         sim->CreateExplosion(ship->Location(), Point(0,0,0), Explosion::QUANTUM_FLASH, 1, 0, ship->GetRegion());

      sim->RequestHyperJump(ship, dst_rgn, esc_vec);

      ShipStats* stats = ShipStats::Find(ship->Name());
      stats->AddEvent(SimEvent::QUANTUM_JUMP, dst_rgn->Name());
   }

   dst_rgn = 0;
   dst_loc = Point();

   active_state = ACTIVE_POSTWARP;
}
