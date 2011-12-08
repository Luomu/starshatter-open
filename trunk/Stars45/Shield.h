/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Shield.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Conventional Shield (system) class
*/

#ifndef Shield_h
#define Shield_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Shot;
class Sound;

// +--------------------------------------------------------------------+

class Shield : public System
{
public:
   enum SUBTYPE { DEFLECTOR = 1, GRAV_SHIELD, HYPER_SHIELD };

   Shield(SUBTYPE s);
   Shield(const Shield& rhs);
   virtual ~Shield();

   virtual void   ExecFrame(double seconds);
   double         DeflectDamage(Shot* shot, double shot_damage);

   double         ShieldLevel()              const { return shield_level * 100;     }
   double         ShieldFactor()             const { return shield_factor;          }
   double         ShieldCurve()              const { return shield_curve;           }
   void           SetShieldFactor(double f)        { shield_factor = (float) f;     }
   void           SetShieldCurve(double c)         { shield_curve  = (float) c;     }
   double         ShieldCutoff()             const { return shield_cutoff;          }
   void           SetShieldCutoff(double f)        { shield_cutoff = (float) f;     }
   double         Capacity()                 const { return capacity;               }
   double         Consumption()              const { return sink_rate;              }
   void           SetConsumption(double r)         { sink_rate = (float)r;          }
   bool           ShieldCapacitor()          const { return shield_capacitor;       }
   void           SetShieldCapacitor(bool c);
   bool           ShieldBubble()             const { return shield_bubble;          }
   void           SetShieldBubble(bool b)          { shield_bubble = b;             }
   double         DeflectionCost()           const { return deflection_cost;        }
   void           SetDeflectionCost(double c)      { deflection_cost = (float) c;   }

   // override from System:
   virtual void   SetPowerLevel(double level);
   virtual void   SetNetShieldLevel(int level);

   virtual void   Distribute(double delivered_energy, double seconds);
   virtual void   DoEMCON(int emcon);

protected:
   bool           shield_capacitor;
   bool           shield_bubble;
   float          shield_factor;
   float          shield_level;
   float          shield_curve;
   float          shield_cutoff;
   float          requested_power_level;
   float          deflection_cost;
};

#endif Shield_h

