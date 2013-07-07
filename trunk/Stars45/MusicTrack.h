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
    FILE:         MusicTrack.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    MusicTrack class
*/


#ifndef MusicTrack_h
#define MusicTrack_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +-------------------------------------------------------------------+

class Sound;

// +-------------------------------------------------------------------+

class MusicTrack
{
public:
    enum STATE { NONE, FADE_IN, PLAY, FADE_OUT, STOP };

    MusicTrack(const Text& name, int mode=0, int index=0);
    virtual ~MusicTrack();

    // Operations:
    virtual void      ExecFrame();

    virtual void      Play();
    virtual void      Stop();
    virtual void      FadeIn();
    virtual void      FadeOut();

    // accessors / mutators
    const Text&       Name()      const { return name;    }
    Sound*            GetSound()  const { return sound;   }
    int               GetState()  const { return state;   }
    int               GetMode()   const { return mode;    }
    int               GetIndex()  const { return index;   }

    int               IsReady()   const;
    int               IsPlaying() const;
    int               IsDone()    const;
    int               IsLooped()  const;

    virtual long      GetVolume() const;
    virtual void      SetVolume(long v);

    virtual double    GetTotalTime()     const;
    virtual double    GetTimeRemaining() const;
    virtual double    GetTimeElapsed()   const;

    virtual double    GetFadeTime()      const { return fade_time; }
    virtual void      SetFadeTime(double t)    { fade_time = t;    }

protected:
    Text              name;
    Sound*            sound;
    int               state;
    int               mode;
    int               index;
    double            fade;
    double            fade_time;
};

#endif MusicTrack_h