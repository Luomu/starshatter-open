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
    FILE:         MusicTrack.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Music Director class to manage selection, setup, and playback
    of background music tracks for both menu and game modes
*/


#include "MemDebug.h"
#include "MusicTrack.h"
#include "MusicDirector.h"
#include "Starshatter.h"
#include "AudioConfig.h"

#include "Game.h"
#include "Sound.h"

// +-------------------------------------------------------------------+

const double FADE_TIME =     1.5;
const double SILENCE   = -5000;

// +-------------------------------------------------------------------+

MusicTrack::MusicTrack(const Text& txt, int m, int n)
: name(txt), sound(0), state(NONE), mode(m), index(n), 
fade(0), fade_time(FADE_TIME)
{
    long max_vol = 0;

    if (mode >= MusicDirector::FLIGHT)
    max_vol = AudioConfig::GameMusic();
    else
    max_vol = AudioConfig::MenuMusic();

    if (max_vol <= AudioConfig::Silence())
    return;

    name.setSensitive(false);

    if (name.contains(".ogg")) {
        sound = Sound::CreateOggStream(name);

        if (name.contains("-loop")) {
            sound->SetFlags(Sound::STREAMED  |
            Sound::OGGVORBIS |
            Sound::LOOP      |
            Sound::LOCKED);
        }

        else {
            sound->SetFlags(Sound::STREAMED  |
            Sound::OGGVORBIS |
            Sound::LOCKED);
        }

        sound->SetVolume((long) SILENCE);
    }
}

MusicTrack::~MusicTrack()
{
    if (sound) {
        sound->Stop();
        sound->Release();
    }
}

// +--------------------------------------------------------------------+

void
MusicTrack::ExecFrame()
{
    bool music_pause = false;

    Starshatter* stars = Starshatter::GetInstance();
    if (stars) {
        music_pause = (stars->GetGameMode() == Starshatter::PLAY_MODE) &&
        Game::Paused();
    }

    if (sound && !music_pause) {
        double fvol    = 1;
        long   volume  = 0;

        switch (state) {
        case PLAY:
            if (sound->IsReady())
            sound->Play();
            SetVolume(volume);
            break;

        case FADE_IN:
            if (sound->IsReady())
            sound->Play();

            if (fade > 0) {
                fvol   = fade/fade_time;
                volume = (long) (fvol * SILENCE);
                SetVolume(volume);
            }

            if (fade < 0.01)
            state = PLAY;
            break;

        case FADE_OUT:
            if (sound->IsReady())
            sound->Play();

            if (fade > 0) {
                fvol   = 1 - fade/fade_time;
                volume = (long) (fvol * SILENCE);
                SetVolume(volume);
            }

            if (fade < 0.01)
            state = STOP;
            break;

        case STOP:
            if (sound->IsPlaying()) {
                sound->Stop();
                sound->Release();
                sound = 0;
            }
            break;
        }

        if (fade > 0)
        fade -= Game::GUITime();

        if (fade < 0)
        fade = 0;
    }
}

// +--------------------------------------------------------------------+

void
MusicTrack::Play()
{
    state = PLAY;
    fade = 0;
}

void
MusicTrack::Stop()
{
    state = STOP;
    fade = 0;
}

void
MusicTrack::FadeIn()
{
    if (state != FADE_IN && state != PLAY) {
        state = FADE_IN;
        fade  = fade_time;
    }
}

void
MusicTrack::FadeOut()
{
    if (state != FADE_OUT && state != STOP) {
        state = FADE_OUT;
        fade  = fade_time;
    }
}

// +--------------------------------------------------------------------+

int
MusicTrack::IsReady() const
{
    if (sound)
    return sound->IsReady();

    return false;
}

int
MusicTrack::IsPlaying() const
{
    if (sound)
    return sound->IsPlaying();

    return false;
}

int
MusicTrack::IsDone() const
{
    if (sound)
    return sound->IsDone() || sound->LoopCount() >= 5;

    return true;
}

int
MusicTrack::IsLooped() const
{
    if (sound)
    return sound->IsDone() || sound->LoopCount() >= 4;

    return true;
}

// +--------------------------------------------------------------------+

long
MusicTrack::GetVolume() const
{
    if (sound)
    return sound->GetVolume();

    return 0;
}

void
MusicTrack::SetVolume(long v)
{
    if (sound) {
        long max_vol = 0;

        if (mode >= MusicDirector::FLIGHT)
        max_vol = AudioConfig::GameMusic();
        else
        max_vol = AudioConfig::MenuMusic();

        if (v > max_vol)
        v = max_vol;

        sound->SetVolume(v);
    }
}

double
MusicTrack::GetTotalTime() const
{
    if (sound)
    return sound->GetTotalTime();

    return 0;
}

double
MusicTrack::GetTimeRemaining() const
{
    if (sound)
    return sound->GetTimeRemaining();

    return 0;
}

double
MusicTrack::GetTimeElapsed() const
{
    if (sound)
    return sound->GetTimeElapsed();

    return 0;
}

