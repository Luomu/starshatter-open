/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         HUDSounds.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    HUDSounds singleton class utility
*/

#ifndef HUDSounds_h
#define HUDSounds_h

#include "Types.h"

// +--------------------------------------------------------------------+

class HUDSounds
{
public:
   enum SOUNDS {
      SND_MFD_MODE,
      SND_NAV_MODE,
      SND_WEP_MODE,
      SND_WEP_DISP,
      SND_HUD_MODE,
      SND_HUD_WIDGET,
      SND_SHIELD_LEVEL,
      SND_RED_ALERT,
      SND_TAC_ACCEPT,
      SND_TAC_REJECT
   };

   static void Initialize();
   static void Close();

   static void PlaySound(int n);
   static void StopSound(int n);
};

#endif HUDSounds_h

