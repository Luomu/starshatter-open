/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         MultiController.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    MultiController Input class
*/

#include "MemDebug.h"
#include "MultiController.h"

// +--------------------------------------------------------------------+

MultiController::MultiController()
   : x(0), y(0), z(0), p(0), r(0), w(0), c(0), p1(0), r1(0), w1(0), t(0)
{
   for (int i = 0; i < MotionController::MaxActions; i++)
      action[i] = 0;

   nctrl = 0;
   for (i = 0; i < 4; i++)
      ctrl[i] = 0;
}

MultiController::~MultiController()
{
   for (int i = 0; i < 4; i++)
      delete ctrl[i];
}

// +--------------------------------------------------------------------+

void
MultiController::AddController(MotionController* c)
{
   if (nctrl < 4 && c)
      ctrl[nctrl++] = c;
}

void
MultiController::MapKeys(KeyMapEntry* mapping, int nkeys)
{
   for (int i = 0; i < nctrl; i++)
      ctrl[i]->MapKeys(mapping, nkeys);
}

int
MultiController::GetSwapYawRoll() const
{
   if (nctrl)
      return ctrl[0]->GetSwapYawRoll();

   return 0;
}

void
MultiController::SwapYawRoll(int swap)
{
   for (int i = 0; i < nctrl; i++)
      ctrl[i]->SwapYawRoll(swap);
}

// +--------------------------------------------------------------------+

inline void clamp(double& x) { if (x<-1)x=-1; else if (x>1)x=1; }

void
MultiController::Acquire()
{
   t = x = y = z = p = r = w = c = 0;

   for (int i = 0; i < MotionController::MaxActions; i++)
      action[i] = 0;

   for (i = 0; i < nctrl; i++) {
      ctrl[i]->Acquire();

      x += ctrl[i]->X();
      y += ctrl[i]->Y();
      z += ctrl[i]->Z();

      r += ctrl[i]->Roll();
      p += ctrl[i]->Pitch();
      w += ctrl[i]->Yaw();
      c += ctrl[i]->Center();
      t += ctrl[i]->Throttle();
      
      for (int a = 0; a < MotionController::MaxActions; a++)
         action[a] += ctrl[i]->Action(a);
   }

   clamp(x);
   clamp(y);
   clamp(z);
   clamp(r);
   clamp(p);
   clamp(w);
   clamp(t);
}

// +--------------------------------------------------------------------+

void
MultiController::SetThrottle(double throttle)
{
   for (int i = 0; i < nctrl; i++)
      ctrl[i]->SetThrottle(throttle);
}

// +--------------------------------------------------------------------+

int
MultiController::ActionMap(int key)
{
   for (int i = 0; i < nctrl; i++) {
      int result = ctrl[i]->ActionMap(key);

      if (result)
         return result;
   }

   return 0;
}

