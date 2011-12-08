/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         ShipCtrl.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Ship Controller class
*/

#include "MemDebug.h"
#include "ShipCtrl.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shield.h"
#include "Sensor.h"
#include "NavSystem.h"
#include "FlightComp.h"
#include "LandingGear.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "HUDView.h"
#include "HUDSounds.h"
#include "KeyMap.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"

#include "MouseController.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "Game.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+

ShipCtrl::ShipCtrl(Ship* s, MotionController* ctrl)
   : ship(s), controller(ctrl), throttle_active(false), launch_latch(false),
     pickle_latch(false), target_latch(false)
{
   for (int i = 0; i < 10; i++)
      GetAsyncKeyState('0'+i);
}

// +--------------------------------------------------------------------+

int
ShipCtrl::KeyDown(int action)
{
   int k = Joystick::KeyDownMap(action) ||
           Keyboard::KeyDownMap(action);

   return k;
}

int
ShipCtrl::Toggled(int action)
{
   static double last_toggle_time = 0;

   if (KeyDown(action)) {
      if ((Game::RealTime() - last_toggle_time) > 250) {
         last_toggle_time = Game::RealTime();
         return 1;
      }
   }

   return 0;
}

// +--------------------------------------------------------------------+

void
ShipCtrl::Launch()
{
   if (controller) {
      ship->SetThrottle(100);
      throttle_active = false;
      launch_latch    = true;
   }
}

// +--------------------------------------------------------------------+

void
ShipCtrl::ExecFrame(double seconds)
{
   Starshatter* stars = Starshatter::GetInstance();
   if (stars && stars->GetChatMode())  return;
   if (!ship)                          return;

   const int DELTA_THROTTLE = 5;

   controller->Acquire();

   if (ship->IsStarship() && ship->GetFLCSMode() == Ship::FLCS_HELM) {
      ship->ApplyHelmPitch(controller->Pitch() * seconds);
      ship->ApplyHelmYaw(controller->Yaw() * seconds);
   }
   else {
      ship->ApplyRoll(controller->Roll());
      ship->ApplyPitch(controller->Pitch());
      ship->ApplyYaw(controller->Yaw());
   }
   
   ship->SetTransX(controller->X() * ship->Design()->trans_x);
   ship->SetTransY(controller->Y() * ship->Design()->trans_y);
   ship->SetTransZ(controller->Z() * ship->Design()->trans_z);

   bool augmenter = false;

   if (controller->Throttle() > 0.05) {
      if (throttle_active) {
         ship->SetThrottle(controller->Throttle() * 100);

         if (controller->Throttle() >= 0.99) 
            augmenter = true;
      }
      else if (!launch_latch || controller->Throttle() > 0.5) {
         throttle_active = true;
         launch_latch    = false;
      }
   }
   else if (throttle_active) {
      ship->SetThrottle(0);
      throttle_active = false;
   }

   ship->ExecFLCSFrame();

   static double time_til_change = 0.0;

   if (time_til_change < 0.001) {
      if (KeyDown(KEY_THROTTLE_UP)) {
         ship->SetThrottle(ship->Throttle() + DELTA_THROTTLE);
         time_til_change = 0.05;
      }
         
      else if (KeyDown(KEY_THROTTLE_DOWN)) {
         ship->SetThrottle(ship->Throttle() - DELTA_THROTTLE);
         time_til_change = 0.05;
      }

      else if (KeyDown(KEY_THROTTLE_ZERO)) {
         ship->SetThrottle(0);
         if (ship->GetFLCS())
            ship->GetFLCS()->FullStop();
         time_til_change = 0.05;
      }

      else if (KeyDown(KEY_THROTTLE_FULL)) {
         ship->SetThrottle(100);
         time_til_change = 0.05;
      }

      else if (KeyDown(KEY_FLCS_MODE_AUTO)) {
         ship->CycleFLCSMode();
         time_til_change = 0.5f;
      }

      else if (KeyDown(KEY_CYCLE_PRIMARY)) {
         HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
         ship->CyclePrimary();
         time_til_change = 0.5f;
      }

      else if (KeyDown(KEY_CYCLE_SECONDARY)) {
         HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
         ship->CycleSecondary();
         time_til_change = 0.5f;
      }

      if (ship->GetShield()) {
         Shield* shield = ship->GetShield();
         double  level  = shield->GetPowerLevel();

         if (KeyDown(KEY_SHIELDS_FULL)) {
            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(100);
            time_til_change = 0.5f;
         }

         else if (KeyDown(KEY_SHIELDS_ZERO)) {
            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(0);
            time_til_change = 0.5f;
         }

         else if (KeyDown(KEY_SHIELDS_UP)) {
            if (level < 25)      level =  25;
            else if (level < 50) level =  50;
            else if (level < 75) level =  75;
            else                 level = 100;

            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(level);
            time_til_change = 0.5f;
         }

         else if (KeyDown(KEY_SHIELDS_DOWN)) {
            if (level > 75)      level =  75;
            else if (level > 50) level =  50;
            else if (level > 25) level =  25;
            else                 level =   0;

            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
            shield->SetPowerLevel(level);
            time_til_change = 0.5f;
         }

      }

      if (ship->GetSensor()) {
         Sensor* sensor = ship->GetSensor();

         if (sensor->GetMode() < Sensor::PST) {
            if (KeyDown(KEY_SENSOR_MODE)) {
               int sensor_mode = sensor->GetMode() + 1;
               if (sensor_mode > Sensor::GM)
                  sensor_mode = Sensor::PAS;

               sensor->SetMode((Sensor::Mode) sensor_mode);
               time_til_change = 0.5f;
            }

            else if (KeyDown(KEY_SENSOR_GROUND_MODE)) {
               if (ship->IsAirborne()) {
                  sensor->SetMode(Sensor::GM);
                  time_til_change = 0.5f;
               }
            }
         }
         else {
            // manual "return to search" command for starships:
            if (KeyDown(KEY_SENSOR_MODE)) {
               ship->DropTarget();
            }
         }

         if (KeyDown(KEY_SENSOR_RANGE_PLUS)) {
            sensor->IncreaseRange();
            time_til_change = 0.5f;
         }

         else if (KeyDown(KEY_SENSOR_RANGE_MINUS)) {
            sensor->DecreaseRange();
            time_til_change = 0.5f;
         }
      }

      if (KeyDown(KEY_EMCON_PLUS)) {
         ship->SetEMCON(ship->GetEMCON()+1);
         time_til_change = 0.5f;
      }

      else if (KeyDown(KEY_EMCON_MINUS)) {
         ship->SetEMCON(ship->GetEMCON()-1);
         time_til_change = 0.5f;
      }
   }
   else
      time_til_change -= seconds;

   if (controller->ActionMap(KEY_ACTION_0))
      ship->FirePrimary();

   if (controller->ActionMap(KEY_ACTION_1)) {
      if (!pickle_latch)
         ship->FireSecondary();

      pickle_latch = true;
   }
   else {
      pickle_latch = false;
   }

   if (controller->ActionMap(KEY_ACTION_3)) {
      if (!target_latch)
         ship->LockTarget(SimObject::SIM_SHIP);

      target_latch = true;
   }
   else {
      target_latch = false;
   }

   ship->SetAugmenter(augmenter || (KeyDown(KEY_AUGMENTER) ? true : false));

   if (Toggled(KEY_DECOY))
      ship->FireDecoy();

   if (Toggled(KEY_LAUNCH_PROBE))
      ship->LaunchProbe();

   if (Toggled(KEY_GEAR_TOGGLE))
      ship->ToggleGear();

   if (Toggled(KEY_NAVLIGHT_TOGGLE))
      ship->ToggleNavlights();

   if (Toggled(KEY_LOCK_TARGET))
      ship->LockTarget(SimObject::SIM_SHIP, false, true);

   else if (Toggled(KEY_LOCK_THREAT))
      ship->LockTarget(SimObject::SIM_DRONE);

   else if (Toggled(KEY_LOCK_CLOSEST_SHIP))
      ship->LockTarget(SimObject::SIM_SHIP, true, false);

   else if (Toggled(KEY_LOCK_CLOSEST_THREAT))
      ship->LockTarget(SimObject::SIM_DRONE, true, false);

   else if (Toggled(KEY_LOCK_HOSTILE_SHIP))
      ship->LockTarget(SimObject::SIM_SHIP, true, true);

   else if (Toggled(KEY_LOCK_HOSTILE_THREAT))
      ship->LockTarget(SimObject::SIM_DRONE, true, true);

   else if (Toggled(KEY_CYCLE_SUBTARGET))
      ship->CycleSubTarget(1);

   else if (Toggled(KEY_PREV_SUBTARGET))
      ship->CycleSubTarget(-1);

   if (Toggled(KEY_AUTO_NAV)) {
      ship->SetAutoNav(true);
      // careful: this object has just been deleted!
      return;
   }

   if (Toggled(KEY_DROP_ORBIT)) {
      ship->DropOrbit();
      // careful: this object has just been deleted!
      return;
   }
}

