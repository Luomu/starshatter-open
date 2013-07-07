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
    FILE:         Combatant.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    One side in a military conflict
*/

#ifndef Combatant_h
#define Combatant_h

#include "Types.h"
#include "Geometry.h"
#include "Text.h"
#include "List.h"

// +--------------------------------------------------------------------+

class CombatGroup;
class Mission;

// +--------------------------------------------------------------------+

class Combatant
{
public:
    static const char* TYPENAME() { return "Combatant"; }

    Combatant(const char* com_name, const char* file_name, int iff);
    Combatant(const char* com_name, CombatGroup* force);
    ~Combatant();

    // operations:

    // accessors:
    const char*             Name()           const { return name;   }
    int                     GetIFF()         const { return iff;    }
    int                     Score()          const { return score;  }
    const char*             GetDescription() const { return name;   }
    CombatGroup*            GetForce()             { return force;  }
    CombatGroup*            FindGroup(int type, int id=-1);
    List<CombatGroup>&      GetTargetList()  { return target_list;  }
    List<CombatGroup>&      GetDefendList()  { return defend_list;  }
    List<Mission>&          GetMissionList() { return mission_list; }

    void                    AddMission(Mission* m);
    void                    SetScore(int points)   { score =  points; }
    void                    AddScore(int points)   { score += points; }

    double                  GetTargetStratFactor(int type);
    void                    SetTargetStratFactor(int type, double f);

private:        
    Text                    name;
    int                     iff;
    int                     score;

    CombatGroup*            force;
    List<CombatGroup>       target_list;
    List<CombatGroup>       defend_list;
    List<Mission>           mission_list;

    double                  target_factor[8];
};


#endif Combatant_h
