/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Light.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Dynamic Light Source
*/

#ifndef Light_h
#define Light_h

#include "Geometry.h"
#include "Color.h"

// +--------------------------------------------------------------------+

#define LIGHT_DESTROY(x) if (x) { x->Destroy(); x = 0; }

// +--------------------------------------------------------------------+

class Scene;

// +--------------------------------------------------------------------+

class Light
{
public:
   static const char* TYPENAME() { return "Light"; }

   enum TYPES {
      LIGHT_POINT       = 1,
      LIGHT_SPOT        = 2,
      LIGHT_DIRECTIONAL = 3,
      LIGHT_FORCE_DWORD = 0x7fffffff
   };

   Light(float l=0.0f, float dl=1.0f, int time=-1);
   virtual ~Light();
   
   int operator == (const Light& l) const { return id == l.id; }

   // operations
   virtual void      Update();

   // accessors / mutators
   int               Identity()        const { return id;      }
   Point             Location()        const { return loc;     }

   DWORD             Type()            const { return type;    }
   void              SetType(DWORD t)        { type = t;       }
   float             Intensity()       const { return light;   }
   void              SetIntensity(float f)   { light = f;      }
   Color             GetColor()        const { return color;   }
   void              SetColor(Color c)       { color = c;      }
   bool              IsActive()        const { return active;  }
   void              SetActive(bool a)       { active = a;     }
   bool              CastsShadow()     const { return shadow;  }
   void              SetShadow(bool s)       { shadow = s;     }

   bool              IsPoint()         const { return type == LIGHT_POINT;       }
   bool              IsSpot()          const { return type == LIGHT_SPOT;        }
   bool              IsDirectional()   const { return type == LIGHT_DIRECTIONAL; }

   virtual void      MoveTo(const Point& dst);
   virtual void      TranslateBy(const Point& ref);
   
   virtual int       Life()            const { return life;    }
   virtual void      Destroy();
   virtual Scene*    GetScene()        const { return scene;   }
   virtual void      SetScene(Scene*s)       { scene = s;      }

protected:
   static int        id_key;

   int               id;
   DWORD             type;
   Point             loc;
   int               life;
   float             light;
   float             dldt;
   Color             color;
   bool              active;
   bool              shadow;
   Scene*            scene;
};

// +--------------------------------------------------------------------+

#endif Light_h

