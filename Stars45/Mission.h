/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Mission.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simulation Universe and Region classes
*/

#ifndef Mission_h
#define Mission_h

#include "Types.h"
#include "Intel.h"
#include "RLoc.h"
#include "Universe.h"
#include "Scene.h"
#include "Skin.h"
#include "Physical.h"
#include "Geometry.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Mission;
class MissionElement;
class MissionLoad;
class MissionEvent;
class MissionShip;

class CombatGroup;
class CombatUnit;

class Ship;
class System;
class Element;
class ShipDesign;
class WeaponDesign;
class StarSystem;
class Instruction;

class Term;
class TermArray;
class TermStruct;

// +--------------------------------------------------------------------+

class Mission
{
public:
   static const char* TYPENAME() { return "Mission"; }

   enum TYPE   
        {
            PATROL,
            SWEEP,
            INTERCEPT,
            AIR_PATROL,
            AIR_SWEEP,
            AIR_INTERCEPT,
            STRIKE,     // ground attack
            ASSAULT,    // starship attack
            DEFEND,
            ESCORT,
            ESCORT_FREIGHT,
            ESCORT_SHUTTLE,
            ESCORT_STRIKE,
            INTEL,
            SCOUT,
            RECON,
            BLOCKADE,
            FLEET,
            BOMBARDMENT,
            FLIGHT_OPS,
            TRANSPORT,
            CARGO,
            TRAINING,
            OTHER
        };

   Mission(int id, const char* filename=0, const char* path=0);
   virtual ~Mission();

   int operator == (const Mission& m)   const { return id == m.id;   }

   virtual void            Validate();
   virtual bool            Load(const char* filename=0, const char* path=0);
   virtual bool            Save();
   virtual bool            ParseMission(const char* buffer);
   virtual void            SetPlayer(MissionElement* player_element);
   virtual MissionElement* GetPlayer();

   // accessors/mutators:
   int                  Identity()      const { return id;           }
   const char*          FileName()      const { return filename;     }
   const char*          Name()          const { return name;         }
   const char*          Description()   const { return desc;         }
   const char*          Situation()     const { return sitrep;       }
   const char*          Objective()     const { return objective;    }
   const char*          Subtitles()     const;
   int                  Start()         const { return start;        }
   double               Stardate()      const { return stardate;     }
   int                  Type()          const { return type;         }
   const char*          TypeName()      const { return RoleName(type); }
   int                  Team()          const { return team;         }
   bool                 IsOK()          const { return ok;           }
   bool                 IsActive()      const { return active;       }
   bool                 IsComplete()    const { return complete;     }

   StarSystem*          GetStarSystem() const { return star_system;  }
   List<StarSystem>&    GetSystemList()       { return system_list;  }
   const char*          GetRegion()     const { return region;       }

   List<MissionElement>& GetElements()        { return elements;     }
   virtual MissionElement* FindElement(const char* name);
   virtual void            AddElement(MissionElement* elem);

   List<MissionEvent>&  GetEvents()           { return events;       }
   MissionEvent*        FindEvent(int event_type) const;
   virtual void         AddEvent(MissionEvent* event);

   MissionElement*      GetTarget()     const { return target;       }
   MissionElement*      GetWard()       const { return ward;         }

   void                 SetName(const char* n)        { name = n;          }
   void                 SetDescription(const char* d) { desc = d;          }
   void                 SetSituation(const char* sit) { sitrep = sit;      }
   void                 SetObjective(const char* obj) { objective = obj;   }
   void                 SetStart(int s)               { start = s;         }
   void                 SetType(int t)                { type = t;          }
   void                 SetTeam(int iff)              { team = iff;        }
   void                 SetStarSystem(StarSystem* s);
   void                 SetRegion(const char* rgn)    { region = rgn;      }
   void                 SetOK(bool a)                 { ok = a;            }
   void                 SetActive(bool a)             { active = a;        }
   void                 SetComplete(bool c)           { complete = c;      }
   void                 SetTarget(MissionElement* t)  { target = t;        }
   void                 SetWard(MissionElement* w)    { ward = w;          }

   void                 ClearSystemList();

   void                 IncreaseElemPriority(int index);
   void                 DecreaseElemPriority(int index);
   void                 IncreaseEventPriority(int index);
   void                 DecreaseEventPriority(int index);

   static const char*   RoleName(int role);
   static int           TypeFromName(const char* n);

   Text                 ErrorMessage() const          { return errmsg;     }
   void                 AddError(Text err);

   Text                 Serialize(const char* player_elem=0, int player_index=0);

protected:
   MissionElement*      ParseElement(TermStruct*  val);
   MissionEvent*        ParseEvent(TermStruct* val);
   MissionShip*         ParseShip(TermStruct* val, MissionElement* element);
   Instruction*         ParseInstruction(TermStruct* val, MissionElement* element);
   void                 ParseLoadout(TermStruct*  val, MissionElement* element);
   RLoc*                ParseRLoc(TermStruct* val);

   int                  id;
   char                 filename[64];
   char                 path[64];
   Text                 region;
   Text                 name;
   Text                 desc;
   int                  type;
   int                  team;
   int                  start;
   double               stardate;
   bool                 ok;
   bool                 active;
   bool                 complete;
   bool                 degrees;
   Text                 objective;
   Text                 sitrep;
   Text                 errmsg;
   Text                 subtitles;
   StarSystem*          star_system;
   List<StarSystem>     system_list;

   List<MissionElement> elements;
   List<MissionEvent>   events;

   MissionElement*      target;
   MissionElement*      ward;
   MissionElement*      current;
};

// +--------------------------------------------------------------------+

class MissionElement
{
   friend class Mission;

public:
   static const char* TYPENAME() { return "MissionElement"; }

   MissionElement();
   ~MissionElement();

   int operator == (const MissionElement& r) const { return id == r.id; }

   int               Identity()     const { return id;            }
   const Text&       Name()         const { return name;          }
   Text              Abbreviation() const;
   const Text&       Carrier()      const { return carrier;       }
   const Text&       Commander()    const { return commander;     }
   const Text&       Squadron()     const { return squadron;      }
   const Text&       Path()         const { return path;          }
   int               ElementID()    const { return elem_id;       }
   const ShipDesign* GetDesign()    const { return design;        }
   const Skin*       GetSkin()      const { return skin;          }
   int               Count()        const { return count;         }
   int               MaintCount()   const { return maint_count;   }
   int               DeadCount()    const { return dead_count;    }
   int               GetIFF()       const { return IFF_code;      }
   int               IntelLevel()   const { return intel;         }
   int               MissionRole()  const { return mission_role;  }
   int               Player()       const { return player;        }
   Text              RoleName()     const;
   Color             MarkerColor()  const;
   bool              IsStarship()   const;
   bool              IsDropship()   const;
   bool              IsStatic()     const;
   bool              IsGroundUnit() const;
   bool              IsSquadron()   const;
   bool              IsCarrier()    const;
   bool              IsAlert()      const { return alert;         }
   bool              IsPlayable()   const { return playable;      }
   bool              IsRogue()      const { return rogue;         }
   bool              IsInvulnerable() const { return invulnerable; }
   int               RespawnCount() const { return respawns;      }
   int               HoldTime()     const { return hold_time;     }
   int               CommandAI()    const { return command_ai;    }
   int               ZoneLock()     const { return zone_lock;     }

   const Text&       Region()       const { return rgn_name;      }
   Point             Location()     const;
   RLoc&             GetRLoc()            { return rloc;          }
   double            Heading()      const { return heading;       }

   Text              GetShipName(int n) const;
   Text              GetRegistry(int n) const;

   List<Instruction>&   Objectives()      { return objectives;    }
   List<Text>&          Instructions()    { return instructions;  }
   List<Instruction>&   NavList()         { return navlist;       }
   List<MissionLoad>&   Loadouts()        { return loadouts;      }
   List<MissionShip>&   Ships()           { return ships;         }

   void              SetName(const char* n)        { name = n;          }
   void              SetCarrier(const char* c)     { carrier = c;       }
   void              SetCommander(const char* c)   { commander = c;     }
   void              SetSquadron(const char* s)    { squadron = s;      }
   void              SetPath(const char* p)        { path = p;          }
   void              SetElementID(int id)          { elem_id = id;      }
   void              SetDesign(const ShipDesign* d){ design = d;        }
   void              SetSkin(const Skin* s)        { skin = s;          }
   void              SetCount(int n)               { count = n;         }
   void              SetMaintCount(int n)          { maint_count = n;   }
   void              SetDeadCount(int n)           { dead_count = n;    }
   void              SetIFF(int iff)               { IFF_code = iff;    }
   void              SetIntelLevel(int i)          { intel = i;         }
   void              SetMissionRole(int r)         { mission_role = r;  }
   void              SetPlayer(int p)              { player = p;        }
   void              SetPlayable(bool p)           { playable = p;      }
   void              SetRogue(bool r)              { rogue = r;         }
   void              SetInvulnerable(bool n)       { invulnerable = n;  }
   void              SetAlert(bool a)              { alert = a;         }
   void              SetCommandAI(int a)           { command_ai = a;    }
   void              SetRegion(const char* rgn)    { rgn_name = rgn;    }
   void              SetLocation(const Point& p);
   void              SetRLoc(const RLoc& r);
   void              SetHeading(double h)          { heading = h;       }
   void              SetRespawnCount(int r)        { respawns = r;      }
   void              SetHoldTime(int t)            { hold_time = t;     }
   void              SetZoneLock(int z)            { zone_lock = z;     }

   void              AddNavPoint(Instruction* pt, Instruction* afterPoint=0);
   void              DelNavPoint(Instruction* pt);
   void              ClearFlightPlan();
   int               GetNavIndex(const Instruction* n);

   void              AddObjective(Instruction* obj) { objectives.append(obj); }
   void              AddInstruction(const char* i)  { instructions.append(new(__FILE__,__LINE__) Text(i)); }

   CombatGroup*      GetCombatGroup()               { return combat_group; }
   void              SetCombatGroup(CombatGroup* g) { combat_group = g;    }
   CombatUnit*       GetCombatUnit()                { return combat_unit;  }
   void              SetCombatUnit(CombatUnit* u)   { combat_unit = u;     }

protected:
   int               id;
   Text              name;
   Text              carrier;
   Text              commander;
   Text              squadron;
   Text              path;
   int               elem_id;
   const ShipDesign* design;
   const Skin*       skin;
   int               count;
   int               maint_count;
   int               dead_count;
   int               IFF_code;
   int               mission_role;
   int               intel;
   int               respawns;
   int               hold_time;
   int               zone_lock;
   int               player;
   int               command_ai;
   bool              alert;
   bool              playable;
   bool              rogue;
   bool              invulnerable;

   Text              rgn_name;
   RLoc              rloc;
   double            heading;

   CombatGroup*      combat_group;
   CombatUnit*       combat_unit;

   List<Instruction> objectives;
   List<Text>        instructions;
   List<Instruction> navlist;
   List<MissionLoad> loadouts;
   List<MissionShip> ships;
};

// +--------------------------------------------------------------------+

class MissionLoad
{
   friend class Mission;

public:
   static const char* TYPENAME() { return "MissionLoad"; }

   MissionLoad(int ship=-1, const char* name=0);
   ~MissionLoad();

   int               GetShip() const;
   void              SetShip(int ship);

   Text              GetName() const;
   void              SetName(Text name);

   int*              GetStations();
   int               GetStation(int index);
   void              SetStation(int index, int selection);

protected:
   int               ship;
   Text              name;
   int               load[16];
};

// +--------------------------------------------------------------------+

class MissionShip
{
   friend class Mission;

public:
   static const char* TYPENAME() { return "MissionShip"; }

   MissionShip();
   ~MissionShip() { }

   const Text&       Name()                  const { return name;       }
   const Text&       RegNum()                const { return regnum;     }
   const Text&       Region()                const { return region;     }
   const Skin*       GetSkin()               const { return skin;       }
   const Point&      Location()              const { return loc;        }
   const Point&      Velocity()              const { return velocity;   }
   int               Respawns()              const { return respawns;   }
   double            Heading()               const { return heading;    }
   double            Integrity()             const { return integrity;  }
   int               Decoys()                const { return decoys;     }
   int               Probes()                const { return probes;     }
   const int*        Ammo()                  const { return ammo;       }
   const int*        Fuel()                  const { return fuel;       }

   void              SetName(const char* n)        { name = n;          }
   void              SetRegNum(const char* n)      { regnum = n;        }
   void              SetRegion(const char* n)      { region = n;        }
   void              SetSkin(const Skin* s)        { skin = s;          }
   void              SetLocation(const Point& p)   { loc = p;           }
   void              SetVelocity(const Point& p)   { velocity = p;      }
   void              SetRespawns(int r)            { respawns = r;      }
   void              SetHeading(double h)          { heading = h;       }
   void              SetIntegrity(double n)        { integrity = n;     }
   void              SetDecoys(int d)              { decoys = d;        }
   void              SetProbes(int p)              { probes = p;        }
   void              SetAmmo(const int* a);
   void              SetFuel(const int* f);

protected:
   Text              name;
   Text              regnum;
   Text              region;
   const Skin*       skin;
   Point             loc;
   Point             velocity;
   int               respawns;
   double            heading;
   double            integrity;
   int               decoys;
   int               probes;
   int               ammo[16];
   int               fuel[4];
};

#endif Mission_h

