/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FighterAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship (low-level) Artifical Intelligence class
*/

#ifndef StarshipAI_h
#define StarshipAI_h

#include "Types.h"
#include "ShipAI.h"

// +--------------------------------------------------------------------+

class StarshipAI : public ShipAI
{
public:
   StarshipAI(SimObject* s);
   virtual ~StarshipAI();

   // convert the goal point from world to local coords:
   virtual void      FindObjective();

protected:
   // accumulate behaviors:
   virtual void      Navigator();
   virtual Steer     SeekTarget();
   virtual Steer     AvoidCollision();

   // steering functions:
   virtual Steer     Seek(const Point& point);
   virtual Steer     Flee(const Point& point);
   virtual Steer     Avoid(const Point& point, float radius);

   virtual Point     Transform(const Point& pt);

   // fire on target if appropriate:
   virtual void      FireControl();
   virtual void      HelmControl();
   virtual void      ThrottleControl();

   System*           SelectSubtarget();
   bool              AssessTargetPointDefense();

   DWORD             sub_select_time;
   DWORD             point_defense_time;
   System*           subtarget;
   bool              tgt_point_defense;
};

// +--------------------------------------------------------------------+

#endif StarshipAI_h

