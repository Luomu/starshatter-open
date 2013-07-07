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
    FILE:         Component.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Generic ship system sub-component class
*/

#ifndef Component_h
#define Component_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class ComponentDesign
{
public:
    static const char* TYPENAME() { return "ComponentDesign"; }

    ComponentDesign();
    ~ComponentDesign();
    int operator == (const ComponentDesign& rhs) const { return (name == rhs.name); }

    // identification:
    Text              name;
    Text              abrv;

    float             repair_time;
    float             replace_time;
    int               spares;
    DWORD             affects;
};

// +--------------------------------------------------------------------+

class System;

// +--------------------------------------------------------------------+

class Component
{
public:
    static const char* TYPENAME() { return "Component"; }

    enum STATUS    { DESTROYED, CRITICAL, DEGRADED, NOMINAL, REPLACE, REPAIR };
    enum DAMAGE    { DAMAGE_EFFICIENCY = 0x01,
        DAMAGE_SAFETY     = 0x02,
        DAMAGE_STABILITY  = 0x04 };

    Component(ComponentDesign* d, System* s);
    Component(const Component& c);
    virtual ~Component();

    const char*       Name()         const { return design->name;           }
    const char*       Abbreviation() const { return design->abrv;           }
    float             RepairTime()   const { return design->repair_time;    }
    float             ReplaceTime()  const { return design->replace_time;   }

    bool              DamageEfficiency() const { return (design->affects & DAMAGE_EFFICIENCY)?true:false; }
    bool              DamageSafety()     const { return (design->affects & DAMAGE_SAFETY)?true:false;     }
    bool              DamageStability()  const { return (design->affects & DAMAGE_STABILITY)?true:false;  }

    STATUS            Status()       const { return status;                 }
    float             Availability() const;
    float             TimeRemaining() const;
    int               SpareCount()   const;
    bool              IsJerried()    const;
    int               NumJerried()   const;

    void              SetSystem(System* s) { system = s;                    }
    System*           GetSystem()    const { return system;                 }

    virtual void      ApplyDamage(double damage);
    virtual void      ExecMaintFrame(double seconds);
    virtual void      Repair();
    virtual void      Replace();

protected:
    ComponentDesign*  design;

    // Component health status:
    STATUS            status;
    float             availability;
    float             time_remaining;
    int               spares;
    int               jerried;
    System*           system;
};

#endif Component_h

