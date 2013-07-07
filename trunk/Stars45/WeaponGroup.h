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
    FILE:         WeaponGroup.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Weapon Control Category (Group) class
*/

#ifndef WeaponGroup_h
#define WeaponGroup_h

#include "Types.h"
#include "Weapon.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class WeaponGroup
{
public:
    static const char* TYPENAME() { return "WeaponGroup"; }

    WeaponGroup(const char* name);
    ~WeaponGroup();

    void              ExecFrame(double factor);

    // identification:
    const char*       Name()                     const { return name;    }
    const char*       Abbreviation()             const { return abrv;    }
    void              SetName(const char* n);
    void              SetAbbreviation(const char* a);

    bool              IsPrimary()                const;
    bool              IsDrone()                  const;
    bool              IsDecoy()                  const;
    bool              IsProbe()                  const;
    bool              IsMissile()                const;
    bool              IsBeam()                   const;
    int               Value()                    const;

    // weapon list:
    void              AddWeapon(Weapon* w);
    int               NumWeapons()               const;
    List<Weapon>&     GetWeapons();
    bool              Contains(const Weapon* w)  const;

    // weapon selection:
    void              SelectWeapon(int n);
    void              CycleWeapon();
    Weapon*           GetWeapon(int n)           const;
    Weapon*           GetSelected()              const;

    // operations:
    bool              GetTrigger()               const    { return trigger; }
    void              SetTrigger(bool t=true)             { trigger = t;    }
    int               Ammo()                     const    { return ammo;    }
    float             Mass()                     const    { return mass;    }
    float             Resistance()               const    { return resist;  }
    void              CheckAmmo();

    void              SetTarget(SimObject* t, System* sub=0);
    SimObject*        GetTarget()                const;
    System*           GetSubTarget()             const;
    void              DropTarget();
    void              SetFiringOrders(int o);
    int               GetFiringOrders()          const    { return orders;  }
    void              SetControlMode(int m);
    int               GetControlMode()           const    { return control; }
    void              SetSweep(int s);
    int               GetSweep()                 const    { return sweep;   }
    int               Status()                   const;

    WeaponDesign*     GetDesign()          const;
    bool              CanTarget(DWORD tgt_class) const;

    void              PowerOn();
    void              PowerOff();

protected:
    // Displayable name:
    Text              name;
    Text              abrv;

    List<Weapon>      weapons;

    int               selected;
    bool              trigger;
    int               ammo;

    int               orders;
    int               control;
    int               sweep;

    float             mass;
    float             resist;
};

#endif WeaponGroup_h

