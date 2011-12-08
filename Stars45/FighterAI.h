/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FighterAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Fighter (low-level) Artifical Intelligence class
*/

#ifndef FighterAI_h
#define FighterAI_h

#include "Types.h"
#include "ShipAI.h"

// +--------------------------------------------------------------------+

class Ship;
class Shot;
class InboundSlot;

// +--------------------------------------------------------------------+

class FighterAI : public ShipAI
{
public:
   FighterAI(SimObject* s);
   virtual ~FighterAI();

   virtual void      ExecFrame(double seconds);
   virtual int       Subframe()  const { return true; }

   // convert the goal point from world to local coords:
   virtual void      FindObjective();
   virtual void      FindObjectiveNavPoint();

protected:
   // behaviors:
   virtual Steer     AvoidTerrain();
   virtual Steer     SeekTarget();
   virtual Steer     EvadeThreat();
   virtual Point     ClosingVelocity();

   // accumulate behaviors:
   virtual void      Navigator();

   // steering functions:
   virtual Steer     Seek(const Point& point);
   virtual Steer     SeekFormationSlot();

   // fire on target if appropriate:
   virtual void      FireControl();
   virtual void      HelmControl();
   virtual void      ThrottleControl();

   virtual double    CalcDefensePerimeter(Ship* starship);
   virtual void      ReturnToBase(Ship* controller);

   Shot*             decoy_missile;
   double            missile_time;
   int               terrain_warning;
   int               drop_state;
   char              dir_info[32];
   double            brakes;
   double            z_shift;
   double            time_to_dock;
   InboundSlot*      inbound;
   int               rtb_code;
   bool              evading;
   DWORD             jink_time;
   Point             jink;
   bool              over_threshold;
   bool              form_up;
   bool              go_manual;
};

// +--------------------------------------------------------------------+

#endif FighterAI_h

