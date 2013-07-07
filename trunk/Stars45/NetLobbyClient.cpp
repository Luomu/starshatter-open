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
    FILE:         NetLobbyClient.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Stream-oriented network client class
*/


#include "MemDebug.h"
#include "NetLobbyClient.h"
#include "NetClientConfig.h"
#include "NetAuth.h"
#include "NetChat.h"
#include "Campaign.h"
#include "Player.h"
#include "Starshatter.h"
#include "ModInfo.h"

#include "NetHost.h"
#include "NetPeer.h"
#include "NetLink.h"
#include "NetMsg.h"
#include "NetLayer.h"
#include "FormatUtil.h"

extern const char* versionInfo;

// +-------------------------------------------------------------------+

NetLobbyClient::NetLobbyClient()
: NetLobby(false), server_id(0), host(false), exit_code(0), temporary(false)
{
    NetHost me;
    Text    server_name;
    WORD    port = 11101;

    ping_req_time = 0;
    chat_req_time = 0;
    user_req_time = 0;
    camp_req_time = 0;
    unit_req_time = 0;
    mods_req_time = 0;

    NetClientConfig* ncc = NetClientConfig::GetInstance();
    if (ncc) {
        NetServerInfo* info = ncc->GetSelectedServer();

        if (info) {
            server_name = info->hostname;
            addr        = info->addr;
            port        = info->port;
            gamepass    = info->password;
        }
    }

    if (server_name.length() && port > 0) {
        Print("  '%s' is a client of '%s'\n", me.Name(), server_name.data());
        link = new(__FILE__,__LINE__) NetLink;
        server_id = link->AddPeer(NetAddr(server_name, port));
    }
    else if (port == 0) {
        Print("  '%s' invalid lobby port number %d\n", me.Name(), port);
    }
    else {
        Print("  '%s' is a client without a server\n", me.Name());
    }
}

NetLobbyClient::NetLobbyClient(const NetAddr& server_addr)
: NetLobby(true), server_id(0), addr(server_addr), host(false), exit_code(0),
temporary(true)
{
    ping_req_time = 0;
    chat_req_time = 0;
    user_req_time = 0;
    camp_req_time = 0;
    unit_req_time = 0;
    mods_req_time = 0;

    if (addr.IPAddr() != 0) {
        link = new(__FILE__,__LINE__) NetLink;
        server_id = link->AddPeer(addr);
    }
}

NetLobbyClient::~NetLobbyClient()
{
    missions.destroy();
}

// +--------------------------------------------------------------------+

void
NetLobbyClient::SendData(int type, Text msg)
{
    if (link && server_id && type > 0 && type < 255) {
        if (msg.length())
        link->SendMessage(server_id, (BYTE) type, msg.data(), msg.length(), NetMsg::RELIABLE);
        else
        link->SendMessage(server_id, (BYTE) type, 0, 0, NetMsg::RELIABLE);
    }
}

// +--------------------------------------------------------------------+

void
NetLobbyClient::ExecFrame()
{
    NetLobby::ExecFrame();

    if (!temporary) {
        // check health of server:
        NetPeer* s = link->FindPeer(server_id);
        if (s && (NetLayer::GetUTC() - s->LastReceiveTime() > NET_DISCONNECT_TIME)) {
            exit_code = 2;
        }

        // send keep alive ping:
        if ((NetLayer::GetUTC() - ping_req_time > NET_DISCONNECT_TIME/3)) {
            SendData(NET_LOBBY_SERVER_INFO, Text());
            ping_req_time = NetLayer::GetUTC();
        }
    }
}

// +--------------------------------------------------------------------+

bool
NetLobbyClient::Login(bool host_req)
{
    Player* player = Player::GetCurrentPlayer();
    if (!player)
    return false;

    host = host_req;

    Text login = "name \"";
    login += SafeQuotes(player->Name());
    login += "\" pass \"";
    login += SafeQuotes(player->Password());
    login += "\" version \"";
    login += versionInfo;
    login += "\" ";

    char buffer[256];

    sprintf_s(buffer, "host %s rank %d time %d miss %d kill %d loss %d ", 
    host ? "true" : "false",
    player->Rank(),
    player->FlightTime(),
    player->Missions(),
    player->Kills(),
    player->Losses());

    login += buffer;

    login += "sig \"";
    login += SafeQuotes(player->Signature());
    login += "\" squad \"";
    login += SafeQuotes(player->Squadron());
    login += "\" ";

    if (gamepass.length() > 0) {
        login += "gamepass \"";
        login += SafeQuotes(gamepass);
        login += "\" ";
    }

    SendData(NET_LOBBY_LOGIN, login);
    ExecFrame();
    return true;
}

bool
NetLobbyClient::Logout()
{
    if (host)
    GameStop();

    SendData(NET_LOBBY_LOGOUT, Text());
    Sleep(250);
    ExecFrame();
    return true;
}

// +--------------------------------------------------------------------+

bool
NetLobbyClient::Ping()
{
    Text no_data;

    SendData(NET_LOBBY_PING, no_data);
    Sleep(100);

    SendData(NET_LOBBY_PING, no_data);
    Sleep(100);

    SendData(NET_LOBBY_PING, no_data);
    Sleep(100);

    SendData(NET_LOBBY_SERVER_INFO, no_data);
    Sleep(700);
    ExecFrame();

    return (server_info.status > NetServerInfo::OFFLINE);
}

void
NetLobbyClient::GameStart()
{
    SendData(NET_LOBBY_GAME_START, Text());
    Sleep(100);

    SetStatus(NetServerInfo::ACTIVE);
    Starshatter::GetInstance()->SetGameMode(Starshatter::PREP_MODE);

    // discard unit map selection data so that
    // it will be refreshed when we return to
    // the lobby after the mission:

    ClearUnitMap();
}

void
NetLobbyClient::GameStop()
{
    SendData(NET_LOBBY_GAME_STOP, Text());
    ExecFrame();
    Sleep(100);

    SetStatus(NetServerInfo::LOBBY);
}

// +--------------------------------------------------------------------+

const Text&
NetLobbyClient::GetMachineInfo()
{
    if (server_info.status > NetServerInfo::OFFLINE)
    return server_info.machine_info;

    return NetLobby::GetMachineInfo();
}

int
NetLobbyClient::GetStatus() const
{
    if (server_info.status > NetServerInfo::OFFLINE)
    return server_info.status;

    return NetLobby::GetStatus();
}

int
NetLobbyClient::NumUsers()
{
    if (server_info.status > NetServerInfo::OFFLINE)
    return server_info.nplayers;

    return NetLobby::NumUsers();
}

bool
NetLobbyClient::HasHost()
{
    if (server_info.status > NetServerInfo::OFFLINE)
    return server_info.hosted ? true : false;

    return NetLobby::HasHost();
}

WORD
NetLobbyClient::GetGamePort()
{
    if (server_info.status > NetServerInfo::OFFLINE)
    return server_info.gameport;

    return NetLobby::GetGamePort();
}

// +-------------------------------------------------------------------+

void
NetLobbyClient::AddChat(NetUser* user, const char* msg, bool route)
{
    if (!msg || !*msg) return;

    char buffer[280];
    sprintf_s(buffer, "msg \"%s\"", SafeQuotes(msg));

    SendData(NET_LOBBY_CHAT, buffer);
    ExecFrame();
}

List<NetChatEntry>&
NetLobbyClient::GetChat()
{
    if (chat_log.size() < 1 && (NetLayer::GetUTC() - chat_req_time > 3)) {
        SendData(NET_LOBBY_CHAT, Text());
        chat_req_time = NetLayer::GetUTC();
    }

    return chat_log;
}

List<NetUser>&
NetLobbyClient::GetUsers()
{
    if (users.size() < 1 && (NetLayer::GetUTC() - user_req_time > 2)) {
        SendData(NET_LOBBY_USER_LIST, Text());
        user_req_time = NetLayer::GetUTC();
    }

    return users;
}

List<ModInfo>&
NetLobbyClient::GetServerMods()
{
    if (server_mods.size() < 1 && (NetLayer::GetUTC() - mods_req_time > 2)) {
        SendData(NET_LOBBY_SERVER_MODS, Text());
        mods_req_time = NetLayer::GetUTC();
    }

    return server_mods;
}

List<NetUnitEntry>&
NetLobbyClient::GetUnitMap()
{
    bool request = selected_mission && 
    unit_map.size() < 1 && 
    (NetLayer::GetUTC() - unit_req_time > 2);

    if (selected_mission && GetStatus() == NetServerInfo::ACTIVE && (NetLayer::GetUTC() - unit_req_time > 5))
    request = true;

    if (request) {
        SendData(NET_LOBBY_UNIT_LIST, Text());
        unit_req_time = NetLayer::GetUTC();
    }

    return unit_map;
}

// +--------------------------------------------------------------------+

List<NetCampaignInfo>&
NetLobbyClient::GetCampaigns()
{
    if (campaigns.size() < 1 && (NetLayer::GetUTC() - camp_req_time > 3)) {
        SendData(NET_LOBBY_MISSION_LIST, Text());
        camp_req_time = NetLayer::GetUTC();
    }

    return campaigns;
}

// +--------------------------------------------------------------------+

void
NetLobbyClient::BanUser(NetUser* user)
{
    char buffer[512];
    sprintf_s(buffer, "user \"%s\"", SafeQuotes(user->Name()));
    SendData(NET_LOBBY_BAN_USER, buffer);
}

// +--------------------------------------------------------------------+

void
NetLobbyClient::SelectMission(DWORD id)
{
    char buffer[32];
    sprintf_s(buffer, "m_id 0x%08x", id);
    SendData(NET_LOBBY_MISSION_SELECT, buffer);
}

void
NetLobbyClient::MapUnit(int n, const char* user, bool lock)
{
    if (user && strlen(user) > 250)
    return;

    char buffer[512];
    sprintf_s(buffer, "id %d user \"%s\" lock %s",
    n, SafeQuotes(user), lock ? "true" : "false");
    SendData(NET_LOBBY_MAP_UNIT, buffer);
}

Mission*
NetLobbyClient::GetSelectedMission()
{
    mission = 0;

    // ask server for mission:
    SendData(NET_LOBBY_MISSION_DATA, Text());

    // wait for answer:
    int i = 150;
    while (i-- > 0 && !mission) {
        Sleep(100);
        ExecFrame();
    }

    return mission;
}

// +--------------------------------------------------------------------+

void
NetLobbyClient::DoAuthUser(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    int  level = NetAuth::NET_AUTH_STANDARD;
    Text salt;

    for (int i = 0; i < params.size(); i++) {
        NetLobbyParam* p = params[i];

        int num = 0;
        sscanf_s(p->value, "%d", &num);

        if (p->name == "level") {
            level = num;
        }

        else if (p->name == "salt") {
            salt = p->value;
        }
    }

    Text response = NetAuth::CreateAuthResponse(level, salt);
    if (response.length() > 0)
    SendData(NET_LOBBY_USER_AUTH, response);
}

void
NetLobbyClient::DoServerInfo(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    for (int i = 0; i < params.size(); i++) {
        NetLobbyParam* p = params[i];

        int num = 0;
        sscanf_s(p->value, "%d", &num);

        if (p->name == "info") {
            server_info.machine_info = p->value;
        }

        else if (p->name == "version") {
            server_info.version = p->value;
        }

        else if (p->name == "mode") {
            server_info.status = num;
        }

        else if (p->name == "users") {
            server_info.nplayers = num;
        }

        else if (p->name == "host") {
            server_info.hosted = (p->value == "true");
        }

        else if (p->name == "port") {
            server_info.gameport = (WORD) num;
        }
    }

    params.destroy();
}

void
NetLobbyClient::DoChat(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    int  id = 0;
    Text user_name;
    Text chat_msg;

    for (int i = 0; i < params.size(); i++) {
        NetLobbyParam* p = params[i];

        int num = 0;
        sscanf_s(p->value, "%d", &num);

        if (p->name == "id")
        id = num;

        else if (p->name == "user")
        user_name = p->value;

        else if (p->name == "msg")
        chat_msg = p->value;
    }

    params.destroy();

    // receive chat from server:
    if (id && chat_msg.length()) {
        NetChatEntry* entry = new(__FILE__,__LINE__) NetChatEntry(id, user_name, chat_msg);

        if (!chat_log.contains(entry))
        chat_log.insertSort(entry);
        else
        delete entry; // received duplicate
    }
}

void
NetLobbyClient::DoServerMods(NetPeer* peer, Text msg)
{
    mods_req_time = NetLayer::GetUTC() + 3600;

    List<NetLobbyParam> params;
    ParseMsg(msg, params);
    server_mods.destroy();

    Text  name;
    Text  version;
    Text  url;

    for (int i = 0; i < params.size(); i++) {
        NetLobbyParam* p = params[i];

        if (p->name == "mod") {
            name = p->value;
        }

        else if (p->name == "url") {
            url = p->value;
        }

        else if (p->name == "ver") {
            version = p->value;

            ModInfo* info = new(__FILE__,__LINE__) ModInfo(name, version, url);
            server_mods.append(info);
        }
    }

    params.destroy();
}

void
NetLobbyClient::DoUserList(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    users.destroy();

    Text  user_name;
    Text  host_flag;
    Text  signature;
    Text  squadron;
    int   rank           = 0;
    int   flight_time    = 0;
    int   mission_count  = 0;
    int   kills          = 0;
    int   losses         = 0;

    for (int i = 0; i < params.size(); i++) {
        NetLobbyParam* p = params[i];

        int num = 0;
        sscanf_s(p->value, "%d", &num);

        if (p->name == "name")
        user_name = p->value;

        else if (p->name == "sig")
        signature = p->value;

        else if (p->name == "squad")
        squadron = p->value;

        else if (p->name == "rank")
        rank = num;

        else if (p->name == "time")
        flight_time = num;

        else if (p->name == "miss")
        mission_count = num;

        else if (p->name == "kill")
        kills = num;

        else if (p->name == "loss")
        losses = num;

        else if (p->name == "host") {
            host_flag = p->value;

            NetUser* u = new(__FILE__,__LINE__) NetUser(user_name);
            u->SetHost((host_flag == "true") ? true : false);
            u->SetSignature(signature);
            u->SetSquadron(squadron);
            u->SetRank(rank);
            u->SetFlightTime(flight_time);
            u->SetMissions(mission_count);
            u->SetKills(kills);
            u->SetLosses(losses);

            AddUser(u);
        }
    }

    params.destroy();
}

void
NetLobbyClient::DoMissionList(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    if (params.size() > 2) {
        campaigns.destroy();

        NetCampaignInfo*  c = 0;
        MissionInfo*      m = 0;

        for (int i = 0; i < params.size(); i++) {
            NetLobbyParam* p = params[i];

            if (p->name == "c_id") {
                c = new(__FILE__,__LINE__) NetCampaignInfo;
                sscanf_s(p->value, "0x%x", &c->id);
                campaigns.append(c);

                m = 0;
            }

            else if (c && p->name == "c_name") {
                c->name = p->value;
            }

            else if (p->name == "m_id") {
                int id = 0;
                sscanf_s(p->value, "0x%x", &id);

                int m_id = id &  NET_MISSION_MASK;
                int c_id = id >> NET_CAMPAIGN_SHIFT;

                for (int i = 0; i < campaigns.size(); i++) {
                    NetCampaignInfo* c = campaigns[i];
                    if (c->id == c_id) {
                        m = new(__FILE__,__LINE__) MissionInfo;
                        m->id = m_id;
                        c->missions.append(m);
                        missions.append(m);  // for later garbage collection
                        break;
                    }
                }
            }

            else if (m && p->name == "m_name") {
                m->name = p->value;
            }

            else if (m && p->name == "m_desc") {
                m->description = p->value;
            }
        }
    }

    params.destroy();
}

void
NetLobbyClient::DoMissionSelect(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    for (int i = 0; i < params.size(); i++) {
        NetLobbyParam* p = params[i];

        int num = 0;
        sscanf_s(p->value, "0x%x", &num);

        if (p->name == "m_id") {
            if (selected_mission != (DWORD) num) {
                selected_mission = num;
                ClearUnitMap();
            }
        }
    }

    params.destroy();
}

void
NetLobbyClient::DoMissionData(NetPeer* peer, Text msg)
{
    Campaign* c = Campaign::SelectCampaign("Multiplayer Missions");

    if (c) {
        c->LoadNetMission(99999, msg.data());
        mission = c->GetMission(99999);
    }

    if (msg.length()) {
        FILE* f;
        ::fopen_s(&f, "multi_mission_recv.def", "w");
        if (f) {
            ::fwrite(msg.data(), msg.length(), 1, f);
            ::fclose(f);
        }
    }
}

void
NetLobbyClient::DoUnitList(NetPeer* peer, Text msg)
{
    List<NetLobbyParam> params;
    ParseMsg(msg, params);

    if (params.size() > 2) {
        ClearUnitMap();

        Text elem_name;
        Text design;
        Text user_name;
        int  iff;
        int  index;
        int  lives = 1;
        int  hull  = 100;
        int  role  = 0;
        int  lock  = 0;

        for (int i = 0; i < params.size(); i++) {
            NetLobbyParam* p = params[i];

            if (p->name == "name") {
                elem_name = p->value;
            }
            else if (p->name == "design") {
                design = p->value;
            }
            else if (p->name == "user") {
                user_name = p->value;
            }
            else if (p->name == "index") {
                sscanf_s(p->value, "%d", &index);
            }
            else if (p->name == "iff") {
                sscanf_s(p->value, "%d", &iff);
            }
            else if (p->name == "lives") {
                sscanf_s(p->value, "%d", &lives);
            }
            else if (p->name == "hull") {
                sscanf_s(p->value, "%d", &hull);
            }
            else if (p->name == "role") {
                sscanf_s(p->value, "%d", &role);
            }
            else if (p->name == "lock") {
                sscanf_s(p->value, "%d", &lock);

                NetUnitEntry* entry = new(__FILE__,__LINE__) NetUnitEntry(elem_name, design, iff, index);
                entry->SetUserName(user_name);
                entry->SetLives(lives);
                entry->SetIntegrity(hull);
                entry->SetMissionRole(role);
                entry->SetLock(lock ? true : false);

                unit_map.append(entry);
            }
        }
    }

    params.destroy();
}

void
NetLobbyClient::DoMapUnit(NetPeer* peer, Text msg)
{
}

void
NetLobbyClient::DoGameStart(NetPeer* peer, Text msg)
{
}

void
NetLobbyClient::DoExit(NetPeer* peer, Text msg)
{
    exit_code = 1;
}
