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
    FILE:         NetPlayer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Player (Director) class
*/

#include "MemDebug.h"
#include "NetPlayer.h"
#include "NetGame.h"
#include "NetMsg.h"
#include "NetData.h"
#include "NetUtil.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shield.h"
#include "Shot.h"
#include "Sim.h"
#include "SimEvent.h"
#include "System.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Element.h"
#include "HUDView.h"
#include "Explosion.h"
#include "Farcaster.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"

#include "NetHost.h"
#include "Game.h"
#include "Light.h"

// +--------------------------------------------------------------------+

NetPlayer::~NetPlayer()
{
    if (ship) {
        ship->SetNetworkControl();
        
        Sim* sim = Sim::GetSim();
        sim->DestroyShip(ship);
    }
}

// +--------------------------------------------------------------------+

void
NetPlayer::SetShip(Ship* s)
{
    if (ship != s) {
        if (ship) {
            ship->EnableRepair(true);
            Ignore(ship);
        }

        ship = s;

        if (ship) {
            Observe(ship);
            ship->SetNetworkControl(this);
            ship->SetObjID(objid);

            iff = ship->GetIFF();

            // Turn off auto-repair.  All repair data should
            // come in over the network from the remote player:

            ship->EnableRepair(false);

            // Set all ship weapons back to manual fire control.
            // All trigger events should come over the network,
            // not from weapon auto aiming ai:

            ListIter<WeaponGroup> iter = ship->Weapons();
            while (++iter) {
                WeaponGroup* group = iter.value();

                ListIter<Weapon> w_iter = group->GetWeapons();
                while (++w_iter) {
                    Weapon* weapon = w_iter.value();

                    weapon->SetFiringOrders(Weapon::MANUAL);
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

const double BLEED = 0.5;

bool
NetPlayer::DoObjLoc(NetObjLoc* obj_loc)
{
    if (ship && obj_loc) {
        loc_error = obj_loc->GetLocation() - ship->Location();
        bleed_time = BLEED;

        ship->SetVelocity(obj_loc->GetVelocity());
        Point o = obj_loc->GetOrientation();
        ship->SetAbsoluteOrientation(o.x, o.y, o.z);
        ship->SetThrottle(obj_loc->GetThrottle() ? 100 : 0);
        ship->SetAugmenter(obj_loc->GetAugmenter());

        if (obj_loc->GetGearDown())
        ship->LowerGear();
        else
        ship->RaiseGear();

        Shield* shield = ship->GetShield();
        if (shield)
        shield->SetPowerLevel(obj_loc->GetShield());

        return true;
    }

    return false;
}

bool
NetPlayer::DoObjHyper(NetObjHyper* obj_hyper)
{
    if (ship && obj_hyper) {
        Sim*        sim       = Sim::GetSim();
        SimRegion*  rgn       = sim->FindRegion(obj_hyper->GetRegion());
        DWORD       fc1_id    = obj_hyper->GetFarcaster1();
        DWORD       fc2_id    = obj_hyper->GetFarcaster2();
        Ship*       fc1       = 0;
        Ship*       fc2       = 0;
        int         trans     = obj_hyper->GetTransitionType();

        if (ship->GetRegion() == rgn) {
            ::Print("NetPlayer::DoObjHyper ship: '%s' rgn: '%s' trans: %d (IGNORED)\n\n",
            ship->Name(), obj_hyper->GetRegion().data(), trans);

            return false;
        }

        ::Print("NetPlayer::DoObjHyper ship: '%s' rgn: '%s' trans: %d\n\n",
        ship->Name(), obj_hyper->GetRegion().data(), trans);

        // orbital transition?
        if (trans == Ship::TRANSITION_DROP_ORBIT) {
            ship->SetTransition(1.0f, Ship::TRANSITION_DROP_ORBIT, ship->Location());
            ship->CompleteTransition();
        }

        else if (trans == Ship::TRANSITION_MAKE_ORBIT) {
            ship->SetTransition(1.0f, Ship::TRANSITION_MAKE_ORBIT, ship->Location());
            ship->CompleteTransition();
        }

        else {
            if (fc1_id)
            fc1 = sim->FindShipByObjID(fc1_id);

            if (fc2_id)
            fc2 = sim->FindShipByObjID(fc2_id);

            sim->CreateExplosion(ship->Location(), Point(0,0,0), 
            Explosion::QUANTUM_FLASH, 1.0f, 0, ship->GetRegion());

            sim->RequestHyperJump(ship, rgn, obj_hyper->GetLocation(), trans, fc1, fc2);

            ShipStats* stats = ShipStats::Find(ship->Name());
            stats->AddEvent(SimEvent::QUANTUM_JUMP, rgn->Name());
        }

        return true;
    }

    return false;
}

bool
NetPlayer::DoObjTarget(NetObjTarget* obj_target)
{
    if (ship && obj_target) {
        DWORD       tgtid  = obj_target->GetTgtID();
        int         subid  = obj_target->GetSubtarget();
        SimObject*  target = 0;
        System*     subtgt = 0;

        NetGame*    net_game = NetGame::GetInstance();
        if (net_game && tgtid) {
            target = net_game->FindShipByObjID(tgtid);

            if (target) {
                if (subid >= 0) {
                    Ship* tgt_ship = (Ship*) target;
                    subtgt = tgt_ship->Systems().at(subid);
                }
            }
            else {
                target = net_game->FindShotByObjID(tgtid);
            }
        }

        ship->SetTarget(target, subtgt, true); // from net = true (don't resend)

        return true;
    }

    return false;
}

bool
NetPlayer::DoObjEmcon(NetObjEmcon* obj_emcon)
{
    if (ship && obj_emcon) {
        int emcon  = obj_emcon->GetEMCON();
        ship->SetEMCON(emcon, true); // from net = true (don't resend)

        return true;
    }

    return false;
}

bool
NetPlayer::DoWepTrigger(NetWepTrigger* trigger)
{
    if (ship && trigger) {
        int   index = trigger->GetIndex();
        int   count = trigger->GetCount();
        DWORD tgtid = trigger->GetTgtID();
        int   subid = trigger->GetSubtarget();
        bool  decoy = trigger->GetDecoy();
        bool  probe = trigger->GetProbe();

        Weapon* w = 0;

        if (decoy)        w = ship->GetDecoy();
        else if (probe)   w = ship->GetProbeLauncher();
        else              w = ship->GetWeaponByIndex(index);

        if (w) {
            SimObject*  target = 0;
            System*     subtgt = 0;

            NetGame* net_game = NetGame::GetInstance();
            if (net_game) {
                target = net_game->FindShipByObjID(tgtid);

                if (target) {
                    if (subid >= 0) {
                        Ship* tgt_ship = (Ship*) target;
                        subtgt = tgt_ship->Systems().at(subid);
                    }
                }
                else {
                    target = net_game->FindShotByObjID(tgtid);
                }

                // re-broadcast:
                if (net_game->IsServer()) {
                    if (w->IsPrimary()) {
                        w->NetFirePrimary(target, subtgt, count);
                        net_game->SendData(trigger);
                    }
                    else {
                        DWORD wepid = NetGame::GetNextObjID(NetGame::SHOT);
                        Shot* shot  = w->NetFireSecondary(target, subtgt, wepid);

                        if (shot && shot->IsDrone()) {
                            if (probe)
                            ship->SetProbe((Drone*) shot);

                            else if (decoy)
                            ship->AddActiveDecoy((Drone*) shot);
                        }

                        NetWepRelease release;
                        release.SetObjID(objid);
                        release.SetTgtID(tgtid);
                        release.SetSubtarget(subid);
                        release.SetWepID(wepid);
                        release.SetIndex(index);
                        release.SetDecoy(decoy);
                        release.SetProbe(probe);

                        net_game->SendData(&release);
                    }
                }

                else {
                    if (w->IsPrimary()) {
                        w->NetFirePrimary(target, subtgt, count);
                    }
                }

                return true;
            }
        }

    }
    return false;
}

bool
NetPlayer::DoWepRelease(NetWepRelease* release)
{
    if (ship && release) {
        int   index = release->GetIndex();
        DWORD tgtid = release->GetTgtID();
        DWORD wepid = release->GetWepID();
        int   subid = release->GetSubtarget();
        bool  decoy = release->GetDecoy();
        bool  probe = release->GetProbe();

        Weapon* w = 0;

        if (decoy)        w = ship->GetDecoy();
        else if (probe)   w = ship->GetProbeLauncher();
        else              w = ship->GetWeaponByIndex(index);

        if (w && !w->IsPrimary()) {
            SimObject*  target = 0;
            System*     subtgt = 0;

            NetGame* net_game = NetGame::GetInstance();
            if (net_game) {
                target = net_game->FindShipByObjID(tgtid);

                if (target) {
                    if (subid >= 0) {
                        Ship* tgt_ship = (Ship*) target;
                        subtgt = tgt_ship->Systems().at(subid);
                    }
                }
                else {
                    target = net_game->FindShotByObjID(tgtid);
                }
            }

            Shot* shot = w->NetFireSecondary(target, subtgt, wepid);

            if (shot && shot->IsDrone()) {
                if (probe)
                ship->SetProbe((Drone*) shot);

                else if (decoy)
                ship->AddActiveDecoy((Drone*) shot);
            }

            return true;
        }
    }

    return false;
}

bool
NetPlayer::DoCommMessage(NetCommMsg* comm_msg)
{
    if (ship && comm_msg) {
        RadioTraffic* traffic   = RadioTraffic::GetInstance();
        RadioMessage* radio_msg = comm_msg->GetRadioMessage();

        if (traffic && radio_msg) {
            if (radio_msg->DestinationElem() || radio_msg->DestinationShip()) {
                // radio traffic owns the sent message,
                // so we must give it a cloned object that is
                // safe to delete:
                traffic->SendMessage(new(__FILE__,__LINE__) RadioMessage(*radio_msg));
                return true;
            }
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

bool
NetPlayer::DoSysDamage(NetSysDamage* sys_damage)
{
    if (ship && sys_damage) {
        System* sys = ship->GetSystem(sys_damage->GetSystem());
        ship->InflictNetSystemDamage( sys,
        sys_damage->GetDamage(),
        sys_damage->GetDamageType());

        return true;
    }

    return false;
}

bool
NetPlayer::DoSysStatus(NetSysStatus* sys_status)
{
    if (ship && sys_status) {
        System* sys = ship->GetSystem(sys_status->GetSystem());
        ship->SetNetSystemStatus(  sys,
        sys_status->GetStatus(),
        sys_status->GetPower(),
        sys_status->GetReactor(),
        sys_status->GetAvailability());

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

void
NetPlayer::ExecFrame(double seconds)
{
    if (ship) {
        // bleed off the location error:
        if (loc_error.length() > 0 && bleed_time > 0) {
            double fragment = min(seconds / BLEED, bleed_time);
            ship->MoveTo(ship->Location() + loc_error * fragment);
            bleed_time -= fragment;
        }

        // update the ship location by dead reckoning:
        ship->MoveTo(ship->Location() + ship->Velocity() * seconds);

        // let the FLCS run, so that the drive flares will work:
        ship->ExecFLCSFrame();

        // now update the graphic rep and light sources:
        if (ship->Rep()) {
            ship->Rep()->MoveTo(ship->Location());
            ship->Rep()->SetOrientation(ship->Cam().Orientation());
        }

        if (ship->LightSrc()) {
            ship->LightSrc()->MoveTo(ship->Location());
        }
    }
}

// +--------------------------------------------------------------------+

bool
NetPlayer::Update(SimObject* obj)
{
    if (obj == ship) {
        ship = 0;
    }

    return SimObserver::Update(obj);
}

const char*
NetPlayer::GetObserverName() const
{
    return "NetPlayer";
}
