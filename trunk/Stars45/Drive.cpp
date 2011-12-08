/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Drive.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Weapon class
*/

#include "MemDebug.h"
#include "Drive.h"
#include "Power.h"
#include "Ship.h"
#include "Sim.h"
#include "DriveSprite.h"
#include "CameraDirector.h"
#include "AudioConfig.h"

#include "Light.h"
#include "Bitmap.h"
#include "Sound.h"
#include "DataLoader.h"
#include "Bolt.h"
#include "Solid.h"
#include "Game.h"

// +--------------------------------------------------------------------+

static int     drive_value[] = {
   1, 1, 1, 1, 1, 1, 1, 1
};

static float   drive_light[] = {
   10.0f, 100.0f, 5.0f, 1.0e3f, 100.0f, 10.0f, 0.0f, 0.0f
};

Bitmap* drive_flare_bitmap[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
Bitmap* drive_trail_bitmap[8]    = { 0, 0, 0, 0, 0, 0, 0, 0 };
Bitmap* drive_glow_bitmap[8]     = { 0, 0, 0, 0, 0, 0, 0, 0 };

static Sound* sound_resource[3]  = { 0, 0, 0 };

#define CLAMP(x, a, b) if (x < (a)) x = (a); else if (x > (b)) x = (b);

// +----------------------------------------------------------------------+

DrivePort::DrivePort(const Point& l, float s)
   : loc(l), flare(0), trail(0), scale(s)
{ }

DrivePort::~DrivePort()
{
   GRAPHIC_DESTROY(flare);
   GRAPHIC_DESTROY(trail);
}

// +----------------------------------------------------------------------+

Drive::Drive(SUBTYPE drive_type, float max_thrust, float max_aug, bool show)
   : System(DRIVE, drive_type, "Drive", drive_value[drive_type],
            max_thrust*2, max_thrust*2, max_thrust*2),
     thrust(max_thrust), augmenter(max_aug), scale(0.0f), 
     throttle(0.0f), augmenter_throttle(0.0f), intensity(0.0f),
     sound(0), burner_sound(0), show_trail(show)
{
   power_flags = POWER_WATTS;

   switch (drive_type) {
   default:
   case PLASMA:   name = Game::GetText("sys.drive.plasma");  break;
   case FUSION:   name = Game::GetText("sys.drive.fusion");  break;
   case GREEN:    name = Game::GetText("sys.drive.green");   break;
   case RED:      name = Game::GetText("sys.drive.red");     break;
   case BLUE:     name = Game::GetText("sys.drive.blue");    break;
   case YELLOW:   name = Game::GetText("sys.drive.yellow");  break;
   case STEALTH:  name = Game::GetText("sys.drive.stealth"); break;
   }

   abrv = Game::GetText("sys.drive.abrv");

   emcon_power[0] = 0;
   emcon_power[1] = 50;
   emcon_power[2] = 100;
}

// +----------------------------------------------------------------------+

Drive::Drive(const Drive& d)
   : System(d), thrust(d.thrust), augmenter(d.augmenter), scale(d.scale), 
     throttle(0.0f), augmenter_throttle(0.0f), intensity(0.0f),
     sound(0), burner_sound(0), show_trail(d.show_trail)
{
   power_flags = POWER_WATTS;

   Mount(d);

   if (subtype != Drive::STEALTH) {
      for (int i = 0; i < d.ports.size(); i++) {
         DrivePort* p = d.ports[i];
         CreatePort(p->loc, p->scale);
      }
   }
}

// +--------------------------------------------------------------------+

Drive::~Drive()
{
   if (sound) {
      sound->Stop();
      sound->Release();
      sound = 0;
   }

   if (burner_sound) {
      burner_sound->Stop();
      burner_sound->Release();
      burner_sound = 0;
   }

   ports.destroy();
}

// +--------------------------------------------------------------------+

void
Drive::Initialize()
{
   static int initialized = 0;
   if (initialized) return;

   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath("Drive/");
   loader->LoadTexture("Drive0.pcx",  drive_flare_bitmap[0], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Drive1.pcx",  drive_flare_bitmap[1], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Drive2.pcx",  drive_flare_bitmap[2], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Drive3.pcx",  drive_flare_bitmap[3], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Drive4.pcx",  drive_flare_bitmap[4], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Drive5.pcx",  drive_flare_bitmap[5], Bitmap::BMP_TRANSLUCENT);

   loader->LoadTexture("Trail0.pcx",  drive_trail_bitmap[0], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Trail1.pcx",  drive_trail_bitmap[1], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Trail2.pcx",  drive_trail_bitmap[2], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Trail3.pcx",  drive_trail_bitmap[3], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Trail4.pcx",  drive_trail_bitmap[4], Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Trail5.pcx",  drive_trail_bitmap[5], Bitmap::BMP_TRANSLUCENT);

   loader->LoadTexture("Glow0.pcx",   drive_glow_bitmap[0],  Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Glow1.pcx",   drive_glow_bitmap[1],  Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Glow2.pcx",   drive_glow_bitmap[2],  Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Glow3.pcx",   drive_glow_bitmap[3],  Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Glow4.pcx",   drive_glow_bitmap[4],  Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("Glow5.pcx",   drive_glow_bitmap[5],  Bitmap::BMP_TRANSLUCENT);

   const int SOUND_FLAGS = Sound::LOCALIZED |
                           Sound::LOC_3D    |
                           Sound::LOOP      |
                           Sound::LOCKED;

   loader->SetDataPath("Sounds/");
   loader->LoadSound("engine.wav",  sound_resource[0], SOUND_FLAGS);
   loader->LoadSound("burner2.wav", sound_resource[1], SOUND_FLAGS);
   loader->LoadSound("rumble.wav",  sound_resource[2], SOUND_FLAGS);
   loader->SetDataPath(0);

   if (sound_resource[0])
      sound_resource[0]->SetMaxDistance(30.0e3f);

   if (sound_resource[1])
      sound_resource[1]->SetMaxDistance(30.0e3f);

   if (sound_resource[2])
      sound_resource[2]->SetMaxDistance(50.0e3f);

   initialized = 1;
}

// +--------------------------------------------------------------------+

void
Drive::Close()
{
   for (int i = 0; i < 3; i++) {
      delete sound_resource[i];
      sound_resource[i] = 0;
   }
}

// +--------------------------------------------------------------------+

void
Drive::StartFrame()
{
}

// +--------------------------------------------------------------------+

void
Drive::AddPort(const Point& loc, float flare_scale)
{
   if (flare_scale == 0) flare_scale = scale;
   DrivePort* port = new(__FILE__,__LINE__) DrivePort(loc, flare_scale);
   ports.append(port);
}

// +--------------------------------------------------------------------+

void
Drive::CreatePort(const Point& loc, float flare_scale)
{
   Bitmap* flare_bmp = drive_flare_bitmap[subtype];
   Bitmap* trail_bmp = drive_trail_bitmap[subtype];
   Bitmap* glow_bmp  = 0;

   if (flare_scale <= 0)
      flare_scale = scale;

   if (augmenter <= 0)
      glow_bmp = drive_glow_bitmap[subtype];

   if (subtype != Drive::STEALTH && flare_scale > 0) {
      DrivePort* port = new(__FILE__,__LINE__) DrivePort(loc, flare_scale);

      if (flare_bmp) {
         DriveSprite* flare_rep = new(__FILE__,__LINE__) DriveSprite(flare_bmp, glow_bmp);
         flare_rep->Scale(flare_scale * 1.5);
         flare_rep->SetShade(0);
         port->flare = flare_rep;
      }

      if (trail_bmp && show_trail) {
         Bolt* trail_rep = new(__FILE__,__LINE__) Bolt(flare_scale * 30, flare_scale * 8, trail_bmp, true);
         port->trail = trail_rep;
      }

      ports.append(port);
   }
}

// +--------------------------------------------------------------------+

void
Drive::Orient(const Physical* rep)
{
   System::Orient(rep);

   const Matrix& orientation = rep->Cam().Orientation();
   Point         ship_loc    = rep->Location();
   
   for (int i = 0; i < ports.size(); i++) {
      DrivePort* p = ports[i];

      Point projector = (p->loc * orientation) + ship_loc;

      if (p->flare) {
         p->flare->MoveTo(projector);
         p->flare->SetFront(rep->Cam().vpn() * -10 * p->scale);
      }

      if (p->trail) {
         if (intensity > 0.5) {
            double  len = -60 * p->scale * intensity;

            if (augmenter > 0 && augmenter_throttle > 0)
               len += len * augmenter_throttle;

            p->trail->Show();
            p->trail->SetEndPoints(projector, projector + rep->Cam().vpn() * len);
         }
         else {
            p->trail->Hide();
         }
      }
   }
}

// +--------------------------------------------------------------------+

static double drive_seconds=0;

void
Drive::SetThrottle(double t, bool aug)
{
   double spool = 1.2 * drive_seconds;
   double throttle_request = t / 100;

   if (throttle < throttle_request) {
      if (throttle_request-throttle < spool) {
         throttle = (float) throttle_request;
      }
      else {
         throttle += (float) spool;
      }
   }

   else if (throttle > throttle_request) {
      if (throttle - throttle_request < spool) {
         throttle = (float) throttle_request;
      }
      else {
         throttle -= (float) spool;
      }
   }

   if (throttle < 0.5)
      aug = false;

   if (aug && augmenter_throttle < 1) {
      augmenter_throttle += (float) spool;

      if (augmenter_throttle > 1)
         augmenter_throttle = 1.0f;
   }
   else if (!aug && augmenter_throttle > 0) {
      augmenter_throttle -= (float) spool;

      if (augmenter_throttle < 0)
         augmenter_throttle = 0.0f;
   }
}

// +----------------------------------------------------------------------+

double
Drive::GetRequest(double seconds) const
{
   if (!power_on) return 0;

   double t_factor = max(throttle + 0.5 * augmenter_throttle, 0.3);

   return t_factor * power_level * sink_rate * seconds;
}

bool
Drive::IsAugmenterOn() const
{
   return   augmenter            > 0      && 
            augmenter_throttle   > 0.05   &&
            IsPowerOn()                   &&
            status               > CRITICAL;
}

// +--------------------------------------------------------------------+

int
Drive::NumEngines() const
{
   return ports.size();
}

DriveSprite*
Drive::GetFlare(int port) const
{
   if (port >= 0 && port < ports.size()) {
      return ports[port]->flare;
   }

   return 0;
}

Bolt*
Drive::GetTrail(int port) const
{
   if (port >= 0 && port < ports.size()) {
      return ports[port]->trail;
   }

   return 0;
}

// +--------------------------------------------------------------------+

float
Drive::Thrust(double seconds)
{
   drive_seconds = seconds;

   float eff    = (energy/capacity) * availability * 100.0f;
   float output = throttle * thrust * eff;
   bool  aug_on = IsAugmenterOn();

   if (aug_on) {
      output += augmenter * augmenter_throttle * eff;

      // augmenter burns extra fuel:
      PowerSource* reac = ship->Reactors()[source_index];
      reac->SetCapacity(reac->GetCapacity() - (0.1 * drive_seconds));
   }

   energy = 0.0f;

   if (output < 0 || GetPowerLevel() < 0.01)
      output = 0.0f;

   int    vol        = -10000;
   int    vol_aug    = -10000;
   double fraction   = output / thrust;

   for (int i = 0; i < ports.size(); i++) {
      DrivePort*  p           = ports[i];

      if (p->flare) {
         if (i == 0) {
            if (fraction > 0)
               intensity += (float) seconds;
            else
               intensity -= (float) seconds;

            // capture volume based on actual output:
            CLAMP(intensity, 0.0f, 1.0f);

            if (intensity > 0.25) {
               vol = (int) ((intensity - 1.0) * 10000.0);
               CLAMP(vol, -10000, -1500);

               if (aug_on && intensity > 0.5) {
                  vol_aug = (int) ((5 * augmenter_throttle - 1.0) * 10000.0);
                  CLAMP(vol_aug, -10000, -1000);
               }
            }
         }

         p->flare->SetShade(intensity);
      }

      if (p->trail) {
         p->trail->SetShade(intensity);
      }
   }

   CameraDirector* cam_dir = CameraDirector::GetInstance();

   // no sound when paused!
   if (!Game::Paused() && subtype != STEALTH && cam_dir && cam_dir->GetCamera()) {
      if (ship && ship->GetRegion() == Sim::GetSim()->GetActiveRegion()) {
         if (!sound) {
            int sound_index = 0;
            if (thrust > 100)
               sound_index = 2;

            if (sound_resource[sound_index])
               sound = sound_resource[sound_index]->Duplicate();
         }

         if (aug_on && !burner_sound) {
            if (sound_resource[1])
               burner_sound = sound_resource[1]->Duplicate();
         }

         Point cam_loc = cam_dir->GetCamera()->Pos();
         double dist = (ship->Location() - cam_loc).length();

         if (sound && dist < sound->GetMaxDistance()) {
            long max_vol = AudioConfig::EfxVolume();

            if (vol > max_vol)
               vol = max_vol;

            if (sound) {
               sound->SetLocation(ship->Location());
               sound->SetVolume(vol);
               sound->Play();
            }

            if (burner_sound) {
               if (vol_aug > max_vol)
                  vol_aug = max_vol;

               burner_sound->SetLocation(ship->Location());
               burner_sound->SetVolume(vol_aug);
               burner_sound->Play();
            }
         }
         else {
            if (sound && sound->IsPlaying())
               sound->Stop();

            if (burner_sound && burner_sound->IsPlaying())
               burner_sound->Stop();
         }
      }
      else {
         if (sound && sound->IsPlaying())
            sound->Stop();

         if (burner_sound && burner_sound->IsPlaying())
            burner_sound->Stop();
      }
   }

   return output;
}
