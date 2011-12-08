/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NavLight.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation Lights System class
*/

#ifndef NavLight_h
#define NavLight_h

#include "Types.h"
#include "System.h"
#include "DriveSprite.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class NavLight : public System
{
public:
   enum Constants { MAX_LIGHTS = 8 };

   NavLight(double period, double scale);
   NavLight(const NavLight& rhs);
   virtual ~NavLight();

   static void    Initialize();
   static void    Close();

   virtual void   ExecFrame(double seconds);

   int            NumBeacons()      const { return nlights;       }
   Sprite*        Beacon(int index) const { return beacon[index]; }
   bool           IsEnabled()       const { return enable;        }

   virtual void   Enable();
   virtual void   Disable();
   virtual void   AddBeacon(Point loc, DWORD pattern, int type=1);
   virtual void   SetPeriod(double p);
   virtual void   SetPattern(int index, DWORD p);
   virtual void   SetOffset(DWORD o);

   virtual void   Orient(const Physical* rep);

protected:
   double      period;
   double      scale;
   bool        enable;

   int         nlights;

   Point          loc[MAX_LIGHTS];
   DriveSprite*   beacon[MAX_LIGHTS];
   DWORD          pattern[MAX_LIGHTS];
   int            beacon_type[MAX_LIGHTS];
   DWORD          offset;
};

#endif NavLight_h

