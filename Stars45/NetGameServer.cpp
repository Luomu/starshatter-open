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
    FILE:         NetGameServer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Server-Side Network Game Manager class
*/

#include "MemDebug.h"
#include "NetGameServer.h"
#include "NetServerConfig.h"
#include "NetLobbyServer.h"
#include "NetPlayer.h"
#include "NetUser.h"
#include "NetMsg.h"
#include "NetData.h"
#include "StarServer.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shield.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Element.h"
#include "HUDView.h"
#include "RadioMessage.h"
#include "RadioView.h"
#include "Instruction.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "Mission.h"

#include "NetLayer.h"
#include "NetHost.h"
#include "NetPeer.h"
#include "NetUtil.h"
#include "Game.h"
#include "Light.h"

// +--------------------------------------------------------------------+

const int MAX_NET_FPS   = 20;
const int MIN_NET_FRAME = 1000 / MAX_NET_FPS;

// +--------------------------------------------------------------------+

NetGameServer::NetGameServer()
{
    Print("Constructing NetGameServer\n");

    WORD    server_port = 11101;

    if (NetServerConfig::GetInstance())
    server_port = NetServerConfig::GetInstance()->GetGamePort();

    NetAddr server(NetHost().Address().IPAddr(), server_port);

    link = new(__FILE__,__LINE__) NetLink(server);

    ListIter<SimRegion> rgn_iter = sim->GetRegions();
    while (++rgn_iter) {
        SimRegion* rgn = rgn_iter.value();

        ListIter<Ship> iter = rgn->Ships();
        while (++iter) {
            Ship* s = iter.value();
            s->SetObjID(GetNextObjID());
            Observe(s);
            ships.append(s);
        }
    }

    if (local_player) {
        Observe(local_player);
        objid = local_player->GetObjID();
    }
}

NetGameServer::~NetGameServer()
{
    ListIter<NetPlayer> player = players;
    while (++player) {
        NetPlayer* p = player.value();

        if (p->GetShip())
        p->GetShip()->SetRespawnCount(0);

        link->SendMessage(p->GetNetID(), NET_GAME_OVER, 0, 0, NetMsg::RELIABLE);
    }

    Sleep(500);

    ListIter<Ship> iter = ships;
    while (++iter) {
        Ship* s = iter.value();
        s->SetRespawnCount(0);
    }

    zombies.destroy();
    ships.clear();
}

// +--------------------------------------------------------------------+

void
NetGameServer::ExecFrame()
{
    NetGame::ExecFrame();
    CheckSessions();

    ListIter<SimRegion> rgn_iter = sim->GetRegions();
    while (++rgn_iter) {
        SimRegion* rgn = rgn_iter.value();

        ListIter<Ship> iter = rgn->Ships();
        while (++iter) {
            Ship* s = iter.value();

            if (s->GetObjID() == 0) {
                s->SetObjID(GetNextObjID());
                Observe(s);
                ships.append(s);

                NetJoinAnnounce join_ann;
                join_ann.SetShip(s);
                join_ann.SetName("Server A.I. Ship");
                SendData(&join_ann);
            }
        }
    }


    static DWORD time_mark = 0;

    if (!time_mark) time_mark = Game::RealTime();
    else if (Game::RealTime() - time_mark > 60000) {
        time_mark = Game::RealTime();

        if (link && players.size() > 0) {
            Print("Server Stats\n-------------\n");
            Print("  packets sent %d\n",          link->GetPacketsSent());
            Print("  packets recv %d\n",          link->GetPacketsRecv());
            Print("  bytes sent   %d\n",          link->GetBytesSent());
            Print("  bytes recv   %d\n",          link->GetBytesRecv());
            Print("  retries      %d\n",          link->GetRetries());
            Print("  drops        %d\n",          link->GetDrops());
            Print("  avg lag      %d msec\n",     link->GetLag());
        }
    }
}

void
NetGameServer::CheckSessions()
{
    if (!link)
    return;

    ListIter<NetPlayer> iter = players;
    while (++iter) {
        NetPlayer* player = iter.value();
        NetPeer*   peer   = link->FindPeer(player->GetNetID());

        if (peer && (NetLayer::GetUTC() - peer->LastReceiveTime()) > NET_DISCONNECT_TIME) {
            // announce drop:
            NetPlayer* zombie = iter.removeItem();
            HUDView::Message(Game::GetText("NetGameServer.remote-discon").data(), zombie->Name());

            // tell everyone else:
            NetQuitAnnounce quit_ann;
            quit_ann.SetObjID(zombie->GetObjID());
            quit_ann.SetDisconnected(true);
            SendData(&quit_ann);

            // return remote ship to ship pool:
            Ship* s = zombie->GetShip();
            if (s) {
                Observe(s);
                ships.append(s);
                s->SetNetworkControl(0);
                zombie->SetShip(0);

                NetJoinAnnounce join_ann;
                join_ann.SetShip(s);
                join_ann.SetName("Server A.I. Ship");
                SendData(&join_ann);
            }

            zombies.append(zombie);
        }
    }
}

NetPlayer*
NetGameServer::FindZombieByObjID(DWORD objid)
{
    for (int i = 0; i < zombies.size(); i++) {
        NetPlayer* p = zombies[i];

        if (p->GetObjID() == objid)
        return p;
    }

    return 0;
}

// +--------------------------------------------------------------------+

void
NetGameServer::DoJoinRequest(NetMsg* msg)
{
    if (!msg) return;

    bool unpause = players.isEmpty();

    NetJoinRequest join_req;
    if (join_req.Unpack(msg->Data())) {
        HUDView::Message(Game::GetText("NetGameServer::join-request").data(), join_req.GetName(), join_req.GetElement(), join_req.GetIndex());

        DWORD nid       = msg->NetID();
        Text  name      = join_req.GetName();
        Text  serno     = join_req.GetSerialNumber();
        Text  elem_name = join_req.GetElement();
        int   index     = join_req.GetIndex();
        Ship* ship      = 0;
        Sim*  sim       = Sim::GetSim();

        if (sim) {
            Element* element = sim->FindElement(elem_name);

            if (element)
            ship = element->GetShip(index);
        }

        if (!ship) {
            Print("  JOIN DENIED: could not locate ship for remote player\n");
            return;
        }

        if (!ship->GetObjID()) {
            Print("  JOIN DENIED: remote player requested ship with objid = 0\n");
            return;
        }

        NetLobbyServer* lobby = NetLobbyServer::GetInstance();

        if (lobby) {
            NetUser* user = lobby->FindUserByName(name);

            if (!user)
            user = lobby->FindUserByNetID(nid);

            if (!user) {
                Print("  JOIN DENIED: remote player '%s' not found in lobby\n", name.data());
                return;
            }

            else if (!user->IsAuthOK()) {
                Print("  JOIN DENIED: remote player '%s' not authenticated\n", name.data());
                return;
            }
        }

        NetPlayer*  remote_player = FindPlayerByNetID(nid);
        if (remote_player && remote_player->GetShip() != ship) {
            Print("  disconnecting remote player from ship '%s'\n", ship->Name());
            players.remove(remote_player);
            delete remote_player;
            remote_player = 0;
        }

        if (!remote_player) {
            Ignore(ship);
            ships.remove(ship);

            remote_player = new(__FILE__,__LINE__) NetPlayer(nid);
            remote_player->SetName(name);
            remote_player->SetSerialNumber(serno);
            remote_player->SetObjID(ship->GetObjID());
            remote_player->SetShip(ship);

            HUDView::Message(Game::GetText("NetGameServer::join-announce").data());
            Print("remote player name = %s\n", name.data());
            Print("              obj  = %d\n", ship->GetObjID());
            Print("              ship = %s\n", ship->Name());

            remote_player->SetObjID(ship->GetObjID());

            // tell the new player about the server:
            if (local_player) {
                NetJoinAnnounce join_ann;
                join_ann.SetShip(local_player);
                join_ann.SetName(player_name);
                link->SendMessage(remote_player->GetNetID(), join_ann.Pack(), NetJoinAnnounce::SIZE, NetMsg::RELIABLE);
            }

            // tell the new player about the existing remote players:
            ListIter<NetPlayer> iter = players;
            while (++iter) {
                Ship* s = iter->GetShip();

                if (s) {
                    NetJoinAnnounce join_ann;
                    join_ann.SetShip(s);
                    join_ann.SetName(iter->Name());
                    join_ann.SetObjID(iter->GetObjID());
                    link->SendMessage(remote_player->GetNetID(), join_ann.Pack(), NetJoinAnnounce::SIZE, NetMsg::RELIABLE);
                }
            }

            // tell the new player about the A.I. controlled ships:
            ListIter<Ship> ai_iter = ships;
            while (++ai_iter) {
                Ship* s = ai_iter.value();
                if (s != local_player) {
                    NetJoinAnnounce join_ann;
                    join_ann.SetShip(s);
                    join_ann.SetName("Server A.I. Ship");
                    link->SendMessage(remote_player->GetNetID(), join_ann.Pack(), NetJoinAnnounce::SIZE, NetMsg::RELIABLE);
                }
            }

            // make the new player an "existing" remote player:
            players.append(remote_player);

            // tell existing players about the new player:
            // NOTE, this also provides the net id to the new player!
            iter.reset();
            while (++iter) {
                Ship* s = remote_player->GetShip();

                NetJoinAnnounce join_ann;
                join_ann.SetShip(s);
                join_ann.SetName(remote_player->Name());
                join_ann.SetObjID(remote_player->GetObjID());
                link->SendMessage(iter->GetNetID(), join_ann.Pack(), NetJoinAnnounce::SIZE, NetMsg::RELIABLE);
            }

            if (unpause) {
                StarServer* s = StarServer::GetInstance();
                if (s)
                s->Pause(false);
            }
        }
    }
}

void
NetGameServer::DoJoinAnnounce(NetMsg* msg)
{
    if (!msg) return;

    NetJoinAnnounce join_ann;
    if (join_ann.Unpack(msg->Data())) {
        Print("Server received Join Announce from '%s'\n", join_ann.GetName());
    }
}

void
NetGameServer::DoQuitRequest(NetMsg* msg)
{
    if (!msg) return;

    NetPlayer* player = FindPlayerByNetID(msg->NetID());

    if (player) {
        NetPlayer* zombie = players.remove(player);
        HUDView::Message(Game::GetText("NetGameServer.remote-quit").data(), zombie->Name());

        // tell everyone else:
        NetQuitAnnounce quit_ann;
        quit_ann.SetObjID(zombie->GetObjID());
        SendData(&quit_ann);

        // return remote ship to ship pool:
        Ship* s = zombie->GetShip();
        if (s) {
            Observe(s);
            ships.append(s);
            s->SetNetworkControl(0);
            zombie->SetShip(0);

            NetJoinAnnounce join_ann;
            join_ann.SetShip(s);
            join_ann.SetName("Server A.I. Ship");
            SendData(&join_ann);
        }

        zombies.append(zombie);
    }
    else {
        Print("Quit Request from unknown player NetID: %08X\n", msg->NetID());
    }
}

void
NetGameServer::DoQuitAnnounce(NetMsg* msg)
{
    if (!msg) return;
    Print("Server received Quit Announce from NetID: %08x\n", msg->NetID());
}

void
NetGameServer::DoGameOver(NetMsg* msg)
{
    if (!msg) return;
    Print("Server received Game Over from NetID: %08x\n", msg->NetID());
}

void
NetGameServer::DoDisconnect(NetMsg* msg)
{
    if (!msg) return;
    Print("Server received Disconnect from NetID: %08x\n", msg->NetID());
}

void
NetGameServer::DoObjLoc(NetMsg* msg)
{
    if (!msg) return;

    NetObjLoc obj_loc;
    obj_loc.Unpack(msg->Data());

    NetPlayer* player = FindPlayerByObjID(obj_loc.GetObjID());
    if (player && player->GetShip()) {
        player->DoObjLoc(&obj_loc);
    }

    else {
        player = FindZombieByObjID(obj_loc.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

void
NetGameServer::DoObjDamage(NetMsg* msg)
{
    if (!msg) return;
    Print("Server received OBJ DAMAGE from NetID: %08x (ignored)\n", msg->NetID());
}

void
NetGameServer::DoObjKill(NetMsg* msg)
{
    if (!msg) return;

    NetObjKill obj_kill;
    obj_kill.Unpack(msg->Data());

    if (obj_kill.GetKillType() != NetObjKill::KILL_DOCK) {
        Print("Server received OBJ KILL from NetID: %08x (ignored)\n", msg->NetID());
        return;
    }

    Ship* ship = FindShipByObjID(obj_kill.GetObjID());
    if (ship) {
        Ship* killer = FindShipByObjID(obj_kill.GetKillerID());
        Text  killer_name = Game::GetText("NetGameServer.unknown");

        if (killer)
        killer_name = killer->Name();

        ShipStats* killee = ShipStats::Find(ship->Name());
        if (killee)
        killee->AddEvent(SimEvent::DOCK, killer_name);

        FlightDeck* deck = killer->GetFlightDeck(obj_kill.GetFlightDeck());
        sim->NetDockShip(ship, killer, deck);
    }
}

void
NetGameServer::DoObjSpawn(NetMsg* msg)
{
    if (!msg) return;
    Print("Server received OBJ SPAWN from NetID: %08x (ignored)\n", msg->NetID());
}

void
NetGameServer::DoObjHyper(NetMsg* msg)
{
    if (!msg) return;
    Print("Server received OBJ HYPER from NetID: %d\n", msg->NetID());

    NetObjHyper obj_hyper;
    obj_hyper.Unpack(msg->Data());

    NetPlayer* player = FindPlayerByObjID(obj_hyper.GetObjID());
    if (player && player->GetShip()) {
        if (player->DoObjHyper(&obj_hyper)) {
            SendData(&obj_hyper);
        }
    }
    else {
        player = FindZombieByObjID(obj_hyper.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

void
NetGameServer::DoObjTarget(NetMsg* msg)
{
    if (!msg) return;

    NetObjTarget obj_target;
    obj_target.Unpack(msg->Data());

    NetPlayer* player = FindPlayerByObjID(obj_target.GetObjID());
    if (player) {
        player->DoObjTarget(&obj_target);
    }
    else {
        player = FindZombieByObjID(obj_target.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

void
NetGameServer::DoObjEmcon(NetMsg* msg)
{
    if (!msg) return;

    NetObjEmcon obj_emcon;
    obj_emcon.Unpack(msg->Data());

    NetPlayer* player = FindPlayerByObjID(obj_emcon.GetObjID());
    if (player) {
        player->DoObjEmcon(&obj_emcon);
    }
    else {
        player = FindZombieByObjID(obj_emcon.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

// +--------------------------------------------------------------------+

void
NetGameServer::DoSysDamage(NetMsg* msg)
{
    if (!msg) return;

    NetSysDamage sys_damage;
    sys_damage.Unpack(msg->Data());

    NetPlayer* player = FindZombieByObjID(sys_damage.GetObjID());

    if (player)
    SendDisconnect(player);
}

void
NetGameServer::DoSysStatus(NetMsg* msg)
{
    if (!msg) return;

    NetSysStatus sys_status;
    sys_status.Unpack(msg->Data());

    Ship*       ship     = FindShipByObjID(sys_status.GetObjID());
    NetPlayer*  player   = FindPlayerByNetID(msg->NetID());

    if (ship) {
        if (!player || ship->GetObjID() != player->GetObjID()) {
            /**
        Print("NetGameServer::DoSysStatus - received request for ship '%s' from wrong player %s\n",
            ship->Name(), player ? player->Name() : "null");
        **/

            return;
        }

        player->DoSysStatus(&sys_status);

        // rebroadcast:
        System* sys = ship->GetSystem(sys_status.GetSystem());
        NetUtil::SendSysStatus(ship, sys);
    }

    else {
        player = FindZombieByObjID(sys_status.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

// +--------------------------------------------------------------------+

void
NetGameServer::DoElemRequest(NetMsg* msg)
{
    if (!msg) return;

    NetElemRequest elem_request;
    elem_request.Unpack(msg->Data());

    Sim*     sim   = Sim::GetSim();
    Element* elem  = sim->FindElement(elem_request.GetName());

    if (elem) {
        int   squadron = -1;
        int   slots[]  = { -1,-1,-1,-1 };
        Ship* carrier  = elem->GetCarrier();

        if (carrier && carrier->GetHangar()) {
            Hangar* hangar = carrier->GetHangar();

            for (int i = 0; i < 4; i++) {
                hangar->FindSquadronAndSlot(elem->GetShip(i+1), squadron, slots[i]);
            }
        }

        NetUtil::SendElemCreate(elem, squadron, slots, false, true);
    }
}

// +--------------------------------------------------------------------+

void
NetGameServer::DoElemCreate(NetMsg* msg)
{
    if (!msg) return;

    NetElemCreate elem_create;
    elem_create.Unpack(msg->Data());

    Sim*     sim   = Sim::GetSim();
    Element* elem  = sim->CreateElement(elem_create.GetName(),
    elem_create.GetIFF(),
    elem_create.GetType());

    int*     load     = elem_create.GetLoadout();
    int*     slots    = elem_create.GetSlots();
    int      squadron = elem_create.GetSquadron();
    int      code     = elem_create.GetObjCode();
    Text     target   = elem_create.GetObjective();
    bool     alert    = elem_create.GetAlert();

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

    NetUtil::SendElemCreate(elem,
    elem_create.GetSquadron(),
    elem_create.GetSlots(),
    elem_create.GetAlert());
}

void
NetGameServer::DoShipLaunch(NetMsg* msg)
{
    if (!msg) return;

    NetShipLaunch ship_launch;
    ship_launch.Unpack(msg->Data());

    Sim*  sim      = Sim::GetSim();
    int   squadron = ship_launch.GetSquadron();
    int   slot     = ship_launch.GetSlot();

    NetPlayer* player = FindPlayerByObjID(ship_launch.GetObjID());
    if (player) {
        Ship* carrier = player->GetShip();

        if (carrier) {
            Hangar* hangar = carrier->GetHangar();

            if (hangar) {
                hangar->Launch(squadron, slot);
            }

            NetUtil::SendShipLaunch(carrier, squadron, slot);
        }
    }
}

void
NetGameServer::DoNavData(NetMsg* msg)
{
    if (!msg) return;

    NetNavData nav_data;
    nav_data.Unpack(msg->Data());

    Element* elem = sim->FindElement(nav_data.GetElem());

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

        SendData(&nav_data);
    }
}

void
NetGameServer::DoNavDelete(NetMsg* msg)
{
    if (!msg) return;

    NetNavDelete nav_delete;
    nav_delete.Unpack(msg->Data());

    Element* elem = sim->FindElement(nav_delete.GetElem());

    if (elem) {
        int index = nav_delete.GetIndex();

        if (index < 0) {
            elem->ClearFlightPlan(false);
        }

        else if (index < elem->FlightPlanLength()) {
            Instruction* npt = elem->GetFlightPlan().at(index);
            elem->DelNavPoint(npt, false);
        }

        SendData(&nav_delete);
    }
}

void
NetGameServer::DoWepTrigger(NetMsg* msg)
{
    if (!msg) return;

    NetWepTrigger trigger;
    trigger.Unpack(msg->Data());

    NetPlayer* player = FindPlayerByObjID(trigger.GetObjID());
    if (player) {
        player->DoWepTrigger(&trigger);
    }
    else {
        player = FindZombieByObjID(trigger.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

void
NetGameServer::DoWepRelease(NetMsg* msg)
{
    if (!msg) return;

    NetWepRelease release;
    release.Unpack(msg->Data());

    NetPlayer* player = FindPlayerByObjID(release.GetObjID());
    if (player) {
        player->DoWepRelease(&release);
    }
    else {
        player = FindZombieByObjID(release.GetObjID());

        if (player)
        SendDisconnect(player);
    }

    Print("WEP RELEASE on server? objid = %d\n", release.GetObjID());
}

void
NetGameServer::DoWepDestroy(NetMsg* msg)
{
}

void
NetGameServer::DoCommMsg(NetMsg* msg)
{
    if (!msg) return;

    NetCommMsg comm_msg;
    comm_msg.Unpack(msg->Data());

    RadioMessage* radio_msg = comm_msg.GetRadioMessage();

    NetPlayer* player = FindPlayerByObjID(comm_msg.GetObjID());
    if (player && radio_msg) {
        player->DoCommMessage(&comm_msg);

        int channel = comm_msg.GetRadioMessage()->Channel();

        ListIter<NetPlayer> dst = players;
        while (++dst) {
            NetPlayer* remote_player = dst.value();
            if (remote_player->GetNetID() &&
                    (channel == 0 || channel == remote_player->GetIFF())) {

                BYTE* data  = comm_msg.Pack();
                int   size  = comm_msg.Length();

                link->SendMessage(remote_player->GetNetID(), data, size, NetMsg::RELIABLE);
            }
        }
    }
    else {
        player = FindZombieByObjID(comm_msg.GetObjID());

        if (player)
        SendDisconnect(player);
    }
}

void
NetGameServer::DoChatMsg(NetMsg* msg)
{
    if (!msg) return;

    NetChatMsg chat_msg;
    chat_msg.Unpack(msg->Data());

    RouteChatMsg(chat_msg);
}

void
NetGameServer::RouteChatMsg(NetChatMsg& chat_msg)
{
    DWORD dst_id = chat_msg.GetDstID();

    // broadcast or team:
    if (dst_id == 0xffff || dst_id <= 10) {
        BYTE* data  = chat_msg.Pack();
        int   size  = chat_msg.Length();

        ListIter<NetPlayer> dst = players;
        while (++dst) {
            NetPlayer* remote_player = dst.value();
            if (remote_player->GetNetID() && chat_msg.GetName() != remote_player->Name()) {
                if (dst_id == 0xffff || dst_id == 0 || remote_player->GetIFF() == (int) dst_id-1)
                link->SendMessage(remote_player->GetNetID(), data, size);
            }
        }

        if (local_player && (dst_id == 0xffff || dst_id == 0 || local_player->GetIFF() == (int) dst_id-1)) {
            Text name = chat_msg.GetName();
            if (name.length() < 1)
            name = Game::GetText("NetGameServer.chat.unknown");

            // don't echo general messages from the local player.
            // they are already displayed by the chat entry code
            // in starshatter.cpp

            if (name != player_name)
            HUDView::Message("%s> %s", name.data(), chat_msg.GetText().data());
        }
    }

    // direct to local player:
    else if (local_player && local_player->GetObjID() == dst_id) {
        Text name = chat_msg.GetName();
        if (name.length() < 1)
        name = Game::GetText("NetGameServer.chat.unknown");

        HUDView::Message("%s> %s", name.data(), chat_msg.GetText().data());
    }

    // ship-to-ship, but not to local player:
    else if (!local_player || local_player->GetObjID() != dst_id) {
        NetPlayer* remote_player = FindPlayerByObjID(dst_id);
        if (remote_player && remote_player->GetNetID()) {
            BYTE* data  = chat_msg.Pack();
            int   size  = chat_msg.Length();
            link->SendMessage(remote_player->GetNetID(), data, size);
        }

        // record message in server log:
        ::Print("%s> %s\n", chat_msg.GetName().data(), chat_msg.GetText().data());
    }

    if (dst_id == 0xffff)
    return;

    // record message in chat log:
    NetLobbyServer*   lobby  = NetLobbyServer::GetInstance();

    if (!lobby)
    return;

    NetUser* user = lobby->FindUserByName(chat_msg.GetName());

    if (user)
    lobby->AddChat(user, chat_msg.GetText(), false);   // don't re-route
}

// +--------------------------------------------------------------------+

const char* FormatGameTime();

void
NetGameServer::DoSelfDestruct(NetMsg* msg)
{
    if (!msg) return;

    NetSelfDestruct self_destruct;
    self_destruct.Unpack(msg->Data());

    Ship*       ship     = FindShipByObjID(self_destruct.GetObjID());
    NetPlayer*  player   = FindPlayerByNetID(msg->NetID());

    if (ship) {
        if (!player || ship->GetObjID() != player->GetObjID()) {
            Print("NetGameServer::DoSelfDestruct - received request for ship '%s' from wrong player %s\n",
            ship->Name(), player ? player->Name() : "null");

            return;
        }

        ship->InflictNetDamage(self_destruct.GetDamage());

        SendData(&self_destruct);

        int ship_destroyed = (!ship->InTransition() && ship->Integrity() < 1.0f);

        // then delete the ship:
        if (ship_destroyed) {
            NetUtil::SendObjKill(ship, 0, NetObjKill::KILL_MISC);
            Print("    %s Self Destruct (%s)\n", ship->Name(), FormatGameTime());

            ShipStats* killee = ShipStats::Find(ship->Name());
            if (killee)
            killee->AddEvent(SimEvent::DESTROYED, ship->Name());

            ship->DeathSpiral();
        }
    }
}

// +--------------------------------------------------------------------+

void
NetGameServer::Send()
{
    if (players.isEmpty())
    return;

    DWORD time = Game::GameTime();

    // don't flood the network...
    if (time - last_send_time < MIN_NET_FRAME)
    return;

    last_send_time = time;

    // tell the remote players where *we* are:
    if (local_player && !local_player->IsNetObserver() && objid) {
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

        SendData(&obj_loc);
    }

    // tell each remote player where all the others are:
    ListIter<NetPlayer> src = players;
    while (++src) {
        NetPlayer* player = src.value();

        Ship* player_ship = player->GetShip();

        if (player_ship) {
            double r, p, y;
            player_ship->Cam().Orientation().ComputeEulerAngles(r,p,y);

            NetObjLoc obj_loc;
            obj_loc.SetObjID(player->GetObjID());
            obj_loc.SetLocation(player_ship->Location());
            obj_loc.SetVelocity(player_ship->Velocity());
            obj_loc.SetOrientation(Point(r,p,y));
            obj_loc.SetThrottle(player_ship->Throttle() > 10);
            obj_loc.SetAugmenter(player_ship->Augmenter());
            obj_loc.SetGearDown(player_ship->IsGearDown());

            Shield* shield = player_ship->GetShield();
            if (shield)
            obj_loc.SetShield((int) shield->GetPowerLevel());
            else
            obj_loc.SetShield(0);

            BYTE* obj_loc_data = obj_loc.Pack();

            ListIter<NetPlayer> dst = players;
            while (++dst) {
                NetPlayer* remote_player = dst.value();
                if (remote_player->GetNetID() && remote_player != player)
                link->SendMessage(remote_player->GetNetID(), obj_loc_data, NetObjLoc::SIZE);
            }
        }
    }

    // tell each remote player where all the A.I. ships are:
    ListIter<Ship> ai_iter = ships;
    while (++ai_iter) {
        Ship* s = ai_iter.value();

        if (s && !s->IsStatic()) {
            double r, p, y;
            s->Cam().Orientation().ComputeEulerAngles(r,p,y);

            NetObjLoc obj_loc;
            obj_loc.SetObjID(s->GetObjID());
            obj_loc.SetLocation(s->Location());
            obj_loc.SetVelocity(s->Velocity());
            obj_loc.SetOrientation(Point(r,p,y));
            obj_loc.SetThrottle(s->Throttle() > 10);
            obj_loc.SetAugmenter(s->Augmenter());
            obj_loc.SetGearDown(s->IsGearDown());

            Shield* shield = s->GetShield();
            if (shield)
            obj_loc.SetShield((int) shield->GetPowerLevel());
            else
            obj_loc.SetShield(0);

            SendData(&obj_loc);
        }
    }
}

// +--------------------------------------------------------------------+

void
NetGameServer::SendData(NetData* net_data)
{
    if (net_data) {
        BYTE* data  = net_data->Pack();
        int   size  = net_data->Length();
        BYTE  flags = 0;
        bool  all   = true;  // include player with objid in net_data?
        DWORD oid   = net_data->GetObjID();

        BYTE  msg_type = net_data->Type();

        if (msg_type >= 0x10)
        flags |= NetMsg::RELIABLE;

        if (msg_type == NET_WEP_TRIGGER   ||
                msg_type == NET_COMM_MESSAGE  ||
                msg_type == NET_CHAT_MESSAGE  ||
                msg_type == NET_OBJ_HYPER     ||
                msg_type == NET_ELEM_CREATE   ||
                msg_type == NET_NAV_DATA      ||
                msg_type == NET_NAV_DELETE) {
            all = false;
        }

        ListIter<NetPlayer> dst = players;
        while (++dst) {
            NetPlayer* remote_player = dst.value();
            if (remote_player->GetNetID() && (all || oid != remote_player->GetObjID()))
            link->SendMessage(remote_player->GetNetID(), data, size, flags);
        }
    }
}

void
NetGameServer::SendDisconnect(NetPlayer* zombie)
{
    if (zombie) {
        NetDisconnect  disconnect;
        BYTE*          data  = disconnect.Pack();
        int            size  = disconnect.Length();
        BYTE           flags = NetMsg::RELIABLE;

        if (zombie->GetNetID())
        link->SendMessage(zombie->GetNetID(), data, size, flags);
    }
}

// +--------------------------------------------------------------------+

bool
NetGameServer::Update(SimObject* obj)
{
    if (obj->Type() == SimObject::SIM_SHIP) {
        Ship* s = (Ship*) obj;
        if (local_player == s)
        local_player = 0;

        if (ships.contains(s))
        ships.remove(s);
    }

    return SimObserver::Update(obj);
}

const char*
NetGameServer::GetObserverName() const
{
    return "NetGameServer";
}

// +--------------------------------------------------------------------+

void
NetGameServer::Respawn(DWORD oid, Ship* spawn)
{
    if (!oid || !spawn) return;

    Print("NetGameServer::Respawn(%d, %s)\n", oid, spawn->Name());
    spawn->SetObjID(oid);
    Observe(spawn);

    NetPlayer* p = FindPlayerByObjID(oid);
    if (p)
    p->SetShip(spawn);
    else
    ships.append(spawn);

    if (objid == oid) {
        Print("  RESPAWN LOCAL PLAYER\n\n");
        local_player = spawn;
    }
}
