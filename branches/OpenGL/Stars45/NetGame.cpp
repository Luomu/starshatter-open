/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetGame.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Game Manager class
*/

#include "MemDebug.h"
#include "NetGame.h"
#include "NetGameClient.h"
#include "NetGameServer.h"
#include "NetClientConfig.h"
#include "NetServerConfig.h"
#include "NetPlayer.h"

#include "NetMsg.h"
#include "NetData.h"
#include "NetLayer.h"

#include "Player.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Sim.h"
#include "Element.h"
#include "HUDView.h"

#include "NetHost.h"
#include "Game.h"
#include "Light.h"

// +--------------------------------------------------------------------+

const int MAX_NET_FPS   = 20;
const int MIN_NET_FRAME = 1000 / MAX_NET_FPS;

const DWORD SHIP_ID_START = 0x0010;
const DWORD SHOT_ID_START = 0x0400;

static NetGame* netgame    = 0;

static DWORD   ship_id_key = SHIP_ID_START;
static DWORD   shot_id_key = SHOT_ID_START;

static long    start_time  = 0;

// +--------------------------------------------------------------------+

NetGame::NetGame()
: objid(0), netid(0), link(0), local_player(0), last_send_time(0), active(true)
{
	netgame = this;
	sim = Sim::GetSim();

	ship_id_key = SHIP_ID_START;
	shot_id_key = SHOT_ID_START;

	if (sim)
	local_player = sim->GetPlayerShip();

	Player* player = Player::GetCurrentPlayer();
	if (player) {
		player_name = player->Name();
		player_pass = player->Password();
	}

	start_time = NetLayer::GetUTC();
}

NetGame::~NetGame()
{
	netgame      = 0;
	local_player = 0;
	players.destroy();

	if (link) {
		double delta     = fabs((double)(NetLayer::GetUTC() - start_time));
		double bandwidth = 10.0 * (link->GetBytesSent() + link->GetBytesRecv()) / delta;
		double recvrate  = link->GetPacketsRecv() / delta;

		Print("NetGame Stats\n-------------\n");
		Print("  packets sent %d\n",          link->GetPacketsSent());
		Print("  packets recv %d\n",          link->GetPacketsRecv());
		Print("  bytes sent   %d\n",          link->GetBytesSent());
		Print("  bytes recv   %d\n",          link->GetBytesRecv());
		Print("  retries      %d\n",          link->GetRetries());
		Print("  drops        %d\n",          link->GetDrops());
		Print("  avg lag      %d msec\n",     link->GetLag());
		Print("  time         %d sec\n",      (int) delta);
		Print("  bandwidth    %d bps\n",      (int) bandwidth);
		Print("  packet rate  %d pps in\n\n", (int) recvrate);

		delete link;
	}
}

// +--------------------------------------------------------------------+

NetGame*
NetGame::Create()
{
	if (!netgame) {
		if (NetServerConfig::GetInstance())
		netgame = new(__FILE__,__LINE__) NetGameServer;

		else if (NetClientConfig::GetInstance() && NetClientConfig::GetInstance()->GetSelectedServer())
		netgame = new(__FILE__,__LINE__) NetGameClient;
	}

	return netgame;
}

NetGame*
NetGame::GetInstance()
{
	return netgame;
}

DWORD
NetGame::GetObjID() const
{
	if (local_player)
	return local_player->GetObjID();

	return 0;
}

DWORD
NetGame::GetNextObjID(int type)
{
	if (type == SHIP) {
		if (ship_id_key >= SHOT_ID_START)
		ship_id_key = SHIP_ID_START;

		return ship_id_key++;
	}

	else if (type == SHOT) {
		if (shot_id_key >= 0xFFFE)
		shot_id_key = SHOT_ID_START;

		return shot_id_key++;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
NetGame::ExecFrame()
{
	Send();
	Recv();
}

void
NetGame::Recv()
{
	NetMsg* msg = link->GetMessage();

	while (msg) {
		if (active) {
			// For Debug Convenience:
			// NetPlayer* player = FindPlayerByNetID(msg->NetID());

			switch (msg->Type()) {
			case NET_JOIN_REQUEST:     DoJoinRequest(msg);     break;
			case NET_JOIN_ANNOUNCE:    DoJoinAnnounce(msg);    break;
			case NET_QUIT_REQUEST:     DoQuitRequest(msg);     break;
			case NET_QUIT_ANNOUNCE:    DoQuitAnnounce(msg);    break;
			case NET_GAME_OVER:        DoGameOver(msg);        break;
			case NET_DISCONNECT:       DoDisconnect(msg);      break;

			case NET_OBJ_LOC:          DoObjLoc(msg);          break;
			case NET_OBJ_DAMAGE:       DoObjDamage(msg);       break;
			case NET_OBJ_KILL:         DoObjKill(msg);         break;
			case NET_OBJ_SPAWN:        DoObjSpawn(msg);        break;
			case NET_OBJ_HYPER:        DoObjHyper(msg);        break;
			case NET_OBJ_TARGET:       DoObjTarget(msg);       break;
			case NET_OBJ_EMCON:        DoObjEmcon(msg);        break;
			case NET_SYS_DAMAGE:       DoSysDamage(msg);       break;
			case NET_SYS_STATUS:       DoSysStatus(msg);       break;

			case NET_ELEM_CREATE:      DoElemCreate(msg);      break;
			case NET_ELEM_REQUEST:     DoElemRequest(msg);     break;
			case NET_SHIP_LAUNCH:      DoShipLaunch(msg);      break;
			case NET_NAV_DATA:         DoNavData(msg);         break;
			case NET_NAV_DELETE:       DoNavDelete(msg);       break;

			case NET_WEP_TRIGGER:      DoWepTrigger(msg);      break;
			case NET_WEP_RELEASE:      DoWepRelease(msg);      break;
			case NET_WEP_DESTROY:      DoWepDestroy(msg);      break;

			case NET_COMM_MESSAGE:     DoCommMsg(msg);         break;
			case NET_CHAT_MESSAGE:     DoChatMsg(msg);         break;
			case NET_SELF_DESTRUCT:    DoSelfDestruct(msg);    break;
			}
		}

		delete msg;
		msg = link->GetMessage();
	}
}

// +--------------------------------------------------------------------+

void
NetGame::Send()
{
}

int
NetGame::NumPlayers()
{
	if (netgame) {
		int num_players = netgame->players.size();

		if (netgame->local_player)
		num_players++;

		return num_players;
	}

	return 0;
}

NetPlayer*
NetGame::FindPlayerByName(const char* name)
{
	for (int i = 0; i < players.size(); i++) {
		NetPlayer* p = players[i];

		if (!strcmp(p->Name(), name))
		return p;
	}

	return 0;
}

NetPlayer*
NetGame::FindPlayerByNetID(DWORD netid)
{
	for (int i = 0; i < players.size(); i++) {
		NetPlayer* p = players[i];

		if (p->GetNetID() == netid)
		return p;
	}

	return 0;
}

NetPlayer*
NetGame::FindPlayerByObjID(DWORD objid)
{
	for (int i = 0; i < players.size(); i++) {
		NetPlayer* p = players[i];

		if (p->GetObjID() == objid)
		return p;
	}

	return 0;
}

Ship*
NetGame::FindShipByObjID(DWORD objid)
{
	if (sim)
	return sim->FindShipByObjID(objid);

	return 0;
}

Shot*
NetGame::FindShotByObjID(DWORD objid)
{
	if (sim)
	return sim->FindShotByObjID(objid);

	return 0;
}

NetPeer*
NetGame::GetPeer(NetPlayer* player)
{
	if (player && link) {
		return link->FindPeer(player->GetNetID());
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
NetGame::Respawn(DWORD objid, Ship* spawn)
{
}

// +--------------------------------------------------------------------+

bool
NetGame::IsNetGame()
{
	return netgame != 0;
}

bool
NetGame::IsNetGameClient()
{
	if (netgame)
	return netgame->IsClient();
	return false;
}

bool
NetGame::IsNetGameServer()
{
	if (netgame)
	return netgame->IsServer();
	return false;
}
