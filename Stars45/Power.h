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
    FILE:         Power.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Power generation and usage classes
*/

#ifndef Power_h
#define Power_h

#include "Types.h"
#include "System.h"
#include "List.h"

// +--------------------------------------------------------------------+

class PowerSource : public System
{
public:
    enum SUBTYPE { BATTERY, AUX, FUSION };

    PowerSource(SUBTYPE s, double max_output, double fuel_ratio=0);
    PowerSource(const PowerSource& rhs);

    virtual void ExecFrame(double seconds);

    void AddClient(System* client);
    void RemoveClient(System* client);

    List<System>& Clients() { return clients; }

    virtual int    Charge() const;

    virtual void   SetFuelRange(double hours);

    bool           RouteChanged() const { return route_changed;  }
    void           RouteScanned()       { route_changed = false; }

    // override from System:
    virtual void   SetPowerLevel(double level);
    virtual void   SetOverride(bool over);

    // for power drain damage:
    virtual void   DrainPower(double to_level);

protected:
    float          max_output;
    float          fuel_ratio;
    List<System>   clients;
    bool           route_changed;
    float          requested_power_level;
};

#endif Power_h

