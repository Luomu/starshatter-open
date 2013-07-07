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
    FILE:         FighterAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fighter (low-level) Artificial Intelligence class
*/

#include "MemDebug.h"
#include "FighterAI.h"
#include "FighterTacticalAI.h"
#include "Ship.h"
#include "Shot.h"
#include "Sensor.h"
#include "Element.h"
#include "ShipDesign.h"
#include "Instruction.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Farcaster.h"
#include "FlightComp.h"
#include "FlightDeck.h"
#include "Hangar.h"
#include "Sim.h"
#include "StarSystem.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"

#include "Game.h"

static const double TIME_TO_DOCK = 30;

// +----------------------------------------------------------------------+

FighterAI::FighterAI(SimObject* s)
: ShipAI(s), brakes(0), drop_state(0), jink_time(0), evading(false),
decoy_missile(0), missile_time(0), terrain_warning(false), inbound(0),
rtb_code(0), form_up(false), over_threshold(false), time_to_dock(0),
go_manual(false)
{
    ai_type   = FIGHTER;
    seek_gain = 22;
    seek_damp = 0.55;
    brakes    = 0;
    z_shift   = 0;

    tactical = new(__FILE__,__LINE__) FighterTacticalAI(this);
}


// +--------------------------------------------------------------------+

FighterAI::~FighterAI()
{ }

// +--------------------------------------------------------------------+

static double frame_time = 0;

void
FighterAI::ExecFrame(double s)
{
    if (!ship) return;

    evading = false;
    inbound = ship->GetInbound();
    missile_time -= s;

    int order = 0;

    if (navpt)
    order = navpt->Action();

    if (inbound) {
        form_up  = false;
        rtb_code = 1;

        // CHEAT LANDING:
        if (inbound->Final() && time_to_dock > 0) {
            FlightDeck* deck = inbound->GetDeck();
            if (deck) {
                Point dst      = deck->EndPoint();
                Point approach = deck->StartPoint() - dst;

                const Ship* carrier = deck->GetCarrier();

                Camera landing_cam;
                landing_cam.Clone(carrier->Cam());
                landing_cam.Yaw(deck->Azimuth());

                if (time_to_dock > TIME_TO_DOCK/2) {
                    double lr, lp, lw;
                    double sr, sp, sw;

                    landing_cam.Orientation().ComputeEulerAngles(lr, lp, lw);
                    ship->Cam().Orientation().ComputeEulerAngles(sr, sp, sw);

                    double nr = sr + s*(lr-sr);
                    double np = sp + s*(lp-sp);
                    double nw = sw + s*(lw-sw)*0.5;

                    Camera work;
                    work.Aim(nr,np,nw);
                    landing_cam.Clone(work);
                }

                ship->CloneCam(landing_cam);
                ship->MoveTo(dst + approach * (time_to_dock / TIME_TO_DOCK));
                ship->SetVelocity(carrier->Velocity() + ship->Heading() * 50);
                ship->SetThrottle(50);
                ship->ExecFLCSFrame();

                time_to_dock -= s;

                if (time_to_dock <= 0) {
                    deck->Dock(ship);
                    time_to_dock = 0;
                }

                return;
            }
        }

        else if (ship->GetFlightPhase() == Ship::DOCKING) {
            // deal with (pathological) moving carrier deck:

            FlightDeck* deck = inbound->GetDeck();
            if (deck) {
                Point dst = deck->EndPoint();

                if (ship->IsAirborne()) {
                    double alt = dst.y;
                    dst = ship->Location();
                    dst.y = alt;
                }

                const Ship* carrier = deck->GetCarrier();

                Camera landing_cam;
                landing_cam.Clone(carrier->Cam());
                landing_cam.Yaw(deck->Azimuth());

                ship->CloneCam(landing_cam);
                ship->MoveTo(dst);
                
                if (!ship->IsAirborne()) {
                    ship->SetVelocity(carrier->Velocity());
                }
                else {
                    Point taxi(landing_cam.vpn());
                    ship->SetVelocity(taxi * 95);
                }

                ship->SetThrottle(0);
                ship->ExecFLCSFrame();
            }

            return;
        }
    }
    else {
        Instruction*   orders  = ship->GetRadioOrders();

        if (orders && 
                (orders->Action() == RadioMessage::WEP_HOLD || 
                    orders->Action() == RadioMessage::FORM_UP)) {
            form_up  = true;
            rtb_code = 0;
        }
        else {
            form_up = false;
        }
    }

    if (!target && order != Instruction::STRIKE)
    ship->SetSensorMode(Sensor::STD);

    ShipAI::ExecFrame(s); // this must be the last line of this method

    // IT IS NOT SAFE TO PLACE CODE HERE
    // if this class decides to break orbit,
    // this object will be deleted during
    // ShipAI::ExecFrame() (which calls
    // FighterAI::Navigator() - see below)
}

// +--------------------------------------------------------------------+

void
FighterAI::FindObjective()
{
    distance = 0;

    // ALWAYS complete initial launch navpt:
    if (!navpt) {
        navpt = ship->GetNextNavPoint();
        if (navpt && (navpt->Action() != Instruction::LAUNCH || navpt->Status() == Instruction::COMPLETE))
        navpt = 0;
    }

    if (navpt && navpt->Action() == Instruction::LAUNCH) {
        if (navpt->Status() != Instruction::COMPLETE) {
            FindObjectiveNavPoint();

            // transform into camera coords:
            objective = Transform(obj_w);
            ship->SetDirectorInfo(Game::GetText("ai.launch"));
            return;
        }
        else {
            navpt = 0;
        }
    }

    // runway takeoff:
    else if (takeoff) {
        obj_w   = ship->Location()   + ship->Heading() * 10e3;
        obj_w.y = ship->Location().y + 2e3;

        // transform into camera coords:
        objective = Transform(obj_w);
        ship->SetDirectorInfo(Game::GetText("ai.takeoff"));
        return;
    }

    // approaching a carrier or runway:
    else if (inbound) {
        FlightDeck* deck = inbound->GetDeck();

        if (!deck) {
            objective = Point();
            return;
        }

        // initial approach
        if (inbound->Approach() > 0 || !inbound->Cleared()) {
            obj_w = deck->ApproachPoint(inbound->Approach()) + inbound->Offset();

            distance = (obj_w - ship->Location()).length();

            // transform into camera coords:
            objective = Transform(obj_w);
            ship->SetDirectorInfo(Game::GetText("ai.inbound"));

            return;
        }

        // final approach
        else {
            ship->SetDirectorInfo(Game::GetText("ai.finals"));

            obj_w = deck->StartPoint();
            if (inbound->Final()) {
                obj_w = deck->EndPoint();

                if (deck->OverThreshold(ship)) {
                    obj_w = deck->MountLocation();
                    over_threshold = true;
                }
            }

            distance = (obj_w - ship->Location()).length();

            // transform into camera coords:
            objective = Transform(obj_w);

            return;
        }
    }

    // not inbound yet, check for RTB order:
    else { 
        Instruction*   orders = (Instruction*) ship->GetRadioOrders();
        int            action = 0;

        if (orders)
        action = orders->Action();

        if (navpt && !action) {
            FindObjectiveNavPoint();
            if (distance < 5e3) {
                action = navpt->Action();
            }
        }

        if (action == RadioMessage::RTB ||
                action == RadioMessage::DOCK_WITH) {

            Ship* controller = ship->GetController();

            if (orders && orders->Action() == RadioMessage::DOCK_WITH && orders->GetTarget()) {
                controller = (Ship*) orders->GetTarget();
            }

            else if (navpt && navpt->Action() == RadioMessage::DOCK_WITH && navpt->GetTarget()) {
                controller = (Ship*) navpt->GetTarget();
            }

            ReturnToBase(controller);

            if (rtb_code)
            return;
        }
    }

    ShipAI::FindObjective();
}

void
FighterAI::ReturnToBase(Ship* controller)
{
    rtb_code = 0;

    if (controller) {
        SimRegion*  self_rgn = ship->GetRegion();
        SimRegion*  rtb_rgn  = controller->GetRegion();

        if (self_rgn && !rtb_rgn) {
            rtb_rgn = self_rgn;
        }

        if (self_rgn && rtb_rgn && self_rgn != rtb_rgn) {
            // is the carrier in orbit above us
            // (or on the ground below us)?

            if (rtb_rgn->GetOrbitalRegion()->Primary() ==
                    self_rgn->GetOrbitalRegion()->Primary()) {

                Point npt = rtb_rgn->Location() - self_rgn->Location();
                obj_w = npt.OtherHand();

                // distance from self to navpt:
                distance = Point(obj_w - ship->Location()).length();

                // transform into camera coords:
                objective = Transform(obj_w);

                if (rtb_rgn->IsAirSpace()) {
                    drop_state = -1;
                }
                else if (rtb_rgn->IsOrbital()) {
                    drop_state = 1;
                }

                rtb_code = 2;
            }

            // try to find a jumpgate that will take us home:
            else {
                QuantumDrive*  qdrive = ship->GetQuantumDrive();
                bool use_farcaster    = !qdrive ||
                !qdrive->IsPowerOn() ||
                qdrive->Status() < System::DEGRADED;

                if (use_farcaster) {
                    if (!farcaster) {
                        ListIter<Ship> s = self_rgn->Ships();
                        while (++s && !farcaster) {
                            if (s->GetFarcaster()) {
                                const Ship* dest = s->GetFarcaster()->GetDest();
                                if (dest && dest->GetRegion() == rtb_rgn) {
                                    farcaster = s->GetFarcaster();
                                }
                            }
                        }
                    }

                    if (farcaster) {
                        Point apt   = farcaster->ApproachPoint(0);
                        Point npt   = farcaster->StartPoint();
                        double r1   = (ship->Location() - npt).length();

                        if (r1 > 50e3) {
                            obj_w     = apt;
                            distance  = r1;
                            objective = Transform(obj_w);
                        }

                        else {
                            double r2 = (ship->Location() - apt).length();
                            double r3 = (npt - apt).length();

                            if (r1+r2 < 1.2*r3) {
                                obj_w     = npt;
                                distance  = r1;
                                objective = Transform(obj_w);
                            }
                            else {
                                obj_w     = apt;
                                distance  = r2;
                                objective = Transform(obj_w);
                            }
                        }

                        rtb_code = 3;
                    }

                    // can't find a way back home, ignore the RTB order:
                    else {
                        ship->ClearRadioOrders();
                        rtb_code = 0;
                        return;
                    }
                }
                else if (qdrive) {
                    if (qdrive->ActiveState() == QuantumDrive::ACTIVE_READY) {
                        qdrive->SetDestination(rtb_rgn, controller->Location());
                        qdrive->Engage();
                    }

                    rtb_code = 3;
                }
            }
        }

        else {
            obj_w = controller->Location();

            distance = (obj_w - ship->Location()).length();

            // transform into camera coords:
            objective = Transform(obj_w);
            ship->SetDirectorInfo(Game::GetText("ai.return-to-base"));

            rtb_code = 1;
        }
    }
}

// +--------------------------------------------------------------------+

void
FighterAI::FindObjectiveNavPoint()
{
    SimRegion*  self_rgn = ship->GetRegion();
    SimRegion*  nav_rgn  = navpt->Region();

    if (self_rgn && !nav_rgn) {
        nav_rgn = self_rgn;
        navpt->SetRegion(nav_rgn);
    }

    if (self_rgn && nav_rgn && self_rgn != nav_rgn) {
        if (nav_rgn->GetOrbitalRegion()->Primary() ==
                self_rgn->GetOrbitalRegion()->Primary()) {

            Point npt = nav_rgn->Location() - self_rgn->Location();
            obj_w = npt.OtherHand();

            // distance from self to navpt:
            distance = Point(obj_w - ship->Location()).length();

            // transform into camera coords:
            objective = Transform(obj_w);

            if (nav_rgn->IsAirSpace()) {
                drop_state = -1;
            }
            else if (nav_rgn->IsOrbital()) {
                drop_state = 1;
            }

            return;
        }

        else {
            QuantumDrive* q = ship->GetQuantumDrive();

            if (q) {
                if (q->ActiveState() == QuantumDrive::ACTIVE_READY) {
                    q->SetDestination(navpt->Region(), navpt->Location());
                    q->Engage();
                    return;
                }
            }
        }
    }

    ShipAI::FindObjectiveNavPoint();
}

// +--------------------------------------------------------------------+

Point
FighterAI::ClosingVelocity()
{
    if (ship) {
        WeaponDesign* wep_design = ship->GetPrimaryDesign();

        if (target && wep_design) {
            Point aim_vec = ship->Heading();
            aim_vec.Normalize();

            Point shot_vel = ship->Velocity() + aim_vec * wep_design->speed;
            return shot_vel - target->Velocity();
        }

        else if (target) {
            return ship->Velocity() - target->Velocity();
        }

        else {
            return ship->Velocity();
        }
    }

    return Point(1,0,0);
}

// +--------------------------------------------------------------------+

void
FighterAI::Navigator()
{
    go_manual = false;

    if (takeoff) {
        accumulator.Clear();
        magnitude = 0;
        brakes    = 0;
        z_shift   = 0;

        Accumulate(SeekTarget());
        HelmControl();
        ThrottleControl();
        ship->ExecFLCSFrame();
        return;
    }

    Element* elem = ship->GetElement();

    if (elem) {
        Ship* lead = elem->GetShip(1);

        if (lead && lead != ship) {
            if (lead->IsDropping() && !ship->IsDropping()) {
                ship->DropOrbit();
                // careful: this object has just been deleted!
                return;
            }

            if (lead->IsAttaining() && !ship->IsAttaining()) {
                ship->MakeOrbit();
                // careful: this object has just been deleted!
                return;
            }
        }

        else {
            if (drop_state < 0) {
                ship->DropOrbit();
                // careful: this object has just been deleted!
                return;
            }

            if (drop_state > 0) {
                ship->MakeOrbit();
                // careful: this object has just been deleted!
                return;
            }
        }
    }

    int order = 0;

    if (navpt)
    order = navpt->Action();

    if (rtb_code == 1 && navpt && navpt->Status() < Instruction::SKIPPED &&
            !inbound && distance < 35e3) { // (this should be distance to the ship)

        if (order == Instruction::RTB) {
            Ship*    controller = ship->GetController();
            Hangar*  hangar     = controller ? controller->GetHangar() : 0;

            if (hangar && hangar->CanStow(ship)) {
                for (int i = 0; i < elem->NumShips(); i++) {
                    Ship* s = elem->GetShip(i+1);

                    if (s && s->GetDirector() && s->GetDirector()->Type() >= ShipAI::FIGHTER)
                    RadioTraffic::SendQuickMessage(s, RadioMessage::CALL_INBOUND);
                }

                if (element_index == 1)
                ship->SetNavptStatus(navpt, Instruction::COMPLETE);
            }

            else {
                if (element_index == 1) {
                    ::Print("WARNING: FighterAI NAVPT RTB, but no controller or hangar found for ship '%s'\n", ship->Name());
                    ship->SetNavptStatus(navpt, Instruction::SKIPPED);
                }
            }
        }

        else {
            Ship* dock_target = (Ship*) navpt->GetTarget();
            if (dock_target) {
                for (int i = 0; i < elem->NumShips(); i++) {
                    Ship* s = elem->GetShip(i+1);

                    if (s) {
                        RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(dock_target, s, RadioMessage::CALL_INBOUND);
                        RadioTraffic::Transmit(msg);
                    }
                }

                if (element_index == 1)
                ship->SetNavptStatus(navpt, Instruction::COMPLETE);
            }

            else {
                if (element_index == 1) {
                    ::Print("WARNING: FighterAI NAVPT DOCK, but no dock target found for ship '%s'\n", ship->Name());
                    ship->SetNavptStatus(navpt, Instruction::SKIPPED);
                }
            }
        }
    }

    if (target)
    ship->SetDirectorInfo(Game::GetText("ai.seek-target"));

    accumulator.Clear();
    magnitude = 0;
    brakes    = 0;
    z_shift   = 0;

    hold = false;
    if ((ship->GetElement() && ship->GetElement()->GetHoldTime() > 0) || 
            (navpt && navpt->Status() == Instruction::COMPLETE && navpt->HoldTime() > 0))
    hold = true;

    if (ship->MissionClock() < 10000) {
        if (ship->IsAirborne())
        Accumulate(SeekTarget());
    }

    else if ((farcaster && distance < 20e3) || (inbound && inbound->Final())) {
        Accumulate(SeekTarget());
    }

    else {
        if (!ship->IsAirborne() || ship->AltitudeAGL() > 100)
        ship->RaiseGear();

        Accumulate(AvoidTerrain());
        Steer avoid = AvoidCollision();

        if (other && inbound && inbound->GetDeck() && inbound->Cleared()) {
            if (other != (SimObject*) inbound->GetDeck()->GetCarrier())
            Accumulate(avoid);
        }
        else {
            Accumulate(avoid);
        }

        if (!too_close && !hold && !terrain_warning) {
            Accumulate(SeekTarget());
            Accumulate(EvadeThreat());
        }
    }

    HelmControl();
    ThrottleControl();
    FireControl();
    AdjustDefenses();

    ship->ExecFLCSFrame();
}

// +--------------------------------------------------------------------+

void
FighterAI::HelmControl()
{
    Camera* cam             = ((Camera*) &(ship->Cam()));
    Point   vrt             = cam->vrt();
    double  deflection      = vrt.y;
    double  theta           = 0;
    bool    formation       = element_index > 1;
    bool    station_keeping = distance < 0;
    bool    inverted        = cam->vup().y < -0.5;
    Ship*   ward            = ship->GetWard();

    if (takeoff || inbound || station_keeping)
    formation = false;

    if (takeoff || navpt || farcaster || patrol || inbound || rtb_code || target || ward || threat || formation) {
        // are we being asked to flee?
        if (fabs(accumulator.yaw) == 1.0 && accumulator.pitch == 0.0) {
            accumulator.pitch  = -0.7f;
            accumulator.yaw   *= 0.25f;

            if (ship->IsAirborne() && ship->GetFlightModel() == 0)
            accumulator.pitch = -0.45f;

            // low ai -> lower turning rate
            accumulator.pitch += 0.1f * (2-ai_level);
        }

        ship->ApplyRoll((float) (accumulator.yaw * -0.7));
        ship->ApplyYaw((float) (accumulator.yaw * 0.2));

        if (fabs(accumulator.yaw) > 0.5 && fabs(accumulator.pitch) < 0.1)
        accumulator.pitch -= 0.1f;

        ship->ApplyPitch((float) accumulator.pitch);
    }

    else {
        ship->SetDirectorInfo(Game::GetText("ai.station-keeping"));
        station_keeping   = true;

        // go into a slow orbit if airborne:
        if (ship->IsAirborne() && ship->Class() < Ship::LCA) {
            accumulator.brake = 0.2;
            accumulator.stop  = 0;

            double compass_pitch = ship->CompassPitch();
            double desired_bank  = -PI/4;
            double current_bank  = asin(deflection);
            double theta         = desired_bank - current_bank;
            ship->ApplyRoll(theta);

            double coord_pitch   = compass_pitch - 0.2 * fabs(current_bank);
            ship->ApplyPitch(coord_pitch);
        }
        else {
            accumulator.brake = 1;
            accumulator.stop  = 1;
        }
    }

    // if not turning, roll to orient with world coords:
    if (ship->Design()->auto_roll > 0) {
        if (fabs(accumulator.pitch) < 0.1 && fabs(accumulator.yaw) < 0.25) {
            // zolon spiral behavior:
            if (ship->Design()->auto_roll > 1) {
                if ((element_index + (ship->MissionClock()>>10)) & 0x4)
                ship->ApplyRoll( 0.60);
                else
                ship->ApplyRoll(-0.35);
            }

            // normal behavior - roll to upright:
            else if (fabs(deflection) > 0.1 || inverted) {
                double theta = asin(deflection/vrt.length()) * 0.5;
                ship->ApplyRoll(-theta);
            }
        }
    }

    // if not otherwise occupied, pitch to orient with world coords:
    if (station_keeping && (!ship->IsAirborne() || ship->Class() < Ship::LCA)) {
        Point heading = ship->Heading();
        double pitch_deflection = heading.y;

        if (fabs(pitch_deflection) > 0.05) {
            double rho = asin(pitch_deflection) * 3;
            ship->ApplyPitch(rho);
        }
    }

    ship->SetTransX(0);
    ship->SetTransY(0);
    ship->SetTransZ(z_shift * ship->Design()->trans_z);
    ship->SetFLCSMode(go_manual ? Ship::FLCS_MANUAL : Ship::FLCS_AUTO);
}

void
FighterAI::ThrottleControl()
{
    Element* elem              = ship->GetElement();
    double   ship_speed        = ship->Velocity() * ship->Heading();
    double   desired           = 1000;
    bool     formation         = element_index > 1;
    bool     station_keeping   = distance < 0;
    bool     augmenter         = false;
    Ship*    ward              = ship->GetWard();

    if (inbound || station_keeping)
    formation = false;

    // LAUNCH / TAKEOFF
    if (ship->MissionClock() < 10000) {
        formation = false;
        throttle  = 100;
        brakes    = 0;
    }

    // STATION KEEPING
    else if (station_keeping) {
        // go into a slow orbit if airborne:
        if (ship->IsAirborne() && ship->Class() < Ship::LCA) {
            throttle = 30;
            brakes   = 0;
        }
        else {
            throttle = 0;
            brakes   = 1;
        }
    }

    // TRY TO STAY AIRBORNE, YES?
    else if (ship->IsAirborne() && ship_speed < 250 && ship->Class() < Ship::LCA) {
        throttle = 100;
        brakes   = 0;

        if (ship_speed < 200)
        augmenter = true;
    }

    // INBOUND
    else if (inbound) {
        double carrier_speed = inbound->GetDeck()->GetCarrier()->Velocity().length();
        desired = 250 + carrier_speed;

        if (distance > 25.0e3)
        desired = 750 + carrier_speed;

        else if (ship->IsAirborne())
        desired = 300;

        else if (inbound->Final())
        desired = 75 + carrier_speed;

        throttle = 0;

        // holding short?
        if (inbound->Approach() == 0 && !inbound->Cleared() && 
                distance < 2000 && !ship->IsAirborne())
        desired = 0;

        if (ship_speed > desired+5)
        brakes   = 0.25;

        else if (ship->IsAirborne() || Ship::GetFlightModel() > 0) {
            throttle = old_throttle + 1;
        }

        else if (ship_speed < 0.85 * desired) {
            throttle = 100;

            if (ship_speed < 0 && ship->GetFuelLevel() > 10)
            augmenter = true;
        }

        else if (ship_speed < desired-5) {
            throttle = 30;
        }
    }

    else if (rtb_code || farcaster) {
        desired = 750;

        if (threat || threat_missile) {
            throttle = 100;

            if (!threat_missile && ship->GetFuelLevel() > 15)
            augmenter = true;
        }

        else {
            throttle = 0;

            if (ship_speed > desired+5)
            brakes   = 0.25;

            else if (Ship::GetFlightModel() > 0) {
                throttle = old_throttle + 1;
            }

            else if (ship_speed < 0.85 * desired) {
                throttle = 100;

                if (ship_speed < 0 && ship->GetFuelLevel() > 10)
                augmenter = true;
            }

            else if (ship_speed < desired-5) {
                throttle = 30;
            }
        }
    }

    // RUN AWAY!!!
    else if (evading) {
        throttle = 100;

        if (!threat_missile && ship->GetFuelLevel() > 15)
        augmenter = true;
    }

    // PATROL AND FORMATION
    else if (!navpt && !target && !ward) {
        if (!elem || !formation) {   // element lead
            if (patrol) {
                desired = 250;

                if (distance > 10e3)
                desired = 750;

                if (ship_speed > desired+5) {
                    brakes   = 0.25;
                    throttle = old_throttle - 5;
                }

                else if (ship_speed < 0.85 * desired) {
                    throttle = 100;

                    if (ship_speed < 0 && ship->GetFuelLevel() > 10)
                    augmenter = true;
                }

                else if (ship_speed < desired-5)
                throttle = old_throttle + 5;
            }

            else {
                throttle = 35;

                if (threat)
                throttle = 100;

                brakes = accumulator.brake;

                if (brakes > 0.1)
                throttle = 0;
            }
        }

        else {                              // wingman
            Ship*  lead = elem->GetShip(1);
            double zone = ship->Radius() * 3;

            if (lead)
            desired = lead->Velocity() * lead->Heading();

            if (fabs(slot_dist) < distance/4) // try to prevent porpoising
            throttle = old_throttle;

            else if (slot_dist > zone*2) {
                throttle = 100;

                if (objective.z > 10e3 && ship_speed < desired && ship->GetFuelLevel() > 25)
                augmenter = true;
            }

            else if (slot_dist > zone)
            throttle = lead->Throttle() + 10;

            else if (slot_dist < -zone*2) {
                throttle = old_throttle - 10;
                brakes   = 1;
            }

            else if (slot_dist < -zone) {
                throttle = old_throttle;
                brakes   = 0.5;
            }

            else if (lead) {
                double lv = lead->Velocity().length();
                double sv = ship_speed;
                double dv = lv-sv;
                double dt = 0;

                if (dv > 0)       dt = dv * 1e-5 * frame_time;
                else if (dv < 0)  dt = dv * 1e-2 * frame_time;

                throttle = old_throttle + dt;
            }

            else {
                throttle = old_throttle;
            }
        }
    }

    // TARGET/WARD/NAVPOINT SEEKING
    else {
        throttle = old_throttle;

        if (target) {
            desired = 1250;

            if (ai_level < 1) {
                throttle = 70;
            }

            else if (ship->IsAirborne()) {
                throttle = 100;

                if (!threat_missile && fabs(objective.z) > 6e3 && ship->GetFuelLevel() > 25)
                augmenter = true;
            }

            else {
                throttle = 100;

                if (objective.z > 20e3 && ship_speed < desired && ship->GetFuelLevel() > 35)
                augmenter = true;

                else if (objective.z > 0 && objective.z < 10e3)
                throttle = 50;
            }
        }
        
        else if (ward) {
            double d = (ship->Location() - ward->Location()).length();

            if (d > 5000) {
                if (ai_level < 1)
                throttle = 50;
                else
                throttle = 80;
            }
            else {
                double speed = ward->Velocity().length();

                if (speed > 0) {
                    if (ship_speed > speed) {
                        throttle = old_throttle - 5;
                        brakes   = 0.25;
                    }
                    else if (ship_speed < speed - 10) {
                        throttle =  old_throttle + 1;
                    }
                }
            }
        }

        else if (navpt) {
            desired = navpt->Speed();

            if (hold) {
                // go into a slow orbit if airborne:
                if (ship->IsAirborne() && ship->Class() < Ship::LCA) {
                    throttle = 25;
                    brakes   = 0;
                }
                else {
                    throttle = 0;
                    brakes   = 1;
                }
            }

            else if (desired > 0) {
                if (ship_speed > desired) {
                    throttle = old_throttle - 5;
                    brakes   = 0.25;
                }

                else if (ship_speed < 0.85 * desired) {
                    throttle = 100;

                    if ((ship->IsAirborne() || ship_speed < 0.35 * desired) && ship->GetFuelLevel() > 30)
                    augmenter = true;
                }

                else if (ship_speed < desired - 10) {
                    throttle =  old_throttle + 1;
                }

                else if (Ship::GetFlightModel() > 0) {
                    throttle = old_throttle;
                }
            }
        }

        else {
            throttle = 0;
            brakes   = 1;
        }
    }

    if (ship->IsAirborne() && throttle < 20 && ship->Class() < Ship::LCA)
    throttle = 20;
    else if (ship->Design()->auto_roll > 1 && throttle < 5)
    throttle = 5;
    else if (throttle < 0)
    throttle = 0;

    old_throttle = throttle;
    ship->SetThrottle((int) throttle);
    ship->SetAugmenter(augmenter);

    if (accumulator.stop && ship->GetFLCS() != 0)
    ship->GetFLCS()->FullStop();

    else if (ship_speed > 1 && brakes > 0)
    ship->SetTransY(-brakes * ship->Design()->trans_y);

    else if (throttle > 10 && (ship->GetEMCON() < 2 || ship->GetFuelLevel() < 10))
    ship->SetTransY(ship->Design()->trans_y);
}

// +--------------------------------------------------------------------+

Steer
FighterAI::AvoidTerrain()
{
    Steer avoid;

    terrain_warning = false;

    if (!ship || !ship->GetRegion() || !ship->GetRegion()->IsActive() ||
            (navpt && navpt->Action() == Instruction::LAUNCH))
    return avoid;

    if (ship->IsAirborne() && ship->GetFlightPhase() == Ship::ACTIVE) {
        // too high?
        if (ship->AltitudeMSL() > 25e3) {
            if (!navpt || (navpt->Region() == ship->GetRegion() && navpt->Location().z < 27e3)) {
                terrain_warning = true;
                ship->SetDirectorInfo(Game::GetText("ai.too-high"));

                // where will we be?
                Point selfpt = ship->Location() + ship->Velocity() + Point(0, -15e3, 0);

                // transform into camera coords:
                Point obj = Transform(selfpt);

                // head down!
                avoid = Seek(obj);
            }
        }

        // too low?
        else if (ship->AltitudeAGL() < 2500) {
            terrain_warning = true;
            ship->SetDirectorInfo(Game::GetText("ai.too-low"));

            // way too low?
            if (ship->AltitudeAGL() < 1500) {
                ship->SetDirectorInfo(Game::GetText("ai.way-too-low"));
                target    = 0;
                drop_time = 5;
            }

            // where will we be?
            Point selfpt = ship->Location() + ship->Velocity() + Point(0, 10e3, 0);

            // transform into camera coords:
            Point obj = Transform(selfpt);

            // pull up!
            avoid = Seek(obj);
        }
    }

    return avoid;
}

// +--------------------------------------------------------------------+

Steer
FighterAI::SeekTarget()
{
    if (ship->GetFlightPhase() < Ship::ACTIVE)
    return Seek(objective);

    Ship* ward = ship->GetWard();

    if ((!target && !ward && !navpt && !farcaster && !patrol && !inbound && !rtb_code) || ship->MissionClock() < 10000) {
        if (element_index > 1) {
            // break formation if threatened:
            if (threat_missile)
            return Steer();

            else if (threat && !form_up)
            return Steer();

            // otherwise, keep in formation:
            return SeekFormationSlot();
        }
        else {
            return Steer();
        }
    }

    if (patrol) {
        Steer result = Seek(objective);
        ship->SetDirectorInfo(Game::GetText("ai.seek-patrol-point"));

        if (distance <  10 * self->Radius()) {
            patrol = 0;
            result.brake = 1;
            result.stop  = 1;
        }
        
        return result;
    }

    if (inbound) {
        Steer result = Seek(objective);

        if (over_threshold && objective.z < 0) {
            result = Steer();
            result.brake = 1;
            result.stop  = 1;
        }
        else {
            ship->SetDirectorInfo(Game::GetText("ai.seek-inbound"));

            // approach legs:
            if (inbound->Approach() > 0) {
                if (distance < 20 * self->Radius())
                inbound->SetApproach(inbound->Approach() - 1);
            }

            // marshall point and finals:
            else {
                if (inbound->Cleared() && distance <  10 * self->Radius()) {
                    if (!inbound->Final()) {
                        time_to_dock = TIME_TO_DOCK;

                        FlightDeck* deck = inbound->GetDeck();
                        if (deck) {
                            double total_dist   = Point(deck->EndPoint() - deck->StartPoint()).length();
                            double current_dist = Point(deck->EndPoint() - ship->Location()).length();

                            time_to_dock *= (current_dist / total_dist);
                        }

                        RadioTraffic::SendQuickMessage(ship, RadioMessage::CALL_FINALS);
                    }

                    inbound->SetFinal(true);
                    ship->LowerGear();
                    result.brake = 1;
                    result.stop  = 1;
                }
                
                else if (!inbound->Cleared() && distance < 2000) {
                    ship->SetDirectorInfo(Game::GetText("ai.hold-final"));
                    result = Steer();
                    result.brake = 1;
                    result.stop  = 1;
                }
            }
        }

        return result;
    }

    else if (rtb_code) {
        return Seek(objective);
    }

    SimObject* tgt = target;

    if (ward && !tgt)
    tgt = ward;

    if (tgt && too_close == tgt->Identity()) {
        drop_time = 4;
        return Steer();
    }

    else if (navpt && navpt->Action() == Instruction::LAUNCH) {
        ship->SetDirectorInfo(Game::GetText("ai.launch"));
        return Seek(objective);
    }

    else if (farcaster) {
        // wingmen should
        if (element_index > 1)
        return SeekFormationSlot();

        ship->SetDirectorInfo(Game::GetText("ai.seek-farcaster"));
        return Seek(objective);
    }

    else if (drop_time > 0) {
        return Steer();
    }

    if (tgt) {
        double basis = self->Radius() + tgt->Radius();
        double gap   = distance - basis;

        // target behind:
        if (objective.z < 0) {
            // leave some room for an attack run:
            if (gap < 8000) {
                Steer s;

                s.pitch = -0.1;
                if (objective.x > 0) s.yaw = 0.1;
                else                 s.yaw = -0.1;   

                return s;
            }

            // start the attack run:
            else {
                return Seek(objective);
            }
        }

        // target in front:
        else {
            if (tgt->Type() == SimObject::SIM_SHIP) {
                Ship* tgt_ship = (Ship*) tgt;

                // capital target strike:
                if (tgt_ship->IsStatic()) {
                    if (gap < 2500)
                    return Flee(objective);
                }

                else if (tgt_ship->IsStarship()) {
                    if (gap < 1000)
                    return Flee(objective);

                    else if (ship->GetFlightModel() == Ship::FM_STANDARD && gap < 20e3)
                    go_manual = true;
                }
            }

            // fighter melee:
            if (tgt->Velocity() * ship->Velocity() < 0) {
                // head-to-head pass:
                if (gap < 1250)
                return Flee(objective);
            }

            else if (gap < 250) {
                return Steer();
            }

            ship->SetDirectorInfo(Game::GetText("ai.seek-target"));
            return Seek(objective);
        }
    }

    if (navpt) {
        ship->SetDirectorInfo(Game::GetText("ai.seek-navpt"));
    }

    return Seek(objective);
}

// +--------------------------------------------------------------------+

Steer
FighterAI::SeekFormationSlot()
{
    Steer s;

    // advance memory pipeline:
    az[2] = az[1]; az[1] = az[0];
    el[2] = el[1]; el[1] = el[0];

    Element* elem = ship->GetElement();
    Ship*    lead = elem->GetShip(1);

    if (lead) {
        SimRegion*  self_rgn = ship->GetRegion();
        SimRegion*  lead_rgn = lead->GetRegion();

        if (self_rgn != lead_rgn) {
            QuantumDrive*  qdrive = ship->GetQuantumDrive();
            bool use_farcaster    = !qdrive ||
            !qdrive->IsPowerOn() ||
            qdrive->Status() < System::DEGRADED;

            if (use_farcaster) {
                FindObjectiveFarcaster(self_rgn, lead_rgn);
            }

            else if (qdrive) {
                if (qdrive->ActiveState() == QuantumDrive::ACTIVE_READY) {
                    qdrive->SetDestination(lead_rgn, lead->Location());
                    qdrive->Engage();
                }
            }
        }
    }

    // do station keeping?
    if (distance < ship->Radius() * 10 && lead->Velocity().length() < 50) {
        distance = -1;
        return  s;
    }

    // approach
    if (objective.z > ship->Radius() * -4) {
        az[0] = atan2(fabs(objective.x), objective.z) * 50;
        el[0] = atan2(fabs(objective.y), objective.z) * 50;

        if (objective.x < 0) az[0] = -az[0];
        if (objective.y > 0) el[0] = -el[0];

        s.yaw   = az[0] - seek_damp * (az[1] + az[2] * 0.5);
        s.pitch = el[0] - seek_damp * (el[1] + el[2] * 0.5);
    }

    // reverse
    else {
        if (objective.x > 0) s.yaw = 1.0f;
        else             s.yaw = -1.0f;

        s.pitch = -objective.y * 0.5f;
    }

    seeking = 1;
    ship->SetDirectorInfo(Game::GetText("ai.seek-formation"));

    return s;
}

// +--------------------------------------------------------------------+

Steer
FighterAI::Seek(const Point& point)
{
    Steer s;

    // advance memory pipeline:
    az[2] = az[1]; az[1] = az[0];
    el[2] = el[1]; el[1] = el[0];

    // approach
    if (point.z > 0.0f) {
        az[0] = atan2(fabs(point.x), point.z) * seek_gain;
        el[0] = atan2(fabs(point.y), point.z) * seek_gain;
        
        if (point.x < 0) az[0] = -az[0];
        if (point.y > 0) el[0] = -el[0];

        s.yaw   = az[0] - seek_damp * (az[1] + az[2] * 0.5);
        s.pitch = el[0] - seek_damp * (el[1] + el[2] * 0.5);

        // pull up:
        if (ship->IsAirborne() && point.y > 5e3)
        s.pitch = -1.0f;
    }

    // reverse
    else {
        if (ship->IsAirborne()) {
            // pull up:
            if (point.y > 5e3) {
                s.pitch = -1.0f;
            }

            // head down:
            else if (point.y < -5e3) {
                s.pitch = 1.0f;
            }

            // level turn:
            else {
                if (point.x > 0) s.yaw = 1.0f;
                else             s.yaw = -1.0f;

                s.brake = 0.5f;
            }
        }

        else {
            if (point.x > 0) s.yaw = 1.0f;
            else             s.yaw = -1.0f;
        }
    }

    seeking = 1;

    return s;
}

// +--------------------------------------------------------------------+

Steer
FighterAI::EvadeThreat()
{
    // MISSILE THREAT REACTION:
    if (threat_missile) {
        evading = true;
        SetTarget(0);
        drop_time = 3 * (3-ai_level);

        // dropped a decoy for this missile yet?
        if (decoy_missile != threat_missile) {
            ship->FireDecoy();
            decoy_missile = threat_missile;
        }

        // beam the missile
        ship->SetDirectorInfo(Game::GetText("ai.evade-missile"));

        Point beam_line = threat_missile->Velocity().cross(Point(0,1,0));
        beam_line.Normalize();
        beam_line *= 1e6;

        Point evade_p;
        Point evade_w1 = threat_missile->Location() + beam_line;
        Point evade_w2 = threat_missile->Location() - beam_line;

        double d1 = Point(evade_w1 - ship->Location()).length();
        double d2 = Point(evade_w2 - ship->Location()).length();

        if (d1 > d2)
        evade_p = Transform(evade_w1);
        else
        evade_p = Transform(evade_w2);

        return Seek(evade_p);
    }

    // GENERAL THREAT EVASION:
    if (threat && !form_up) {
        double threat_range = 20e3;

        Ship*          threat_ship = (Ship*) threat;
        double         threat_dist = Point(threat->Location() - ship->Location()).length();

        if (threat_ship->IsStarship()) {
            threat_range = CalcDefensePerimeter(threat_ship);
        }

        if (threat_dist <= threat_range) {
            ship->SetDirectorInfo(Game::GetText("ai.evade-threat"));

            if (ship->IsAirborne()) {
                evading = true;
                Point beam_line = threat->Velocity().cross(Point(0,1,0));
                beam_line.Normalize();
                beam_line *= threat_range;

                Point evade_w = threat->Location() + beam_line;
                Point evade_p = Transform(evade_w);

                return Seek(evade_p);
            }

            else if (threat_ship->IsStarship()) {
                evading = true;

                if (target == threat_ship && threat_dist < threat_range / 4) {
                    SetTarget(0);
                    drop_time = 5;
                }

                if (!target) {
                    ship->SetDirectorInfo(Game::GetText("ai.evade-starship"));

                    // flee for three seconds:
                    if ((ship->MissionClock() & 3) != 3) {
                        return Flee(Transform(threat->Location()));
                    }

                    // jink for one second:
                    else {
                        if (Game::GameTime() - jink_time > 1500) {
                            jink_time = Game::GameTime();
                            jink      = Point(rand() - 16384,
                            rand() - 16384,
                            rand() - 16384) * 15e3;
                        }

                        Point evade_w = ship->Location() + jink;
                        Point evade_p = Transform(evade_w);

                        return Seek(evade_p);
                    }
                }

                else {
                    ship->SetDirectorInfo(Game::GetText("ai.evade-and-seek"));

                    // seek for three seconds:
                    if ((ship->MissionClock() & 3) < 3) {
                        return Steer(); // no evasion
                    }

                    // jink for one second:
                    else {
                        if (Game::GameTime() - jink_time > 1000) {
                            jink_time = Game::GameTime();
                            jink      = Point(rand() - 16384,
                            rand() - 16384,
                            rand() - 16384);
                        }

                        Point evade_w = target->Location() + jink;
                        Point evade_p = Transform(evade_w);

                        return Seek(evade_p);
                    }
                }
            }

            else {
                evading = true;
                
                if (target != nullptr) {
                    if (target == threat) {
                        if (target->Type() == SimObject::SIM_SHIP) {
                            Ship* tgt_ship = (Ship*) target;
                            if (tgt_ship->GetTrigger(0)) {
                                SetTarget(0);
                                drop_time = 3;
                            }
                        }
                    }

                    else if (target && threat_dist < threat_range / 2) {
                        SetTarget(0);
                        drop_time = 3;
                    }
                }

                if (target)
                ship->SetDirectorInfo(Game::GetText("ai.evade-and-seek"));
                else
                ship->SetDirectorInfo(Game::GetText("ai.random-evade"));

                // beam the threat
                Point beam_line = threat->Velocity().cross(Point(0,1,0));
                beam_line.Normalize();
                beam_line *= 1e6;

                Point evade_p;
                Point evade_w1 = threat->Location() + beam_line;
                Point evade_w2 = threat->Location() - beam_line;

                double d1 = Point(evade_w1 - ship->Location()).length();
                double d2 = Point(evade_w2 - ship->Location()).length();

                if (d1 > d2)
                evade_p = Transform(evade_w1);
                else
                evade_p = Transform(evade_w2);

                if (!target) {
                    DWORD jink_rate = 400 + 200 * (3-ai_level);

                    if (Game::GameTime() - jink_time > jink_rate) {
                        jink_time = Game::GameTime();
                        jink      = Point(rand() - 16384,
                        rand() - 16384,
                        rand() - 16384) * 2000;
                    }

                    evade_p += jink;
                }

                Steer steer = Seek(evade_p);

                if (target)
                return steer / 4;

                return steer;
            }
        }
    }

    return Steer();
}

// +--------------------------------------------------------------------+

void
FighterAI::FireControl()
{
    // if nothing to shoot at, forget it:
    if (!target || target->Integrity() < 1)
    return;

    // if the objective is a navpt or landing bay (not a target), then don't shoot!
    if (inbound || farcaster || navpt && navpt->Action() < Instruction::DEFEND)
    return;

    // object behind us, or too close:
    if (objective.z < 0 || distance < 4 * self->Radius())
    return;

    // compute the firing cone:
    double cross_section    = 2 * target->Radius() / distance;
    double gun_basket       = cross_section * 2;

    Weapon*              primary        = ship->GetPrimary();
    Weapon*              secondary      = ship->GetSecondary();
    const WeaponDesign*  dsgn_primary   = 0;
    const WeaponDesign*  dsgn_secondary = 0;
    bool                 use_primary    = true;
    Ship*                tgt_ship       = 0;

    if (target->Type() == SimObject::SIM_SHIP) {
        tgt_ship = (Ship*) target;

        if (tgt_ship->InTransition())
        return;
    }

    if (primary) {
        dsgn_primary = primary->Design();

        if (dsgn_primary->aim_az_max > 5*DEGREES && distance > dsgn_primary->max_range/2)
        gun_basket = cross_section * 4;

        gun_basket *= (3-ai_level);

        if (tgt_ship) {
            if (!primary->CanTarget(tgt_ship->Class()))
            use_primary = false;

            /*** XXX NEED TO SUBTARGET SYSTEMS IF TARGET IS STARSHIP...
        else if (tgt_ship->ShieldStrength() > 10)
            use_primary = false;
        ***/
        }

        if (use_primary) {
            // is target in the basket?
            double dx = fabs(objective.x / distance);
            double dy = fabs(objective.y / distance);

            if (primary->GetFiringOrders() == Weapon::MANUAL &&
                    dx < gun_basket && dy < gun_basket && 
                    distance > dsgn_primary->min_range &&
                    distance < dsgn_primary->max_range &&
                    !primary->IsBlockedFriendly())
            {
                ship->FirePrimary();
            }
        }
    }

    if (secondary && secondary->GetFiringOrders() == Weapon::MANUAL) {
        dsgn_secondary = secondary->Design();

        if (missile_time <= 0 && secondary->Ammo() && !secondary->IsBlockedFriendly()) {
            if (secondary->Locked() || !dsgn_secondary->self_aiming) {
                // is target in basket?
                Point tgt = AimTransform(target->Location());
                double tgt_range = tgt.Normalize();

                int    factor     = 2-ai_level;
                double s_range    = 0.5 + 0.2 * factor;
                double s_basket   = 0.3 + 0.2 * factor;
                double extra_time = 10        * factor * factor + 5;

                if (!dsgn_secondary->self_aiming)
                s_basket *= 0.33;

                if (tgt_ship) {
                    if (tgt_ship->Class() == Ship::MINE) {
                        extra_time = 10;
                        s_range    = 0.75;
                    }
                    
                    else if (!tgt_ship->IsDropship()) {
                        extra_time = 0.5 * factor + 0.5;
                        s_range = 0.9;
                    }
                }

                // is target in decent range?
                if (tgt_range < secondary->Design()->max_range * s_range) {
                    double dx = fabs(tgt.x);
                    double dy = fabs(tgt.y);

                    if (dx < s_basket && dy < s_basket && tgt.z > 0) {
                        if (ship->FireSecondary()) {
                            missile_time = secondary->Design()->salvo_delay + extra_time;

                            if (Game::GameTime() - last_call_time > 6000) {
                                // call fox:
                                int call = RadioMessage::FOX_3;                 // A2A

                                if (secondary->CanTarget(Ship::GROUND_UNITS))   // AGM
                                call = RadioMessage::FOX_1;

                                else if (secondary->CanTarget(Ship::DESTROYER)) // ASM
                                call = RadioMessage::FOX_2;

                                RadioTraffic::SendQuickMessage(ship, call);
                                last_call_time = Game::GameTime();
                            }
                        }
                    }
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

double
FighterAI::CalcDefensePerimeter(Ship* starship)
{
    double perimeter = 15e3;

    if (starship) {
        ListIter<WeaponGroup> g_iter = starship->Weapons();
        while (++g_iter) {
            WeaponGroup* group = g_iter.value();

            ListIter<Weapon> w_iter = group->GetWeapons();
            while (++w_iter) {
                Weapon* weapon = w_iter.value();

                if (weapon->Ammo() && 
                        weapon->GetTarget() == ship && 
                        !weapon->IsBlockedFriendly()) {

                    double range = weapon->Design()->max_range * 1.2;
                    if (range > perimeter)
                    perimeter = range;
                }
            }
        }
    }

    return perimeter;
}



