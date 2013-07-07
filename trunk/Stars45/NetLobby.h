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
    FILE:         NetLobby.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Base Class for Multiplayer Game Lobby classes
*/

#ifndef NetLobby_h
#define NetLobby_h

#include "Types.h"
#include "NetLink.h"
#include "List.h"

#define NET_CAMPAIGN_SHIFT    12
#define NET_MISSION_MASK      0xfff
#define NET_DISCONNECT_TIME   30

// +-------------------------------------------------------------------+

class Campaign;
class Mission;
class MissionElement;
class MissionInfo;
class NetCampaignInfo;
class NetChatEntry;
class NetUser;
class NetUnitEntry;
class NetLobbyParam;
class ModInfo;

// +--------------------------------------------------------------------+

class NetLobby
{
public:
    static const char* TYPENAME() { return "NetLobby"; }

    NetLobby(bool temporary = false);
    virtual ~NetLobby();

    virtual bool            IsClient()     const { return false; }
    virtual bool            IsServer()     const { return false; }
    virtual bool            IsActive()     const { return active; }

    virtual void            ExecFrame();
    virtual void            Recv();
    virtual void            Send();
    virtual int             GetLastError() const { return 0; }

    virtual NetUser*        FindUserByAddr(const NetAddr& addr);
    virtual NetUser*        FindUserByName(const char* name);
    virtual NetUser*        FindUserByNetID(DWORD id);

    virtual void            BanUser(NetUser* user);
    virtual void            AddUser(NetUser* user);
    virtual void            DelUser(NetUser* user);
    virtual bool            SetUserHost(NetUser* user, bool host);
    virtual int             NumUsers();
    virtual NetUser*        GetHost();
    virtual bool            HasHost();
    virtual bool            IsHost()       const { return false; }
    virtual List<NetUser>&  GetUsers();

    virtual List<ModInfo>&  GetServerMods();

    virtual NetUser*        GetLocalUser();
    virtual void            SetLocalUser(NetUser* user);

    virtual int             GetStatus()    const { return status; }
    virtual void            SetStatus(int s)     { status = s;    }

    virtual void            AddChat(NetUser* user, const char* msg, bool route=true);
    virtual List<NetChatEntry>&
    GetChat();
    virtual void            ClearChat();
    virtual void            SaveChat()           { }
    virtual DWORD           GetStartTime() const { return start_time; }

    virtual List<NetCampaignInfo>&
    GetCampaigns();

    virtual void            AddUnitMap(MissionElement* elem, int index=0);
    virtual List<NetUnitEntry>&
    GetUnitMap();
    virtual void            ClearUnitMap();
    virtual void            MapUnit(int n, const char* user, bool lock=false);
    virtual void            UnmapUnit(const char* user);
    virtual bool            IsMapped(const char* user);

    virtual Mission*        GetSelectedMission()         { return mission;          }
    virtual DWORD           GetSelectedMissionID() const { return selected_mission; }
    virtual void            SelectMission(DWORD id);

    virtual const Text&     GetMachineInfo()     { return machine_info;  }
    virtual WORD            GetGamePort()        { return 0;             }

    // actions:
    virtual bool            Ping();
    virtual void            GameStart()          { }
    virtual void            GameStop()           { }
    virtual DWORD           GetLag();

    // instance management:
    static NetLobby*        GetInstance();
    static bool             IsNetLobbyClient();
    static bool             IsNetLobbyServer();

protected:
    virtual void            DoPing(NetPeer* peer, Text msg)           { }
    virtual void            DoServerInfo(NetPeer* peer, Text msg)     { }
    virtual void            DoServerMods(NetPeer* peer, Text msg)     { }
    virtual void            DoLogin(NetPeer* peer, Text msg)          { }
    virtual void            DoLogout(NetPeer* peer, Text msg)         { }
    virtual void            DoAuthUser(NetPeer* peer, Text msg)       { }
    virtual void            DoUserAuth(NetPeer* peer, Text msg)       { }
    virtual void            DoChat(NetPeer* peer, Text msg)           { }
    virtual void            DoUserList(NetPeer* peer, Text msg)       { }
    virtual void            DoBanUser(NetPeer* peer, Text msg)        { }
    virtual void            DoMissionList(NetPeer* peer, Text msg)    { }
    virtual void            DoMissionSelect(NetPeer* peer, Text msg)  { }
    virtual void            DoMissionData(NetPeer* peer, Text msg)    { }
    virtual void            DoUnitList(NetPeer* peer, Text msg)       { }
    virtual void            DoMapUnit(NetPeer* peer, Text msg)        { }
    virtual void            DoGameStart(NetPeer* peer, Text msg)      { }
    virtual void            DoGameStop(NetPeer* peer, Text msg)       { }
    virtual void            DoExit(NetPeer* peer, Text msg)           { }

    virtual void            ParseMsg(Text msg, List<NetLobbyParam>& params);

    NetLink*                link;
    NetUser*                local_user;
    List<NetUser>           users;
    List<NetChatEntry>      chat_log;
    List<NetCampaignInfo>   campaigns;
    List<NetUnitEntry>      unit_map;
    Text                    machine_info;
    List<ModInfo>           server_mods;

    bool                    active;
    DWORD                   last_send_time;
    DWORD                   start_time;
    DWORD                   selected_mission;
    Mission*                mission;
    int                     status;
};

// +-------------------------------------------------------------------+

class NetLobbyParam
{
public:
    static const char* TYPENAME() { return "NetLobbyParam"; }

    NetLobbyParam(const char* n, const char* v) : name(n), value(v) { }

    int operator == (const NetLobbyParam& p) const { return name == p.name; }

    Text  name;
    Text  value;
};

// +-------------------------------------------------------------------+

class NetUnitEntry
{
public:
    static const char* TYPENAME() { return "NetUnitEntry"; }

    NetUnitEntry(MissionElement* elem, int index=0);
    NetUnitEntry(const char* elem, const char* design, int iff, int index=0);
    ~NetUnitEntry();

    int operator == (const NetUnitEntry& e) const { return (elem == e.elem) && (index == e.index); }

    Text              GetDescription()        const;
    const Text&       GetUserName()           const { return user;    }
    const Text&       GetElemName()           const { return elem;    }
    const Text&       GetDesign()             const { return design;  }
    int               GetIFF()                const { return iff;     }
    int               GetIndex()              const { return index;   }
    int               GetLives()              const { return lives;   }
    int               GetIntegrity()          const { return hull;    }
    int               GetMissionRole()        const { return role;    }
    bool              GetLocked()             const { return lock;    }

    void              SetUserName(const char* u)    { user = u;       }
    void              SetElemName(const char* e)    { elem = e;       }
    void              SetDesign(const char* d)      { design = d;     }
    void              SetIFF(int i)                 { iff = i;        }
    void              SetIndex(int i)               { index = i;      }
    void              SetLives(int i)               { lives = i;      }
    void              SetIntegrity(int i)           { hull = i;       }
    void              SetMissionRole(int i)         { role = i;       }
    void              SetLock(bool l)               { lock = l;       }

private:
    Text  user;
    Text  elem;
    Text  design;
    int   iff;
    int   index;
    int   lives;
    int   hull;
    int   role;
    bool  lock;
};

// +--------------------------------------------------------------------+

class NetServerInfo
{
public:
    static const char* TYPENAME() { return "NetServerInfo"; }

    enum STATUS { OFFLINE, LOBBY, BRIEFING, ACTIVE, DEBRIEFING, PERSISTENT };

    NetServerInfo();

    Text        name;
    Text        hostname;
    Text        password;
    Text        type;
    NetAddr     addr;
    WORD        port;
    WORD        gameport;
    bool        save;

    Text        version;
    Text        machine_info;
    int         nplayers;
    int         hosted;
    int         status;

    DWORD       ping_time;
};

// +--------------------------------------------------------------------+

class NetCampaignInfo
{
public:
    static const char* TYPENAME() { return "NetCampaignInfo"; }

    NetCampaignInfo() : id(0) { }
    ~NetCampaignInfo()        { }

    int               id;
    Text              name;

    List<MissionInfo> missions;
};

// +--------------------------------------------------------------------+

enum NET_LOBBY_MESSAGES {
    NET_LOBBY_PING        = 0x10,
    NET_LOBBY_SERVER_INFO,
    NET_LOBBY_SERVER_MODS,
    NET_LOBBY_LOGIN,
    NET_LOBBY_LOGOUT,
    NET_LOBBY_CHAT,
    NET_LOBBY_USER_LIST,
    NET_LOBBY_BAN_USER,
    NET_LOBBY_MISSION_LIST,
    NET_LOBBY_MISSION_SELECT,
    NET_LOBBY_MISSION_DATA,
    NET_LOBBY_UNIT_LIST,
    NET_LOBBY_MAP_UNIT,

    NET_LOBBY_AUTH_USER,
    NET_LOBBY_USER_AUTH,

    NET_LOBBY_GAME_START,
    NET_LOBBY_GAME_STOP,
    NET_LOBBY_EXIT
};

// +--------------------------------------------------------------------+

#endif NetLobby_h

