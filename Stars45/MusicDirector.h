/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MusicDirector.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Music Director class to manage selection, setup, and playback
    of background music tracks for both menu and game modes
*/


#ifndef MusicDirector_h
#define MusicDirector_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +-------------------------------------------------------------------+

class MusicTrack;

// +-------------------------------------------------------------------+

class MusicDirector
{
public:
   enum MODES {
      NONE,

      // menu modes:

      MENU,
      INTRO,
      BRIEFING,
      DEBRIEFING,
      PROMOTION,
      VICTORY,
      DEFEAT,
      CREDITS,

      // in game modes:

      FLIGHT,
      COMBAT,
      LAUNCH,
      RECOVERY,

      // special modes:
      SHUTDOWN
   };

   enum TRANSITIONS {
      CUT,
      FADE_OUT,
      FADE_IN,
      FADE_BOTH,
      CROSS_FADE
   };

   MusicDirector();
   ~MusicDirector();

   // Operations:
   void                    ExecFrame();
   void                    ScanTracks();

   int                     CheckMode(int mode);
   int                     GetMode()   const { return mode; }

   static void             Initialize();
   static void             Close();
   static MusicDirector*   GetInstance();
   static void             SetMode(int mode);
   static const char*      GetModeName(int mode);
   static bool             IsNoMusic();

protected:
   void                    StartThread();
   void                    StopThread();
   void                    GetNextTrack(int index);
   void                    ShuffleTracks();

   int               mode;
   int               transition;

   MusicTrack*       track;
   MusicTrack*       next_track;

   List<Text>        menu_tracks;
   List<Text>        intro_tracks;
   List<Text>        brief_tracks;
   List<Text>        debrief_tracks;
   List<Text>        promote_tracks;
   List<Text>        flight_tracks;
   List<Text>        combat_tracks;
   List<Text>        launch_tracks;
   List<Text>        recovery_tracks;
   List<Text>        victory_tracks;
   List<Text>        defeat_tracks;
   List<Text>        credit_tracks;

   bool              no_music;

   HANDLE            hproc;
   ThreadSync        sync;
};

#endif MusicDirector_h