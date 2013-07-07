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

