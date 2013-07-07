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
    FILE:         CombatUnit.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A ship, station, or ground unit in the dynamic campaign.
*/

#ifndef CombatUnit_h
#define CombatUnit_h

#include "Types.h"
#include "Geometry.h"
#include "Color.h"
#include "Text.h"
#include "List.h"

// +--------------------------------------------------------------------+

class CombatGroup;
class ShipDesign;

// +--------------------------------------------------------------------+

class CombatUnit
{
public:
    static const char* TYPENAME() { return "CombatUnit"; }

    CombatUnit(const char* n, const char* reg, int t, const char* dname, int number, int i);
    CombatUnit(const CombatUnit& unit);

    int operator == (const CombatUnit& u)  const { return this == &u; }

    const char*    GetDescription()           const;

    int            GetValue()                    const;
    int            GetSingleValue()              const;
    bool           CanDefend(CombatUnit* unit)   const;
    bool           CanLaunch()                   const;
    double         PowerVersus(CombatUnit* tgt)  const;
    int            AssignMission();
    void           CompleteMission();

    double         MaxRange()                    const;
    double         MaxEffectiveRange()           const;
    double         OptimumRange()                const;

    void           Engage(CombatUnit* tgt);
    void           Disengage();

    // accessors and mutators:
    const Text&    Name()                        const { return name;          }
    const Text&    Registry()                    const { return regnum;        }
    const Text&    DesignName()                  const { return design_name;   }
    const Text&    Skin()                        const { return skin;          }
    void           SetSkin(const char* s)              { skin = s;             }
    int            Type()                        const { return type;          }
    int            Count()                       const { return count;         }
    int            LiveCount()                   const { return count - dead_count; }
    int            DeadCount()                   const { return dead_count;    }
    void           SetDeadCount(int n)                 { dead_count = n;       }
    int            Kill(int n);
    int            Available()                   const { return available;     }
    int            GetIFF()                      const { return iff;           }
    bool           IsLeader()                    const { return leader;        }
    void           SetLeader(bool l)                   { leader = l;           }
    Point          Location()                    const { return location;      }
    void           MoveTo(const Point& loc);
    Text           GetRegion()                   const { return region;        }
    void           SetRegion(Text rgn)                 { region = rgn;         }
    CombatGroup*   GetCombatGroup()        const { return group;               }
    void           SetCombatGroup(CombatGroup* g){ group = g;                  }

    Color          MarkerColor()                 const;
    bool           IsGroundUnit()                const;
    bool           IsStarship()                  const;
    bool           IsDropship()                  const;
    bool           IsStatic()                    const;

    CombatUnit*    GetCarrier()               const { return carrier;          }
    void           SetCarrier(CombatUnit* c)        { carrier = c;             }

    const ShipDesign* GetDesign();
    int            GetShipClass()                const;

    List<CombatUnit>& GetAttackers()                   { return attackers;     }

    double         GetPlanValue()                const { return plan_value;    }
    void           SetPlanValue(int v)                 { plan_value = v;       }

    double         GetSustainedDamage()          const { return sustained_damage; }
    void           SetSustainedDamage(double d)        { sustained_damage = d;    }

    double         GetHeading()                  const { return heading;       }
    void           SetHeading(double d)                { heading = d;          }

    double         GetNextJumpTime()             const { return jump_time;     }

private:
    Text                 name;
    Text                 regnum;
    Text                 design_name;
    Text                 skin;
    int                  type;
    const ShipDesign*    design;
    int                  count;
    int                  dead_count;
    int                  available;
    int                  iff;
    bool                 leader;
    Text                 region;
    Point                location;
    double               plan_value; // scratch pad for plan modules
    double               launch_time;
    double               jump_time;
    double               sustained_damage;
    double               heading;

    CombatUnit*          carrier;
    List<CombatUnit>     attackers;
    CombatUnit*          target;
    CombatGroup*         group;
};

#endif CombatUnit_h

