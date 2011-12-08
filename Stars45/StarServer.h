/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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
