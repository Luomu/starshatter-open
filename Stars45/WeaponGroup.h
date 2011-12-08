/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

