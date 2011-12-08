/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         SimEvent.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simulation Universe and Region classes
*/

#ifndef SimEvent_h
#define SimEvent_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Sim;
class SimRegion;
class SimObject;
class SimObserver;
class SimHyper;
class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+

class SimEvent
{
public:
   static const char* TYPENAME() { return "SimEvent"; }

   enum EVENT { LAUNCH=1, DOCK, LAND, EJECT, CRASH, COLLIDE, DESTROYED,
                MAKE_ORBIT, BREAK_ORBIT, QUANTUM_JUMP,
                LAUNCH_SHIP, RECOVER_SHIP,
                FIRE_GUNS, FIRE_MISSILE, DROP_DECOY,
                GUNS_KILL, MISSILE_KILL,
                LAUNCH_PROBE, SCAN_TARGET
   };

   SimEvent(int event, const char* tgt=0, const char* info=0);
   ~SimEvent();

   int            GetEvent()        const { return event;         }
   int            GetTime()         const { return time;          }
   Text           GetEventDesc()    const;
   const char*    GetTarget()       const { return target;        }
   const char*    GetInfo()         const { return info;          }
   int            GetCount()        const { return count;         }

   void           SetTarget(const char* tgt);
   void           SetInfo(const char* info);
   void           SetCount(int count);
   void           SetTime(int time);

private:
   int            event;
   int            time;
   Text           target;
   Text           info;
   int            count;
};

// +--------------------------------------------------------------------+

class ShipStats
{
public:
   static const char* TYPENAME() { return "ShipStats"; }

   ShipStats(const char* name, int iff=0);
   ~ShipStats();

   static void       Initialize();
   static void       Close();
   static ShipStats* Find(const char* name);
   static int        NumStats();
   static ShipStats* GetStats(int i);

   void           Summarize();

   const char*    GetName()         const { return name;          }
   const char*    GetType()         const { return type;          }
   const char*    GetRole()         const { return role;          }
   const char*    GetRegion()       const { return region;        }
   CombatGroup*   GetCombatGroup()  const { return combat_group;  }
   CombatUnit*    GetCombatUnit()   const { return combat_unit;   }
   int            GetElementIndex() const { return elem_index;    }
   int            GetShipClass()    const { return ship_class;    }
   int            GetIFF()          const { return iff;           }
   int            GetGunKills()     const { return kill1;         }
   int            GetMissileKills() const { return kill2;         }
   int            GetDeaths()       const { return lost;          }
   int            GetColls()        const { return coll;          }
   int            GetPoints()       const { return points;        }
   int            GetCommandPoints()const { return cmd_points;    }

   int            GetGunShots()     const { return gun_shots;     }
   int            GetGunHits()      const { return gun_hits;      }
   int            GetMissileShots() const { return missile_shots; }
   int            GetMissileHits()  const { return missile_hits;  }

   bool           IsPlayer()        const { return player;        }

   List<SimEvent>&
                  GetEvents()       { return events;  }
   SimEvent*      AddEvent(SimEvent* e);
   SimEvent*      AddEvent(int event, const char* tgt=0, const char* info=0);
   bool           HasEvent(int event);

   void           SetShipClass(int c)     { ship_class = c;       }
   void           SetIFF(int i)           { iff = i;              }
   void           SetType(const char* t);
   void           SetRole(const char* r);
   void           SetRegion(const char* r);
   void           SetCombatGroup(CombatGroup* g);
   void           SetCombatUnit(CombatUnit* u);
   void           SetElementIndex(int n);
   void           SetPlayer(bool p);

   void           AddGunShot()            { gun_shots++;          }
   void           AddGunHit()             { gun_hits++;           }
   void           AddMissileShot()        { missile_shots++;      }
   void           AddMissileHit()         { missile_hits++;       }
   void           AddPoints(int p)        { points += p;          }
   void           AddCommandPoints(int p) { cmd_points += p;      }

private:
   Text           name;
   Text           type;
   Text           role;
   Text           region;
   CombatGroup*   combat_group;
   CombatUnit*    combat_unit;
   bool           player;
   int            elem_index;
   int            ship_class;
   int            iff;
   int            kill1;
   int            kill2;
   int            lost;
   int            coll;

   int            gun_shots;
   int            gun_hits;

   int            missile_shots;
   int            missile_hits;

   int            points;
   int            cmd_points;

   List<SimEvent> events;
};

#endif SimEvent_h

