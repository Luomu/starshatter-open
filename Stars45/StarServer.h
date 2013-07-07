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

    SUBSYSTEM:    Stars
    FILE:         StarServer.h
    AUTHOR:       John DiCamillo

*/

#ifndef StarServer_h
#define StarServer_h

#include "Types.h"
#include "Game.h"
#include "Bitmap.h"
#include "KeyMap.h"

// +--------------------------------------------------------------------+

class Campaign;
class Ship;
class Sim;
class FadeView;
class CameraDirector;
class MultiController;
class MouseController;
class DataLoader;

class NetServer;
class NetLobbyServer;

// +--------------------------------------------------------------------+

class StarServer : public Game
{
public:
    StarServer();
    virtual ~StarServer();

    virtual bool      Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow);
    virtual bool      InitGame();
    virtual void      GameState();
    virtual bool      OnPaint();

    enum MODE {       MENU_MODE,  // main menu
        LOAD_MODE,  // loading mission into simulator
        PLAY_MODE   // active simulation
    };

    int       GetGameMode()       { return game_mode; }
    void      SetGameMode(int mode);
    void      SetNextMission(const char* script);

    void      CreateWorld();
    void      Shutdown(bool restart=false);

    static StarServer*   GetInstance()     { return instance; }


protected:
    virtual bool      GameLoop();
    virtual void      UpdateWorld();
    virtual void      InstantiateMission();

    static StarServer*      instance;
    NetServer*              admin_server;
    NetLobbyServer*         lobby_server;
    DataLoader*             loader;

    int                     game_mode;
    DWORD                   time_mark;
    DWORD                   minutes;
};

#endif StarServer_h
