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
    FILE:         ShipAI.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship (low-level) Artificial Intelligence class
*/

#include "MemDebug.h"
#include "ShipAI.h"
#include "TacticalAI.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Shot.h"
#include "Element.h"
#include "NavLight.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Contact.h"
#include "WeaponGroup.h"
#include "Drive.h"
#include "Shield.h"
#include "Sim.h"
#include "Player.h"
#include "StarSystem.h"
#include "FlightComp.h"
#include "Farcaster.h"
#include "QuantumDrive.h"
#include "Debris.h"
#include "Asteroid.h"

#include "Game.h"
#include "Random.h"

// +----------------------------------------------------------------------+

ShipAI::ShipAI(SimObject* s)
    : SteerAI(s),
      support(0), rumor(0), threat(0), threat_missile(0), drop_time(0),
      too_close(0), navpt(0), patrol(0), engaged_ship_id(0),
      bracket(false), identify(false), hold(false), takeoff(false),
      throttle(0), old_throttle(0), element_index(1), splash_count(0),
      tactical(0), farcaster(0), ai_level(2), last_avoid_time(0),
      last_call_time(0)
{
    ship = (Ship*) self;

    Sim*  sim         = Sim::GetSim();
    Ship* pship       = sim->GetPlayerShip();
    int   player_team = 1;

    if (pship)
    player_team = pship->GetIFF();

    Player* player = Player::GetCurrentPlayer();
    if (player) {
        if (ship && ship->GetIFF() && ship->GetIFF() != player_team) {
            ai_level = player->AILevel();
        }
        else if (player->AILevel() == 0) {
            ai_level = 1;
        }
    }

    // evil alien ships are *always* smart:
    if (ship && ship->GetIFF() > 1 && ship->Design()->auto_roll > 1) {
        ai_level = 2;
    }
}


// +--------------------------------------------------------------------+

ShipAI::~ShipAI()
{ 
    delete tactical;
}

void
ShipAI::ClearTactical()
{
    delete tactical;
    tactical = 0;
}

// +--------------------------------------------------------------------+

Ship*
ShipAI::GetWard() const
{
    return ship->GetWard();
}

void
ShipAI::SetWard(Ship* s)
{
    if (ship == nullptr)
    return;
    if (s == ship->GetWard())
    return;

    if (ship)
        ship->SetWard(s);

    Point form = RandomDirection();
    form.SwapYZ();

    if (fabs(form.x) < 0.5) {
        if (form.x < 0)
        form.x = -0.5;
        else
        form.x = 0.5;
    }

    if (ship && ship->IsStarship()) {
        form *= 30e3;
    }
    else {
        form *= 15e3;
        form.y = 500;
    }

    SetFormationDelta(form);
}

void
ShipAI::SetSupport(Ship* s)
{
    if (support == s)
    return;

    support = s;

    if (support)
    Observe(support);
}

void
ShipAI::SetRumor(Ship* s)
{
    if (!s || rumor == s)
    return;

    rumor = s;

    if (rumor)
    Observe(rumor);
}

void
ShipAI::ClearRumor()
{
    rumor = 0;
}

void
ShipAI::SetThreat(Ship* s)
{
    if (threat == s)
    return;

    threat = s;

    if (threat)
    Observe(threat);
}

void
ShipAI::SetThreatMissile(Shot* s)
{
    if (threat_missile == s)
    return;

    threat_missile = s;

    if (threat_missile)
    Observe(threat_missile);
}

bool
ShipAI::Update(SimObject* obj)
{
    if (obj == support)
    support = 0;

    if (obj == threat)
    threat = 0;

    if (obj == threat_missile)
    threat_missile = 0;

    if (obj == rumor)
    rumor = 0;

    return SteerAI::Update(obj);
}

const char*
ShipAI::GetObserverName() const
{
    static char name[64];
    sprintf_s(name, "ShipAI(%s)", self->Name());
    return name;
}

// +--------------------------------------------------------------------+

Point
ShipAI::GetPatrol() const
{
    return patrol_loc;
}

void
ShipAI::SetPatrol(const Point& p)
{
    patrol = 1;
    patrol_loc = p;
}

void
ShipAI::ClearPatrol()
{
    patrol = 0;
}

// +--------------------------------------------------------------------+

void
ShipAI::ExecFrame(double secs)
{
    seconds = secs;

    if (drop_time > 0) drop_time -= seconds;
    if (!ship) return;

    ship->SetDirectorInfo(" ");

    // check to make sure current navpt is still valid:
    if (navpt)
    navpt = ship->GetNextNavPoint();

    if (ship->GetFlightPhase() == Ship::TAKEOFF || ship->GetFlightPhase() == Ship::LAUNCH)
    takeoff = true;

    if (takeoff) {
        FindObjective();
        Navigator();

        if (ship->MissionClock() > 10000)
        takeoff = false;

        return;
    }

    // initial assessment:
    if (ship->MissionClock() < 5000)
    return;

    element_index = ship->GetElementIndex();

    NavlightControl();
    CheckTarget();

    if (tactical)
    tactical->ExecFrame(seconds);

    if (target && target != ship->GetTarget()) {
        ship->LockTarget(target);

        // if able to lock target, and target is a ship (not a shot)...
        if (target == ship->GetTarget() && target->Type() == SimObject::SIM_SHIP) {

            // if this isn't the same ship we last called out:
            if (target->Identity() != engaged_ship_id && Game::GameTime() - last_call_time > 10000) {
                // call engaging:
                RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(ship->GetElement(), ship, RadioMessage::CALL_ENGAGING);
                msg->AddTarget(target);
                RadioTraffic::Transmit(msg);
                last_call_time = Game::GameTime();

                engaged_ship_id = target->Identity();
            }
        }
    }

    else if (!target) {
        target = ship->GetTarget();

        if (engaged_ship_id && !target) {
            engaged_ship_id = 0;

            /***
        *** XXX 
        *** Not the right place to make this decision.
        ***
        *** There is a brief wait between killing a target and
        *** selecting a new one, so this message is sent after
        *** every kill.
        ***
        *** Need to track when the entire element has been
        *** put down.

        if (element_index == 1) {
            RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(ship->GetElement(), ship, RadioMessage::RESUME_MISSION);
            RadioTraffic::Transmit(msg);
        }

        ***
        ***/
        }
    }

    FindObjective();
    Navigator();
}

// +--------------------------------------------------------------------+

Point
ShipAI::ClosingVelocity()
{
    if (ship && target) {
        if (ship->GetPrimaryDesign()) {
            WeaponDesign* guns  = ship->GetPrimaryDesign();
            Point         delta = target->Location() - ship->Location();

            // fighters need to aim the ship so that the guns will hit the target
            if (guns->firing_cone < 10*DEGREES && guns->max_range <= delta.length()) {
                Point aim_vec = ship->Heading();
                aim_vec.Normalize();

                Point shot_vel = ship->Velocity() + aim_vec * guns->speed;
                return shot_vel - target->Velocity();
            }

            // ships with turreted weapons just need to worry about actual closing speed
            else {
                return ship->Velocity() - target->Velocity();
            }
        }

        else {
            return ship->Velocity();
        }
    }

    return Point(1,0,0);
}

// +--------------------------------------------------------------------+

void
ShipAI::FindObjective()
{
    distance = 0;

    int   order = ship->GetRadioOrders()->Action();

    if (order == RadioMessage::QUANTUM_TO ||
            order == RadioMessage::FARCAST_TO) {

        FindObjectiveQuantum();
        objective = Transform(obj_w);
        return;
    }

    bool  form  = (order == RadioMessage::WEP_HOLD)    ||
    (order == RadioMessage::FORM_UP)     ||
    (order == RadioMessage::MOVE_PATROL) ||
    (order == RadioMessage::RTB)         ||
    (order == RadioMessage::DOCK_WITH)   ||
    (!order && !target)                  ||
    (farcaster);

    Ship* ward  = ship->GetWard();

    // if not the element leader, stay in formation:
    if (form && element_index > 1) {
        ship->SetDirectorInfo(Game::GetText("ai.formation"));

        if (navpt && navpt->Action() == Instruction::LAUNCH) {
            FindObjectiveNavPoint();
        }
        else {
            navpt = 0;
            FindObjectiveFormation();
        }

        // transform into camera coords:
        objective = Transform(obj_w);
        return;
    }

    // under orders?
    bool directed = false;
    if (tactical)
    directed = (tactical->RulesOfEngagement() == TacticalAI::DIRECTED);

    // threat processing:
    if (threat && !directed) {
        double d_threat = Point(threat->Location() - ship->Location()).length();

        // seek support:
        if (support) {
            double d_support = Point(support->Location() - ship->Location()).length();
            if (d_support > 35e3) {
                ship->SetDirectorInfo(Game::GetText("ai.regroup"));
                FindObjectiveTarget(support);
                objective = Transform(obj_w);
                return;
            }
        }

        // run away:
        else if (threat != target) {
            ship->SetDirectorInfo(Game::GetText("ai.retreat"));
            obj_w = ship->Location() + Point(ship->Location() - threat->Location()) * 100;
            objective = Transform(obj_w);
            return;
        }
    }

    // normal processing:
    if (target) {
        ship->SetDirectorInfo(Game::GetText("ai.seek-target"));
        FindObjectiveTarget(target);
        objective = AimTransform(obj_w);
    }

    else if (patrol) {
        ship->SetDirectorInfo(Game::GetText("ai.patrol"));
        FindObjectivePatrol();
        objective = Transform(obj_w);
    }

    else if (ward) {
        ship->SetDirectorInfo(Game::GetText("ai.seek-ward"));
        FindObjectiveFormation();
        objective = Transform(obj_w);
    }

    else if (navpt && form) {
        ship->SetDirectorInfo(Game::GetText("ai.seek-navpt"));
        FindObjectiveNavPoint();
        objective = Transform(obj_w);
    }

    else if (rumor) {
        ship->SetDirectorInfo(Game::GetText("ai.search"));
        FindObjectiveTarget(rumor);
        objective = Transform(obj_w);
    }

    else {
        obj_w     = Point();
        objective = Point();
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::FindObjectiveTarget(SimObject* tgt)
{
    if (!tgt) {
        obj_w = Point();
        return;
    }

    navpt = 0; // this tells fire control that we are chasing a target,
    // instead of a navpoint!

    Point  cv   = ClosingVelocity();
    double cvl  = cv.length();
    double time = 0;

    if (cvl > 50) {
        // distance from self to target:
        distance = Point(tgt->Location() - self->Location()).length();

        // time to reach target:
        time = distance / cvl;

        // where the target will be when we reach it:
        if (time < 15) {
            Point run_vec = tgt->Velocity();
            obj_w   = tgt->Location() + (run_vec * time);
            

            if (time < 10)
            obj_w += (tgt->Acceleration() * 0.33 * time * time);
        }
        else {
            obj_w = tgt->Location();
        }
    }

    else {
        obj_w = tgt->Location();
    }

    distance = Point(obj_w - self->Location()).length();

    if (cvl > 50) {
        time = distance / cvl;

        // where we will be when the target gets there:
        if (time < 15) {
            Point self_dest = self->Location() + cv * time;
            Point err = obj_w - self_dest;

            obj_w += err;
        }
    }

    Point approach = obj_w - self->Location();
    distance = approach.length();

    if (bracket && distance > 25e3) {
        Point offset = approach.cross(Point(0,1,0));
        offset.Normalize();
        offset *= 15e3;

        Ship* s = (Ship*) self;
        if (s->GetElementIndex() & 1)
        obj_w -= offset;
        else
        obj_w += offset;
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::FindObjectivePatrol()
{
    navpt = 0;

    Point npt = patrol_loc;
    obj_w = npt;

    // distance from self to navpt:
    distance = Point(obj_w - self->Location()).length();

    if (distance < 1000) {
        ship->ClearRadioOrders();
        ClearPatrol();
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::FindObjectiveNavPoint()
{
    SimRegion*     self_rgn = ship->GetRegion();
    SimRegion*     nav_rgn  = navpt->Region();
    QuantumDrive*  qdrive   = ship->GetQuantumDrive();

    if (!self_rgn)
    return;

    if (!nav_rgn) {
        nav_rgn = self_rgn;
        navpt->SetRegion(nav_rgn);
    }

    bool use_farcaster      = self_rgn != nav_rgn && 
    (navpt->Farcast() || 
    !qdrive ||
    !qdrive->IsPowerOn() ||
    qdrive->Status() < System::DEGRADED
    );

    if (use_farcaster) {
        FindObjectiveFarcaster(self_rgn, nav_rgn);
    }

    else {
        if (farcaster) {
            if (farcaster->GetShip()->GetRegion() != self_rgn)
            farcaster = farcaster->GetDest()->GetFarcaster();

            obj_w = farcaster->EndPoint();
        }

        else {
            // transform from starsystem to world coordinates:
            Point npt = navpt->Region()->Location() + navpt->Location();

            SimRegion* active_region = ship->GetRegion();

            if (active_region)
            npt -= active_region->Location();

            npt = npt.OtherHand();

            obj_w = npt;
        }

        // distance from self to navpt:
        distance = Point(obj_w - ship->Location()).length();

        if (farcaster && distance < 1000)
        farcaster = 0;

        if (distance <  1000 || (navpt->Action() == Instruction::LAUNCH && distance > 25000))
        ship->SetNavptStatus(navpt, Instruction::COMPLETE);
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::FindObjectiveQuantum()
{
    Instruction*   orders   = ship->GetRadioOrders();
    SimRegion*     self_rgn = ship->GetRegion();
    SimRegion*     nav_rgn  = orders->Region();
    QuantumDrive*  qdrive   = ship->GetQuantumDrive();

    if (!self_rgn || !nav_rgn)
    return;

    bool use_farcaster      = self_rgn != nav_rgn && 
    (orders->Farcast() || 
    !qdrive ||
    !qdrive->IsPowerOn() ||
    qdrive->Status() < System::DEGRADED
    );

    if (use_farcaster) {
        FindObjectiveFarcaster(self_rgn, nav_rgn);
    }

    else {
        if (farcaster) {
            if (farcaster->GetShip()->GetRegion() != self_rgn)
            farcaster = farcaster->GetDest()->GetFarcaster();

            obj_w = farcaster->EndPoint();
        }

        else {
            // transform from starsystem to world coordinates:
            Point npt = orders->Region()->Location() + orders->Location();

            SimRegion* active_region = ship->GetRegion();

            if (active_region)
            npt -= active_region->Location();

            npt = npt.OtherHand();

            obj_w = npt;

            if (qdrive && qdrive->ActiveState() == QuantumDrive::ACTIVE_READY) {
                qdrive->SetDestination(nav_rgn, orders->Location());
                qdrive->Engage();
                return;
            }
        }

        // distance from self to navpt:
        distance = Point(obj_w - ship->Location()).length();

        if (farcaster) {
            if (distance < 1000) {
                farcaster = 0;
                ship->ClearRadioOrders();
            }
        }
        else if (self_rgn == nav_rgn) {
            ship->ClearRadioOrders();
        }
    }
}

void
ShipAI::FindObjectiveFarcaster(SimRegion* src_rgn, SimRegion* dst_rgn)
{
    if (!farcaster) {
        ListIter<Ship> s = src_rgn->Ships();
        while (++s && !farcaster) {
            if (s->GetFarcaster()) {
                const Ship* dest = s->GetFarcaster()->GetDest();
                if (dest && dest->GetRegion() == dst_rgn) {
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
        }

        else {
            double r2 = (ship->Location() - apt).length();
            double r3 = (npt - apt).length();

            if (r1+r2 < 1.2*r3) {
                obj_w     = npt;
                distance  = r1;
            }
            else {
                obj_w     = apt;
                distance  = r2;
            }
        }

        objective = Transform(obj_w);
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::SetFormationDelta(const Point& point)
{
    formation_delta = point;
}

void
ShipAI::FindObjectiveFormation()
{
    const double prediction = 5;

    // find the base position:
    Element* elem = ship->GetElement();
    Ship*    lead = elem->GetShip(1);
    Ship*    ward = ship->GetWard();

    if (!lead || lead == ship) {
        lead = ward;

        distance = (lead->Location() - self->Location()).length();
        if (distance < 30e3 && lead->Velocity().length() < 50) {
            obj_w = self->Location() + lead->Heading() * 1e6;
            distance = -1;
            return;
        }
    }

    obj_w       = lead->Location() + lead->Velocity() * prediction;
    Matrix m;   m.Rotate(0, 0, lead->CompassHeading() - PI);
    Point  fd   = formation_delta * m;
    obj_w       += fd;

    // try to avoid smacking into the ground...
    if (ship->IsAirborne()) {
        if (ship->AltitudeAGL() < 3000 || lead->AltitudeAGL() < 3000) {
            obj_w.y += 500;
        }
    }

    Point dst_w = self->Location() + self->Velocity() * prediction;
    Point dlt_w = obj_w - dst_w;

    distance    = dlt_w.length();

    // get slot z distance:
    dlt_w       += ship->Location();
    slot_dist   = Transform(dlt_w).z;

    Director* lead_dir = lead->GetDirector();
    if (lead_dir && (lead_dir->Type() == FIGHTER || lead_dir->Type() == STARSHIP)) {
        ShipAI* lead_ai = (ShipAI*) lead_dir;
        farcaster = lead_ai->GetFarcaster();
    }
    else {
        Instruction* navpt = elem->GetNextNavPoint();
        if (!navpt) {
            farcaster = 0;
            return;
        }

        SimRegion*     self_rgn = ship->GetRegion();
        SimRegion*     nav_rgn  = navpt->Region();
        QuantumDrive*  qdrive   = ship->GetQuantumDrive();

        if (self_rgn && !nav_rgn) {
            nav_rgn = self_rgn;
            navpt->SetRegion(nav_rgn);
        }

        bool use_farcaster      = self_rgn != nav_rgn && 
        (navpt->Farcast() || 
        !qdrive ||
        !qdrive->IsPowerOn() ||
        qdrive->Status() < System::DEGRADED
        );

        if (use_farcaster) {
            ListIter<Ship> s = self_rgn->Ships();
            while (++s && !farcaster) {
                if (s->GetFarcaster()) {
                    const Ship* dest = s->GetFarcaster()->GetDest();
                    if (dest && dest->GetRegion() == nav_rgn) {
                        farcaster = s->GetFarcaster();
                    }
                }
            }
        }
        else if (farcaster) {
            if (farcaster->GetShip()->GetRegion() != self_rgn)
            farcaster = farcaster->GetDest()->GetFarcaster();

            obj_w = farcaster->EndPoint();
            distance = Point(obj_w - ship->Location()).length();

            if (distance < 1000)
            farcaster = 0;
        }
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::Splash(const Ship* targ)
{
    if (splash_count > 6)
    splash_count = 4;

    // call splash:
    RadioTraffic::SendQuickMessage(ship, RadioMessage::SPLASH_1 + splash_count);
    splash_count++;
}

// +--------------------------------------------------------------------+

void
ShipAI::SetTarget(SimObject* targ, System* sub)
{
    if (targ != target) {
        bracket = false;
    }

    SteerAI::SetTarget(targ, sub);
}

void
ShipAI::DropTarget(double dtime)
{
    SetTarget(0);
    drop_time = dtime;    // seconds until we can re-acquire

    ship->DropTarget();
}

void
ShipAI::SetBracket(bool b)
{
    bracket = b;
    identify = false;
}

void
ShipAI::SetIdentify(bool i)
{
    identify = i;
    bracket = false;
}

// +--------------------------------------------------------------------+

void
ShipAI::Navigator()
{
    accumulator.Clear();
    magnitude = 0;

    hold = false;
    if ((ship->GetElement() && ship->GetElement()->GetHoldTime() > 0) || 
            (navpt && navpt->Status() == Instruction::COMPLETE && navpt->HoldTime() > 0))
    hold = true;

    ship->SetFLCSMode(Ship::FLCS_HELM);

    if (target)
    ship->SetDirectorInfo(Game::GetText("ai.seek-target"));
    else if (rumor)
    ship->SetDirectorInfo(Game::GetText("ai.seek-rumor"));
    else
    ship->SetDirectorInfo(Game::GetText("ai.none"));   

    Accumulate(AvoidCollision());
    Accumulate(AvoidTerrain());

    if (!hold)
    Accumulate(SeekTarget());

    HelmControl();
    ThrottleControl();
    FireControl();
    AdjustDefenses();
}

// +--------------------------------------------------------------------+

void
ShipAI::HelmControl()
{
    double trans_x = 0;
    double trans_y = 0;
    double trans_z = 0;

    ship->SetHelmHeading(accumulator.yaw);

    if (fabs(accumulator.pitch) < 5*DEGREES || fabs(accumulator.pitch) > 45*DEGREES) {
        trans_z = objective.y;
        ship->SetHelmPitch(0);
    }

    else {
        ship->SetHelmPitch(accumulator.pitch);
    }

    ship->SetTransX(trans_x);
    ship->SetTransY(trans_y);
    ship->SetTransZ(trans_z);

    ship->ExecFLCSFrame();
}

/*****************************************
**
**  NOTE:
**  No one is really using this method.
**  It is overridden by both StarshipAI
**  and FighterAI.
**
*****************************************/

void
ShipAI::ThrottleControl()
{
    if (navpt && !threat && !target) {     // lead only, get speed from navpt
        double speed = navpt->Speed();

        if (speed > 0)
        throttle = speed / ship->VelocityLimit() * 100;
        else
        throttle = 50;
    }

    else if (patrol && !threat && !target) { // lead only, get speed from navpt
        double speed = 200;

        if (distance > 5000)
        speed = 500;

        if (ship->Velocity().length() > speed)
        throttle = 0;
        else
        throttle = 50;
    }

    else {
        if (threat || target || element_index < 2) { // element lead
            throttle = 100;

            if (!threat && !target)
            throttle = 50;

            if (accumulator.brake > 0) {
                throttle *= (1 - accumulator.brake);
            }
        }

        else {                                       // wingman
            Ship*  lead = ship->GetElement()->GetShip(1);
            double lv = lead->Velocity().length();
            double sv = ship->Velocity().length();
            double dv = lv-sv;
            double dt = 0;

            if (dv > 0)       dt = dv * 1e-2 * seconds;
            else if (dv < 0)  dt = dv * 1e-2 * seconds;

            throttle = old_throttle + dt;
        }
    }

    old_throttle = throttle;
    ship->SetThrottle((int) throttle);
}

// +--------------------------------------------------------------------+

void
ShipAI::NavlightControl()
{
    Ship* leader  = ship->GetLeader();

    if (leader && leader != ship) {
        bool navlight_enabled = false;

        if (leader->NavLights().size() > 0)
        navlight_enabled = leader->NavLights().at(0)->IsEnabled();

        for (int i = 0; i < ship->NavLights().size(); i++) {
            if (navlight_enabled)
            ship->NavLights().at(i)->Enable();
            else
            ship->NavLights().at(i)->Disable();
        }
    }
}

// +--------------------------------------------------------------------+

Steer
ShipAI::AvoidTerrain()
{
    Steer avoid;
    return avoid;
}

// +--------------------------------------------------------------------+

Steer
ShipAI::AvoidCollision()
{
    Steer avoid;

    if (!ship || !ship->GetRegion() || !ship->GetRegion()->IsActive())
    return avoid;

    if (other && (other->Life() == 0 || other->Integrity() < 1)) {
        other = 0;
        last_avoid_time = 0; // check for a new obstacle immediately
    }

    if (!other && Game::GameTime() - last_avoid_time < 500)
    return avoid;

    brake = 0;

    // don't get closer than this:
    double avoid_dist = 5 * self->Radius();

    if      (avoid_dist <  1e3) avoid_dist =  1e3;
    else if (avoid_dist > 12e3) avoid_dist = 12e3;

    // find the soonest potential collision,
    // ignore any that occur after this:
    double avoid_time = 15;

    if (ship->Design()->avoid_time > 0)
    avoid_time = ship->Design()->avoid_time;
    else if (ship->IsStarship())
    avoid_time *= 1.5;

    Point  bearing = self->Velocity();
    bearing.Normalize();

    bool              found          = false;
    int               num_contacts   = ship->NumContacts();
    ListIter<Contact> contact        = ship->ContactList();

    // check current obstacle first:
    if (other) {
        found = AvoidTestSingleObject(other, bearing, avoid_dist, avoid_time, avoid);
    }

    if (!found) {
        // avoid ships:
        while (++contact && !found) {
            Ship* c_ship = contact->GetShip();

            if (c_ship && c_ship != ship && c_ship->IsStarship()) {
                found = AvoidTestSingleObject(c_ship, bearing, avoid_dist, avoid_time, avoid);
            }
        }

        // also avoid large pieces of debris:
        if (!found) {
            ListIter<Debris> iter = ship->GetRegion()->Rocks();
            while (++iter && !found) {
                Debris* debris = iter.value();

                if (debris->Mass() > ship->Mass())
                found = AvoidTestSingleObject(debris, bearing, avoid_dist, avoid_time, avoid);
            }
        }

        // and asteroids:
        if (!found) {
            // give asteroids a wider berth -
            avoid_dist *= 8;

            ListIter<Asteroid> iter = ship->GetRegion()->Roids();
            while (++iter && !found) {
                Asteroid* roid = iter.value();
                found = AvoidTestSingleObject(roid, bearing, avoid_dist, avoid_time, avoid);
            }

            if (!found)
            avoid_dist /= 8;
        }

        // if found, steer to avoid:
        if (other) {
            avoid = Avoid(obstacle, (float) (ship->Radius() + other->Radius() + avoid_dist * 0.9));
            avoid.brake = brake;

            ship->SetDirectorInfo(Game::GetText("ai.avoid-collision"));
        }
    }

    last_avoid_time = Game::GameTime();
    return avoid;
}

bool
ShipAI::AvoidTestSingleObject(SimObject*     obj,
const Point&   bearing,
double         avoid_dist,
double&        avoid_time,
Steer&         avoid)
{
    if (too_close == obj->Identity()) {
        double dist = (ship->Location() - obj->Location()).length();
        double closure = (ship->Velocity() - obj->Velocity()) * bearing;

        if (closure > 1 && dist < avoid_dist) {
            avoid = AvoidCloseObject(obj);
            return true;
        }
        else {
            too_close = 0;
        }
    }

    // will we get close?
    double time = ClosestApproachTime(ship->Location(), ship->Velocity(),
    obj->Location(),  obj->Velocity());

    // already past the obstacle:
    if (time <= 0) {
        if (other == obj) other = 0;
        return false;
    }

    // how quickly could we collide?
    Point  current_relation = ship->Location() - obj->Location();
    double current_distance = current_relation.length() - ship->Radius() - obj->Radius();

    // are we really far away?
    if (current_distance > 25e3) {
        if (other == obj) other = 0;
        return false;
    }

    // is the obstacle a farcaster?
    if (obj->Type() == SimObject::SIM_SHIP) {
        Ship* c_ship = (Ship*) obj;

        if (c_ship->GetFarcaster()) {
            // are we on a safe vector?
            Point dir = ship->Velocity();
            dir.Normalize();

            double angle_off = fabs(acos(dir * obj->Cam().vpn()));

            if (angle_off > 90*DEGREES)
            angle_off = 180*DEGREES - angle_off;

            if (angle_off < 35*DEGREES) {
                // will we pass through the center?
                Point  d   = ship->Location() + dir * (current_distance + ship->Radius() + obj->Radius());
                double err = (obj->Location() - d).length();

                if (err < 0.667 * obj->Radius()) {
                    return false;
                }
            }
        }
    }

    // rate of closure:
    double closing_velocity = (ship->Velocity() - obj->Velocity()) * bearing;

    // are we too close already?
    if (current_distance < (avoid_dist * 0.35)) {
        if (closing_velocity > 1 || current_distance < ship->Radius()) {
            avoid = AvoidCloseObject(obj);
            return true;
        }
    }

    // too far away to worry about:
    double separation = (avoid_dist + obj->Radius());
    if ((current_distance-separation) / closing_velocity > avoid_time) {
        if (other == obj) other = 0;
        return false;
    }

    // where will we be?
    Point selfpt = ship->Location()   + ship->Velocity()   * time;
    Point testpt = obj->Location() + obj->Velocity() * time;

    // how close will we get?
    double dist = (selfpt - testpt).length()
    - ship->Radius()
    - obj->Radius();

    // that's too close:
    if (dist < avoid_dist) {
        if (dist < avoid_dist * 0.25 && time < avoid_time * 0.5) {
            avoid = AvoidCloseObject(obj);
            return true;
        }

        obstacle = Transform(testpt);

        if (obstacle.z > 0) {
            other      = obj;
            avoid_time = time;
            brake      = 0.5;

            Observe(other);
        }
    }

    // hysteresis:
    else if (other == obj && dist > avoid_dist * 1.25) {
        other = 0;
    }

    return false;
}

// +--------------------------------------------------------------------+

Steer
ShipAI::AvoidCloseObject(SimObject* obj)
{
    too_close  = obj->Identity();
    obstacle   = Transform(obj->Location());
    other      = obj;

    Observe(other);

    Steer avoid = Flee(obstacle);
    avoid.brake = 0.3;

    ship->SetDirectorInfo(Game::GetText("ai.avoid-collision"));
    return avoid;
}

// +--------------------------------------------------------------------+

Steer
ShipAI::SeekTarget()
{
    Ship* ward  = ship->GetWard();

    if (!target && !ward && !navpt && !patrol) {
        if (element_index > 1) {
            // wingmen keep in formation:
            return Seek(objective);
        }

        if (farcaster) {
            return Seek(objective);
        }

        if (rumor) {
            return Seek(objective);
        }

        return Steer();
    }

    if (patrol) {
        Steer result = Seek(objective);

        if (distance <  2000) {
            result.brake = 1;
        }
        
        return result;
    }

    if (target && too_close == target->Identity()) {
        drop_time = 4;
        return Avoid(objective, 0.0f);
    }
    else if (drop_time > 0) {
        return Steer();
    }

    return Seek(objective);
}

// +--------------------------------------------------------------------+

Steer
ShipAI::EvadeThreat()
{
    return Steer();
}

// +--------------------------------------------------------------------+

void
ShipAI::FireControl()
{
}

// +--------------------------------------------------------------------+

void
ShipAI::AdjustDefenses()
{
    Shield* shield = ship->GetShield();

    if (shield) {
        double  desire = 50;

        if (threat_missile || threat)
        desire = 100;

        shield->SetPowerLevel(desire);
    }
}

// +--------------------------------------------------------------------+

void
ShipAI::CheckTarget()
{
    if (target) {
        if (target->Life() == 0)
        target = 0;

        else if (target->Type() == SimObject::SIM_SHIP) {
            Ship* tgt_ship = (Ship*) target;

            if (tgt_ship->GetIFF() == ship->GetIFF() && !tgt_ship->IsRogue())
            target = 0;
        }
    }
}
