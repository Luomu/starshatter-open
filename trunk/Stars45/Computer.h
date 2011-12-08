/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Computer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Various computer systems class
*/

#ifndef Computer_h
#define Computer_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Computer : public System
{
public:
   enum CompType { AVIONICS=1, FLIGHT, TACTICAL };

   Computer(int comp_type, const char* comp_name);
   Computer(const Computer& rhs);
   virtual ~Computer();

   virtual void      ApplyDamage(double damage);
   virtual void      ExecFrame(double seconds);
   virtual void      Distribute(double delivered_energy, double seconds);

protected:
};

#endif Computer_h

