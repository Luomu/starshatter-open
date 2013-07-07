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