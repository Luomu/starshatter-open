/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetUtil.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Utility class to simplify sending NetData messages.
*/


#include "MemDebug.h"
#include "NetUtil.h"
#include "NetData.h"
#include "NetGame.h"
#include "NetGameServer.h"

#include "Element.h"
#include "Instruction.h"
#include "Random.h"
#include "Ship.h"
#include "Shot.h"
#include "System.h"
#include "Weapon.h"

// +-------------------------------------------------------------------+

void
NetUtil::SendObjDamage(SimObject* obj, double dmg, Shot* shot)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj) return;

   if (net_game->IsServer() && obj->GetObjID()) {
      NetObjDamage damage;
      damage.SetObjID(obj->GetObjID());
      damage.SetDamage((float) dmg);

      if (shot)
         damage.SetShotID(shot->GetObjID());

      net_game->SendData(&damage);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendSysDamage(Ship* obj, System* sys, double dmg)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj || !sys) return;

   if (net_game->IsServer() && obj->GetObjID()) {
      NetSysDamage damage;
      damage.SetObjID(obj->GetObjID());
      damage.SetDamage(dmg);
      damage.SetSystem(sys->GetID());

      net_game->SendData(&damage);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendSysStatus(Ship* obj, System* sys)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj || !sys) return;

   if (obj->GetObjID()) {
      NetSysStatus status;
      status.SetObjID(obj->GetObjID());
      status.SetSystem( (int) sys->GetID());
      status.SetStatus( (int) sys->Status());
      status.SetPower(  (int) sys->GetPowerLevel());
      status.SetReactor((int) sys->GetSourceIndex());
      status.SetAvailablility(sys->Availability());

      net_game->SendData(&status);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendObjKill(Ship* obj, const Ship* killer, int type, int deck)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj) return;

   if (type == NetObjKill::KILL_DOCK || (net_game->IsServer() && obj->GetObjID())) {
      NetObjKill kill;
      kill.SetObjID(obj->GetObjID());
      kill.SetFlightDeck(deck);

      if (killer)
         kill.SetKillerID(killer->GetObjID());

      kill.SetKillType(type);

      if (type != NetObjKill::KILL_DOCK && obj->RespawnCount() > 0) {
         Print("NetObjKill preparing respawn for %s\n", obj->Name());

         Point respawn_loc = RandomPoint() * 1.75;
         kill.SetRespawn(true);
         kill.SetRespawnLoc(respawn_loc);
         obj->SetRespawnLoc(respawn_loc);
      }
      else {
         Print("NetObjKill no respawn for %s\n", obj->Name());
      }

      net_game->SendData(&kill);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendObjHyper(Ship* obj, const char* rgn, const Point& loc,
                      const Ship* fc1, const Ship* fc2, int transtype)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj) return;

   if (obj->GetObjID()) {
      NetObjHyper obj_hyper;
      obj_hyper.SetObjID(obj->GetObjID());
      obj_hyper.SetRegion(rgn);
      obj_hyper.SetLocation(loc);
      obj_hyper.SetTransitionType(transtype);

      if (fc1)
         obj_hyper.SetFarcaster1(fc1->GetObjID());

      if (fc2)
         obj_hyper.SetFarcaster2(fc2->GetObjID());

      net_game->SendData(&obj_hyper);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendObjTarget(Ship* obj)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj) return;

   if (obj->GetObjID()) {
      NetObjTarget obj_target;
      obj_target.SetObjID(obj->GetObjID());

      SimObject* target = obj->GetTarget();
      if (target) {
         obj_target.SetTgtID(target->GetObjID());

         System* subtarget = obj->GetSubTarget();

         if (subtarget) {
            Ship* s = (Ship*) target;
            obj_target.SetSubtarget(subtarget->GetID());
         }
      }

      net_game->SendData(&obj_target);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendObjEmcon(Ship* obj)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj) return;

   if (obj->GetObjID()) {
      NetObjEmcon obj_emcon;
      obj_emcon.SetObjID(obj->GetObjID());
      obj_emcon.SetEMCON(obj->GetEMCON());
      net_game->SendData(&obj_emcon);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendWepTrigger(Weapon* wep, int count)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !wep) return;

   if (wep->IsPrimary() || net_game->IsClient()) {
      NetWepTrigger trigger;
      trigger.SetObjID(wep->Owner()->GetObjID());

      SimObject* target = wep->GetTarget();
      if (target) {
         trigger.SetTgtID(target->GetObjID());

         System* subtarget = wep->GetSubTarget();

         if (subtarget) {
            Ship* s = (Ship*) target;
            trigger.SetSubtarget(subtarget->GetID());
         }
      }

      trigger.SetIndex(wep->GetIndex());
      trigger.SetCount(count);
      trigger.SetDecoy(wep->IsDecoy());
      trigger.SetProbe(wep->IsProbe());

      net_game->SendData(&trigger);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendWepRelease(Weapon* wep, Shot* shot)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !wep || !shot) return;

   if (net_game->IsServer() && wep->IsMissile()) {
      DWORD wepid = NetGame::GetNextObjID(NetGame::SHOT);
      shot->SetObjID(wepid);

      NetWepRelease release;
      release.SetObjID(wep->Owner()->GetObjID());

      SimObject* target = wep->GetTarget();
      if (target)
         release.SetTgtID(target->GetObjID());

      System* subtarget = wep->GetSubTarget();
      if (target && subtarget && target->Type() == SimObject::SIM_SHIP) {
         Ship* tgt = (Ship*) target;
         release.SetSubtarget(subtarget->GetID());
      }

      release.SetWepID(wepid);
      release.SetIndex(wep->GetIndex());
      release.SetDecoy(shot->IsDecoy());
      release.SetProbe(shot->IsProbe());

      net_game->SendData(&release);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendWepDestroy(Shot* shot)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !shot) return;

   if (net_game->IsServer() && shot->GetObjID()) {
      NetWepDestroy destroy;

      destroy.SetObjID(shot->GetObjID());

      net_game->SendData(&destroy);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendChat(DWORD dst, const char* name, const char* text)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !name || !*name || !text || !*text) return;

   NetChatMsg chat_msg;
   chat_msg.SetDstID(dst);
   chat_msg.SetName(name);
   chat_msg.SetText(text);

   if (net_game->IsClient()) {
      net_game->SendData(&chat_msg);
   }

   else {
      NetGameServer* net_game_server = (NetGameServer*) net_game;
      net_game_server->RouteChatMsg(chat_msg);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendElemRequest(const char* name)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !name) return;

   NetElemRequest elem_request;
   elem_request.SetName(name);

   ::Print("NetUtil::SendElemRequest name: '%s'\n", name);
   net_game->SendData(&elem_request);
}

// +-------------------------------------------------------------------+

void
NetUtil::SendElemCreate(Element* elem, int squadron, int* slots, bool alert, bool in_flight)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !elem) return;

   NetElemCreate elem_create;
   elem_create.SetName(elem->Name());
   elem_create.SetType(elem->Type());
   elem_create.SetIFF(elem->GetIFF());
   elem_create.SetIntel(elem->IntelLevel());
   elem_create.SetLoadout(elem->Loadout());
   elem_create.SetSquadron(squadron);
   elem_create.SetSlots(slots);
   elem_create.SetAlert(alert);
   elem_create.SetInFlight(in_flight);

   if (elem->NumObjectives() > 0) {
      Instruction* obj = elem->GetObjective(0);

      if (obj) {
         elem_create.SetObjCode(obj->Action());
         elem_create.SetObjective(obj->TargetName());
      }
   }

   if (elem->GetCarrier())
      elem_create.SetCarrier(elem->GetCarrier()->Name());

   if (elem->GetCommander())
      elem_create.SetCommander(elem->GetCommander()->Name());

   ::Print("NetUtil::SendElemCreate iff: %d name: '%s'\n", elem->GetIFF(), elem->Name().data());
   net_game->SendData(&elem_create);
}

// +-------------------------------------------------------------------+

void
NetUtil::SendShipLaunch(Ship* carrier, int squadron, int slot)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !carrier) return;

   if (carrier->GetObjID()) {
      NetShipLaunch ship_launch;

      ship_launch.SetObjID(carrier->GetObjID());
      ship_launch.SetSquadron(squadron);
      ship_launch.SetSlot(slot);

      net_game->SendData(&ship_launch);
   }
}

// +-------------------------------------------------------------------+

void
NetUtil::SendNavData(bool add, Element* elem, int index, Instruction* navpt)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !elem || !navpt) return;

   // resolve rloc before copying the navpoint into the net nav data structure:
   Point loc = navpt->Location();

   NetNavData nav_data;
   nav_data.SetObjID(net_game->GetObjID());
   nav_data.SetAdd(add);
   nav_data.SetElem(elem->Name());
   nav_data.SetIndex(index);
   nav_data.SetNavPoint(navpt);

   net_game->SendData(&nav_data);
}

// +-------------------------------------------------------------------+

void
NetUtil::SendNavDelete(Element* elem, int index)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !elem) return;

   NetNavDelete nav_delete;
   nav_delete.SetObjID(net_game->GetObjID());
   nav_delete.SetElem(elem->Name());
   nav_delete.SetIndex(index);

   net_game->SendData(&nav_delete);
}

// +-------------------------------------------------------------------+

void
NetUtil::SendSelfDestruct(Ship* obj, double dmg)
{
   NetGame* net_game = NetGame::GetInstance();
   if (!net_game || !obj) return;

   if (obj->GetObjID()) {
      NetSelfDestruct sd;
      sd.SetObjID(obj->GetObjID());
      sd.SetDamage((float) dmg);

      net_game->SendData(&sd);
   }
}
