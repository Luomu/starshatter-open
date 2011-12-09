/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetLobby.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Base Class for Multiplayer Game Lobby classes
*/

#include "MemDebug.h"
#include "NetLobby.h"
#include "NetLobbyClient.h"
#include "NetLobbyServer.h"
#include "NetClientConfig.h"
#include "NetServerConfig.h"
#include "NetChat.h"
#include "NetUser.h"

#include "NetMsg.h"
#include "NetData.h"
#include "NetLayer.h"

#include "Player.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Campaign.h"
#include "Element.h"
#include "Mission.h"

#include "NetHost.h"
#include "Game.h"
#include "Light.h"
#include "parseutil.h"

// +--------------------------------------------------------------------+

const int MAX_NET_FPS   = 50;
const int MIN_NET_FRAME = 1000 / MAX_NET_FPS;

const DWORD SHIP_ID_START = 0x0010;
const DWORD SHOT_ID_START = 0x0400;

static NetLobby* instance = 0;

static DWORD   ship_id_key = SHIP_ID_START;
static DWORD   shot_id_key = SHOT_ID_START;

// +--------------------------------------------------------------------+

NetLobby::NetLobby(bool temporary)
: link(0), local_user(0), last_send_time(0), active(true),
selected_mission(0), mission(0), status(0)
{
	if (!temporary)
	instance   = this;
}

NetLobby::~NetLobby()
{
	if (instance == this)
	instance = 0;

	unit_map.destroy();
	users.destroy();
	campaigns.destroy();
	chat_log.destroy();

	if (local_user) {
		delete local_user;
		local_user = 0;
	}

	if (link) {
		delete link;
		link = 0;
	}

	mission = 0;
}

// +--------------------------------------------------------------------+

NetLobby*
NetLobby::GetInstance()
{
	return instance;
}

// +--------------------------------------------------------------------+

void
NetLobby::ExecFrame()
{
	Send();
	Recv();
}

void
NetLobby::Recv()
{
	NetMsg* msg = link->GetMessage();

	while (msg) {
		if (active) {
			NetPeer* peer = link->FindPeer(msg->NetID());

			static char buffer[256];
			char*       text = 0;

			if (msg->Length() > 2) {
				if (msg->Length() < 250) {
					ZeroMemory(buffer, sizeof(buffer));
					CopyMemory(buffer, msg->Data()+2, msg->Length()-2);
					text = buffer;
				}
				else if (msg->Length() < 1024 * 1024) {
					text = new(__FILE__,__LINE__) char[msg->Length()];
					ZeroMemory(text, msg->Length());
					CopyMemory(text, msg->Data()+2, msg->Length()-2);
				}
			}

			switch (msg->Type()) {
			case NET_LOBBY_PING:             DoPing(peer, text);           break;
			case NET_LOBBY_SERVER_INFO:      DoServerInfo(peer, text);     break;
			case NET_LOBBY_SERVER_MODS:      DoServerMods(peer, text);     break;
			case NET_LOBBY_LOGIN:            DoLogin(peer, text);          break;
			case NET_LOBBY_LOGOUT:           DoLogout(peer, text);         break;

			case NET_LOBBY_AUTH_USER:        DoAuthUser(peer, text);       break;
			case NET_LOBBY_USER_AUTH:        DoUserAuth(peer, text);       break;

			case NET_LOBBY_CHAT:             DoChat(peer, text);           break;
			case NET_LOBBY_USER_LIST:        DoUserList(peer, text);       break;
			case NET_LOBBY_BAN_USER:         DoBanUser(peer, text);        break;
			case NET_LOBBY_MISSION_LIST:     DoMissionList(peer, text);    break;
			case NET_LOBBY_MISSION_SELECT:   DoMissionSelect(peer, text);  break;
			case NET_LOBBY_MISSION_DATA:     DoMissionData(peer, text);    break;
			case NET_LOBBY_UNIT_LIST:        DoUnitList(peer, text);       break;
			case NET_LOBBY_MAP_UNIT:         DoMapUnit(peer, text);        break;
			case NET_LOBBY_GAME_START:       DoGameStart(peer, text);      break;
			case NET_LOBBY_GAME_STOP:        DoGameStop(peer, text);       break;
			case NET_LOBBY_EXIT:             DoExit(peer, text);           break;
			}

			if (text && text != buffer)
			delete [] text;
		}

		delete msg;
		msg = link->GetMessage();
	}
}

// +--------------------------------------------------------------------+

void
NetLobby::Send()
{
}

// +-------------------------------------------------------------------+

DWORD
NetLobby::GetLag()
{
	if (link)
	return link->GetLag();

	return 0;
}

// +-------------------------------------------------------------------+

void
NetLobby::SetLocalUser(NetUser* user)
{
	if (user != local_user) {
		if (local_user) {
			delete local_user;
			local_user = 0;
		}

		if (user) {
			local_user = user;
			local_user->SetHost(true);
		}
	}
}

void
NetLobby::BanUser(NetUser* user)
{
}

void
NetLobby::AddUser(NetUser* user)
{
	if (user) {
		if (user != local_user && !users.contains(user)) {
			users.append(user);
			Print("NetLobby User Logged In  - name: '%s' id: %d host: %d\n",
			user->Name().data(),
			user->GetNetID(),
			user->IsHost());
		}
	}
}

void
NetLobby::DelUser(NetUser* user)
{
	if (user) {
		if (user == local_user)
		local_user = 0;

		else
		users.remove(user);

		UnmapUnit(user->Name());

		Print("NetLobby User Logged Out - name: '%s' id: %d host: %d\n",
		user->Name().data(),
		user->GetNetID(),
		user->IsHost());

		user->SetHost(false);
		delete user;
	}
}

int
NetLobby::NumUsers()
{
	int num = 0;

	if (local_user)
	num = 1;

	num += users.size();

	return num;
}

NetUser*
NetLobby::GetHost()
{
	for (int i = 0; i < users.size(); i++) {
		NetUser* u = users[i];

		if (u->IsHost())
		return u;
	}

	return 0;
}

bool
NetLobby::HasHost()
{
	bool host = false;

	if (local_user && local_user->IsHost())
	host = true;

	for (int i = 0; i < users.size() && !host; i++) {
		if (users[i]->IsHost())
		host = true;
	}

	if (status > NetServerInfo::LOBBY)
	host = true;

	return host;
}

bool
NetLobby::SetUserHost(NetUser* user, bool host)
{
	bool ok = false;

	if (user && users.contains(user)) {
		if (host && !HasHost()) {
			user->SetHost(true);
			ok = true;
		}
		else if (!host) {
			user->SetHost(false);
			ok = true;
		}
	}

	return ok;
}

NetUser*
NetLobby::GetLocalUser()
{
	return local_user;
}

List<NetUser>&
NetLobby::GetUsers()
{
	return users;
}

List<ModInfo>&
NetLobby::GetServerMods()
{
	return server_mods;
}

NetUser*
NetLobby::FindUserByAddr(const NetAddr& addr)
{
	for (int i = 0; i < users.size(); i++) {
		NetUser* u = users[i];
		if (u->GetAddress().IPAddr() == addr.IPAddr())
		return u;
	}

	return 0;
}

NetUser*
NetLobby::FindUserByName(const char* name)
{
	if (local_user && !stricmp(local_user->Name(), name))
	return local_user;

	for (int i = 0; i < users.size(); i++) {
		NetUser* u = users[i];
		if (!stricmp(u->Name(), name))
		return u;
	}

	return 0;
}

NetUser*
NetLobby::FindUserByNetID(DWORD id)
{
	for (int i = 0; i < users.size(); i++) {
		NetUser* u = users[i];
		if (u->GetNetID() == id)
		return u;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
NetLobby::ParseMsg(Text msg, List<NetLobbyParam>& params)
{
	params.destroy();

	BlockReader reader(msg.data(), msg.length());
	Scanner     lexer(&reader);

	Token name  = lexer.Get();

	while (name.type() == Token::AlphaIdent) {
		Token value = lexer.Get();
		if (value.type() != Token::EOT) {
			Text val = value.symbol();

			if (val[0] == '"' && val[val.length()-1] == '"') {
				val = val.substring(1, val.length()-2);
			}

			NetLobbyParam* param = new(__FILE__,__LINE__) NetLobbyParam(name.symbol(), val);
			params.append(param);

			name = lexer.Get();
		}

		else {
			name = Token(Token::EOT);
		}
	}
}

// +--------------------------------------------------------------------+

bool
NetLobby::Ping()
{
	Sleep(500);
	return false;
}

// +-------------------------------------------------------------------+

void
NetLobby::AddChat(NetUser* user, const char* msg, bool route)
{
	if (user && msg && *msg)
	chat_log.append(new(__FILE__,__LINE__) NetChatEntry(user, msg));
}

List<NetChatEntry>&
NetLobby::GetChat()
{
	return chat_log;
}

void
NetLobby::ClearChat()
{
	chat_log.destroy();
}

// +-------------------------------------------------------------------+

List<NetCampaignInfo>&
NetLobby::GetCampaigns()
{
	return campaigns;
}

// +-------------------------------------------------------------------+

void
NetLobby::AddUnitMap(MissionElement* elem, int index)
{
	if (elem)
	unit_map.append(new(__FILE__,__LINE__) NetUnitEntry(elem, index));
}

List<NetUnitEntry>&
NetLobby::GetUnitMap()
{
	return unit_map;
}

void
NetLobby::ClearUnitMap()
{
	unit_map.destroy();
}

void
NetLobby::MapUnit(int n, const char* user, bool lock)
{
	if (n >= 0 && n < unit_map.size()) {
		NetUnitEntry* unit = unit_map[n];

		if (!lock && unit->GetLocked())
		return;

		if (user && *user) {
			for (int i = 0; i < unit_map.size(); i++) {
				NetUnitEntry* u = unit_map[i];
				if (u->GetUserName() == user) {
					if (!lock && u->GetLocked()) return;
					u->SetUserName("");
				}
			}
		}

		unit->SetUserName(user);
		unit->SetLock(lock);
	}
}

void
NetLobby::UnmapUnit(const char* user)
{
	if (user && *user) {
		for (int i = 0; i < unit_map.size(); i++) {
			NetUnitEntry* u = unit_map[i];
			if (u->GetUserName() == user) {
				u->SetUserName("");
				u->SetLock(false);
				return;
			}
		}
	}
}

bool
NetLobby::IsMapped(const char* user)
{
	if (user && *user) {
		for (int i = 0; i < unit_map.size(); i++) {
			NetUnitEntry* u = unit_map[i];
			if (u->GetUserName() == user) {
				return true;
			}
		}
	}

	return false;
}

// +--------------------------------------------------------------------+

void
NetLobby::SelectMission(DWORD id)
{
	if (selected_mission != id) {
		selected_mission = id;
		ClearUnitMap();

		int         mission_id  = selected_mission;
		int         campaign_id = -1;
		Campaign*   campaign    = 0;
		mission     = 0;

		campaign_id = mission_id >> NET_CAMPAIGN_SHIFT;
		mission_id  = mission_id &  NET_MISSION_MASK;

		ListIter<Campaign> c_iter = Campaign::GetAllCampaigns();
		while (++c_iter) {
			campaign = c_iter.value();

			if (campaign->GetCampaignId() == campaign_id) {
				mission = campaign->GetMission(mission_id);
				break;
			}
		}

		if (campaign && mission) {
			Campaign::SelectCampaign(campaign->Name());
			campaign->SetMissionId(mission_id);

			ListIter<MissionElement> iter = mission->GetElements();
			while (++iter) {
				MissionElement* elem = iter.value();

				if (elem->IsPlayable()) {
					if (elem->Count() == 1) {
						AddUnitMap(elem);
					}
					else {
						for (int i = 0; i < elem->Count(); i++)
						AddUnitMap(elem, i+1);
					}
				}
			}
		}

		else {
			selected_mission = 0;
			mission          = 0;
		}
	}
}

// +--------------------------------------------------------------------+

bool
NetLobby::IsNetLobbyClient()
{
	if (instance)
	return instance->IsClient();
	return false;
}

bool
NetLobby::IsNetLobbyServer()
{
	if (instance)
	return instance->IsServer();
	return false;
}

// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

NetUnitEntry::NetUnitEntry(MissionElement* e, int n)
: index(n), lock(false), lives(1), hull(100), role(0)
{
	if (e) {
		elem  = e->Name();
		iff   = e->GetIFF();

		if (e->GetDesign())
		design = e->GetDesign()->name;
	}
}

NetUnitEntry::NetUnitEntry(const char* e, const char* d, int i, int n)
: elem(e), design(d), iff(i), index(n), lock(false), lives(1),
hull(100), role(0)
{ }

NetUnitEntry::~NetUnitEntry()
{ }

Text
NetUnitEntry::GetDescription() const
{
	if (elem) {
		static char buffer[1024];

		sprintf(buffer, "name \"%s\" index %d design \"%s\" iff %d user \"%s\" lives %d hull %d role %d lock %d ",
		elem.data(),
		index,
		design.data(),
		iff,
		user.data(),
		lives,
		hull,
		role,
		lock);

		return buffer;
	}

	return "name \"Not Found\" ";
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

NetServerInfo::NetServerInfo()
: nplayers(0), hosted(0), status(0), port(0), gameport(0), save(false), ping_time(0)
{ }

