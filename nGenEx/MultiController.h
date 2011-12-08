/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         MultiController.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ComboController Motion Controller class
*/

#ifndef MultiController_h
#define MultiController_h

#include "MotionController.h"

// +--------------------------------------------------------------------+

class MultiController : public MotionController
{
public:
   static const char* TYPENAME() { return "MultiController"; }

   MultiController();
   virtual ~MultiController();
   
   virtual void   AddController(MotionController* c);
   virtual void   MapKeys(KeyMapEntry* mapping, int nkeys);
   virtual void   SwapYawRoll(int swap);
   virtual int    GetSwapYawRoll() const;

   // sample the physical device
   virtual void   Acquire();

   // translations
   virtual double X()         { return x; }
   virtual double Y()         { return y; }
   virtual double Z()         { return z; }

   // rotations
   virtual double Pitch()     { return p; }
   virtual double Roll()      { return r; }
   virtual double Yaw()       { return w; }
   virtual int    Center()    { return c; }

   // throttle
   virtual double Throttle()  { return t; }
   virtual void   SetThrottle(double throttle);

   // actions
   virtual int    Action(int n) { return action[n]; }
   virtual int    ActionMap(int n);

protected:
   int                  nctrl;
   MotionController*    ctrl[4];

   double         x,y,z,p,r,w,t;
   double         p1, r1, w1;
   int            c;
   int            action[MotionController::MaxActions];
};

#endif // MultiController_h

