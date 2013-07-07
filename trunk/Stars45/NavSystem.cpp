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
    FILE:         NavSystem.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation System class implementation
*/

#include "MemDebug.h"
#include "NavSystem.h"
#include "Ship.h"
#include "Sim.h"
#include "HUDSounds.h"
#include "Button.h"
#include "Game.h"

// +----------------------------------------------------------------------+

NavSystem::NavSystem()
: System(COMPUTER, 2, "Auto Nav System", 1, 1,1,1),
autonav(0)
{
    name = Game::GetText("sys.nav-system");
    abrv = Game::GetText("sys.nav-system.abrv");

    power_flags = POWER_WATTS | POWER_CRITICAL;
}

// +----------------------------------------------------------------------+

NavSystem::NavSystem(const NavSystem& s)
: System(s), autonav(0)
{
    Mount(s);

    power_flags = POWER_WATTS | POWER_CRITICAL;
}

// +--------------------------------------------------------------------+

NavSystem::~NavSystem()
{ }

// +--------------------------------------------------------------------+

void
NavSystem::ExecFrame(double seconds)
{
    if (autonav && ship && !ship->GetNextNavPoint())
    autonav = false;

    energy = 0.0f;
    System::ExecFrame(seconds);
}

// +----------------------------------------------------------------------+

bool
NavSystem::AutoNavEngaged()
{
    return ship && autonav && IsPowerOn();
}

void
NavSystem::EngageAutoNav()
{
    if (IsPowerOn() && !autonav) {
        if (!ship->GetNextNavPoint()) {
            Button::PlaySound(Button::SND_REJECT);
        }
        else {
            HUDSounds::PlaySound(HUDSounds::SND_NAV_MODE);
            autonav = true;
        }
    }
}

void
NavSystem::DisengageAutoNav()
{
    if (autonav)
    HUDSounds::PlaySound(HUDSounds::SND_NAV_MODE);

    autonav = false;
}

// +--------------------------------------------------------------------+

void
NavSystem::Distribute(double delivered_energy, double seconds)
{
    if (IsPowerOn()) {
        // convert Joules to Watts:
        energy = (float) (delivered_energy/seconds);

        // brown out:
        if (energy < capacity*0.75f)
        power_on = false;

        // spike:
        else if (energy > capacity*1.5f) {
            power_on = false;
            ApplyDamage(50);
        }
    }
}

