/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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
