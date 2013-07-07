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
    FILE:         Element.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Package Element (e.g. Flight) class
*/

#ifndef Element_h
#define Element_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Ship;
class Instruction;
class RadioMessage;

class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+

class Element : public SimObserver
{
public:
    // CONSTRUCTORS:
    Element(const char* call_sign, int iff, int type=0 /*PATROL*/);
    virtual ~Element();

    int operator == (const Element& e) const { return id == e.id; }

    // GENERAL ACCESSORS:
    int               Identity()        const { return id;               }
    int               Type()            const { return type;             }
    const Text&       Name()            const { return name;             }
    void              SetName(const char* s)  { name = s;                }
    virtual int       GetIFF()          const { return iff;              }
    int               Player()          const { return player;           }
    void              SetPlayer(int p)        { player = p;              }
    DWORD             GetLaunchTime()   const { return launch_time;      }
    void              SetLaunchTime(DWORD t);
    int               IntelLevel()      const { return intel;            }
    void              SetIntelLevel(int i)    { intel = i;               }

    // ELEMENT COMPONENTS:
    int               NumShips()        const { return ships.size();  }
    int               AddShip(Ship*, int index=-1);
    void              DelShip(Ship*);
    Ship*             GetShip(int index);
    int               GetShipClass();
    int               FindIndex(const Ship* s);
    bool              Contains(const Ship* s);
    bool              IsActive()        const;
    bool              IsFinished()      const;
    bool              IsNetObserver()   const;
    bool              IsSquadron()      const;
    bool              IsStatic()        const;
    bool              IsHostileTo(const Ship* s)          const;
    bool              IsHostileTo(int iff_code)           const;
    bool              IsObjectiveTargetOf(const Ship* s)  const;
    bool              IsRogue()         const { return rogue;            }
    bool              IsPlayable()      const { return playable;         }
    int*              Loadout()               { return load;             }

    void              SetRogue(bool r)        { rogue = r;               }
    void              SetPlayable(bool p)     { playable = p;            }
    void              SetLoadout(int* l);
    virtual void      SetIFF(int iff);

    virtual void         ExecFrame(double seconds);
    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    // OBJECTIVES:
    void              ClearObjectives();
    void              AddObjective(Instruction* obj);
    Instruction*      GetObjective(int index);
    Instruction*      GetTargetObjective();
    int               NumObjectives()   const { return objectives.size(); }

    void              ClearInstructions();
    void              AddInstruction(const char* instr);
    Text              GetInstruction(int index);
    int               NumInstructions()   const { return instructions.size(); }

    // ORDERS AND NAVIGATION:
    double            GetHoldTime();
    void              SetHoldTime(double t);
    bool              GetZoneLock();
    void              SetZoneLock(bool z);
    void              AddNavPoint(Instruction* pt, Instruction* afterPoint=0, bool send=true);
    void              DelNavPoint(Instruction* pt, bool send=true);
    void              ClearFlightPlan(bool send=true);
    Instruction*      GetNextNavPoint();
    int               GetNavIndex(const Instruction* n);
    List<Instruction>& GetFlightPlan();
    int               FlightPlanLength();
    virtual void      HandleRadioMessage(RadioMessage* msg);

    // CHAIN OF COMMAND:
    Element*          GetCommander()             const { return commander;  }
    void              SetCommander(Element* e)         { commander = e;     }
    Element*          GetAssignment()            const { return assignment; }
    void              SetAssignment(Element* e)        { assignment = e;    }
    void              ResumeAssignment();
    bool              CanCommand(Element* e);
    Ship*             GetCarrier()               const { return carrier;    }
    void              SetCarrier(Ship* c)              { carrier = c;       }
    int               GetCommandAILevel()        const { return command_ai; }
    void              SetCommandAILevel(int n)         { command_ai = n;    }
    const Text&       GetSquadron()              const { return squadron;   }
    void              SetSquadron(const char* s)       { squadron = s;      }

    // DYNAMIC CAMPAIGN:
    CombatGroup*      GetCombatGroup()               { return combat_group; }
    void              SetCombatGroup(CombatGroup* g) { combat_group = g;    }
    CombatUnit*       GetCombatUnit()                { return combat_unit;  }
    void              SetCombatUnit(CombatUnit* u)   { combat_unit = u;     }

    // SQUADRON STUFF:
    int               GetCount()                 const { return count;      }
    void              SetCount(int n)                  { count = n;         }

protected:
    int               id;
    int               iff;
    int               type;
    int               player;
    int               command_ai;
    int               respawns;
    int               intel;
    Text              name;

    // squadron elements only:
    int               count;

    List<Ship>        ships;
    List<Text>        ship_names;
    List<Text>        instructions;
    List<Instruction> objectives;
    List<Instruction> flight_plan;

    Element*          commander;
    Element*          assignment;
    Ship*             carrier;
    Text              squadron;

    CombatGroup*      combat_group;
    CombatUnit*       combat_unit;
    DWORD             launch_time;
    double            hold_time;

    bool              rogue;
    bool              playable;
    bool              zone_lock;
    int               load[16];
};

#endif Element_h

