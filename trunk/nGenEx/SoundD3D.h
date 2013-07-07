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
    FILE:         SoundD3D.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    DirectSound3D Audio Output and Buffer classes
*/

#ifndef SoundD3D_h
#define SoundD3D_h

//#define DIRECT_SOUND_3D
#include "SoundCard.h"
#include "Sound.h"
#include "Camera.h"
#include "ThreadSync.h"
#include <stdio.h>
#include <dsound.h>
#include "vorbis/vorbisfile.h"

// +--------------------------------------------------------------------+

class SoundD3D;
class SoundCardD3D;

// +--------------------------------------------------------------------+
// Sound Implementation for DirectSound and DirectSound3D

class SoundD3D : public Sound
{
public:
    static const char* TYPENAME() { return "SoundD3D"; }

    SoundD3D(LPDIRECTSOUND card, DWORD flags, LPWAVEFORMATEX format);
    SoundD3D(LPDIRECTSOUND card, DWORD flags, LPWAVEFORMATEX format, DWORD len, LPBYTE data);
    virtual ~SoundD3D();

    virtual void      Update();

    virtual HRESULT   StreamFile(const char* name, DWORD offset);
    virtual HRESULT   Load(DWORD bytes, BYTE* data);
    virtual HRESULT   Play();
    virtual HRESULT   Rewind();
    virtual HRESULT   Pause();
    virtual HRESULT   Stop();

    virtual Sound*    Duplicate();

    // (only for streamed sounds)
    virtual double    GetTotalTime()     const   { return total_time; }
    virtual double    GetTimeRemaining() const;
    virtual double    GetTimeElapsed()   const;

    // (only used for localized sounds)
    virtual void      SetVolume(long v);
    virtual long      GetPan() const;
    virtual void      SetPan(long p);
    virtual void      SetLocation(const Vec3& l);
    virtual void      SetVelocity(const Vec3& v);

    virtual float     GetMinDistance() const;
    virtual void      SetMinDistance(float f);
    virtual float     GetMaxDistance() const;
    virtual void      SetMaxDistance(float f);


protected:
    void      Localize();
    HRESULT   AllocateBuffer(DWORD bytes);
    HRESULT   StreamOggFile();

    void      StreamBlock();
    void      StreamOggBlock();
    void      RewindStream();
    void      RewindOggStream();

    LPDIRECTSOUND              soundcard;
    WAVEFORMATEX               wfex;
    DSBUFFERDESC               dsbd;
    LPDIRECTSOUNDBUFFER        buffer;

    DWORD                      data_len;
    LPBYTE                     data;

#ifdef DIRECT_SOUND_3D
    LPDIRECTSOUND3DBUFFER      sound3d;
#endif

    float                      min_dist;
    float                      max_dist;

    // STREAMED SOUND SUPPORT:
    FILE*             stream;
    DWORD             stream_left;
    double            total_time;
    DWORD             min_safety;
    DWORD             read_size;
    BYTE*             transfer;
    DWORD             w, r;
    DWORD             stream_offset;
    bool              eos_written;
    BYTE              eos_latch;
    bool              moved;

    ThreadSync        sync;
    OggVorbis_File*   ov_file;
};

// +--------------------------------------------------------------------+
// Sound Card Implementation for DS and DS3D

class SoundCardD3D : public SoundCard
{
    friend class SoundD3D;

public:
    static const char* TYPENAME() { return "SoundCardD3D"; }

    SoundCardD3D(HWND hwnd);
    virtual ~SoundCardD3D();

    // Format of the sound card's primary buffer:
    virtual bool   GetFormat(LPWAVEFORMATEX format);
    virtual bool   SetFormat(LPWAVEFORMATEX format);
    virtual bool   SetFormat(int bits, int channels, int hertz);

    virtual void   ShowFormat();

    // Get a blank, writable sound buffer:
    virtual Sound* CreateSound(DWORD flags, LPWAVEFORMATEX format);

    // Create a sound resource:
    virtual Sound* CreateSound(DWORD flags, LPWAVEFORMATEX format, DWORD len, LPBYTE data);

    virtual void   SetListener(const Camera& cam, const Vec3& vel);
    virtual bool   Pause();
    virtual bool   Resume();
    virtual bool   StopSoundEffects();

protected:
    LPDIRECTSOUND              soundcard;
    LPDIRECTSOUNDBUFFER        primary;

#ifdef DIRECT_SOUND_3D
    LPDIRECTSOUND3DLISTENER    listener;
#else
    Camera                     listener;
    Vec3                       velocity;
#endif

    WAVEFORMATEX               wfex;
    DSBUFFERDESC               dsbd;
};

#endif SoundD3D_h

