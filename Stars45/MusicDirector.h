/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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