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
    FILE:         RadioVox.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#include "MemDebug.h"
#include "RadioVox.h"
#include "RadioView.h"
#include "AudioConfig.h"

#include "DataLoader.h"
#include "Game.h"
#include "Sound.h"
#include "ThreadSync.h"

// +====================================================================+
//
// RADIO VOX CONTROLLER:
//

DWORD WINAPI VoxUpdateProc(LPVOID link);

class RadioVoxController
{
public:
    enum { MAX_QUEUE = 5 };

    RadioVoxController();
    ~RadioVoxController();

    bool  Add(RadioVox* vox);
    void  Update();
    DWORD UpdateThread();

    bool           shutdown;
    HANDLE         hthread;
    List<RadioVox> queue;
    ThreadSync     sync;
};

static RadioVoxController* controller = 0;

// +--------------------------------------------------------------------+

RadioVoxController::RadioVoxController()
: hthread(0), shutdown(false)
{
    DWORD thread_id = 0;
    hthread = CreateThread(0, 4096, VoxUpdateProc,
    (LPVOID) this, 0, &thread_id);
}

// +--------------------------------------------------------------------+

RadioVoxController::~RadioVoxController()
{
    shutdown = true;

    WaitForSingleObject(hthread, 500);
    CloseHandle(hthread);
    hthread = 0;

    queue.destroy();
}

// +--------------------------------------------------------------------+

DWORD WINAPI VoxUpdateProc(LPVOID link)
{
    RadioVoxController* controller = (RadioVoxController*) link;

    if (controller)
    return controller->UpdateThread();

    return (DWORD) E_POINTER;
}

// +--------------------------------------------------------------------+

DWORD
RadioVoxController::UpdateThread()
{
    while (!shutdown) {
        Update();
        Sleep(50);
    }

    return 0;
}

// +--------------------------------------------------------------------+

void
RadioVoxController::Update()
{
    AutoThreadSync a(sync);

    if (queue.size()) {
        RadioVox* vox = queue.first();

        if (!vox->Update())
        delete queue.removeIndex(0);
    }
}

bool
RadioVoxController::Add(RadioVox* vox)
{
    if (!vox || vox->sounds.isEmpty())
    return false;

    AutoThreadSync a(sync);

    if (queue.size() < MAX_QUEUE) {
        queue.append(vox);
        return true;
    }

    return false;
}

// +====================================================================+
//
// RADIO VOX MESSAGE:
//

void
RadioVox::Initialize()
{
    if (!controller) {
        controller = new(__FILE__,__LINE__) RadioVoxController;
    }
}

void
RadioVox::Close()
{
    delete controller;
    controller = 0;
}

// +--------------------------------------------------------------------+

RadioVox::RadioVox(int n, const char* p, const char* m)
: path(p), message(m), index(0), channel(n)
{
}

RadioVox::~RadioVox()
{
    sounds.destroy();
}

// +--------------------------------------------------------------------+

bool
RadioVox::AddPhrase(const char* key)
{
    if (AudioConfig::VoxVolume() <= AudioConfig::Silence())
    return false;

    DataLoader* loader = DataLoader::GetLoader();
    if (!loader)
    return false;

    if (key && *key) {
        char datapath[256];
        char filename[256];

        sprintf_s(datapath, "Vox/%s/", path.data());
        sprintf_s(filename, "%s.wav",   key);

        bool        use_fs = loader->IsFileSystemEnabled();
        Sound*      sound  = 0;

        loader->UseFileSystem(true);
        loader->SetDataPath(datapath);
        loader->LoadSound(filename, sound, Sound::LOCALIZED, true); // optional sound
        loader->SetDataPath(0);
        loader->UseFileSystem(use_fs);

        if (sound) {
            sound->SetVolume(AudioConfig::VoxVolume());
            sound->SetFlags(Sound::LOCALIZED | Sound::LOCKED);
            sound->SetFilename(filename);
            sounds.append(sound);

            return true;
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

bool
RadioVox::Start()
{
    if (controller)
    return controller->Add(this);

    return false;
}

bool
RadioVox::Update()
{
    if (message.length()) {
        RadioView::Message(message);
        message = "";
    }

    bool active = false;

    while (!active && index < sounds.size()) {
        Sound* s = sounds[index];

        if (s->IsReady()) {
            if (channel & 1)
            s->SetPan(channel * -3000);
            else
            s->SetPan(channel *  3000);

            s->Play();
            active = true;
        }

        else if (s->IsPlaying()) {
            s->Update();
            active = true;
        }

        else {
            index++;
        }
    }

    return active;
}
