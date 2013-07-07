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
    FILE:         Hangar.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Everything needed to store and maintain space craft

    See Also: FlightDeck
*/

#include "MemDebug.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Sim.h"
#include "Instruction.h"
#include "Element.h"
#include "Mission.h"
#include "RadioMessage.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"

#include "Game.h"
#include "Random.h"

// +======================================================================+

class HangarSlot
{
    friend class Hangar;

public:
    static const char* TYPENAME() { return "HangarSlot"; }

    HangarSlot();
    ~HangarSlot();
    int operator == (const HangarSlot& that)  const { return this == &that; }

private:
    Text              squadron;
    CombatGroup*      group;
    Ship*             ship;
    int               iff;
    const ShipDesign* design;
    FlightDeck*       deck;
    int               slot;
    int               state;
    double            time;
    Element*          package;
    bool              alert_hold;
    int               loadout[16];
};

// +--------------------------------------------------------------------+

HangarSlot::HangarSlot()
    :  ship(0), group(0), design(0), deck(0), slot(0),
       state(0), time(0), package(0), alert_hold(false)
{ 
    for (int i = 0; i < 16; i++)
    loadout[i] = -1;
}

HangarSlot::~HangarSlot()
{ 
}

// +======================================================================+

Hangar::Hangar()
    : ship(0), nsquadrons(0), last_patrol_launch(0)
{
    ZeroMemory(nslots,    sizeof(nslots));
    ZeroMemory(squadrons, sizeof(squadrons));
}

// +----------------------------------------------------------------------+

Hangar::Hangar(const Hangar& s)
    : ship(0), nsquadrons(s.nsquadrons), last_patrol_launch(s.last_patrol_launch)
{
    ZeroMemory(nslots,    sizeof(nslots));
    ZeroMemory(squadrons, sizeof(squadrons));
}

// +--------------------------------------------------------------------+

Hangar::~Hangar()
{
    for (int i = 0; i < MAX_SQUADRONS; i++)
    delete [] squadrons[i];
}

// +--------------------------------------------------------------------+

void
Hangar::ExecFrame(double seconds)
{
    for (int n = 0; n < nsquadrons; n++) {
        if (squadrons[n] && nslots[n] > 0) {
            for (int i = 0; i < nslots[n]; i++) {
                HangarSlot* slot = &squadrons[n][i];

                switch (slot->state) {
                case UNAVAIL:
                case STORAGE:
                case APPROACH:
                    break;

                case ACTIVE:
                    if (slot->ship && slot->ship->GetFlightPhase() == Ship::APPROACH)
                    slot->state = APPROACH;
                    break;

                case MAINT:
                    if (slot->time > 0) {
                        slot->time -= seconds;
                    }
                    else {
                        slot->time  = 0;
                        slot->state = STORAGE;
                    }
                    break;


                case PREP:
                    if (slot->time > 0) {
                        slot->time -= seconds;
                    }
                    else {
                        slot->time  = 0;
                        FinishPrep(slot);
                    }
                    break;

                case ALERT:
                    if (slot->time > 0) {
                        slot->time -= seconds;
                    }
                    else if (slot->deck) {
                        slot->time = 0;

                        // if package has specific objective, launch as soon as possible:
                        if (!slot->alert_hold)
                        slot->deck->Launch(slot->slot);

                        switch (slot->deck->State(slot->slot)) {
                        case FlightDeck::READY:    slot->state = ALERT;    break;
                        case FlightDeck::QUEUED:   slot->state = QUEUED;   break;
                        case FlightDeck::LOCKED:   slot->state = LOCKED;   break;
                        case FlightDeck::LAUNCH:   slot->state = LAUNCH;   break;
                        default:                   slot->state = STORAGE;  break;
                        }
                    }
                    break;

                case QUEUED:
                case LOCKED:
                    if (slot->deck) {
                        switch (slot->deck->State(slot->slot)) {
                        case FlightDeck::READY:    slot->state = ALERT;    break;
                        case FlightDeck::QUEUED:   slot->state = QUEUED;   break;
                        case FlightDeck::LOCKED:   slot->state = LOCKED;   break;
                        case FlightDeck::LAUNCH:   slot->state = LAUNCH;   break;
                        default:                   slot->state = STORAGE;  break;
                        }

                        slot->time = slot->deck->TimeRemaining(slot->slot);
                    }
                    break;

                case LAUNCH:
                    if (slot->deck) {
                        slot->time  = slot->deck->TimeRemaining(slot->slot);

                        if (slot->ship && slot->ship->GetFlightPhase() > Ship::LAUNCH) {
                            slot->state = ACTIVE;
                            slot->time  = 0;
                        }
                    }
                    break;

                case RECOVERY:
                    break;
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

bool
Hangar::FinishPrep(HangarSlot* slot)
{
    if (slot->deck->SpaceLeft(slot->design->type)) {
        Sim* sim = Sim::GetSim();

        Text ship_name = slot->squadron;

        slot->ship = sim->CreateShip(ship_name, "",
        (ShipDesign*) slot->design,
        ship->GetRegion()->Name(),
        Point(0, 0, 0),
        slot->iff,
        ship->GetCommandAILevel(),
        slot->loadout);

        Observe(slot->ship);

        if (slot->package) {
            slot->package->SetCommander(ship->GetElement());
            slot->package->AddShip(slot->ship);

            if (slot->group) {
                slot->package->SetCombatGroup(slot->group);
                slot->package->SetCombatUnit(slot->group->GetNextUnit());
            }

            char name[64];
            sprintf_s(name, "%s %d", 
            (const char*) slot->package->Name(), 
            slot->ship->GetElementIndex());
            slot->ship->SetName(name);
        }

        slot->slot = -1;  // take first available slot
        if (slot->deck->Spot(slot->ship, slot->slot)) {
            slot->state = ALERT;
            return true;
        }

        Print("WARNING: Could not spot alert ship - carrier: '%s' ship '%s'\n",
        ship->Name(), slot->ship->Name());
    }

    return false;
}

// +--------------------------------------------------------------------+

bool
Hangar::Update(SimObject* obj)
{
    bool found = false;

    for (int n = 0; !found && n < nsquadrons; n++) {
        if (squadrons[n] && nslots[n] > 0) {
            for (int i = 0; !found && i < nslots[n]; i++) {
                HangarSlot* slot = &squadrons[n][i];

                if (slot->ship == obj) {
                    // was ship destroyed in combat,
                    // or did it just dock here?
                    if (slot->state != MAINT) {
                        slot->state    = UNAVAIL;
                        slot->ship     = 0;
                        slot->deck     = 0;
                        slot->time     = 0;
                        slot->package  = 0;
                    }

                    found = true;
                }
            }
        }
    }

    return SimObserver::Update(obj);
}

const char*
Hangar::GetObserverName() const
{
    static char name[64];
    if (ship)
    sprintf_s(name, "Hangar(%s)", ship->Name());
    else
    sprintf_s(name, "Hangar");
    return name;
}

// +--------------------------------------------------------------------+

bool
Hangar::CreateSquadron(Text squadron, CombatGroup* group,
const ShipDesign* design, int count, int iff,
int* def_load, int maint_count, int dead_count)
{
    if (nsquadrons < MAX_SQUADRONS && count > 0) {
        HangarSlot* s = new(__FILE__,__LINE__) HangarSlot[count];

        for (int i = 0; i < count; i++) {
            s[i].squadron = squadron;
            s[i].group    = group;
            s[i].design   = design;
            s[i].iff      = iff;

            if (def_load)
            CopyMemory(s[i].loadout, def_load, sizeof(s[i].loadout));
        }

        squadrons[nsquadrons] = s;
        nslots[nsquadrons]    = count;
        names[nsquadrons]     = squadron;

        int i = count-1;
        while (dead_count-- > 0)
        s[i--].state = UNAVAIL;

        while (maint_count-- > 0) {
            s[i].state = MAINT;
            s[i--].time = 600 + rand() / 15;
        }

        nsquadrons++;
        return true;
    }

    return false;
}

bool
Hangar::GotoActiveFlight(int squadron, int slot_index, Element* elem, int* loadout)
{
    if (elem && squadron < nsquadrons && slot_index < nslots[squadron]) {
        HangarSlot* slot = &(squadrons[squadron][slot_index]);

        if (slot->state == STORAGE) {
            slot->deck       = 0;
            slot->state      = ACTIVE;
            slot->time       = 0;
            slot->package    = elem;
            slot->alert_hold = false;

            if (loadout)
            CopyMemory(slot->loadout, loadout, sizeof(slot->loadout));

            Sim* sim = Sim::GetSim();

            Text ship_name = slot->squadron;

            slot->ship = sim->CreateShip(ship_name, "",
            (ShipDesign*) slot->design,
            ship->GetRegion()->Name(),
            ship->Location() + RandomPoint(),
            slot->iff,
            ship->GetCommandAILevel(),
            slot->loadout);

            if (slot->ship) {
                Observe(slot->ship);

                elem->SetCommander(ship->GetElement());
                elem->AddShip(slot->ship);

                if (slot->group) {
                    elem->SetCombatGroup(slot->group);
                    elem->SetCombatUnit(slot->group->GetNextUnit());
                }

                char name[64];
                sprintf_s(name, "%s %d", 
                (const char*) elem->Name(), 
                slot->ship->GetElementIndex());

                slot->ship->SetName(name);
            }

            return true;
        }
    }

    return false;
}

bool
Hangar::GotoAlert(int squadron, int slot, FlightDeck* d, Element* elem, int* loadout, bool pkg, bool expedite)
{
    if (squadron < nsquadrons && slot < nslots[squadron]) {
        HangarSlot* s = &(squadrons[squadron][slot]);

        if (s->state == STORAGE) {
            s->deck       = d;
            s->state      = PREP;
            s->time       = expedite ? 3 : s->design->prep_time;
            s->package    = elem;
            s->alert_hold = !pkg;

            if (loadout)
            CopyMemory(s->loadout, loadout, sizeof(s->loadout));

            if (expedite)
            FinishPrep(s);

            return true;
        }
    }

    return false;
}

bool
Hangar::Launch(int squadron, int slot)
{
    if (squadron < nsquadrons && slot < nslots[squadron]) {
        HangarSlot* s = &(squadrons[squadron][slot]);

        if (s->state == ALERT && s->deck)
        return s->deck->Launch(s->slot);
    }

    return false;
}

bool
Hangar::StandDown(int squadron, int slot)
{
    if (squadron < nsquadrons && slot < nslots[squadron]) {
        HangarSlot* s = &(squadrons[squadron][slot]);

        Element* package    = 0;
        bool     clear_slot = false;

        if (s->state == ALERT && s->deck) {
            if (s->deck->Clear(s->slot)) {
                if (s->ship) {
                    Sim* sim = Sim::GetSim();

                    if (s->package) {
                        package = s->package;
                        package->DelShip(s->ship);
                    }

                    sim->DestroyShip(s->ship);
                }

                clear_slot = true;
            }
        }

        else if (s->state == PREP) {
            clear_slot = true;
            package    = s->package;
        }

        if (clear_slot) {
            s->state    = STORAGE;
            s->deck     = 0;
            s->slot     = 0;
            s->ship     = 0;
            s->package  = 0;

            if (package) {
                int npkg = 0;
                for (int i = 0; i < nslots[squadron]; i++) {
                    if (squadrons[squadron][i].package == package)
                    npkg++;
                }

                if (npkg == 0) {
                    Sim::GetSim()->DestroyElement(package);
                }
            }

            return true;
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

bool
Hangar::CanStow(Ship* incoming)
{
    int squadron = -1;
    int slot     = -1;

    if (FindSlot(incoming, squadron, slot))
    return true;

    return false;
}

// +--------------------------------------------------------------------+

bool
Hangar::Stow(Ship* incoming)
{
    int squadron = -1;
    int slot     = -1;

    if (FindSlot(incoming, squadron, slot)) {
        HangarSlot* s = &(squadrons[squadron][slot]);
        s->state   = MAINT;
        s->design  = incoming->Design();
        s->time    = 2400;
        s->package = 0;   // XXX MEMORY LEAK?

        // extra maintenance time?
        if (incoming->Integrity() < incoming->Design()->integrity) {
            double damage = 100 * ((double) incoming->Design()->integrity - (double) incoming->Integrity()) / (double) incoming->Design()->integrity;

            if      (damage < 10)   s->time *= 1.2;
            else if (damage < 25)   s->time *= 2;
            else if (damage < 50)   s->time *= 4;
            else                    s->time *= 10;
        }

        // quicker turnaround during network play:
        Sim* sim = Sim::GetSim();
        if (sim && sim->IsNetGame())
        s->time /= 40;

        return true;
    }

    return false;
}

bool
Hangar::FindSlot(Ship* test, int& squadron, int& slot, int desired_state)
{
    if (test) {
        // if test is already inbound to this carrier,
        // keep the inbound squadron and slot selections:
        if (desired_state == UNAVAIL && test->GetInbound()) {
            InboundSlot* inbound = test->GetInbound();
            FlightDeck*  deck    = inbound->GetDeck();

            if (deck && deck->GetCarrier() == ship && deck->IsPowerOn()) {
                squadron = inbound->Squadron();
                slot     = inbound->Index();
                return true;
            }
        }

        int         avail_squadron = -1;
        int         avail_slot     = -1;

        for (int i = 0; i < nsquadrons; i++) {
            if (squadron < 0 || squadron == i) {
                for (int j = 0; j < nslots[i]; j++) {
                    HangarSlot* s = &(squadrons[i][j]);
                    if (s->ship == test) {
                        squadron = i;
                        slot     = j;
                        return true;
                    }

                    else if (avail_slot < 0 && s->ship == 0) {
                        if ((desired_state > STORAGE && s->state == STORAGE) ||
                                (desired_state < STORAGE && s->state == UNAVAIL)) {
                            avail_squadron = i;
                            avail_slot     = j;
                        }
                    }
                }
            }
        }

        if (avail_squadron >= 0 && avail_slot >= 0) {
            squadron = avail_squadron;
            slot     = avail_slot;

            if (desired_state > STORAGE) {
                HangarSlot* s = &(squadrons[squadron][slot]);

                s->ship     = test;
                s->design   = test->Design();
                s->state    = desired_state;
                s->deck     = 0;
                s->slot     = 0;
                s->package  = test->GetElement();
                s->time     = 0;

                Observe(s->ship);
            }

            return true;
        }
    }

    return false;
}

bool
Hangar::FindSquadronAndSlot(Ship* test, int& squadron, int& slot)
{
    if (test) {
        for (int i = 0; i < nsquadrons; i++) {
            if (squadron < 0 || squadron == i) {
                for (int j = 0; j < nslots[i]; j++) {
                    HangarSlot* s = &(squadrons[i][j]);
                    if (s->ship == test) {
                        squadron = i;
                        slot     = j;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}


bool
Hangar::FindAvailSlot(const ShipDesign* design, int& squadron, int& slot)
{
    if (design) {
        for (int i = 0; i < nsquadrons; i++) {
            if (nslots[i] > 0 && squadrons[i]->design == design) {
                for (int j = 0; j < nslots[i]; j++) {
                    HangarSlot* s = &(squadrons[i][j]);

                    if (s->state == STORAGE) {
                        squadron = i;
                        slot     = j;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool
Hangar::Ready(int squadron, int slot, FlightDeck* d)
{
    if (squadron < 0 || squadron >= nsquadrons || slot < 0 || slot >= nslots[squadron] || !d)
    return false;

    HangarSlot* s = &(squadrons[squadron][slot]);

    s->time = 3;   // 5;
    s->deck = d;
    s->slot = -1;  // take first available slot

    if (d->Spot(s->ship, s->slot)) {
        s->state = ALERT;
        s->alert_hold = false;
        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

Text
Hangar::SquadronName(int n) const
{
    if (n >= 0 && n < nsquadrons)
    return names[n];

    return Game::GetText("Unknown");
}

int
Hangar::SquadronSize(int n) const
{
    if (n >= 0 && n < nsquadrons)
    return nslots[n];

    return 0;
}

int
Hangar::SquadronIFF(int n) const
{
    if (n >= 0 && n < nsquadrons)
    return squadrons[n]->iff;

    return 0;
}

const ShipDesign*
Hangar::SquadronDesign(int n) const
{
    if (n >= 0 && n < nsquadrons && nslots[n])
    return squadrons[n]->design;

    return 0;
}

const HangarSlot*
Hangar::GetSlot(int i, int j) const
{
    if (i >= 0 && i < nsquadrons)
    if (j >= 0 && j < nslots[i])
    return squadrons[i] + j;

    return 0;
}

// +--------------------------------------------------------------------+

Ship*
Hangar::GetShip(const HangarSlot* s) const
{
    if (s) return s->ship;
    return 0;
}

const ShipDesign*
Hangar::GetDesign(const HangarSlot* s) const
{
    if (s) return s->design;
    return 0;
}

FlightDeck*
Hangar::GetFlightDeck(const HangarSlot* s) const
{
    if (s) return s->deck;
    return 0;
}

int
Hangar::GetFlightDeckSlot(const HangarSlot* s) const
{
    if (s) return s->slot;
    return 0;
}

int
Hangar::GetState(const HangarSlot* s) const
{
    if (s) return s->state;
    return 0;
}

double
Hangar::TimeRemaining(const HangarSlot* s) const
{
    if (s) return s->time;
    return 0;
}

Element*
Hangar::GetPackageElement(const HangarSlot* s) const
{
    if (s) return s->package;
    return 0;
}

const int*
Hangar::GetLoadout(const HangarSlot* s) const
{
    if (s) return s->loadout;
    return 0;
}

Text
Hangar::StatusName(const HangarSlot* s) const
{
    switch (s->state) {
    default:
    case UNAVAIL:     return Game::GetText("hangar.UNAVAIL");
    case MAINT:       return Game::GetText("hangar.MAINT");
    case STORAGE:     return Game::GetText("hangar.STORAGE");
    case PREP:        return Game::GetText("hangar.PREP");
    case ALERT:       return Game::GetText("hangar.ALERT");
    case QUEUED:      {
            Text state = Game::GetText("hangar.QUEUED");
            char seq[8];
            sprintf_s(seq, " %d", s->deck->Sequence(s->slot));
            return state + seq;
        }
    case LOCKED:      return Game::GetText("hangar.LOCKED");
    case LAUNCH:      return Game::GetText("hangar.LAUNCH");
    case ACTIVE:      return Game::GetText("hangar.ACTIVE");
    case APPROACH:    return Game::GetText("hangar.APPROACH");
    case RECOVERY:    return Game::GetText("hangar.RECOVERY");
    }
}

// +--------------------------------------------------------------------+

int
Hangar::PreflightQueue(FlightDeck* d) const
{
    int result = 0;

    for (int n = 0; n < nsquadrons; n++) {
        if (squadrons[n] && nslots[n] > 0) {
            for (int i = 0; i < nslots[n]; i++) {
                HangarSlot* slot = &squadrons[n][i];

                if (slot->deck == d)
                result++;
            }
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

int
Hangar::NumShipsReady(int n) const
{
    int result = 0;

    if (n >= 0 && n < nsquadrons && squadrons[n] && nslots[n] > 0) {
        for (int i = 0; i < nslots[n]; i++) {
            HangarSlot* slot = &squadrons[n][i];

            if (slot->state == STORAGE)
            result++;
        }
    }

    return result;
}

int
Hangar::NumShipsMaint(int n) const
{
    int result = 0;

    if (n >= 0 && n < nsquadrons && squadrons[n] && nslots[n] > 0) {
        for (int i = 0; i < nslots[n]; i++) {
            HangarSlot* slot = &squadrons[n][i];

            if (slot->state == MAINT)
            result++;
        }
    }

    return result;
}

int
Hangar::NumShipsDead(int n) const
{
    int result = 0;

    if (n >= 0 && n < nsquadrons && squadrons[n] && nslots[n] > 0) {
        for (int i = 0; i < nslots[n]; i++) {
            HangarSlot* slot = &squadrons[n][i];

            if (slot->state == UNAVAIL)
            result++;
        }
    }

    return result;
}

int
Hangar::NumSlotsEmpty() const
{
    int result = 0;

    for (int n = 0; n < nsquadrons; n++) {
        if (squadrons[n] && nslots[n] > 0) {
            for (int i = 0; i < nslots[n]; i++) {
                HangarSlot* slot = &squadrons[n][i];

                if (slot->state == UNAVAIL)
                result++;
            }
        }
    }

    return result;
}

int
Hangar::GetActiveElements(List<Element>& active_list)
{
    active_list.clear();

    for (int n = 0; n < nsquadrons; n++) {
        if (squadrons[n] && nslots[n] > 0) {
            for (int i = 0; i < nslots[n]; i++) {
                HangarSlot* slot = &squadrons[n][i];

                if (slot->package != 0 && !active_list.contains(slot->package))
                active_list.append(slot->package);
            }
        }
    }

    return active_list.size();
}

// +--------------------------------------------------------------------+

DWORD
Hangar::GetLastPatrolLaunch() const
{
    return last_patrol_launch;
}

void
Hangar::SetLastPatrolLaunch(DWORD t)
{
    last_patrol_launch = t;
}

// +--------------------------------------------------------------------+

void
Hangar::SetAllIFF(int iff)
{
    for (int n = 0; n < nsquadrons; n++) {
        if (squadrons[n] && nslots[n] > 0) {
            for (int i = 0; i < nslots[n]; i++) {
                HangarSlot* slot = &squadrons[n][i];

                if (slot->ship)
                slot->ship->SetIFF(iff);
            }
        }
    }
}
