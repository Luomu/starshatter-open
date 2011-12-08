/*  Project STARSHATTER
    John DiCamillo
    Copyright © 1997-2002. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FlightComp.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Flight Computer systems class
*/

#ifndef FLIGHT_COMP_H
#define FLIGHT_COMP_H

#include "Types.h"
#include "Computer.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class FlightComp : public Computer
{
public:
   enum CompType { AVIONICS=1, FLIGHT, TACTICAL };

   FlightComp(int comp_type, const char* comp_name);
   FlightComp(const Computer& rhs);
   virtual ~FlightComp();

   virtual void      ExecSubFrame();

   int               Mode()                  const { return mode; }
   double            Throttle()              const { return throttle; }

   void              SetMode(int m)                { mode   = m; }
   void              SetVelocityLimit(double v)    { vlimit = (float) v; }
   void              SetTransLimit(double x, double y, double z);

   void              FullStop()                    { halt   = true; }

protected:
   virtual void      ExecTrans();
   virtual void      ExecThrottle();

   int               mode;
   int               halt;
   float             throttle;

   float             vlimit;
   float             trans_x_limit;
   float             trans_y_limit;
   float             trans_z_limit;
};

#endif FLIGHT_COMP_H

