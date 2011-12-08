/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Computer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Computer System class
*/

#include "MemDebug.h"
#include "Computer.h"
#include "Game.h"

// +----------------------------------------------------------------------+

static int     computer_value[] = {
   0, 1, 1, 1, 1
};

// +----------------------------------------------------------------------+

Computer::Computer(int comp_type, const char* comp_name)
   : System(COMPUTER, comp_type, comp_name, 1, 1, 1, 1)
{
   SetAbbreviation(Game::GetText("sys.computer.abrv"));
   power_flags = POWER_WATTS | POWER_CRITICAL;

   if (subtype == FLIGHT) {
      crit_level = -1.0f;
   }
}

// +----------------------------------------------------------------------+

Computer::Computer(const Computer& c)
   : System(c)
{
   Mount(c);
   SetAbbreviation(c.Abbreviation());
   power_flags = POWER_WATTS | POWER_CRITICAL;

   if (subtype == FLIGHT) {
      crit_level = -1.0f;
   }
}

// +--------------------------------------------------------------------+

Computer::~Computer()
{ }

// +--------------------------------------------------------------------+

void
Computer::ApplyDamage(double damage)
{
   System::ApplyDamage(damage);
}

// +--------------------------------------------------------------------+

void
Computer::ExecFrame(double seconds)
{
   energy = 0.0f;
   System::ExecFrame(seconds);
}

// +--------------------------------------------------------------------+

void
Computer::Distribute(double delivered_energy, double seconds)
{
   if (IsPowerOn()) {
      // convert Joules to Watts:
      energy = (float) (delivered_energy/seconds);

      // brown out:
      if (energy < capacity*0.75f)
         power_on = false;

      // spike:
      else if (energy > capacity*1.5f) {
         power_on = false;
         ApplyDamage(50);
      }
   }
}
