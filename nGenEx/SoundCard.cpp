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
    FILE:         SoundCard.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract sound card class
*/

#include "MemDebug.h"
#include "SoundCard.h"
#include "Sound.h"

// +--------------------------------------------------------------------+

DWORD WINAPI SoundCardUpdateProc(LPVOID link);

// +--------------------------------------------------------------------+

SoundCard::SoundCard()
: status(SC_UNINITIALIZED), hthread(0), shutdown(false)
{
    DWORD thread_id = 0;
    hthread = CreateThread(0, 4096, SoundCardUpdateProc,
    (LPVOID) this, 0, &thread_id);
}

// +--------------------------------------------------------------------+

SoundCard::~SoundCard()
{
    shutdown = true;

    WaitForSingleObject(hthread, 500);
    CloseHandle(hthread);
    hthread = 0;

    sounds.destroy();
    status = SC_UNINITIALIZED;
}

// +--------------------------------------------------------------------+

DWORD WINAPI SoundCardUpdateProc(LPVOID link)
{
    SoundCard* card = (SoundCard*) link;

    if (card)
    return card->UpdateThread();

    return (DWORD) E_POINTER;
}

// +--------------------------------------------------------------------+

DWORD
SoundCard::UpdateThread()
{
    while (!shutdown) {
        Update();
        Sleep(50);
    }

    return 0;
}

// +--------------------------------------------------------------------+

void
SoundCard::Update()
{
    AutoThreadSync a(sync);

    ListIter<Sound> iter = sounds;
    while (++iter) {
        Sound* s = iter.value();

        s->Update();

        if (s->GetStatus() == Sound::DONE &&
                !(s->GetFlags()  &  Sound::LOCKED)) {

            delete iter.removeItem();
        }
    }
}

// +--------------------------------------------------------------------+

void
SoundCard::AddSound(Sound* s)
{
    AutoThreadSync a(sync);

    if (!sounds.contains(s))
    sounds.append(s);
}

