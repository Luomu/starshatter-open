/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CampaignPlan.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    CampaignPlan defines the interface for all campaign
    planning algorithms.  Known subclasses:
      CampaignPlanStrategic  - strategic planning
      CampaignPlanAssignment - logistics planning
      CampaignPlanMission    - mission planning
      CampaignPlanMovement   - starship movement
      CampaignPlanEvent      - scripted events
*/

#ifndef CampaignPlan_h
#define CampaignPlan_h

#include "Types.h"
#include "Text.h"
#include "Term.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Campaign;
class Combatant;
class CombatGroup;
class CombatUnit;

// +--------------------------------------------------------------------+

class CampaignPlan
{
public:
   static const char* TYPENAME() { return "CampaignPlan"; }

   CampaignPlan(Campaign* c) : campaign(c), exec_time(-1e6) { }
   virtual ~CampaignPlan()                                  { }
   
   int operator == (const CampaignPlan& p) const { return this == &p; }
   
   // operations:
   virtual void      ExecFrame() { }
   virtual void      SetLockout(int seconds) { }

protected:
   Campaign*         campaign;
   double            exec_time;
};

#endif CampaignPlan_h

