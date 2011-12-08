/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

