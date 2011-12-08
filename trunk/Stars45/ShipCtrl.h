/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         ShipCtrl.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship (or space/ground station) class
*/

#ifndef ShipCtrl_h
#define ShipCtrl_h

#include "Types.h"
#include "SimObject.h"
#include "MotionController.h"
#include "Director.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;
class ShipDesign;
class KeyMap;

// +--------------------------------------------------------------------+

class ShipCtrl : public Director
{
public:
   enum TYPE { DIR_TYPE = 1 };

   ShipCtrl(Ship* s, MotionController* m);

   virtual void      ExecFrame(double seconds);
   virtual int       Subframe()  const { return true; }
   virtual void      Launch();

   static  int       KeyDown(int action);
   static  int       Toggled(int action);

   virtual int       Type()      const { return DIR_TYPE; }

protected:
   Ship*             ship;
   MotionController* controller;

   bool              throttle_active;
   bool              launch_latch;
   bool              pickle_latch;
   bool              target_latch;
};

#endif ShipCtrl_h

