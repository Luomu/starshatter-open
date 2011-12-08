/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetLobbyServer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    UDP Server Engine for Multiplayer Lobby
*/


#ifndef NetLobbyServer_h
#define NetLobbyServer_h

// FOR NETWORK MENU TESTING (extra latency in msec):
// #define EXTRA_LATENCY 300

#include "NetLobby.h"

// +-------------------------------------------------------------------+

class Mission;
class MissionElement;
class NetChatEntry;
class NetServerConfig;
class NetUser;
class NetUnitEntry;

// +-------------------------------------------------------------------+

class NetLobbyServer : public NetLobby
{
public:
   NetLobbyServer();
   virtual ~NetLobbyServer();

   int operator == (const NetLobbyServer& s) const { return this == &s; }

   virtual void            ExecFrame();
   virtual bool            IsHost()        const { return true; }
   virtual bool            IsServer()      const { return true; }

   virtual void            BanUser(NetUser* user);
   virtual void            AddUser(NetUser* user);
   virtual void            DelUser(NetUser* user);
   virtual void            SendUsers();
   virtual void            RequestAuth(NetUser* user);

   virtual void            AddChat(NetUser* user, const char* msg, bool route=true);
   virtual void            ClearChat();
   virtual void            SaveChat();

   virtual List<NetUnitEntry>&
                           GetUnitMap();
   virtual void            MapUnit(int n, const char* user, bool lock=false);
   virtual void            UnmapUnit(const char* user);
   virtual void            SendUnits();

   virtual void            SelectMission(DWORD id);
   virtual Text            Serialize(Mission* m, NetUser* u=0);
   virtual Mission*        GetSelectedMission();

   virtual Text            GetServerName()         const { return server_name;      }
   virtual void            SetServerName(const char* s)  { server_name = s;         }
   virtual Text            GetServerMission()      const { return server_mission;   }
   virtual void            SetServerMission(const char* script)
                                                         { server_mission = script; }

   virtual void            GameStart();
   virtual void            GameStop();

   virtual void            GameOn();
   virtual void            GameOff();

   // singleton locator:
   static NetLobbyServer*  GetInstance();

protected:
   virtual void            CheckSessions();

   virtual void            SendData(NetUser* dst, int type, Text msg);
   virtual void            DoPing(NetPeer* peer, Text msg);
   virtual void            DoServerInfo(NetPeer* peer, Text msg);
   virtual void            DoServerMods(NetPeer* peer, Text msg);
   virtual void            DoLogin(NetPeer* peer, Text msg);
   virtual void            DoLogout(NetPeer* peer, Text msg);
   virtual void            DoUserAuth(NetPeer* peer, Text msg);
   virtual void            DoChat(NetPeer* peer, Text msg);
   virtual void            DoUserList(NetPeer* peer, Text msg);
   virtual void            DoBanUser(NetPeer* peer, Text msg);
   virtual void            DoMissionList(NetPeer* peer, Text msg);
   virtual void            DoMissionSelect(NetPeer* peer, Text msg);
   virtual void            DoMissionData(NetPeer* peer, Text msg);
   virtual void            DoUnitList(NetPeer* peer, Text msg);
   virtual void            DoMapUnit(NetPeer* peer, Text msg);
   virtual void            DoGameStart(NetPeer* peer, Text msg);
   virtual void            DoGameStop(NetPeer* peer, Text msg);

   virtual void            LoadMOTD();
   virtual void            SendMOTD(NetUser* user);
   virtual void            SendMods(NetUser* user);

   Text                    server_name;
   NetAddr                 server_addr;
   DWORD                   announce_time;
   NetServerConfig*        server_config;
   Text                    server_mission;
   int                     motd_index;

   List<Text>              motd;
};

// +-------------------------------------------------------------------+

#endif NetLobbyServer_h