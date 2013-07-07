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
    FILE:         MissionTemplate.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simulation Universe and Region classes
*/

#ifndef MissionTemplate_h
#define MissionTemplate_h

#include "Types.h"
#include "Mission.h"

// +--------------------------------------------------------------------+

class MissionTemplate;
class MissionAlias;
class MissionCallsign;
class MissionEvent;

// +--------------------------------------------------------------------+

class MissionTemplate : public Mission
{
public:
    static const char* TYPENAME() { return "MissionTemplate"; }

    MissionTemplate(int id, const char* filename=0, const char* path=0);
    virtual ~MissionTemplate();

    virtual bool         Load(const char* filename=0, const char* path=0);

    // accessors/mutators:
    virtual MissionElement* FindElement(const char* name);
    virtual void         AddElement(MissionElement* elem);
    virtual bool         MapElement(MissionElement* elem);
    virtual Text         MapShip(Text name);
    virtual CombatGroup* GetPlayerSquadron()           const { return player_squadron;  }
    virtual void         SetPlayerSquadron(CombatGroup* ps)  { player_squadron = ps;    }
    virtual Text         MapCallsign(const char* name, int iff);
    virtual bool         MapEvent(MissionEvent* event);


protected:
    CombatGroup*         FindCombatGroup(int iff, const ShipDesign* dsn);
    void                 ParseAlias(TermStruct*  val);
    void                 ParseCallsign(TermStruct*  val);
    bool                 ParseOptional(TermStruct* val);
    void                 CheckObjectives();

    List<MissionAlias>      aliases;
    List<MissionCallsign>   callsigns;
    CombatGroup*            player_squadron;
};

// +--------------------------------------------------------------------+

class MissionAlias
{
    friend class MissionTemplate;

public:
    static const char* TYPENAME() { return "MissionAlias"; }

    MissionAlias()                                  : elem(0)            { }
    MissionAlias(const char* n, MissionElement* e)  : name(n), elem(e)   { }
    virtual ~MissionAlias()                                              { }

    int operator == (const MissionAlias& a)   const { return name == a.name;  }

    Text              Name()                  const { return name;       }
    MissionElement*   Element()               const { return elem;       }

    void              SetName(const char* n)        { name = n;          }
    void              SetElement(MissionElement* e) { elem = e;          }

protected:
    Text              name;
    MissionElement*   elem;
};

// +--------------------------------------------------------------------+

class MissionCallsign
{
    friend class MissionTemplate;

public:
    static const char* TYPENAME() { return "MissionCallsign"; }

    MissionCallsign()                                                   { }
    MissionCallsign(const char* c, const char* n)  : call(c), name(n)   { }
    virtual ~MissionCallsign()                                          { }

    int operator == (const MissionCallsign& a)const { return call == a.call;  }

    Text              Callsign()              const { return call;       }
    Text              Name()                  const { return name;       }

    void              SetCallsign(const char* c)    { call = c;          }
    void              SetName(const char* n)        { name = n;          }

protected:
    Text              call;
    Text              name;
};


#endif MissionTemplate_h

