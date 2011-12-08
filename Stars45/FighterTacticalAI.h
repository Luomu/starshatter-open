/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FighterTacticalAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fighter-specific mid-level (tactical) AI
*/

#ifndef FighterTacticalAI_h
#define FighterTacticalAI_h

#include "Types.h"
#include "TacticalAI.h"
#include "List.h"

// +--------------------------------------------------------------------+

class WeaponGroup;

// +--------------------------------------------------------------------+

class FighterTacticalAI : public TacticalAI
{
public:
   FighterTacticalAI(ShipAI* ai);
   virtual ~FighterTacticalAI();

protected:
   virtual bool      CheckFlightPlan();
   virtual bool      IsStrikeComplete(Instruction* instr=0);

   virtual void      SelectTarget();
   virtual void      SelectTargetDirected(Ship* tgt=0);
   virtual void      SelectTargetOpportunity();
   virtual void      FindFormationSlot(int formation);
   virtual void      FindThreat();

   virtual void      SelectSecondaryForTarget(Ship* tgt);
   virtual int       ListSecondariesForTarget(Ship* tgt, List<WeaponGroup>& weps);

   bool              winchester[4];
   DWORD             THREAT_REACTION_TIME;
   DWORD             secondary_selection_time;
   int               ai_level;
};

// +--------------------------------------------------------------------+

#endif FighterTacticalAI_h

