/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TacticalAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Common base class and interface for mid-level (tactical) AI
*/

#ifndef TacticalAI_h
#define TacticalAI_h

#include "Types.h"
#include "Director.h"

// +--------------------------------------------------------------------+

class Ship;
class ShipAI;
class Instruction;
class CarrierAI;

// +--------------------------------------------------------------------+

class TacticalAI : public Director
{
public:
   TacticalAI(ShipAI* ai);
   virtual ~TacticalAI();

   enum ROE {
      NONE,
      SELF_DEFENSIVE,
      DEFENSIVE,
      DIRECTED,
      FLEXIBLE,
      AGRESSIVE
   };

   virtual void      ExecFrame(double seconds);

   virtual ROE       RulesOfEngagement()  const { return roe;           }
   virtual double    ThreatLevel()        const { return threat_level;  }
   virtual double    SupportLevel()       const { return support_level; }

protected:
   // pick the best target if we don't have one yet:
   virtual void      CheckOrders();
   virtual bool      CheckShipOrders();
   virtual bool      ProcessOrders();
   virtual bool      CheckFlightPlan();
   virtual bool      CheckObjectives();

   virtual void      SelectTarget();
   virtual void      SelectTargetDirected(Ship* tgt=0);
   virtual void      SelectTargetOpportunity();
   virtual void      CheckTarget();
   virtual void      FindThreat();
   virtual void      FindSupport();
   virtual void      FindFormationSlot(int formation);

   virtual bool      CanTarget(Ship* tgt);
   virtual void      ClearRadioOrders();

   Ship*             ship;
   ShipAI*           ship_ai;
   CarrierAI*        carrier_ai;

   Instruction*      navpt;
   Instruction*      orders;

   double            agression;
   ROE               roe;
   int               element_index;
   int               action;
   int               exec_time;
   int               directed_tgtid;

   double            threat_level;
   double            support_level;
};

// +--------------------------------------------------------------------+

#endif TacticalAI_h

