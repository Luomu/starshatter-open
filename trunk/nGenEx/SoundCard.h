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

    SUBSYSTEM:    nGenEx.lib
    FILE:         SoundCard.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Audio Output class (hides details of DirectSound)
*/

#ifndef SoundCard_h
#define SoundCard_h

#include "Types.h"
#include "List.h"
#include "ThreadSync.h"

// +--------------------------------------------------------------------+

class  Sound;
class  Camera;
struct Vec3;

// +--------------------------------------------------------------------+

class SoundCard
{
public:
    static const char* TYPENAME() { return "SoundCard"; }

    SoundCard();
    virtual ~SoundCard();

    enum SoundStatus { SC_UNINITIALIZED,
        SC_OK,
        SC_ERROR,
        SC_BAD_PARAM };
    SoundStatus    Status() const { return status; }

    // Format of the sound card's primary buffer:
    virtual bool   GetFormat(LPWAVEFORMATEX format)                { return false; }
    virtual bool   SetFormat(LPWAVEFORMATEX format)                { return false; }
    virtual bool   SetFormat(int bits, int channels, int hertz)    { return false; }
    virtual bool   Pause()                                         { return false; }
    virtual bool   Resume()                                        { return false; }
    virtual bool   StopSoundEffects()                              { return false; }

    // Get a blank, writable sound buffer:
    virtual Sound* CreateSound(DWORD flags, LPWAVEFORMATEX format) { return 0;     }

    // Create a sound resource:
    virtual Sound* CreateSound(DWORD flags, LPWAVEFORMATEX format,
    DWORD len, LPBYTE data)             { return 0;     }

    // once per frame:
    virtual void   Update();

    virtual void   SetListener(const Camera& cam, const Vec3& vel) { }
    virtual DWORD  UpdateThread();
    virtual void   AddSound(Sound* s);

protected:

    bool           shutdown;
    HANDLE         hthread;
    SoundStatus    status;
    List<Sound>    sounds;
    ThreadSync     sync;
};

#endif SoundCard_h

