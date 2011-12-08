/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

      sprintf(datapath, "Vox/%s/", path.data());
      sprintf(filename, "%s.wav",   key);

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
