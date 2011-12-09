/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetLobbyServer.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	NetLink Engine for Multiplayer Lobby
*/

#include "MemDebug.h"
#include "NetLobbyServer.h"
#include "NetServerConfig.h"
#include "NetClientConfig.h"
#include "NetBrokerClient.h"
#include "NetAuth.h"
#include "NetChat.h"
#include "NetUser.h"
#include "Campaign.h"
#include "Mission.h"
#include "StarServer.h"
#include "Starshatter.h"
#include "StarServer.h"
#include "ShipDesign.h"
#include "Sim.h"

#include "ModConfig.h"
#include "ModInfo.h"

#include "NetGame.h"
#include "NetPlayer.h"
#include "NetUtil.h"

#include "NetPeer.h"
#include "NetLayer.h"
#include "NetHost.h"
#include "NetMsg.h"

#include "MachineInfo.h"
#include "Game.h"
#include "FormatUtil.h"

extern const char* versionInfo;

// +-------------------------------------------------------------------+

static NetLobbyServer*  net_lobby_server = 0;

NetLobbyServer::NetLobbyServer()
: announce_time(0), server_config(0), motd_index(1)
{
	status      = NetServerInfo::LOBBY;
	server_name = Text("Starshatter NetLobbyServer ") + versionInfo;
	start_time = NetLayer::GetUTC();

	selected_mission = 0;

	WORD    server_port = 11100;

	server_config = NetServerConfig::GetInstance();
	if (server_config) {
		server_name    = server_config->Name();
		server_port    = server_config->GetLobbyPort();
		server_mission = server_config->GetMission();

		NetAuth::SetAuthLevel(server_config->GetAuthLevel());

		server_addr = NetAddr(NetHost().Address().IPAddr(), server_port);
		link = new(__FILE__,__LINE__) NetLink(server_addr);
	}

	LoadMOTD();

	StarServer* star_server = StarServer::GetInstance();
	DWORD       mission_id = 0;

	// only one mission:
	if (star_server && server_mission.length() > 0) {
		NetCampaignInfo* c = new(__FILE__,__LINE__) NetCampaignInfo;
		c->id   = Campaign::MULTIPLAYER_MISSIONS;
		c->name = "Persistent Multiplayer";
		campaigns.append(c);

		ListIter<Campaign> c_iter = Campaign::GetAllCampaigns();
		while (++c_iter && !mission_id) {
			Campaign* campaign = c_iter.value();

			if (campaign->GetCampaignId() == Campaign::MULTIPLAYER_MISSIONS) {
				ListIter<MissionInfo> m_iter = campaign->GetMissionList();
				while (++m_iter && !mission_id) {
					MissionInfo* m = m_iter.value();

					if (m->script == server_mission) {
						c->missions.append(m);
						mission_id = (Campaign::MULTIPLAYER_MISSIONS << NET_CAMPAIGN_SHIFT) + m->id;

						SelectMission(mission_id);
						star_server->SetGameMode(StarServer::LOAD_MODE);

						// lock in mission:
						SetStatus(NetServerInfo::PERSISTENT);
					}
				}
			}
		}
	}

	// player host may select mission:
	if (!mission_id) {
		campaigns.destroy();

		ListIter<Campaign> c_iter = Campaign::GetAllCampaigns();
		while (++c_iter) {
			Campaign* campaign = c_iter.value();

			if (campaign->GetCampaignId() >= Campaign::MULTIPLAYER_MISSIONS) {
				NetCampaignInfo* c = new(__FILE__,__LINE__) NetCampaignInfo;
				c->id   = campaign->GetCampaignId();
				c->name = campaign->Name();
				campaigns.append(c);

				ListIter<MissionInfo> m_iter = campaign->GetMissionList();
				while (++m_iter) {
					MissionInfo* m = m_iter.value();
					c->missions.append(m);
				}
			}
		}
	}

	ModConfig*     config = ModConfig::GetInstance();
	List<ModInfo>& mods   = config->GetModInfoList();

	server_mods.clear();
	server_mods.append(mods);

	net_lobby_server = this;
}

NetLobbyServer::~NetLobbyServer()
{
	ListIter<NetUser> iter = users;
	while (++iter) {
		NetUser* u = iter.value();
		SendData(u, NET_LOBBY_EXIT, Text());
		ExecFrame();
	}

	Sleep(500);

	unit_map.destroy();
	chat_log.destroy();
	users.destroy();
	motd.destroy();

	if (net_lobby_server == this)
	net_lobby_server = 0;
}

NetLobbyServer*
NetLobbyServer::GetInstance()
{
	return net_lobby_server;
}

// +--------------------------------------------------------------------+

void
NetLobbyServer::LoadMOTD()
{
	motd.destroy();

	FILE* f = fopen("motd.txt", "r");

	if (f) {
		char line[256];

		while (fgets(line, 256, f)) {
			int n = strlen(line) - 1;
			
			while (n >= 0 && isspace(line[n]))
			line[n--] = 0;

			motd.append(new(__FILE__,__LINE__) Text(line));
		}
	}
}

void
NetLobbyServer::SendMOTD(NetUser* user)
{
	if (motd.size() < 1) return;

	char  buffer[512];

	for (int i = 0; i < motd.size(); i++) {
		Text* line = motd[i];

		sprintf(buffer, "id %d user \" \" msg \"%s\"",
		motd_index++, *line);

		SendData(user, NET_LOBBY_CHAT, buffer);
	}

	sprintf(buffer, "id %d user \" \" msg \" \"", motd_index++);
	SendData(user, NET_LOBBY_CHAT, buffer);
}

void
NetLobbyServer::SendMods(NetUser* user)
{
	char  buffer[300];

	ModConfig*        config   = ModConfig::GetInstance();
	List<ModInfo>&    mods     = config->GetModInfoList();

	if (mods.size() < 1) return;

	for (int i = 0; i < mods.size(); i++) {
		ModInfo* info = mods[i];

		sprintf(buffer, "id %d user \"Enabled Mods:\" msg \"%d. '%s' ",
		motd_index++, i+1, info->Name().data());

		Text msg = buffer;

		if (info->Version().length() > 0) {
			msg += "version ";
			msg += info->Version().data();
		}

		if (info->URL().length() > 0) {
			msg += "  - ";
			msg += info->URL().data();
		}

		msg += "\"";

		SendData(user, NET_LOBBY_CHAT, msg);
	}

	sprintf(buffer, "id %d user \" \" msg \" \"", motd_index++);
	SendData(user, NET_LOBBY_CHAT, buffer);
}

// +--------------------------------------------------------------------+

void
NetLobbyServer::ExecFrame()
{
	NetLobby::ExecFrame();

	if (announce_time == 0 || Game::RealTime() - announce_time > 200000) {
		GameOn();
		announce_time = Game::RealTime();
	}

	if (GetStatus() == NetServerInfo::BRIEFING) {
		NetGame* net_game = NetGame::GetInstance();

		if (net_game && net_game->NumPlayers() > 0) {
			SetStatus(NetServerInfo::ACTIVE);
		}
	}

	StarServer* star_server = StarServer::GetInstance();
	DWORD       mission_id = 0;

	// restart persistent mission?
	if (star_server &&
			star_server->GetGameMode() == StarServer::MENU_MODE &&
			server_mission.length() > 0) {

		NetCampaignInfo* c = campaigns.last();
		
		if (!c || c->name != "Persistent Multiplayer") {
			c = new(__FILE__,__LINE__) NetCampaignInfo;
			c->id   = Campaign::MULTIPLAYER_MISSIONS;
			c->name = "Persistent Multiplayer";
			campaigns.append(c);
		}
		else {
			c->missions.clear();
		}

		ListIter<Campaign> c_iter = Campaign::GetAllCampaigns();
		while (++c_iter && !mission_id) {
			Campaign* campaign = c_iter.value();

			if (campaign->GetCampaignId() == Campaign::MULTIPLAYER_MISSIONS) {
				ListIter<MissionInfo> m_iter = campaign->GetMissionList();
				while (++m_iter && !mission_id) {
					MissionInfo* m = m_iter.value();

					if (m->script == server_mission) {
						c->missions.append(m);
						mission_id = (Campaign::MULTIPLAYER_MISSIONS << NET_CAMPAIGN_SHIFT) + m->id;

						// unlock old mission:
						SetStatus(NetServerInfo::LOBBY);

						SelectMission(mission_id);

						if (star_server->GetGameMode() == StarServer::MENU_MODE) {
							star_server->SetGameMode(StarServer::LOAD_MODE);
						}

						// lock in new mission:
						SetStatus(NetServerInfo::PERSISTENT);
					}
				}
			}
		}
	}

	CheckSessions();
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::SendData(NetUser* dst, int type, Text msg)
{
	if (link && dst && type > 0 && type < 255) {
		if (msg.length())
		link->SendMessage(dst->GetNetID(), (BYTE) type, msg.data(), msg.length(), NetMsg::RELIABLE);
		else
		link->SendMessage(dst->GetNetID(), (BYTE) type, 0, 0, NetMsg::RELIABLE);
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::CheckSessions()
{
	if (!link)
	return;

	bool dropped = false;

	ListIter<NetUser> u_iter = users;
	while (++u_iter) {
		NetUser* u = u_iter.value();
		NetPeer* p = link->FindPeer(u->GetNetID());

		if (p && (NetLayer::GetUTC() - p->LastReceiveTime()) > NET_DISCONNECT_TIME) {
			// check game peer for activity:
			NetGame*    game   = NetGame::GetInstance();
			NetPlayer*  player = 0;
			NetPeer*    p2     = 0;

			if (game) {
				player = game->FindPlayerByName(u->Name());

				if (player) {
					p2 = game->GetPeer(player);

					if (p2 && (NetLayer::GetUTC() - p2->LastReceiveTime()) < NET_DISCONNECT_TIME) {
						p->SetLastReceiveTime(p2->LastReceiveTime());
						continue;
					}
				}
				else {
					::Print("NetLobbyServer::CheckSessions() Could not find player for '%s'\n", u->Name().data());
				}
			}
			else {
				::Print("NetLobbyServer::CheckSessions() Could not find net game for '%s'\n", u->Name().data());
			}

			// announce drop:
			char timestr[64];
			FormatTime(timestr, Game::RealTime()/1000);
			Print("NetLobbyServer: Dropped inactive connection '%s' %s\n",
			u->Name().data(), timestr);

			if (u->IsHost()) {
				Print("              User was host - ending net game.\n");
				GameStop();
			}

			u_iter.removeItem();             // first remove user from list
			NetLobby::UnmapUnit(u->Name());  // then unmap unit
			delete u;                        // now it is safe to discard the inactive user

			dropped = true;
		}
	}

	if (dropped) {
		SendUsers();
		SendUnits();
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::GameStart()
{
	if (status < NetServerInfo::ACTIVE) {
		SetStatus(NetServerInfo::BRIEFING);
		
		if (Starshatter::GetInstance()) {
			Starshatter::GetInstance()->SetGameMode(Starshatter::PREP_MODE);
		}
		else {
			StarServer* s = StarServer::GetInstance();
			if (s && s->GetGameMode() == StarServer::MENU_MODE) {
				s->SetGameMode(StarServer::LOAD_MODE);
			}
		}
	}
}

void
NetLobbyServer::GameStop()
{
	if (GetStatus() != NetServerInfo::PERSISTENT) {
		SetStatus(NetServerInfo::LOBBY);

		StarServer* s = StarServer::GetInstance();
		if (s && s->GetGameMode() != StarServer::MENU_MODE) {
			s->SetGameMode(StarServer::MENU_MODE);
		}
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::BanUser(NetUser* user)
{
	if (user && !user->IsHost()) {
		::Print("NetLobbyServer::BanUser name '%s' addr %d.%d.%d.%d\n",
		user->Name().data(),
		user->GetAddress().B1(),
		user->GetAddress().B2(),
		user->GetAddress().B3(),
		user->GetAddress().B4());

		SendData(user, NET_LOBBY_EXIT, Text());

		if (server_config)
		server_config->BanUser(user);

		DelUser(user);
	}
}

void
NetLobbyServer::AddUser(NetUser* user)
{
	if (server_config && server_config->IsUserBanned(user)) {
		delete user;
		return;
	}

	NetLobby::AddUser(user);
	SendUsers();
}

void
NetLobbyServer::DelUser(NetUser* user)
{
	NetLobby::DelUser(user);
	SendUsers();
}

void
NetLobbyServer::SendUsers()
{
	Text content;

	ListIter<NetUser> u_iter = users;
	while (++u_iter) {
		NetUser* u = u_iter.value();
		content += u->GetDescription();
	}

	u_iter.reset();

	while (++u_iter) {
		NetUser* u = u_iter.value();
		SendData(u, NET_LOBBY_USER_LIST, content);
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::RequestAuth(NetUser* user)
{
	if (user) {
		Text request = NetAuth::CreateAuthRequest(user);

		if (request.length() > 0)
		SendData(user, NET_LOBBY_AUTH_USER, request);
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::AddChat(NetUser* user, const char* msg, bool route)
{
	NetChatEntry* entry = 0;

	if (user && msg && *msg) {
		bool        msg_ok = false;
		const char* p      = msg;

		while (*p && !msg_ok) {
			if (!isspace(*p++))
			msg_ok = true;
		}

		if (msg_ok) {
			entry = new(__FILE__,__LINE__) NetChatEntry(user, msg);

			chat_log.append(entry);

			// forward to all clients:
			if (users.size()) {
				char buffer[768];
				char msg_buf[256];
				char usr_buf[256];

				// safe quotes uses a static buffer,
				// so make sure to save copies of the
				// results when using more than one in
				// a function call...

				strcpy(msg_buf, SafeQuotes(msg));
				strcpy(usr_buf, SafeQuotes(user->Name()));

				sprintf(buffer, "id %d user \"%s\" msg \"%s\"",
				entry->GetID(), usr_buf, msg_buf);

				ListIter<NetUser> iter = users;
				while (++iter) {
					NetUser* u = iter.value();
					SendData(u, NET_LOBBY_CHAT, buffer);
				}

				if (route) {
					// send to active game:
					NetUtil::SendChat(0xffff, usr_buf, msg_buf);
				}
			}
		}
	}
}

void
NetLobbyServer::ClearChat()
{
	NetLobby::ClearChat();
}

void
NetLobbyServer::SaveChat()
{
	FILE* f = fopen("chat.txt", "w");
	if (f) {
		for (int i = 0; i < chat_log.size(); i++) {
			NetChatEntry* c = chat_log[i];
			fprintf(f, "%08x [%s] %s\n",
			c->GetTime(),
			c->GetUser().data(), 
			c->GetMessage().data());
		}

		fclose(f);
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::SelectMission(DWORD id)
{
	if (GetStatus() == NetServerInfo::PERSISTENT)
	return;

	NetLobby::SelectMission(id);

	// inform all users of the selection:
	char buffer[32];
	sprintf(buffer, "m_id 0x%08x", selected_mission);

	ListIter<NetUser> iter = users;
	while (++iter) {
		NetUser* u = iter.value();
		SendData(u, NET_LOBBY_MISSION_SELECT, buffer);
	}
}

// +-------------------------------------------------------------------+

List<NetUnitEntry>&
NetLobbyServer::GetUnitMap()
{
	if (!mission) {
		unit_map.destroy();
		return unit_map;
	}

	List<NetUnitEntry>       units;
	ListIter<MissionElement> iter = mission->GetElements();
	int                      i    = 0;

	Sim* sim = Sim::GetSim();
	if (sim && sim->GetElements().size() > 0)
	iter = sim->GetMissionElements();

	// create new entries for the playable elements in the mission or simulation:
	while (++iter) {
		MissionElement* elem = iter.value();

		if (elem->IsPlayable()) {
			NetUnitEntry* u = 0;
			if (elem->Count() == 1) {
				u = new(__FILE__,__LINE__) NetUnitEntry(elem, 0);
				u->SetLives(elem->RespawnCount() + 1);
				u->SetMissionRole(elem->MissionRole());
				u->SetIFF(elem->GetIFF());

				if (elem->GetDesign())
				u->SetDesign(elem->GetDesign()->name);

				if (elem->Ships().size() > 0) {
					MissionShip* s = elem->Ships()[0];
					u->SetIntegrity((int) s->Integrity());
				}
				units.append(u);
			}
			else {
				for (int i = 0; i < elem->Count(); i++) {
					u = new(__FILE__,__LINE__) NetUnitEntry(elem, i+1);
					u->SetMissionRole(elem->MissionRole());
					u->SetIFF(elem->GetIFF());

					if (elem->GetDesign())
					u->SetDesign(elem->GetDesign()->name);

					if (elem->Ships().size() > i) {
						MissionShip* s = elem->Ships()[i];
						u->SetLives(s->Respawns() + 1);
						u->SetIntegrity((int) s->Integrity());
					}
					units.append(u);
				}
			}
		}
	}

	// match new entries with any existing map entries:
	if (unit_map.size()) {
		for (i = 0; i < units.size(); i++) {
			NetUnitEntry* e_new = units[i];
			NetUnitEntry* e_old = unit_map.find(e_new);

			if (e_old) {
				e_new->SetUserName(e_old->GetUserName());
				e_new->SetLock(e_old->GetLocked());
			}
		}
	}

	// rewrite the unit map with the new entries:
	ClearUnitMap();
	for (i = 0; i < units.size(); i++) {
		unit_map.append(units[i]);
	}

	return unit_map;
}

void
NetLobbyServer::MapUnit(int n, const char* user, bool lock)
{
	NetLobby::MapUnit(n, user, lock);

	Text reply;

	ListIter<NetUnitEntry> map_iter = GetUnitMap();
	while (++map_iter) {
		NetUnitEntry* unit = map_iter.value();
		reply += unit->GetDescription();
	}

	ListIter<NetUser> u_iter = users;
	while (++u_iter) {
		NetUser* u = u_iter.value();
		SendData(u, NET_LOBBY_UNIT_LIST, reply);
	}
}

void
NetLobbyServer::UnmapUnit(const char* user)
{
	NetLobby::UnmapUnit(user);

	Text reply;

	ListIter<NetUnitEntry> map_iter = GetUnitMap();
	while (++map_iter) {
		NetUnitEntry* unit = map_iter.value();
		reply += unit->GetDescription();
	}

	ListIter<NetUser> u_iter = users;
	while (++u_iter) {
		NetUser* u = u_iter.value();
		SendData(u, NET_LOBBY_UNIT_LIST, reply);
	}
}

void
NetLobbyServer::SendUnits()
{
	Text content;

	ListIter<NetUnitEntry> map_iter = GetUnitMap();
	while (++map_iter) {
		NetUnitEntry* unit = map_iter.value();
		content += unit->GetDescription();
	}

	ListIter<NetUser> u_iter = users;
	while (++u_iter) {
		NetUser* u = u_iter.value();
		SendData(u, NET_LOBBY_UNIT_LIST, content);
	}
}

// +-------------------------------------------------------------------+

Text
NetLobbyServer::Serialize(Mission* m, NetUser* user)
{
	Text          s;

	if (!m || !user)
	return s;

	NetUnitEntry* unit = 0;

	ListIter<NetUnitEntry> u_iter = GetUnitMap();
	while (++u_iter && !unit) {
		NetUnitEntry* u = u_iter.value();
		if (u->GetUserName() == user->Name())
		unit = u;
	}

	if (unit)
	s = m->Serialize(unit->GetElemName(), unit->GetIndex());

	return s;
}

Mission*
NetLobbyServer::GetSelectedMission()
{
	if (mission) {
		Text content = Serialize(mission, GetLocalUser());
		Campaign*   c = Campaign::SelectCampaign("Multiplayer Missions");

		if (c) {
			c->LoadNetMission(99999, content.data());
			return c->GetMission(99999);
		}
	}

	return mission;
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::GameOn()
{
	NetHost           host;
	const char*       type     = "Starshatter";
	const char*       password = "No";
	char              address[32];

	strcpy(address, "0");

	if (server_config) {
		if (server_config->GetGameType() == NetServerConfig::NET_GAME_PRIVATE)
		return;

		if (server_config->GetGameType() == NetServerConfig::NET_GAME_LAN) {
			type = "Starshatter-LAN";
			sprintf(address, "%d.%d.%d.%d",
			host.Address().B1(),
			host.Address().B2(),
			host.Address().B3(),
			host.Address().B4());
		}
		else {
			type = "Starshatter";
			sprintf(address, "0.0.0.0");
		}

		if (server_config->GetGamePass().length() > 0)
		password = "Yes";
	}

	NetBrokerClient::GameOn(server_name,
	type,
	address,
	server_addr.Port(),
	password);
}

void
NetLobbyServer::GameOff()
{
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::DoPing(NetPeer* peer, Text s)
{ }

void
NetLobbyServer::DoServerInfo(NetPeer* peer, Text s)
{
	if (peer && peer->NetID()) {
		char buffer[1024];
		WORD gameport = 11101;

		if (server_config)
		gameport = server_config->GetGamePort();

		sprintf(buffer, "info \"%s\" version \"%s\" mode %d users %d host %s port %d",
		MachineInfo::GetShortDescription(),
		versionInfo,
		GetStatus(),
		NumUsers(),
		HasHost() ? "true" : "false",
		gameport);

		link->SendMessage(peer->NetID(), (BYTE) NET_LOBBY_SERVER_INFO, buffer, strlen(buffer), NetMsg::RELIABLE);
	}
}

void
NetLobbyServer::DoServerMods(NetPeer* peer, Text s)
{
	if (peer && peer->NetID()) {
		Text              response;
		ModConfig*        config   = ModConfig::GetInstance();
		List<ModInfo>&    mods     = config->GetModInfoList();
		ListIter<ModInfo> mod_iter = mods;

		char buffer[32];
		sprintf(buffer, "num %d ", mods.size());
		response += buffer;

		while (++mod_iter) {
			ModInfo* info = mod_iter.value();

			response += "mod \"";
			response += info->Name();
			response += "\" url \"";
			response += info->URL();
			response += "\" ver \"";
			response += info->Version();
			response += "\" ";
		}

		link->SendMessage(peer->NetID(), (BYTE) NET_LOBBY_SERVER_MODS, response, response.length(), NetMsg::RELIABLE);
	}
}

// +-------------------------------------------------------------------+

void
NetLobbyServer::DoLogin(NetPeer* peer, Text msg)
{
	List<NetLobbyParam> params;
	ParseMsg(msg, params);

	Text  name;
	Text  pass;
	Text  host;
	Text  gamepass;
	Text  signature;
	Text  squadron;
	Text  version;
	int   rank        = 0;
	int   flight_time = 0;
	int   missions    = 0;
	int   kills       = 0;
	int   losses      = 0;

	for (int i = 0; i < params.size(); i++) {
		NetLobbyParam* p = params[i];

		int num = 0;
		sscanf(p->value, "%d", &num);

		if (p->name == "name")
		name = p->value;

		else if (p->name == "pass")
		pass = p->value;

		else if (p->name == "gamepass")
		gamepass = p->value;

		else if (p->name == "host")
		host = p->value;

		else if (p->name == "sig")
		signature = p->value;

		else if (p->name == "squad")
		squadron = p->value;

		else if (p->name == "version")
		version = p->value;

		else if (p->name == "rank")
		rank = num;

		else if (p->name == "time")
		flight_time = num;

		else if (p->name == "miss")
		missions = num;

		else if (p->name == "kill")
		kills = num;

		else if (p->name == "loss")
		losses = num;
	}

	params.destroy();

	// first check the game version:
	if (version != versionInfo) {
		Print("NetLobbyServer - user '%s' tried to login with invalid game version '%s'\n",
		name.data(), version.data());

		return;
	}

	// next check the game password:
	if (server_config && server_config->GetGamePass().length() > 0) {
		if (gamepass != server_config->GetGamePass()) {
			Print("NetLobbyServer - user '%s' tried to login with invalid game password '%s'\n",
			name.data(), gamepass.data());

			return;
		}
	}

	// now try to log the user in:
	NetUser* pre_existing = FindUserByName(name);

	// is user already logged in?
	if (pre_existing) {
		if (pre_existing->Pass() == pass && 
				pre_existing->GetAddress().IPAddr() == peer->Address().IPAddr()) {
		}
	}

	// otherwise, create a new user:
	else {
		NetUser* user = new(__FILE__,__LINE__) NetUser(name);
		user->SetAddress(peer->Address());
		user->SetNetID(peer->NetID());
		user->SetPass(pass);
		user->SetSignature(signature);
		user->SetSquadron(squadron);
		user->SetRank(rank);
		user->SetFlightTime(flight_time);
		user->SetMissions(missions);
		user->SetKills(kills);
		user->SetLosses(losses);

		if (host == "true" && !HasHost())
		user->SetHost(true);

		AddUser(user);
		RequestAuth(user);
		SendMOTD(user);
		SendMods(user);
	}
}

void
NetLobbyServer::DoLogout(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user) {
		if (user->IsHost())
		GameStop();

		DelUser(user);
	}
}

void
NetLobbyServer::DoUserAuth(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user) {
		NetAuth::AuthUser(user, msg);

		if (!user->IsAuthOK()) {
			char buffer[256];

			sprintf(buffer, "id %d user \"SERVER\" msg \"**********\"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);

			sprintf(buffer, "id %d user \"SERVER\" msg \"*** Your game configuration does not match the server.\"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);

			if (server_mods.size() > 0) {
				sprintf(buffer, "id %d user \"SERVER\" msg \"*** Please check that you have the proper mods deployed in\"", motd_index++);
				SendData(user, NET_LOBBY_CHAT, buffer);
				sprintf(buffer, "id %d user \"SERVER\" msg \"*** the order shown above.\"", motd_index++);
				SendData(user, NET_LOBBY_CHAT, buffer);
			}

			else {
				sprintf(buffer, "id %d user \"SERVER\" msg \"*** Please verify that you have no mods deployed.\"", motd_index++);
				SendData(user, NET_LOBBY_CHAT, buffer);
			}

			sprintf(buffer, "id %d user \"SERVER\" msg \"*** You will not be permitted to join the game with an invalid\"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);

			sprintf(buffer, "id %d user \"SERVER\" msg \"*** configuration.  You may reconnect to this server after you\"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);

			sprintf(buffer, "id %d user \"SERVER\" msg \"*** have corrected your mod configuration.\"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);

			sprintf(buffer, "id %d user \"SERVER\" msg \"**********\"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);

			sprintf(buffer, "id %d user \" \" msg \" \"", motd_index++);
			SendData(user, NET_LOBBY_CHAT, buffer);
		}
	}
}

void
NetLobbyServer::DoChat(NetPeer* peer, Text msg)
{
	List<NetLobbyParam> params;
	ParseMsg(msg, params);

	Text chat_msg;

	for (int i = 0; i < params.size(); i++) {
		NetLobbyParam* p = params[i];

		int num = 0;
		sscanf(p->value, "%d", &num);

		if (p->name == "msg") {
			chat_msg = p->value;
		}
	}

	params.destroy();

	NetUser* user = FindUserByNetID(peer->NetID());

	if (user) {
		// receive chat from client:
		if (chat_msg.length()) {
			AddChat(user, chat_msg);
		}

		// request for chat log:
		else {
			ListIter<NetChatEntry> iter = chat_log;
			while (++iter) {
				NetChatEntry* entry = iter.value();

				char buffer[512];
				char msg_buf[256];
				char usr_buf[256];

				// safe quotes uses a static buffer,
				// so make sure to save copies of the
				// results when using more than one in
				// a function call...

				strcpy(msg_buf, SafeQuotes(entry->GetMessage()));
				strcpy(usr_buf, SafeQuotes(entry->GetUser()));

				sprintf(buffer, "id %d user \"%s\" msg \"%s\"",
				entry->GetID(), usr_buf, msg_buf);

				SendData(user, NET_LOBBY_CHAT, buffer);
			}
		}
	}
}

void
NetLobbyServer::DoUserList(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user) {
		Text content;

		if (local_user)
		content += local_user->GetDescription();

		ListIter<NetUser> iter = users;
		while (++iter) {
			NetUser* u = iter.value();
			content += u->GetDescription();
		}

		SendData(user, NET_LOBBY_USER_LIST, content);
	}
}

void
NetLobbyServer::DoBanUser(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user && user->IsHost() && user->IsAuthOK()) {
		List<NetLobbyParam> params;
		ParseMsg(msg, params);

		if (params.size() > 0) {
			NetLobbyParam* p = params[0];

			if (p->name == "user") {
				Text user_name = p->value;

				NetUser* u = FindUserByName(user_name);
				if (u && !u->IsHost())
				BanUser(u);
			}
		}

		params.destroy();
	}
}

void
NetLobbyServer::DoMissionList(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user) {
		Text reply;
		char buffer[4096];

		ListIter<Campaign> c_iter = Campaign::GetAllCampaigns();
		while (++c_iter) {
			Campaign* c = c_iter.value();

			if (c->GetCampaignId() >= Campaign::MULTIPLAYER_MISSIONS) {
				sprintf(buffer, "c_id 0x%08x c_name \"%s\" ",
				c->GetCampaignId(),
				SafeQuotes(c->Name()));

				reply += buffer;
			}
		}

		c_iter.reset();

		while (++c_iter) {
			Campaign* c = c_iter.value();

			if (c->GetCampaignId() >= Campaign::MULTIPLAYER_MISSIONS) {

				ListIter<MissionInfo> m_iter = c->GetMissionList();
				while (++m_iter) {
					MissionInfo* m = m_iter.value();

					int mission_id = (c->GetCampaignId() << NET_CAMPAIGN_SHIFT) + m->id;

					sprintf(buffer, "m_id 0x%08x ", mission_id);
					reply += buffer;

					reply += "m_name \"";
					reply += SafeQuotes(m->name);

					// long version of safe quotes:
					int         n = 0;
					const char* s = m->description.data();

					while (*s && n < 4090) {
						if (*s == '"') {
							buffer[n++] = '\'';
							s++;
						}
						else if (*s == '\n') {
							buffer[n++] = '\\';
							buffer[n++] = 'n';
							s++;
						}
						else if (*s == '\t') {
							buffer[n++] = '\\';
							buffer[n++] = 't';
							s++;
						}
						else {
							buffer[n++] = *s++;
						}
					}

					// don't forget the null terminator!
					buffer[n] = 0;

					reply += "\" m_desc \"";
					reply += buffer;

					reply += "\" ";
				}
			}
		}

		SendData(user, NET_LOBBY_MISSION_LIST, reply);

		sprintf(buffer, "m_id 0x%08x", selected_mission);
		SendData(user, NET_LOBBY_MISSION_SELECT, buffer);
	}
}

void
NetLobbyServer::DoMissionSelect(NetPeer* peer, Text msg)
{
	if (GetStatus() == NetServerInfo::PERSISTENT)
	return;

	NetUser* user = FindUserByNetID(peer->NetID());

	if (user && user->IsHost() && user->IsAuthOK()) {
		List<NetLobbyParam> params;
		ParseMsg(msg, params);

		for (int i = 0; i < params.size(); i++) {
			NetLobbyParam* p = params[i];

			int num = 0;
			sscanf(p->value, "0x%x", &num);

			if (p->name == "m_id") {
				SelectMission(num);
			}
		}

		params.destroy();
	}
}

void
NetLobbyServer::DoMissionData(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user && mission && user->IsAuthOK()) {
		Text reply = Serialize(mission, user);
		SendData(user, NET_LOBBY_MISSION_DATA, reply);

		FILE* f = ::fopen("multi_mission_send.def", "w");
		if (f) {
			::fwrite(reply.data(), reply.length(), 1, f);
			::fclose(f);
		}
	}
}

void
NetLobbyServer::DoUnitList(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user && unit_map.size() && user->IsAuthOK()) {
		Text reply;

		ListIter<NetUnitEntry> iter = GetUnitMap();
		while (++iter) {
			NetUnitEntry* unit = iter.value();
			reply += unit->GetDescription();
		}

		SendData(user, NET_LOBBY_UNIT_LIST, reply);
	}
}

void
NetLobbyServer::DoMapUnit(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user && unit_map.size() && user->IsAuthOK()) {
		List<NetLobbyParam> params;
		ParseMsg(msg, params);

		int   id   = 0;
		bool  lock = false;
		Text  user_name;

		for (int i = 0; i < params.size(); i++) {
			NetLobbyParam* p = params[i];

			if (p->name == "id") {
				sscanf(p->value, "%d", &id);
			}

			else if (p->name == "user") {
				user_name = p->value;
			}

			else if (p->name == "lock") {
				lock = (p->value == "true") ? true : false;
			}
		}

		params.destroy();

		MapUnit(id, user_name, lock);
	}
}

void
NetLobbyServer::DoGameStart(NetPeer* peer, Text msg)
{
	GameStart();
}

void
NetLobbyServer::DoGameStop(NetPeer* peer, Text msg)
{
	NetUser* user = FindUserByNetID(peer->NetID());

	if (user && user->IsHost() && user->IsAuthOK())
	GameStop();
}
