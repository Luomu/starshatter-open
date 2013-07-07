/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

