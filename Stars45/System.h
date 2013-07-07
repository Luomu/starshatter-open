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
    FILE:         System.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Generic ship System class
*/

#ifndef System_h
#define System_h

#include "Types.h"
#include "Physical.h"
#include "Geometry.h"

#include "List.h"
#include "text.h"

// +--------------------------------------------------------------------+

class Component;
class Ship;
class SystemDesign;

// +--------------------------------------------------------------------+

class System
{
    friend Component;

public:
    static const char* TYPENAME() { return "System"; }

    enum CATEGORY    { MISC_SYSTEM=0, DRIVE=1, WEAPON, SHIELD, SENSOR,
        COMPUTER, POWER_SOURCE, FLIGHT_DECK, FARCASTER };
    enum STATUS      { DESTROYED, CRITICAL, DEGRADED, NOMINAL, MAINT };
    enum POWER_FLAGS { POWER_WATTS=1, POWER_CRITICAL=2 };

    System(CATEGORY t, int s, const char* n, int maxv,
    double energy=0, double capacity=100, double sink_rate=1);
    System(const System& s);
    virtual ~System();

    int operator==(const System& s)  const { return this == &s; }

    CATEGORY          Type()         const { return type;    }
    int               Subtype()      const { return subtype; }
    const char*       Name()         const { return name;    }
    const char*       Abbreviation() const { return abrv;    }

    void              SetName(const char* n)           { name = n; }
    void              SetAbbreviation(const char* a)   { abrv = a; }
    void              SetDesign(SystemDesign* d);

    virtual int       Value()        const { return (int) (max_value*availability*100); }
    int               MaxValue()     const { return (int) (max_value*100); }
    STATUS            Status()       const { return       status;        }
    double            Availability() const { return       availability*100;  }
    double            Safety()       const { return       safety*100;        }
    double            Stability()    const { return       stability*100;     }
    virtual void      CalcStatus();
    virtual void      Repair();

    double            NetAvail()     const { return net_avail;        }
    void              SetNetAvail(double d){ net_avail = (float) d;   }

    List<Component>&  GetComponents()      { return components; }

    virtual void      ApplyDamage(double damage);
    virtual void      ExecFrame(double seconds);
    virtual void      ExecMaintFrame(double seconds);
    virtual void      DoEMCON(int emcon);

    // PHYSICAL LOCATION (for inflicting system damage):
    virtual void      Orient(const Physical* rep);
    virtual void      Mount(Point loc, float radius, float hull_factor=0.5f);
    virtual void      Mount(const System& system);

    Point             MountLocation()  const { return mount_loc;   }
    double            Radius()         const { return radius;      }
    double            HullProtection() const { return hull_factor; }

    // POWER UTILIZATION:
    bool              IsPowerCritical() const { return (power_flags & POWER_CRITICAL)?true:false; }
    bool              UsesWatts()       const { return (power_flags & POWER_WATTS)?true:false;    }

    virtual double    GetRequest(double seconds) const;
    virtual void      Distribute(double delivered_energy, double seconds);

    int               GetSourceIndex() const { return source_index; }
    void              SetSourceIndex(int i)  { source_index = i;    }

    virtual int       Charge()    const { return (int) (100 * energy/capacity); }

    bool              IsPowerOn() const { return power_on;  }
    virtual void      PowerOn()         { power_on = true;  }
    virtual void      PowerOff()        { power_on = false; }

    // percentage, but stored as 0-1
    virtual double    GetPowerLevel()  const { return power_level * 100; }
    virtual void      SetPowerLevel(double level);
    virtual void      SetOverride(bool over);

    // for power drain damage:
    virtual void      DrainPower(double to_level);

    void              SetCapacity(double c) { capacity = (float) c; }
    double            GetCapacity()  const { return capacity;  }
    double            GetEnergy()    const { return energy;    }
    double            GetSinkRate()  const { return sink_rate; }
    void              SetEMCONPower(int emcon, int power_level);
    int               GetEMCONPower(int emcon);

    int               GetExplosionType() const { return explosion_type; }
    void              SetExplosionType(int t)  { explosion_type = t;    }

    Ship*             GetShip()      const { return ship;    }
    void              SetShip(Ship* s)     { ship = s;       }
    int               GetID()        const { return id;      }
    void              SetID(int n)         { id = n;         }

protected:
    // AI information:
    CATEGORY          type;
    Ship*             ship;
    int               id;
    int               subtype;
    int               max_value;

    // Displayable name:
    Text              name;
    Text              abrv;

    // System health status:
    STATUS            status;
    float             crit_level;
    float             availability;
    float             safety;
    float             stability;
    float             safety_overload;
    float             net_avail;

    // Mounting:
    Point             mount_loc;  // world space
    Point             mount_rel;  // object space
    float             radius;
    float             hull_factor;

    // Power Sink:
    float             energy;
    float             capacity;
    float             sink_rate;
    float             power_level;
    int               source_index;
    DWORD             power_flags;
    bool              power_on;
    BYTE              emcon_power[3];
    BYTE              emcon;

    int               explosion_type;

    // Subcomponents:
    SystemDesign*     design;
    List<Component>   components;
};

#endif System_h

