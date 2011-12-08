/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Thruster.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Conventional Thruster (system) class
*/

#ifndef Thruster_h
#define Thruster_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

struct ThrusterPort {
   static const char* TYPENAME() { return "ThrusterPort"; }

   ThrusterPort(int t, const Point& l, DWORD f, float s);
   ~ThrusterPort();

   int         type;
   DWORD       fire;
   float       burn;
   float       scale;
   Point       loc;
   Graphic*    flare;
   Graphic*    trail;
};

// +--------------------------------------------------------------------+

class Thruster : public System
{
public:
   static const char* TYPENAME() { return "Thruster"; }

   enum Constants {
      LEFT,  RIGHT, FORE,    AFT,     TOP,    BOTTOM,
      YAW_L, YAW_R, PITCH_D, PITCH_U, ROLL_L, ROLL_R
   };

   Thruster(int dtype, double thrust, float flare_scale=0);
   Thruster(const Thruster& rhs);
   virtual ~Thruster();

   static void       Initialize();
   static void       Close();

   virtual void      ExecFrame(double seconds);
   virtual void      ExecTrans(double x, double y, double z);
   virtual void      SetShip(Ship* s);

   virtual double    TransXLimit();
   virtual double    TransYLimit();
   virtual double    TransZLimit();

   virtual void      AddPort(int type, const Point& loc, DWORD fire, float flare_scale=0);
   virtual void      CreatePort(int type, const Point& loc, DWORD fire, float flare_scale);

   int               NumThrusters()       const;
   Graphic*          Flare(int engine)    const;
   Graphic*          Trail(int engine)    const;
   virtual void      Orient(const Physical* rep);
   virtual double    GetRequest(double seconds) const;

protected:
   void              IncBurn(int inc, int dec);
   void              DecBurn(int a,   int b);

   Ship*             ship;
   float             thrust;
   float             scale;
   float             burn[12];

   float             avail_x,   avail_y,    avail_z;
   float             trans_x,   trans_y,    trans_z;
   float             roll_rate, pitch_rate, yaw_rate;
   float             roll_drag, pitch_drag, yaw_drag;

   List<ThrusterPort>   ports;
};

#endif Thruster_h

