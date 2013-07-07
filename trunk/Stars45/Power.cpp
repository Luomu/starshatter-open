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
    FILE:         Power.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Power generation and usage classes
*/

#include "MemDebug.h"
#include "Power.h"
#include "Ship.h"
#include "NetUtil.h"

#include "Game.h"

// +----------------------------------------------------------------------+

static char*   source_type[] = {
    "sys.power.battery", "sys.power.auxilliary", "sys.power.fusion"
};

static int     source_value[] = {
    1, 2, 4
};

PowerSource::PowerSource(SUBTYPE s, double max_out, double f_ratio)
: System(POWER_SOURCE, (int) s, "Power", source_value[s], 0),
max_output((float) max_out), fuel_ratio((float) f_ratio),
route_changed(false), requested_power_level(1.0f)
{
    name = Game::GetText(source_type[s]);
    abrv = Game::GetText(Text(source_type[s]) + ".abrv");

    if (fuel_ratio < 1) {
        switch (subtype) {    // enough to last for [n] hours at full power
        case BATTERY:     fuel_ratio = max_output *   5 * 3600 / 100; break;
        case AUX:         fuel_ratio = max_output *  50 * 3600 / 100; break;
        case FUSION:      fuel_ratio = max_output * 100 * 3600 / 100; break;
        }
    }

    capacity = 100.0f;

    if (subtype != BATTERY) {
        emcon_power[0] =  10;
        emcon_power[1] =  50;
        emcon_power[2] = 100;
    }
}

PowerSource::PowerSource(const PowerSource& p)
: System(p),
max_output(p.max_output), fuel_ratio(p.fuel_ratio),
route_changed(false), requested_power_level(1.0f)
{
    Mount(p);
    SetAbbreviation(p.Abbreviation());
}

// +----------------------------------------------------------------------+

void
PowerSource::AddClient(System* client)
{
    if (client) {
        int old_src_index = client->GetSourceIndex();

        client->SetSourceIndex(GetID());
        clients.append(client);
        route_changed = true;

        if (ship && old_src_index != GetID())
        NetUtil::SendSysStatus(ship, client);
    }
}

void
PowerSource::RemoveClient(System* client)
{
    if (client && clients.contains(client)) {
        client->SetSourceIndex(-1);
        clients.remove(client);
        route_changed = true;
    }
}

// +----------------------------------------------------------------------+

int
PowerSource::Charge() const
{
    return (int) capacity;
}

void
PowerSource::SetFuelRange(double hours)
{
    if (hours > 0)
    fuel_ratio = (float) (max_output * hours * 3600 / 100);
}

// +----------------------------------------------------------------------+

void
PowerSource::ExecFrame(double seconds)
{
    if (seconds < 0.001)
    seconds = 0.001;

    if (capacity <= 0)
    capacity = 0;

    System::ExecFrame(seconds);

    double energy_requested = 0;
    double energy_avail     = 0;
    double total_distrib    = 0;

    // fuel leak?
    if (availability < 0.4 && capacity > 0)
    capacity -= (float) (0.03 * seconds);

    if (IsPowerOn() && capacity > 0) {
        energy_avail = max_output * seconds * power_level * availability;

        if (power_level < requested_power_level) {
            power_level += (float) (seconds * 0.03);     // thirty seconds to charge up

            if (power_level > requested_power_level)
            power_level = (float) requested_power_level;
        }
        else if (power_level > requested_power_level) {
            power_level -= (float) (seconds * 0.10);     // ten seconds to power down

            if (power_level < requested_power_level)
            power_level = (float) requested_power_level;
        }
    }

    ListIter<System> iter = clients;
    while (++iter) {
        System* sink = iter.value();
        
        if (sink->IsPowerOn()) {
            double joules = sink->GetRequest(seconds);

            if (joules > 0) {
                if (sink->IsPowerCritical()) {

                    if (joules > energy_avail)
                    joules = energy_avail;

                    energy_avail  -= joules;
                    total_distrib += joules;
                    sink->Distribute(joules, seconds);
                }
                else {
                    energy_requested += joules;
                }
            }
        }
        else {
            sink->Distribute(-0.2 * sink->GetCapacity() * seconds, seconds);
        }
    }

    if (energy_avail > 0) {

        // enough to go around:
        if (energy_requested <= energy_avail) {
            iter.reset();
            
            while (++iter) {
                System* sink = iter.value();

                if (sink->IsPowerOn() && !sink->IsPowerCritical()) {
                    double joules = sink->GetRequest(seconds);
                    total_distrib += joules;
                    sink->Distribute(joules, seconds);
                }
            }
        }

        // load balancing:
        else {
            iter.reset();
            while (++iter) {
                System* sink = iter.value();
                
                if (sink->IsPowerOn() && !sink->IsPowerCritical()) {
                    double request  = sink->GetRequest(seconds);
                    double delivery = 0;

                    if (request > 0)
                    delivery = energy_avail * (request/energy_requested);
                    
                    if (delivery > 0) {
                        total_distrib += delivery;
                        sink->Distribute(delivery, seconds);
                    }
                }
            }
        }
    }

    if (IsPowerOn() && capacity > 0 && !Game::Paused()) {
        // reactors always burn at least 10% of max (to maintain operating temp):
        if (subtype != BATTERY) {
            double baseline = 0.1 * max_output * seconds;

            if (total_distrib < baseline)
            total_distrib = baseline;
        }

        // expend fuel:
        if (total_distrib > 0) {
            double effective_fuel_ratio = fuel_ratio;
            
            switch (Ship::GetFlightModel()) {
            default:
            case Ship::FM_STANDARD:
                effective_fuel_ratio = 1 * fuel_ratio * (0.25 + 0.75 * availability);
                break;

            case Ship::FM_RELAXED:
                effective_fuel_ratio = 3 * fuel_ratio * (0.25 + 0.75 * availability);
                break;

            case Ship::FM_ARCADE:
                effective_fuel_ratio = 4 * fuel_ratio * (0.25 + 0.75 * availability);
                break;
            }

            capacity -= (float) (total_distrib / effective_fuel_ratio);
        }
    }

    else if (capacity <= 0) {
        capacity = 0.0f;
        PowerOff();
    }
}

// +--------------------------------------------------------------------+

void
PowerSource::SetPowerLevel(double level)
{
    if (level > 100)
    level = 100;
    else if (level < 0)
    level = 0;

    level /= 100;

    if (requested_power_level != level) {
        // if the system is on emergency override power,
        // do not let the EMCON system use this method
        // to drop it back to normal power:
        if (requested_power_level > 1 && level == 1) {
            requested_power_level = 1.2f;
        }

        else {
            requested_power_level = (float) level;
        }
    }
}

void
PowerSource::SetOverride(bool over)
{
    bool changed = false;

    if (over && requested_power_level != 1.2f) {
        requested_power_level = 1.2f;
        changed = true;
    }

    else if (!over && requested_power_level > 1) {
        requested_power_level = 1.0f;
        changed = true;
    }

    if (changed)
    NetUtil::SendSysStatus(ship, this);
}

void
PowerSource::DrainPower(double to_level)
{
    if (to_level >= 0 && to_level < power_level)
    power_level = (float) to_level;
}
