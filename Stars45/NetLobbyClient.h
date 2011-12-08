/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetLobbyClient.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    UDP-oriented network lobby client class
*/


#ifndef NetLobbyClient_h
#define NetLobbyClient_h

#include "NetLobby.h"
#include "NetClientConfig.h"

// +-------------------------------------------------------------------+

class NetLobbyClient : public NetLobby
{
public:
   NetLobbyClient();
   NetLobbyClient(const NetAddr& server_addr);
   virtual ~NetLobbyClient();

   int operator == (const NetLobbyClient& c)   const { return this == &c; }

   virtual void   ExecFrame();
   virtual bool   Login(bool host=false);
   virtual bool   Logout();
   virtual int    GetLastError()  const { return exit_code; }

   // actions:
   virtual bool   Ping();
   virtual void   GameStart();
   virtual void   GameStop();

   virtual void   BanUser(NetUser* user);

   virtual void   AddChat(NetUser* user, const char* msg, bool route=true);
   virtual List<NetChatEntry>&
                  GetChat();

   NetAddr        GetServerAddr() const { return addr; }
   virtual bool   IsHost()        const { return host; }
   virtual bool   IsClient()      const { return true; }

   virtual List<NetUser>&           GetUsers();
   virtual List<NetCampaignInfo>&   GetCampaigns();
   virtual List<NetUnitEntry>&      GetUnitMap();
   virtual void                     MapUnit(int n, const char* user, bool lock=false);
   virtual void                     SelectMission(DWORD id);
   virtual Mission*                 GetSelectedMission();

   virtual List<ModInfo>&           GetServerMods();

   // overrides for ping support:
   virtual const Text&              GetMachineInfo();
   virtual int                      GetStatus()     const;
   virtual int                      NumUsers();
   virtual bool                     HasHost();
   virtual WORD                     GetGamePort();

protected:
   virtual void   SendData(int type, Text msg);
   virtual void   DoServerInfo(NetPeer* peer, Text msg);
   virtual void   DoServerMods(NetPeer* peer, Text msg);
   virtual void   DoAuthUser(NetPeer* peer, Text msg);
   virtual void   DoChat(NetPeer* peer, Text msg);
   virtual void   DoUserList(NetPeer* peer, Text msg);
   virtual void   DoMissionList(NetPeer* peer, Text msg);
   virtual void   DoMissionSelect(NetPeer* peer, Text msg);
   virtual void   DoMissionData(NetPeer* peer, Text msg);
   virtual void   DoUnitList(NetPeer* peer, Text msg);
   virtual void   DoMapUnit(NetPeer* peer, Text msg);
   virtual void   DoGameStart(NetPeer* peer, Text msg);
   virtual void   DoExit(NetPeer* peer, Text msg);

   DWORD             server_id;
   NetAddr           addr;
   bool              host;
   Text              gamepass;
   int               exit_code;

   NetServerInfo     server_info;
   List<MissionInfo> missions;

   bool              temporary;
   DWORD             ping_req_time;
   DWORD             chat_req_time;
   DWORD             user_req_time;
   DWORD             camp_req_time;
   DWORD             unit_req_time;
   DWORD             mods_req_time;
};

// +-------------------------------------------------------------------+

#endif NetLobbyClient_h