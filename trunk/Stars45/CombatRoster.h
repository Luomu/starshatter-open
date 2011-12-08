/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CombatRoster.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    The complete roster of all known persistent entities
    for all combatants in the game.
*/

#ifndef CombatRoster_h
#define CombatRoster_h

#include "Types.h"
#include "Geometry.h"
#include "List.h"

// +--------------------------------------------------------------------+

class CombatGroup;

// +--------------------------------------------------------------------+

class CombatRoster
{
   CombatRoster();
   ~CombatRoster();

public:
   static const char* TYPENAME() { return "CombatRoster"; }

   static void             Initialize();
   static void             Close();
   static CombatRoster*    GetInstance();

   CombatGroup*            GetForce(const char* name);

private:
   List<CombatGroup>       forces;
};


#endif CombatRoster_h
