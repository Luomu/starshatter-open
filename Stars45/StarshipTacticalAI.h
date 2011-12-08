/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         StarshipTacticalAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship-specific mid-level (tactical) AI
*/

#ifndef StarshipTacticalAI_h
#define StarshipTacticalAI_h

#include "Types.h"
#include "TacticalAI.h"

// +--------------------------------------------------------------------+

class StarshipTacticalAI : public TacticalAI
{
public:
   StarshipTacticalAI(ShipAI* ai);
   virtual ~StarshipTacticalAI();

   virtual void      ExecFrame(double seconds);

protected:
   virtual void      FindThreat();
   virtual void      FindSupport();

   virtual void      CheckBugOut(Ship* c_ship, double range);

   DWORD             THREAT_REACTION_TIME;
   int               ai_level;
   double            drop_time;
   double            initial_integrity;
   bool              bugout;
};

// +--------------------------------------------------------------------+

#endif StarshipTacticalAI_h

