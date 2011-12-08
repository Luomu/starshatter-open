/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MissionEvent.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Events for mission scripting
*/

#include "MemDebug.h"
#include "MissionEvent.h"
#include "Mission.h"
#include "StarSystem.h"
#include "Galaxy.h"
#include "Starshatter.h"
#include "StarServer.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "DisplayView.h"
#include "HUDView.h"
#include "Instruction.h"
#include "QuantumDrive.h"
#include "Sim.h"
#include "AudioConfig.h"
#include "CameraDirector.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Player.h"
#include "Campaign.h"
#include "CombatGroup.h"

#include "NetData.h"
#include "NetUtil.h"

#include "Game.h"
#include "DataLoader.h"
#include "Font.h"
#include "FontMgr.h"
#include "Sound.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "Random.h"

const char* FormatGameTime();

// +--------------------------------------------------------------------+

MissionEvent::MissionEvent()
   :  id(0), status(PENDING), time(0), delay(0), event(0), event_nparams(0),
      event_chance(100), trigger(0), trigger_nparams(0), sound(0)
{
   ZeroMemory(event_param,   sizeof(event_param));
   ZeroMemory(trigger_param, sizeof(trigger_param));
}

MissionEvent::~MissionEvent()
{
   if (sound) {
      sound->Stop();
      sound->Release();
   }
}

// +--------------------------------------------------------------------+

void
MissionEvent::ExecFrame(double seconds)
{
   Sim* sim = Sim::GetSim();

   if (!sim) {
      status = PENDING;
      return;
   }

   if (status == PENDING)
      CheckTrigger();

   if (status == ACTIVE) {
      if (delay > 0)
         delay -= seconds;

      else
         Execute();
   }
}

// +--------------------------------------------------------------------+

void
MissionEvent::Activate()
{
   if (status == PENDING) {
      if (event_chance > 0 && event_chance < 100) {
         if (Random(0, 100) < event_chance)
            status = ACTIVE;
         else
            status = SKIPPED;
      }

      else {
         status = ACTIVE;
      }

      if (status == SKIPPED) {
         Sim::GetSim()->ProcessEventTrigger(TRIGGER_SKIPPED, id);
      }
   }
}

void
MissionEvent::Skip()
{
   if (status == PENDING) {
      status = SKIPPED;
   }
}

// +--------------------------------------------------------------------+

bool
MissionEvent::CheckTrigger()
{
   Sim* sim = Sim::GetSim();

   if (time > 0 && time > sim->MissionClock())
      return false;

   switch (trigger) {
   case TRIGGER_TIME: {
            if (time <= sim->MissionClock())
               Activate();
         }
         break;

   case TRIGGER_DAMAGE: {
            Ship* ship = sim->FindShip(trigger_ship);
            if (ship) {
               double damage = 100.0 * (ship->Design()->integrity - ship->Integrity()) /
                                       (ship->Design()->integrity);
         
               if (damage >= trigger_param[0])
                  Activate();
            }
         }
         break;

   case TRIGGER_DETECT: {
            Ship* ship = sim->FindShip(trigger_ship);
            Ship* tgt  = sim->FindShip(trigger_target);
            
            if (ship && tgt) {
               if (ship->FindContact(tgt))
                  Activate();
            }
            else {
               Skip();
            }
         }
         break;

   case TRIGGER_RANGE: {
            Ship* ship = sim->FindShip(trigger_ship);
            Ship* tgt  = sim->FindShip(trigger_target);
            
            if (ship && tgt) {
               double range     = (ship->Location() - tgt->Location()).length();
               double min_range = 0;
               double max_range = 1e12;

               if (trigger_param[0] > 0)
                  min_range = trigger_param[0];
               else
                  max_range = -trigger_param[0];

               if (range < min_range || range > max_range)
                  Activate();
            }
            else {
               Skip();
            }
         }
         break;

   case TRIGGER_SHIPS_LEFT: {
            int alive   = 0;
            int count   = 0;
            int iff     = -1;
            int nparams = NumTriggerParams();

            if (nparams > 0) count = TriggerParam(0);
            if (nparams > 1) iff   = TriggerParam(1);

            ListIter<SimRegion> iter = sim->GetRegions();
            while (++iter) {
               SimRegion* rgn = iter.value();

               ListIter<Ship> s_iter = rgn->Ships();
               while (++s_iter) {
                  Ship* ship = s_iter.value();

                  if (ship->Type() >= Ship::STATION)
                     continue;

                  if (ship->Life() == 0 && ship->RespawnCount() < 1)
                     continue;

                  if (iff < 0 || ship->GetIFF() == iff)
                     alive++;
               }
            }

            if (alive <= count)
               Activate();
         }
         break;

   case TRIGGER_EVENT_ALL: {
            bool  all     = true;
            int   nparams = NumTriggerParams();
            for (int i = 0; all && i < nparams; i++) {
               int trigger_id = TriggerParam(i);

               ListIter<MissionEvent> iter = sim->GetEvents();
               while (++iter) {
                  MissionEvent* e = iter.value();
                  if (e->EventID() == trigger_id) {
                     if (e->Status() != COMPLETE)
                        all = false;
                     break;
                  }

                  else if (e->EventID() == -trigger_id) {
                     if (e->Status() == COMPLETE)
                        all = false;
                     break;
                  }
               }
            }

            if (all)
               Activate();
         }
         break;

   case TRIGGER_EVENT_ANY: {
            bool  any     = false;
            int   nparams = NumTriggerParams();
            for (int i = 0; !any && i < nparams; i++) {
               int trigger_id = TriggerParam(i);

               ListIter<MissionEvent> iter = sim->GetEvents();
               while (++iter) {
                  MissionEvent* e = iter.value();
                  if (e->EventID() == trigger_id) {
                     if (e->Status() == COMPLETE)
                        any = true;
                     break;
                  }
               }
            }

            if (any)
               Activate();
         }
         break;
   }

   return status == ACTIVE;
}

// +--------------------------------------------------------------------+

void
MissionEvent::Execute(bool silent)
{
   Starshatter*   stars    = Starshatter::GetInstance();
   HUDView*       hud      = HUDView::GetInstance();
   Sim*           sim      = Sim::GetSim();
   Ship*          player   = sim->GetPlayerShip();
   Ship*          ship     = 0;
   Ship*          src      = 0;
   Ship*          tgt      = 0;
   Element*       elem     = 0;
   int            pan      = 0;
   bool           end_mission = false;

   if (event_ship.length())
      ship = sim->FindShip(event_ship);
   else
      ship = player;

   if (event_source.length())
      src = sim->FindShip(event_source);

   if (event_target.length())
      tgt = sim->FindShip(event_target);

   if (ship)
      elem = ship->GetElement();

   else if (event_ship.length()) {
      elem = sim->FindElement(event_ship);

      if (elem)
         ship = elem->GetShip(1);
   }

   // expire the delay, if any remains
   delay = 0;

   // fire the event action
   switch (event) {
   case MESSAGE:
      if (event_message.length() > 0) {
         if (ship) {
            RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(ship, src, event_param[0]);
            msg->SetInfo(event_message);
            msg->SetChannel(ship->GetIFF());
            if (tgt)
               msg->AddTarget(tgt);
            RadioTraffic::Transmit(msg);
         }

         else if (elem) {
            RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(elem, src, event_param[0]);
            msg->SetInfo(event_message);
            msg->SetChannel(elem->GetIFF());
            if (tgt)
               msg->AddTarget(tgt);
            RadioTraffic::Transmit(msg);
         }
      }

      if (event_sound.length() > 0) {
         pan = event_param[0];
      }
      break;

   case OBJECTIVE: 
      if (elem) {
         if (event_param[0]) {
            elem->ClearInstructions();
            elem->ClearObjectives();
         }

         Instruction* obj = new(__FILE__,__LINE__) Instruction(event_param[0], 0);
         obj->SetTarget(event_target);
         elem->AddObjective(obj);

         if (elem->Contains(player)) {
            HUDView* hud = HUDView::GetInstance();

            if (hud)
               hud->ShowHUDInst();
         }
      }
      break;

   case INSTRUCTION:
      if (elem) {
         if (event_param[0])
            elem->ClearInstructions();

         elem->AddInstruction(event_message);

         if (elem->Contains(player) && event_message.length() > 0) {
            HUDView* hud = HUDView::GetInstance();

            if (hud)
               hud->ShowHUDInst();
         }
      }
      break;

   case IFF:
      if (elem) {
         elem->SetIFF(event_param[0]);
      }

      else if (ship) {
         ship->SetIFF(event_param[0]);
      }
      break;

   case DAMAGE:
      if (ship) {
         ship->InflictDamage(event_param[0]);

         if (ship->Integrity() < 1) {
            NetUtil::SendObjKill(ship, 0, NetObjKill::KILL_MISC);
            ship->DeathSpiral();
            Print("    %s Killed By Scripted Event %d (%s)\n", (const char*) ship->Name(), id, FormatGameTime());
         }
      }
      else {
         Print("   EVENT %d: Could not apply damage to ship '%s' (not found).\n", id, (const char*) event_ship);
      }
      break;

   case JUMP:
      if (ship) {
         SimRegion* rgn = sim->FindRegion(event_target);

         if (rgn && ship->GetRegion() != rgn) {
            if (rgn->IsOrbital()) {
               QuantumDrive* quantum_drive = ship->GetQuantumDrive();
               if (quantum_drive) {
                  quantum_drive->SetDestination(rgn, Point(0,0,0));
                  quantum_drive->Engage(true); // request immediate jump
               }

               else if (ship->IsAirborne()) {
                  ship->MakeOrbit();
               }
            }

            else {
               ship->DropOrbit();
            }
         }

      }
      break;

   case HOLD:
      if (elem)
         elem->SetHoldTime(event_param[0]);
      break;

   case SKIP: {
         for (int i = 0; i < event_nparams; i++) {
            int skip_id = event_param[i];

            ListIter<MissionEvent> iter = sim->GetEvents();
            while (++iter) {
               MissionEvent* e = iter.value();
               if (e->EventID() == skip_id) {
                  if (e->status != COMPLETE)
                     e->status = SKIPPED;
               }
            }
         }
      }
      break;

   case END_MISSION:
      Print("    END MISSION By Scripted Event %d (%s)\n", id, FormatGameTime());
      end_mission = true;
      break;

   //
   // NOTE: CUTSCENE EVENTS DO NOT APPLY IN MULTIPLAYER
   //
   case BEGIN_SCENE:
      Print("    ------------------------------------\n");
      Print("    Begin Cutscene '%s'\n", event_message.data());
      stars->BeginCutscene();
      break;

   case END_SCENE:
      Print("    End Cutscene '%s'\n", event_message.data());
      Print("    ------------------------------------\n");
      stars->EndCutscene();
      break;

   case CAMERA:
      if (stars->InCutscene()) {
         CameraDirector* cam_dir = CameraDirector::GetInstance();

         if (!cam_dir->GetShip())
            cam_dir->SetShip(player);

         switch (event_param[0]) {
         case 1:  
            if (cam_dir->GetMode() != CameraDirector::MODE_COCKPIT)
               cam_dir->SetMode(CameraDirector::MODE_COCKPIT, event_rect.x);
            break;

         case 2:
            if (cam_dir->GetMode() != CameraDirector::MODE_CHASE)
               cam_dir->SetMode(CameraDirector::MODE_CHASE, event_rect.x);
            break;

         case 3:
            if (cam_dir->GetMode() != CameraDirector::MODE_ORBIT)
               cam_dir->SetMode(CameraDirector::MODE_ORBIT, event_rect.x);
            break;

         case 4:
            if (cam_dir->GetMode() != CameraDirector::MODE_TARGET)
               cam_dir->SetMode(CameraDirector::MODE_TARGET, event_rect.x);
            break;
         }

         if (event_target.length()) {
            ::Print("Mission Event %d: setting camera target to %s\n", id, (const char*) event_target);
            Ship* s_tgt = 0;
            
            if (event_target.indexOf("body:") < 0)
               s_tgt = sim->FindShip(event_target);

            if (s_tgt) {
               ::Print("   found ship %s\n", s_tgt->Name());
               cam_dir->SetViewOrbital(0);

               if (cam_dir->GetViewObject() != s_tgt) {

                  if (event_param[0] == 6) {
                     s_tgt->DropCam(event_param[1], event_param[2]);
                     cam_dir->SetShip(s_tgt);
                     cam_dir->SetMode(CameraDirector::MODE_DROP, 0);
                  }
                  else {
                     Ship* cam_ship = cam_dir->GetShip();

                     if (cam_ship && cam_ship->IsDropCam()) {
                        cam_ship->CompleteTransition();
                     }

                     if (cam_dir->GetShip() != sim->GetPlayerShip())
                        cam_dir->SetShip(sim->GetPlayerShip());
                     cam_dir->SetViewObject(s_tgt, true); // immediate, no transition
                  }
               }
            }

            else {
               const char* body_name = event_target.data();

               if (!strncmp(body_name, "body:", 5))
                  body_name += 5;

               Orbital* orb = sim->FindOrbitalBody(body_name);

               if (orb) {
                  ::Print("   found body %s\n", orb->Name());
                  cam_dir->SetViewOrbital(orb);
               }
            }
         }

         if (event_param[0] == 3) {
            cam_dir->SetOrbitPoint(event_point.x, event_point.y, event_point.z);
         }

         else if (event_param[0] == 5) {
            cam_dir->SetOrbitRates(event_point.x, event_point.y, event_point.z);
         }
      }
      break;

   case VOLUME:
      if (stars->InCutscene()) {
         AudioConfig* audio_cfg = AudioConfig::GetInstance();

         audio_cfg->SetEfxVolume(event_param[0]);
         audio_cfg->SetWrnVolume(event_param[0]);
      }
      break;

   case DISPLAY:
      if (stars->InCutscene()) {
         DisplayView* disp_view = DisplayView::GetInstance();

         if (disp_view) {
            Color color;
            color.Set(event_param[0]);

            if (event_message.length() && event_source.length()) {

               if (event_message.contains('$')) {
                  Campaign*      campaign = Campaign::GetCampaign();
                  Player*        user     = Player::GetCurrentPlayer();
                  CombatGroup*   group    = campaign->GetPlayerGroup();

                  if (user) {
                     event_message = FormatTextReplace(event_message, "$NAME",  user->Name().data());
                     event_message = FormatTextReplace(event_message, "$RANK",  Player::RankName(user->Rank()));
                  }

                  if (group) {
                     event_message = FormatTextReplace(event_message, "$GROUP", group->GetDescription());
                  }

                  if (event_message.contains("$TIME")) {
                     char timestr[32];
                     FormatDayTime(timestr, campaign->GetTime(), true);
                     event_message = FormatTextReplace(event_message, "$TIME", timestr);
                  }
               }

               disp_view->AddText(  event_message,
                                    FontMgr::Find(event_source),
                                    color,
                                    event_rect,
                                    event_point.y,
                                    event_point.x,
                                    event_point.z);

            }

            else if (event_target.length()) {
               DataLoader* loader = DataLoader::GetLoader();

               if (loader) {
                  loader->SetDataPath(0);
                  loader->LoadBitmap(event_target, image, 0, true);
               }

               if (image.Width() && image.Height())
               disp_view->AddImage( &image,
                                    color,
                                    Video::BLEND_ALPHA,
                                    event_rect,
                                    event_point.y,
                                    event_point.x,
                                    event_point.z);
            }
         }
      }
      break;

   case FIRE_WEAPON:
      if (ship) {
         // fire single weapon:
         if (event_param[0] >= 0) {
            ship->FireWeapon(event_param[0]);
         }

         // fire all weapons:
         else {
            ListIter<WeaponGroup> g_iter = ship->Weapons();
            while (++g_iter) {
               ListIter<Weapon> w_iter = g_iter->GetWeapons();
               while (++w_iter) {
                  Weapon* w = w_iter.value();
                  w->Fire();
               }
            }
         }
      }
      break;

   default:
      break;
   }

   sim->ProcessEventTrigger(TRIGGER_EVENT, id);

   if (!silent && !sound && event_sound.length()) {
      DataLoader* loader = DataLoader::GetLoader();
      bool        use_fs = loader->IsFileSystemEnabled();
      DWORD       flags  = pan ? Sound::LOCKED|Sound::LOCALIZED : 
                                 Sound::LOCKED|Sound::AMBIENT;

      loader->UseFileSystem(true);
      loader->SetDataPath("Sounds/");
      loader->LoadSound(event_sound, sound, flags);
      loader->SetDataPath(0);

      if (!sound) {
         loader->SetDataPath("Mods/Sounds/");
         loader->LoadSound(event_sound, sound, flags);
         loader->SetDataPath(0);
      }

      if (!sound) {
         loader->LoadSound(event_sound, sound, flags);
      }

      loader->UseFileSystem(use_fs);

      // fire and forget:
      if (sound) {
         if (sound->GetFlags() & Sound::STREAMED) {
            sound->SetFlags(flags | sound->GetFlags());
            sound->SetVolume(AudioConfig::VoxVolume());
            sound->Play();
         }
         else {
            sound->SetFlags(flags);
            sound->SetVolume(AudioConfig::VoxVolume());
            sound->SetPan(pan);
            sound->SetFilename(event_sound);
            sound->AddToSoundCard();
            sound->Play();
         }
      }
   }

   status = COMPLETE;

   if (end_mission) {
      StarServer*  server = StarServer::GetInstance();

      if (stars) {
         stars->EndMission();
      }

      else if (server) {
         // end mission event uses event_target member
         // to forward server to next mission in the chain:
         if (event_target.length())
            server->SetNextMission(event_target);

         server->SetGameMode(StarServer::MENU_MODE);
      }
   }
}

// +--------------------------------------------------------------------+

Text
MissionEvent::TriggerParamStr() const
{

   Text result;
   char buffer[8];

   if (trigger_param[0] == 0) {
      // nothing
   }

   else if (trigger_param[1] == 0) {
      sprintf(buffer, "%d", trigger_param[0]);
      result = buffer;
   }

   else {
      result = "(";

      for (int i = 0; i < 8; i++) {
         if (trigger_param[i] == 0)
            break;

         if (i < 7 && trigger_param[i+1] != 0)
            sprintf(buffer, "%d, ", trigger_param[i]);
         else
            sprintf(buffer, "%d", trigger_param[i]);

         result += buffer;
      }

      result += ")";
   }

   return result;
}

// +--------------------------------------------------------------------+

int
MissionEvent::EventParam(int index) const
{
   if (index >= 0 && index < NumEventParams())
      return event_param[index];

   return 0;
}

int
MissionEvent::NumEventParams() const
{
   return event_nparams;
}

// +--------------------------------------------------------------------+

int
MissionEvent::TriggerParam(int index) const
{
   if (index >= 0 && index < NumTriggerParams())
      return trigger_param[index];

   return 0;
}

int
MissionEvent::NumTriggerParams() const
{
   return trigger_nparams;
}

// +--------------------------------------------------------------------+

static const char* event_names[] = {
   "Message", 
   "Objective",
   "Instruction", 
   "IFF",
   "Damage",
   "Jump",
   "Hold",
   "Skip",
   "Exit",

   "BeginScene",
   "Camera",
   "Volume",
   "Display",
   "Fire",
   "EndScene"
};

static const char* trigger_names[] = {
   "Time",
   "Damage",
   "Destroyed", 
   "Jump",
   "Launch",
   "Dock",
   "Navpoint",
   "Event",
   "Skipped",
   "Target",
   "Ships Left",
   "Detect",
   "Range",
   "Event (ALL)",
   "Event (ANY)"
};

const char*
MissionEvent::EventName() const
{
   return event_names[event];
}

const char*
MissionEvent::EventName(int n)
{
   return event_names[n];
}

int
MissionEvent::EventForName(const char* n)
{
   for (int i = 0; i < NUM_EVENTS; i++)
      if (!stricmp(n, event_names[i]))
         return i;

   return 0;
}

const char*
MissionEvent::TriggerName() const
{
   return trigger_names[trigger];
}

const char*
MissionEvent::TriggerName(int n)
{
   return trigger_names[n];
}

int
MissionEvent::TriggerForName(const char* n)
{
   for (int i = 0; i < NUM_TRIGGERS; i++)
      if (!stricmp(n, trigger_names[i]))
         return i;

   return 0;
}