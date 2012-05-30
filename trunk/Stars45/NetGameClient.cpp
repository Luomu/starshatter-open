/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright (C) 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetGameClient.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Game Manager class
*/

#include "MemDebug.h"
#include "NetGameClient.h"
#include "NetClientConfig.h"
#include "NetLobby.h"
#include "NetPlayer.h"
#include "NetData.h"
#include "NetUtil.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shield.h"
#include "Shot.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Weapon.h"
#include "Element.h"
#include "Explosion.h"
#include "HUDView.h"
#include "RadioView.h"
#include "Instruction.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "Mission.h"

#include "NetMsg.h"
#include "NetHost.h"
#include "NetLayer.h"
#include "NetPeer.h"

#include "Game.h"
#include "Light.h"

// +--------------------------------------------------------------------+

const int MAX_NET_FPS   = 20;
const int MIN_NET_FRAME = 1000 / MAX_NET_FPS;

const char* FormatGameTime();

// +--------------------------------------------------------------------+

NetGameClient::NetGameClient()
: server_id(0), join_req_time(0)
{
	Print("Constructing NetGameClient\n");

	NetHost me;
	Text    server_name;
	WORD    port = 11101;

	NetClientConfig* ncc = NetClientConfig::GetInstance();
	if (ncc) {
		NetServerInfo* info = ncc->GetSelectedServer();

		if (info) {
			server_name = info->hostname;
			port        = info->gameport;
		}
	}

	if (server_name.length() && port > 0) {
		Print("  '%s' is a client of '%s'\n", me.Name(), server_name.data());
		link = new(__FILE__,__LINE__) NetLink;
		server_id = link->AddPeer(NetAddr(server_name, port));
		SendJoinRequest();
	}
	else if (port == 0) {
		Print("  '%s' invalid game port number %d\n", me.Name(), port);
	}
	else {
		Print("  '%s' is a client without a server\n", me.Name());
	}
}

NetGameClient::~NetGameClient()
{
	link->SendMessage(server_id, NET_QUIT_REQUEST, 0, 0, NetMsg::RELIABLE);

	// wait for message to be delivered before shutting down the link:
	Sleep(500);

	join_backlog.destroy();
}

// +--------------------------------------------------------------------+

void
NetGameClient::SendJoinRequest()
{
	if ((NetLayer::GetTime() - join_req_time) < 5000)
	return;

	if (local_player && local_player->GetElement() && server_id) {
		Print("  sending join request - name: '%s' elem: '%s' index: %d\n",
		player_name.data(),
		local_player->GetElement()->Name().data(),
		local_player->GetElementIndex());

		NetJoinRequest join_req;
		join_req.SetName(player_name);
		join_req.SetPassword(player_pass);
		join_req.SetElement(local_player->GetElement()->Name());
		join_req.SetIndex(local_player->GetElementIndex());

		link->SendMessage(server_id, join_req.Pack(), NetJoinRequest::SIZE, NetMsg::RELIABLE);
		join_req_time = NetLayer::GetTime();

		local_player->SetNetObserver(true);
	}
}

// +--------------------------------------------------------------------+

void
NetGameClient::DoJoinRequest(NetMsg* msg)
{
	if (!msg) return;

	NetJoinRequest join_req;
	if (join_req.Unpack(msg->Data())) {
		Print("Client received Join Request from '%s'\n", join_req.GetName());
	}
}

void
NetGameClient::DoJoinAnnounce(NetMsg* msg)
{
	if (!msg) return;

	Sim* sim = Sim::GetSim();
	if (!sim) return;

	NetJoinAnnounce*  join_ann = new(__FILE__,__LINE__) NetJoinAnnounce;
	bool              saved    = false;

	if (join_ann->Unpack(msg->Data())) {
		DWORD nid       = msg->NetID();
		DWORD oid       = join_ann->GetObjID();
		Text  name      = join_ann->GetName();
		Text  elem_name = join_ann->GetElement();
		Text  region    = join_ann->GetRegion();
		Point loc       = join_ann->GetLocation();
		Point velocity  = join_ann->GetVelocity();
		int   index     = join_ann->GetIndex();
		int   shld_lvl  = join_ann->GetShield();
		join_ann->SetNetID(nid);
		Ship* ship      = 0;
		char  ship_name[128];

		strcpy_s(ship_name, Game::GetText("NetGameClient.no-ship").data());

		if (local_player && player_name == name) {
			HUDView::Message(Game::GetText("NetGameClient.local-accept"), name.data(), local_player->Name());

			objid = oid;
			netid = nid;
			local_player->SetObjID(oid);
			local_player->SetNetObserver(false);
			Observe(local_player);

			SimRegion* rgn = local_player->GetRegion();
			if (rgn && region != rgn->Name()) {
				SimRegion* dst = sim->FindRegion(region);
				if (dst) dst->InsertObject(local_player);
			}

			local_player->MoveTo(loc);
			local_player->SetVelocity(velocity);

			Shield* shield = local_player->GetShield();
			if (shield)
			shield->SetNetShieldLevel(shld_lvl);
		}
		else {
			NetPlayer* remote_player = FindPlayerByObjID(oid);
			if (remote_player) {
				remote_player->SetName(name);
				remote_player->SetObjID(oid);

				if (index > 0)
				sprintf_s(ship_name, "%s %d", elem_name.data(), index);
				else
				sprintf_s(ship_name, "%s", elem_name.data());
			}
			else {
				Element* element = sim->FindElement(elem_name);

				if (element) {
					ship = element->GetShip(index);
				}
				else {
					Print("NetGameClient::DoJoinAnnounce() could not find elem %s for player '%s' objid %d\n",
					elem_name.data(), name.data(), oid);

					NetUtil::SendElemRequest(elem_name.data());
				}

				if (!ship) {
					// save it for later:
					join_backlog.append(join_ann);
					saved = true;
				}
				else {
					strcpy_s(ship_name, ship->Name());

					SimRegion* rgn = ship->GetRegion();
					if (rgn && region != rgn->Name()) {
						SimRegion* dst = sim->FindRegion(region);
						if (dst) dst->InsertObject(ship);
					}

					ship->MoveTo(loc);
					ship->SetVelocity(velocity);

					Shield* shield = ship->GetShield();
					if (shield)
					shield->SetNetShieldLevel(shld_lvl);

					NetPlayer* remote_player = new(__FILE__,__LINE__) NetPlayer(nid);
					remote_player->SetName(name);
					remote_player->SetObjID(oid);
					remote_player->SetShip(ship);

					players.append(remote_player);

					if (name == "Server A.I. Ship") {
						Print("Remote Player '%s' has joined as '%s' with ID %d\n", name.data(), ship_name, oid);
					}
					else {
						HUDView::Message(Game::GetText("NetGameClient.remote-join").data(), name.data(), ship_name);
					}
				}
			}
		}
	}

	if (!saved)
	delete join_ann;
}

bool
NetGameClient::DoJoinBacklog(NetJoinAnnounce* join_ann)
{
	bool finished = false;

	if (!join_ann)
	return finished;

	Sim* sim = Sim::GetSim();
	if (!sim)
	return finished;

	DWORD nid       = join_ann->GetNetID();
	DWORD oid       = join_ann->GetObjID();
	Text  name      = join_ann->GetName();
	Text  elem_name = join_ann->GetElement();
	Text  region    = join_ann->GetRegion();
	Point loc       = join_ann->GetLocation();
	Point velocity  = join_ann->GetVelocity();
	int   index     = join_ann->GetIndex();
	int   shld_lvl  = join_ann->GetShield();
	Ship* ship      = 0;
	char  ship_name[128];

	strcpy_s(ship_name, Game::GetText("NetGameClient.no-ship").data());

	if (nid && oid) {
		NetPlayer* remote_player = FindPlayerByObjID(oid);
		if (remote_player) {
			remote_player->SetName(name);
			remote_player->SetObjID(oid);

			if (index > 0)
			sprintf_s(ship_name, "%s %d", elem_name.data(), index);
			else
			sprintf_s(ship_name, "%s", elem_name.data());
		}
		else {
			Element* element = sim->FindElement(elem_name);

			if (element) {
				ship = element->GetShip(index);
			}

			if (ship) {
				strcpy_s(ship_name, ship->Name());

				SimRegion* rgn = ship->GetRegion();
				if (rgn && region != rgn->Name()) {
					SimRegion* dst = sim->FindRegion(region);
					if (dst) dst->InsertObject(ship);
				}

				ship->MoveTo(loc);
				ship->SetVelocity(velocity);

				Shield* shield = ship->GetShield();
				if (shield)
				shield->SetNetShieldLevel(shld_lvl);

				NetPlayer* remote_player = new(__FILE__,__LINE__) NetPlayer(nid);
				remote_player->SetName(name);
				remote_player->SetObjID(oid);
				remote_player->SetShip(ship);

				players.append(remote_player);
				finished = true;

				if (name == "Server A.I. Ship") {
					Print("NetGameClient::DoJoinBacklog() Remote Player '%s' has joined as '%s' with ID %d\n", name.data(), ship_name, oid);
				}
				else {
					HUDView::Message(Game::GetText("NetGameClient.remote-join").data(), name.data(), ship_name);
				}
			}
		}
	}

	return finished;
}


void
NetGameClient::DoQuitRequest(NetMsg* msg)
{
	if (!msg) return;

	Print("Client received Quit Request from NetID: %08X\n", msg->NetID());
}

void
NetGameClient::DoQuitAnnounce(NetMsg* msg)
{
	if (!msg) return;

	NetQuitAnnounce quit_ann;
	quit_ann.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(quit_ann.GetObjID());

	if (player) {
		NetPlayer* zombie = players.remove(player);
		
		// return remote ship to ship pool:
		Ship* s = zombie->GetShip();
		if (s) {
			s->SetNetworkControl(0);
			zombie->SetShip(0);
		}

		if (quit_ann.GetDisconnected())
		HUDView::Message(Game::GetText("NetGameClient.remote-discon").data(), zombie->Name());
		else
		HUDView::Message(Game::GetText("NetGameClient.remote-quit").data(), zombie->Name());
		delete zombie;
	}
	else {
		Print("Quit Announce for unknown player %08X disconnected = %d\n", msg->NetID(), quit_ann.GetDisconnected());
	}
}

void
NetGameClient::DoGameOver(NetMsg* msg)
{
	if (!msg) return;

	HUDView::Message(Game::GetText("NetGameClient.game-over").data());
	players.destroy();
	active = false;
}

void
NetGameClient::DoDisconnect(NetMsg* msg)
{
	if (!msg) return;

	HUDView::Message(Game::GetText("NetGameClient.discon-detect").data());
	HUDView::Message(Game::GetText("NetGameClient.please-exit").data());
	players.destroy();
	active = false;
}

void
NetGameClient::DoObjLoc(NetMsg* msg)
{
	if (!msg) return;

	NetObjLoc obj_loc;
	obj_loc.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(obj_loc.GetObjID());
	if (player)
	player->DoObjLoc(&obj_loc);
}

void
NetGameClient::DoObjDamage(NetMsg* msg)
{
	if (!msg) return;

	NetObjDamage obj_damage;
	obj_damage.Unpack(msg->Data());

	Ship* ship = FindShipByObjID(obj_damage.GetObjID());
	if (ship) {
		Sim*        sim         = Sim::GetSim();
		Shot*       shot        = FindShotByObjID(obj_damage.GetShotID());
		const Ship* owner       = 0;
		const char* owner_name  = "[NET]";

		ship->InflictNetDamage(obj_damage.GetDamage(), shot);

		if (shot && sim) {
			if (shot->Owner()) {
				owner       = shot->Owner();
				owner_name  = owner->Name();
			}

			if (shot->IsMissile()) {
				SimRegion*  region = ship->GetRegion();
				float       scale  = ship->Design()->explosion_scale;

				if (scale <= 0)
				scale = ship->Design()->scale;

				if (owner) {
					const ShipDesign* owner_design = owner->Design();
					if (owner_design && owner_design->scale < scale)
					scale = (float) owner_design->scale;
				}

				sim->CreateExplosion(shot->Location(), Point(), Explosion::SHOT_BLAST, 20.0f * scale, scale, region);
			}

			if (!shot->IsBeam()) {
				if (owner) {
					ShipStats* stats = ShipStats::Find(owner_name);

					if (stats) {
						if (shot->IsPrimary())
						stats->AddGunHit();
						else if (shot->Damage() > 0)
						stats->AddMissileHit();
					}
				}

				shot->Destroy();
			}
		}
	}
}

void
NetGameClient::DoObjKill(NetMsg* msg)
{
	if (!msg) return;

	NetObjKill obj_kill;
	obj_kill.Unpack(msg->Data());

	Ship* ship = FindShipByObjID(obj_kill.GetObjID());
	if (ship) {
		Ship* killer = FindShipByObjID(obj_kill.GetKillerID());
		Text  killer_name = Game::GetText("NetGameClient.unknown");

		if (killer)
		killer_name = killer->Name();

		// log the kill:
		switch (obj_kill.GetKillType()) {
		default:
		case NetObjKill::KILL_MISC:
			Print("Ship '%s' destroyed (misc) (%s)\n", ship->Name(), FormatGameTime());
			break;

		case NetObjKill::KILL_PRIMARY:
		case NetObjKill::KILL_SECONDARY:
			Print("Ship '%s' killed by '%s' (%s)\n", ship->Name(), killer_name.data(), FormatGameTime());
			break;

		case NetObjKill::KILL_COLLISION:
			Print("Ship '%s' killed in collision with '%s' (%s)\n", ship->Name(), killer_name.data(), FormatGameTime());
			break;

		case NetObjKill::KILL_CRASH:
			Print("Ship '%s' destroyed (crash) (%s)\n", ship->Name(), FormatGameTime());

		case NetObjKill::KILL_DOCK:
			Print("Ship '%s' docked (%s)\n", ship->Name(), FormatGameTime());
		}

		// record the kill in the stats:
		if (killer && obj_kill.GetKillType() != NetObjKill::KILL_DOCK) {
			ShipStats* kstats = ShipStats::Find(killer->Name());
			if (kstats) {
				if (obj_kill.GetKillType() == NetObjKill::KILL_PRIMARY)
				kstats->AddEvent(SimEvent::GUNS_KILL, ship->Name());
				
				else if (obj_kill.GetKillType() == NetObjKill::KILL_SECONDARY)
				kstats->AddEvent(SimEvent::MISSILE_KILL, ship->Name());
			}

			if (killer && killer->GetIFF() != ship->GetIFF()) {
				if (ship->GetIFF() > 0 || killer->GetIFF() > 1)
				kstats->AddPoints(ship->Value());
			}
		}

		ShipStats* killee = ShipStats::Find(ship->Name());
		if (killee) {
			if (obj_kill.GetKillType() == NetObjKill::KILL_DOCK)
			killee->AddEvent(SimEvent::DOCK, killer_name);
			else 
			killee->AddEvent(SimEvent::DESTROYED, killer_name);
		}

		if (obj_kill.GetKillType() == NetObjKill::KILL_DOCK) {
			FlightDeck* deck = killer->GetFlightDeck(obj_kill.GetFlightDeck());
			sim->NetDockShip(ship, killer, deck);
		}
		else {
			ship->InflictNetDamage(ship->Integrity());
			ship->DeathSpiral();

			if (!obj_kill.GetRespawn())
			ship->SetRespawnCount(0);
			else
			ship->SetRespawnLoc(obj_kill.GetRespawnLoc());
		}
	}

	// this shouldn't happen in practice, 
	// but if it does, this is what should be done:
	else {
		Shot* shot = FindShotByObjID(obj_kill.GetObjID());

		if (shot) {
			::Print("NetGameClient::DoObjKill destroying shot '%s'\n", shot->Name());
			shot->Destroy();
		}
	}
}

void
NetGameClient::DoObjSpawn(NetMsg* msg)
{
}

void
NetGameClient::DoObjHyper(NetMsg* msg)
{
	if (!msg) return;
	Print("Client received OBJ HYPER from NetID: %08x\n", msg->NetID());

	NetObjHyper obj_hyper;
	obj_hyper.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(obj_hyper.GetObjID());
	if (player && player->GetShip())
	player->DoObjHyper(&obj_hyper);
}

void
NetGameClient::DoObjTarget(NetMsg* msg)
{
	if (!msg) return;

	NetObjTarget obj_target;
	obj_target.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(obj_target.GetObjID());
	if (player)
	player->DoObjTarget(&obj_target);
}

void
NetGameClient::DoObjEmcon(NetMsg* msg)
{
	if (!msg) return;

	NetObjEmcon obj_emcon;
	obj_emcon.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(obj_emcon.GetObjID());
	if (player)
	player->DoObjEmcon(&obj_emcon);
}

// +--------------------------------------------------------------------+

void
NetGameClient::DoSysDamage(NetMsg* msg)
{
	if (!msg) return;

	NetSysDamage sys_damage;
	sys_damage.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(sys_damage.GetObjID());
	if (player && player->GetShip())
	player->DoSysDamage(&sys_damage);
}

void
NetGameClient::DoSysStatus(NetMsg* msg)
{
	if (!msg) return;

	NetSysStatus sys_status;
	sys_status.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(sys_status.GetObjID());
	if (player && player->GetShip())
	player->DoSysStatus(&sys_status);
}

// +--------------------------------------------------------------------+

void
NetGameClient::DoElemCreate(NetMsg* msg)
{
	if (!msg) return;

	NetElemCreate elem_create;
	elem_create.Unpack(msg->Data());

	const char* elem_name = elem_create.GetName().data();

	::Print("NetGameClient::DoElemCreate name: %s iff: %d type %s\n",
	elem_name,
	elem_create.GetIFF(),
	Mission::RoleName(elem_create.GetType()));

	Sim*     sim   = Sim::GetSim();
	Element* elem  = sim->FindElement(elem_name);
	if (elem) {
		::Print("  element '%' already exists - ignored\n", elem_name);
		return;
	}

	elem = sim->CreateElement(elem_name,
	elem_create.GetIFF(),
	elem_create.GetType());

	int*     load     = elem_create.GetLoadout();
	int*     slots    = elem_create.GetSlots();
	int      squadron = elem_create.GetSquadron();
	int      code     = elem_create.GetObjCode();
	Text     target   = elem_create.GetObjective();
	bool     alert    = elem_create.GetAlert();
	bool     active   = elem_create.GetInFlight();

	elem->SetIntelLevel(elem_create.GetIntel());
	elem->SetLoadout(load);

	if (code > Instruction::RTB || target.length() > 0) {
		Instruction* obj  = new(__FILE__,__LINE__) Instruction(code, target);
		elem->AddObjective(obj);
	}

	Ship* carrier = sim->FindShip(elem_create.GetCarrier());
	if (carrier) {
		elem->SetCarrier(carrier);

		Hangar* hangar = carrier->GetHangar();
		if (hangar) {
			Text squadron_name = hangar->SquadronName(squadron);
			elem->SetSquadron(squadron_name);

			if (active) {
				for (int i = 0; i < 4; i++) {
					int slot = slots[i];
					if (slot > -1) {
						hangar->GotoActiveFlight(squadron, slot, elem, load);
					}
				}
			}

			else {
				FlightDeck* deck   = 0;
				int         queue  = 1000;

				for (int i = 0; i < carrier->NumFlightDecks(); i++) {
					FlightDeck* d = carrier->GetFlightDeck(i);

					if (d && d->IsLaunchDeck()) {
						int dq = hangar->PreflightQueue(d);

						if (dq < queue) {
							queue = dq;
							deck  = d;
						}
					}
				}

				for (int i = 0; i < 4; i++) {
					int slot = slots[i];
					if (slot > -1) {
						hangar->GotoAlert(squadron, slot, deck, elem, load, !alert);
					}
				}
			}
		}
	}
}

void
NetGameClient::DoShipLaunch(NetMsg* msg)
{
	if (!msg) return;

	NetShipLaunch ship_launch;
	ship_launch.Unpack(msg->Data());

	Sim*  sim      = Sim::GetSim();
	int   squadron = ship_launch.GetSquadron();
	int   slot     = ship_launch.GetSlot();

	Ship* carrier = FindShipByObjID(ship_launch.GetObjID());

	if (carrier) {
		Hangar* hangar = carrier->GetHangar();

		if (hangar) {
			hangar->Launch(squadron, slot);
		}
	}
}

void
NetGameClient::DoWepTrigger(NetMsg* msg)
{
	if (!msg) return;

	NetWepTrigger trigger;
	trigger.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(trigger.GetObjID());
	if (player)
	player->DoWepTrigger(&trigger);
}

void
NetGameClient::DoWepRelease(NetMsg* msg)
{
	if (!msg) return;

	NetWepRelease release;
	release.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(release.GetObjID());
	if (player) {
		player->DoWepRelease(&release);
	}

	else {
		Ship* shooter = FindShipByObjID(release.GetObjID());
		if (shooter) {
			int   index = release.GetIndex();
			DWORD tgtid = release.GetTgtID();
			DWORD wepid = release.GetWepID();
			int   subid = release.GetSubtarget();
			bool  decoy = release.GetDecoy();
			bool  probe = release.GetProbe();

			Weapon* w = 0;

			if (decoy)        w = shooter->GetDecoy();
			else if (probe)   w = shooter->GetProbeLauncher();
			else              w = shooter->GetWeaponByIndex(index);

			if (w && !w->IsPrimary()) {
				SimObject*  target = FindShipByObjID(tgtid);
				System*     subtgt = 0;

				if (target) {
					if (subid >= 0) {
						Ship* tgt_ship = (Ship*) target;
						subtgt = tgt_ship->Systems().at(subid);
					}
				}
				else {
					target = FindShotByObjID(tgtid);
				}

				Shot* shot = w->NetFireSecondary(target, subtgt, wepid);

				if (shot && shot->IsDrone()) {
					if (probe)
					shooter->SetProbe((Drone*) shot);

					else if (decoy)
					shooter->AddActiveDecoy((Drone*) shot);
				}
			}
		}
	}
}

void
NetGameClient::DoNavData(NetMsg* msg)
{
	if (!msg) return;

	NetNavData nav_data;
	nav_data.Unpack(msg->Data());

	Element* elem = sim->FindElement(nav_data.GetElem());
	Ship*    ship = FindShipByObjID(nav_data.GetObjID());

	if (elem) {
		if (nav_data.IsAdd()) {
			Instruction* navpt = new(__FILE__,__LINE__) Instruction(*nav_data.GetNavPoint());
			Instruction* after = 0;
			int          index = nav_data.GetIndex();

			if (index >= 0 && index < elem->GetFlightPlan().size())
			after = elem->GetFlightPlan().at(index);

			elem->AddNavPoint(navpt, after, false);
		}

		else {
			Instruction* navpt = nav_data.GetNavPoint();
			Instruction* exist = 0;
			int          index = nav_data.GetIndex();

			if (navpt && index >= 0 && index < elem->GetFlightPlan().size()) {
				exist = elem->GetFlightPlan().at(index);

				*exist = *navpt;
			}
		}
	}
}

void
NetGameClient::DoNavDelete(NetMsg* msg)
{
	if (!msg) return;

	NetNavDelete nav_delete;
	nav_delete.Unpack(msg->Data());

	Element* elem = sim->FindElement(nav_delete.GetElem());
	Ship*    ship = FindShipByObjID(nav_delete.GetObjID());

	if (elem) {
		int index = nav_delete.GetIndex();

		if (index < 0) {
			elem->ClearFlightPlan(false);
		}

		else if (index < elem->FlightPlanLength()) {
			Instruction* npt = elem->GetFlightPlan().at(index);
			elem->DelNavPoint(npt, false);
		}
	}
}

void
NetGameClient::DoWepDestroy(NetMsg* msg)
{
	if (!msg) return;

	NetWepDestroy destroy;
	destroy.Unpack(msg->Data());

	Shot* shot = FindShotByObjID(destroy.GetObjID());
	if (shot) {
		if (shot->IsBeam())
		::Print("NetGameClient::DoWepDestroy shot '%s'\n", shot->Name());

		shot->Destroy();
	}
}

void
NetGameClient::DoCommMsg(NetMsg* msg)
{
	if (!msg) return;

	NetCommMsg comm_msg;
	comm_msg.Unpack(msg->Data());

	NetPlayer* player = FindPlayerByObjID(comm_msg.GetObjID());
	if (player)
	player->DoCommMessage(&comm_msg);
}

void
NetGameClient::DoChatMsg(NetMsg* msg)
{
	if (!msg) return;

	NetChatMsg chat_msg;
	chat_msg.Unpack(msg->Data());

	Text name = chat_msg.GetName();
	if (name.length() < 1)
	name = Game::GetText("NetGameClient.chat.unknown");

	HUDView::Message("%s> %s", name.data(), chat_msg.GetText().data());
}

void
NetGameClient::DoSelfDestruct(NetMsg* msg)
{
	if (!msg) return;

	NetSelfDestruct self_destruct;
	self_destruct.Unpack(msg->Data());

	Ship* ship = FindShipByObjID(self_destruct.GetObjID());
	if (ship) {
		ship->InflictNetDamage(self_destruct.GetDamage());
	}
}

// +--------------------------------------------------------------------+

void
NetGameClient::Send()
{
	DWORD time = Game::GameTime();

	// don't flood the network...
	if (time - last_send_time < MIN_NET_FRAME)
	return;

	last_send_time = time;

	if (local_player && objid && server_id) {
		double r, p, y;
		local_player->Cam().Orientation().ComputeEulerAngles(r,p,y);

		NetObjLoc obj_loc;

		obj_loc.SetObjID(objid);
		obj_loc.SetLocation(local_player->Location());
		obj_loc.SetVelocity(local_player->Velocity());
		obj_loc.SetOrientation(Point(r,p,y));
		obj_loc.SetThrottle(local_player->Throttle() > 10);
		obj_loc.SetAugmenter(local_player->Augmenter());
		obj_loc.SetGearDown(local_player->IsGearDown());

		Shield* shield = local_player->GetShield();
		if (shield)
		obj_loc.SetShield((int) shield->GetPowerLevel());
		else
		obj_loc.SetShield(0);

		BYTE* obj_loc_data = obj_loc.Pack();

		link->SendMessage(server_id, obj_loc_data, NetObjLoc::SIZE);
	}
}

// +--------------------------------------------------------------------+

void
NetGameClient::SendData(NetData* net_data)
{
	if (!net_data || !server_id)
	return;

	if (local_player || net_data->Type() < 0x20) {
		BYTE* data  = net_data->Pack();
		BYTE  flags = 0;

		if (net_data->Type() >= 0x10)
		flags |= NetMsg::RELIABLE;

		link->SendMessage(server_id, data, net_data->Length(), flags);
	}
}

// +--------------------------------------------------------------------+

void
NetGameClient::ExecFrame()
{
	if (local_player) {
		if (local_player->GetObjID() == 0) {
			SendJoinRequest();
		}

		else if (active) {
			// check health of server:
			NetPeer* server_peer = link->FindPeer(server_id);
			if (server_peer && (NetLayer::GetUTC() - server_peer->LastReceiveTime() > 15)) {
				NetMsg net_disco(0, NET_DISCONNECT, 0, 0, 0);
				DoDisconnect(&net_disco);
			}

			// if server is still there,
			else if (server_peer) {

				// check if any old join announcements still need to be processed:
				ListIter<NetJoinAnnounce> iter = join_backlog;
				while (++iter) {
					NetJoinAnnounce* join_ann = iter.value();

					if (DoJoinBacklog(join_ann)) {
						iter.removeItem();
						delete join_ann;
					}
				}
			}
		}
	}

	NetGame::ExecFrame();
}

// +--------------------------------------------------------------------+

bool
NetGameClient::Update(SimObject* obj)
{
	if (obj->Type() == SimObject::SIM_SHIP) {
		Ship* s = (Ship*) obj;
		if (local_player == s)
		local_player = 0;
	}

	return SimObserver::Update(obj);
}

const char*
NetGameClient::GetObserverName() const
{
	return "NetGameClient";
}

// +--------------------------------------------------------------------+

void
NetGameClient::Respawn(DWORD oid, Ship* spawn)
{
	if (!oid || !spawn) return;

	Print("NetGameClient::Respawn(%d, %s)\n", oid, spawn->Name());
	spawn->SetObjID(oid);
	Observe(spawn);

	NetPlayer* p = FindPlayerByObjID(oid);
	if (p)
	p->SetShip(spawn);

	if (objid == oid) {
		Print("  RESPAWN LOCAL PLAYER\n\n");
		local_player = spawn;
	}
}
