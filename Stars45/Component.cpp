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
    FILE:         Component.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Generic ship system sub-component class 
*/

#include "MemDebug.h"
#include "Component.h"
#include "System.h"
#include "Game.h"

// +----------------------------------------------------------------------+

ComponentDesign::ComponentDesign()
: repair_time(0.0f), replace_time(0.0f), spares(0), affects(0)
{ }

// +----------------------------------------------------------------------+

ComponentDesign::~ComponentDesign()
{ }

// +----------------------------------------------------------------------+

Component::Component(ComponentDesign* d, System* s)
: design(d), system(s),
status(NOMINAL), availability(100.0f), time_remaining(0.0f),
spares(0), jerried(0)
{
    if (design)
    spares = design->spares;
}

// +----------------------------------------------------------------------+

Component::Component(const Component& c)
: design(c.design), system(c.system),
status(c.status), availability(c.availability), time_remaining(c.time_remaining),
spares(c.spares), jerried(c.jerried)
{
}

// +--------------------------------------------------------------------+

Component::~Component()
{ }

// +--------------------------------------------------------------------+

void
Component::ExecMaintFrame(double seconds)
{
    if (status > NOMINAL) {
        time_remaining -= (float) seconds;

        // when repairs are complete:
        if (time_remaining <= 0) {
            if (status == REPAIR) {
                // did we just jerry-rig a failed component?
                if (availability < 50)
                jerried++;

                if (jerried < 5)
                availability += 50.0f - 10 * jerried;
                if (availability > 100) availability = 100.0f;
            }
            else {
                availability = 100.0f;
            }

            if (availability > 99)
            status = NOMINAL;
            else if (availability > 49)
            status = DEGRADED;
            else
            status = CRITICAL;

            time_remaining = 0.0f;

            if (system)
            system->CalcStatus();
        }
    }
}

// +--------------------------------------------------------------------+

void
Component::ApplyDamage(double damage)
{
    availability -= (float) damage;
    if (availability < 1) availability = 0.0f;

    if (status < REPLACE) {
        if (availability > 99)
        status = NOMINAL;
        else if (availability > 49)
        status = DEGRADED;
        else
        status = CRITICAL;
    }

    if (system)
    system->CalcStatus();
}

// +--------------------------------------------------------------------+

void
Component::Repair()
{
    if (status < NOMINAL) {
        status = REPAIR;
        time_remaining = design->repair_time;

        if (system)
        system->CalcStatus();
    }
}

// +--------------------------------------------------------------------+

void
Component::Replace()
{
    if (status <= NOMINAL) {
        status = REPLACE;
        spares--;
        time_remaining = design->replace_time;

        if (system)
        system->CalcStatus();
    }
}

// +--------------------------------------------------------------------+

float
Component::Availability() const
{
    if (status > NOMINAL && availability > 50)
    return 50.0f;

    return availability;
}

float
Component::TimeRemaining() const
{
    return (float) time_remaining;
}

int
Component::SpareCount() const
{
    return spares;
}

bool
Component::IsJerried() const
{
    return jerried?true:false;
}

int
Component::NumJerried() const
{
    return jerried;
}