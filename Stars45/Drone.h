/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Drone.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Decoy / Weapons Drone class
*/

#ifndef Drone_h
#define Drone_h

#include "Types.h"
#include "Geometry.h"
#include "Shot.h"

// +--------------------------------------------------------------------+

class Camera;
class Ship;
class Trail;
class System;
class WeaponDesign;
class Sprite3D;
class Sound;

// +--------------------------------------------------------------------+

class Drone : public Shot
{
public:
   static const char* TYPENAME() { return "Drone"; }

   Drone(const Point& pos, const Camera& cam, WeaponDesign* design, const Ship* ship=0);
   virtual ~Drone();

   virtual void      SeekTarget(SimObject* target, System* sub=0);
   virtual void      ExecFrame(double factor);

   virtual bool      IsDrone()   const    { return true;             }
   virtual bool      IsDecoy()   const    { return decoy_type != 0;  }
   virtual bool      IsProbe()   const    { return probe?true:false; }

   virtual void      Disarm();
   virtual void      Destroy();

   // SENSORS AND VISIBILITY:
   virtual double       PCS()             const;
   virtual double       ACS()             const;
   virtual const char*  ClassName()       const;
   virtual int          Class()           const;

   // DAMAGE RESOLUTION:
   void        SetLife(int seconds) { life = seconds; }
   virtual int HitBy(Shot* shot, Point& impact);

protected:
   int               iff_code;
   int               decoy_type;
   int               probe;
};

#endif Drone_h

