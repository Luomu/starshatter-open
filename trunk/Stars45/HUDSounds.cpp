/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         HUDSounds.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HUDSounds singleton class utility implementation
*/

#include "MemDebug.h"
#include "HUDSounds.h"
#include "AudioConfig.h"

#include "Sound.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+

static Sound* mfd_mode     = 0;
static Sound* nav_mode     = 0;
static Sound* wep_mode     = 0;
static Sound* wep_disp     = 0;
static Sound* hud_mode     = 0;
static Sound* hud_widget   = 0;
static Sound* shield_level = 0;
static Sound* red_alert    = 0;
static Sound* tac_accept   = 0;
static Sound* tac_reject   = 0;

// +--------------------------------------------------------------------+

static void LoadInterfaceSound(DataLoader* loader, const char* wave, Sound*& s)
{
   loader->LoadSound(wave, s, 0, true);   // optional sound effect
}

void
HUDSounds::Initialize()
{
   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath("Sounds/");

   LoadInterfaceSound(loader, "mfd_mode.wav",      mfd_mode);
   LoadInterfaceSound(loader, "nav_mode.wav",      nav_mode);
   LoadInterfaceSound(loader, "wep_mode.wav",      wep_mode);
   LoadInterfaceSound(loader, "wep_disp.wav",      wep_disp);
   LoadInterfaceSound(loader, "hud_mode.wav",      hud_mode);
   LoadInterfaceSound(loader, "hud_widget.wav",    hud_widget);
   LoadInterfaceSound(loader, "shield_level.wav",  shield_level);
   LoadInterfaceSound(loader, "alarm.wav",         red_alert);
   LoadInterfaceSound(loader, "tac_accept.wav",    tac_accept);
   LoadInterfaceSound(loader, "tac_reject.wav",    tac_reject);

   if (red_alert)
      red_alert->SetFlags(Sound::AMBIENT | Sound::LOOP | Sound::LOCKED);

   loader->SetDataPath(0);
}

// +--------------------------------------------------------------------+

void
HUDSounds::Close()
{
   delete mfd_mode;
   delete nav_mode;
   delete wep_mode;
   delete wep_disp;
   delete hud_mode;
   delete hud_widget;
   delete shield_level;
   delete red_alert;
   delete tac_accept;
   delete tac_reject;
}

void HUDSounds::PlaySound(int n)
{
   Sound* sound = 0;

   switch (n) {
   default:
   case SND_MFD_MODE:      if (mfd_mode)     sound = mfd_mode->Duplicate();      break;
   case SND_NAV_MODE:      if (nav_mode)     sound = nav_mode->Duplicate();      break;
   case SND_WEP_MODE:      if (wep_mode)     sound = wep_mode->Duplicate();      break;
   case SND_WEP_DISP:      if (wep_disp)     sound = wep_disp->Duplicate();      break;
   case SND_HUD_MODE:      if (hud_mode)     sound = hud_mode->Duplicate();      break;
   case SND_HUD_WIDGET:    if (hud_widget)   sound = hud_widget->Duplicate();    break;
   case SND_SHIELD_LEVEL:  if (shield_level) sound = shield_level->Duplicate();  break;
   case SND_TAC_ACCEPT:    if (tac_accept)   sound = tac_accept->Duplicate();    break;
   case SND_TAC_REJECT:    if (tac_reject)   sound = tac_reject->Duplicate();    break;

   // RED ALERT IS A SPECIAL CASE!
   case SND_RED_ALERT:     
      if (red_alert) {
         sound = red_alert;
      }
      break;
   }

   if (sound && !sound->IsPlaying()) {
      int gui_volume = AudioConfig::GuiVolume();
      sound->SetVolume(gui_volume);
      sound->Play();
   }
}

void HUDSounds::StopSound(int n)
{
   if (n == SND_RED_ALERT && red_alert) {
      red_alert->Stop();
   }
}
