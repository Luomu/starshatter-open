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
    FILE:         Shot.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Laser and Missile class
*/

#ifndef Shot_h
#define Shot_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "Color.h"

// +--------------------------------------------------------------------+

class Camera;
class Ship;
class Trail;
class System;
class WeaponDesign;
class Sprite;
class Sound;

// +--------------------------------------------------------------------+

class Shot : public SimObject,
public SimObserver
{
public:
    static const char* TYPENAME() { return "Shot"; }

    Shot(const Point& pos, const Camera& cam, WeaponDesign* design, const Ship* ship=0);
    virtual ~Shot();

    virtual void      SeekTarget(SimObject* target, System* sub=0);
    virtual void      ExecFrame(double factor);
    static void       Initialize();
    static void       Close();

    virtual void      Activate(Scene& scene);
    virtual void      Deactivate(Scene& scene);

    const Ship*       Owner()        const { return owner;      }
    double            Damage()       const;
    int               ShotType()     const { return type;       }
    virtual SimObject* GetTarget()   const;

    virtual bool      IsPrimary()    const { return primary;    }
    virtual bool      IsDrone()      const { return false;      }
    virtual bool      IsDecoy()      const { return false;      }
    virtual bool      IsProbe()      const { return false;      }
    virtual bool      IsMissile()    const { return !primary;   }
    virtual bool      IsArmed()      const { return armed;      }
    virtual bool      IsBeam()       const { return beam;       }
    virtual bool      IsFlak()       const;
    virtual bool      IsHostileTo(const SimObject* o) const;

    bool              HitTarget()    const { return hit_target; }
    void              SetHitTarget(bool h) { hit_target = h;    }

    virtual bool      IsTracking(Ship* tgt) const;
    virtual double    PCS()          const { return 0;          }
    virtual double    ACS()          const { return 0;          }
    virtual int       GetIFF()       const;
    virtual Color     MarkerColor()  const;

    const Point&      Origin()       const { return origin;     }
    float             Charge()       const { return charge;     }
    void              SetCharge(float c);
    double            Length()       const;
    Graphic*          GetTrail()     const { return (Graphic*) trail; }
    void              SetFuse(double seconds);

    void              SetBeamPoints(const Point& from, const Point& to);
    virtual void      Disarm();
    virtual void      Destroy();

    const WeaponDesign* Design()     const { return design;     }
    const char*       DesignName()   const;
    int               GetEta()       const { return eta;        }
    void              SetEta(int t)        { eta = t;           }

    double            AltitudeMSL()  const;
    double            AltitudeAGL()  const;

    // SimObserver interface:
    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    int operator == (const Shot& s) const { return id == s.id; }

protected:
    const Ship*       owner;

    int               type;
    float             base_damage;
    float             charge;
    float             offset;
    float             altitude_agl;
    short             eta;
    BYTE              iff_code;
    bool              first_frame;
    bool              primary;
    bool              beam;
    bool              armed;
    bool              hit_target;

    Sprite*           flash;   // muzzle flash
    Sprite*           flare;   // drive flare
    Trail*            trail;   // exhaust trail

    Sound*            sound;
    WeaponDesign*     design;

    // for beam weapons:
    Point             origin;
};

#endif Shot_h

