/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CarrierAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    "Air Boss" AI class for managing carrier fighter squadrons
*/

#ifndef CarrierAI_h
#define CarrierAI_h

#include "Types.h"
#include "Director.h"

// +--------------------------------------------------------------------+

class Sim;
class Ship;
class ShipAI;
class Instruction;
class Hangar;
class Element;
class FlightPlanner;

// +--------------------------------------------------------------------+

class CarrierAI : public Director
{
public:
   CarrierAI(Ship* s, int level);
   virtual ~CarrierAI();

   virtual void      ExecFrame(double seconds);

protected:
   virtual bool      CheckPatrolCoverage();
   virtual bool      CheckHostileElements();

   virtual bool      CreateStrike(Element* elem);

   virtual Element*  CreatePackage(int squad, int size, int code, const char* target=0, const char* loadname=0);
   virtual bool      LaunchElement(Element* elem);

   Sim*              sim;
   Ship*             ship;
   Hangar*           hangar;
   FlightPlanner*    flight_planner;
   int               exec_time;
   int               hold_time;
   int               ai_level;

   Element*          patrol_elem[4];
};

// +--------------------------------------------------------------------+

#endif CarrierAI_h

