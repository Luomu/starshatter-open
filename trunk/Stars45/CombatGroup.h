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
    FILE:         CombatGroup.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#ifndef CombatGroup_h
#define CombatGroup_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"
#include "List.h"
#include "Intel.h"

// +--------------------------------------------------------------------+

class Campaign;
class Combatant;
class CombatGroup;
class CombatUnit;
class CombatZone;
class CombatAssignment;

// +--------------------------------------------------------------------+

class CombatGroup
{
public:
    static const char* TYPENAME() { return "CombatGroup"; }

    enum GROUP_TYPE {   
        FORCE = 1,           // Commander In Chief

        WING,                // Air Force
        INTERCEPT_SQUADRON,  // a2a fighter
        FIGHTER_SQUADRON,    // multi-role fighter
        ATTACK_SQUADRON,     // strike / attack
        LCA_SQUADRON,        // landing craft

        FLEET,               // Navy
        DESTROYER_SQUADRON,  // destroyer
        BATTLE_GROUP,        // heavy cruiser(s)
        CARRIER_GROUP,       // fleet carrier

        BATTALION,           // Army
        MINEFIELD,
        BATTERY,
        MISSILE,
        STATION,             // orbital station
        STARBASE,            // planet-side base

        C3I,                 // Command, Control, Communications, Intelligence
        COMM_RELAY,
        EARLY_WARNING,
        FWD_CONTROL_CTR,
        ECM,

        SUPPORT,
        COURIER,
        MEDICAL,
        SUPPLY,
        REPAIR,

        CIVILIAN,            // root for civilian groups

        WAR_PRODUCTION,
        FACTORY,
        REFINERY,
        RESOURCE,

        INFRASTRUCTURE,
        TRANSPORT,
        NETWORK,
        HABITAT,
        STORAGE,

        NON_COM,             // other civilian traffic
        FREIGHT,
        PASSENGER,
        PRIVATE
    };

    CombatGroup(int t, int n, const char* s, int i, int e, CombatGroup* p=0);
    ~CombatGroup();

    // comparison operators are used to sort combat groups into a priority list
    // in DESCENDING order, so the sense of the comparison is backwards from
    // usual...
    int operator <  (const CombatGroup& g)  const { return value >  g.value; }
    int operator <= (const CombatGroup& g)  const { return value >= g.value; }
    int operator == (const CombatGroup& g)  const { return this  == &g;      }

    // operations:
    static CombatGroup*  LoadOrderOfBattle(const char* fname, int iff, Combatant* combatant);
    static void          SaveOrderOfBattle(const char* fname, CombatGroup* force);
    static void          MergeOrderOfBattle(BYTE* block, const char* fname, int iff, Combatant* combatant, Campaign* campaign);

    void                 AddComponent(CombatGroup* g);
    CombatGroup*         FindGroup(int t, int n=-1);
    CombatGroup*         Clone(bool deep=true);

    // accessors and mutators:
    const char* GetDescription()           const;
    const char* GetShortDescription()      const;

    void                 SetCombatant(Combatant* c) { combatant = c; }

    Combatant*           GetCombatant()          { return combatant;  }
    CombatGroup*         GetParent()             { return parent;     }
    List<CombatGroup>&   GetComponents()         { return components; }
    List<CombatGroup>&   GetLiveComponents()     { return live_comp;  }
    List<CombatUnit>&    GetUnits()              { return units;      }
    CombatUnit*          GetRandomUnit();
    CombatUnit*          GetFirstUnit();
    CombatUnit*          GetNextUnit();
    CombatUnit*          FindUnit(const char* name);
    CombatGroup*         FindCarrier();

    const Text&    Name()                        const { return name;       }
    int            Type()                        const { return type;       }
    int            CountUnits()                  const;
    int            IntelLevel()                  const { return enemy_intel;}
    int            GetID()                       const { return id;         }
    int            GetIFF()                      const { return iff;        }
    Point          Location()                    const { return location;   }
    void           MoveTo(const Point& loc);
    const Text&    GetRegion()                   const { return region;     }
    void           SetRegion(Text rgn)                 { region = rgn;      }
    void           AssignRegion(Text rgn);
    int            Value()                       const { return value;      }
    int            Sorties()                     const { return sorties;    }
    void           SetSorties(int n)                   { sorties = n;       }
    int            Kills()                       const { return kills;      }
    void           SetKills(int n)                     { kills = n;         }
    int            Points()                      const { return points;     }
    void           SetPoints(int n)                    { points = n;        }
    int            UnitIndex()                   const { return unit_index; }

    double         GetNextJumpTime()             const;

    double         GetPlanValue()                const { return plan_value; }
    void           SetPlanValue(double v)              { plan_value = v;    }

    bool           IsAssignable()                const;
    bool           IsTargetable()                const;
    bool           IsDefensible()                const;
    bool           IsStrikeTarget()              const;
    bool           IsMovable()                   const;
    bool           IsFighterGroup()              const;
    bool           IsStarshipGroup()             const;
    bool           IsReserve()                   const;

    // these two methods return zero terminated arrays of
    // integers identifying the preferred assets for attack
    // or defense in priority order:
    static const int* PreferredAttacker(int type);
    static const int* PreferredDefender(int type);

    bool           IsExpanded()                  const { return expanded; }
    void           SetExpanded(bool e)                 { expanded = e;    }

    const Text&    GetAssignedSystem()           const { return assigned_system; }
    void           SetAssignedSystem(const char* s);
    CombatZone*    GetCurrentZone()              const { return current_zone;  }
    void           SetCurrentZone(CombatZone* z)       { current_zone = z;     }
    CombatZone*    GetAssignedZone()             const { return assigned_zone; }
    void           SetAssignedZone(CombatZone* z);
    void           ClearUnlockedZones();
    bool           IsZoneLocked()                const { return assigned_zone && zone_lock;   }
    void           SetZoneLock(bool lock=true);
    bool           IsSystemLocked()              const { return assigned_system.length() > 0; }

    const Text&    GetStrategicDirection()       const { return strategic_direction; }
    void           SetStrategicDirection(Text dir)     { strategic_direction = dir;  }

    void           SetIntelLevel(int n);
    int            CalcValue();

    List<CombatAssignment>& GetAssignments()           { return assignments; }
    void                    ClearAssignments();

    static int           TypeFromName(const char* name);
    static const char*   NameFromType(int type);

private:        
    const char* GetOrdinal()               const;

    // attributes:
    int                  type;
    int                  id;
    Text                 name;
    int                  iff;
    int                  enemy_intel;

    double               plan_value; // scratch pad for plan modules

    List<CombatUnit>     units;
    List<CombatGroup>    components;
    List<CombatGroup>    live_comp;
    Combatant*           combatant;
    CombatGroup*         parent;
    Text                 region;
    Point                location;
    int                  value;
    int                  unit_index;

    int                  sorties;
    int                  kills;
    int                  points;

    bool                 expanded;   // for tree control

    Text                    assigned_system;
    CombatZone*             current_zone;
    CombatZone*             assigned_zone;
    bool                    zone_lock;
    List<CombatAssignment>  assignments;

    Text                 strategic_direction;
};

#endif CombatGroup_h

