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
    FILE:         Galaxy.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Galaxy (list of star systems) for a single campaign.
*/

#ifndef Galaxy_h
#define Galaxy_h

#include "Types.h"
#include "Solid.h"
#include "Bitmap.h"
#include "Geometry.h"
#include "Text.h"
#include "Term.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Star;
class StarSystem;
class Graphic;
class Light;
class Scene;

// +--------------------------------------------------------------------+

class Galaxy
{
public:
    Galaxy(const char* name);
    virtual ~Galaxy();

    int operator == (const Galaxy& s)   const { return name == s.name; }

    // operations:
    virtual void         Load();
    virtual void         Load(const char* filename);
    virtual void         ExecFrame();

    // accessors:
    const char*          Name()         const { return name;        }
    const char*          Description()  const { return description; }
    List<StarSystem>&    GetSystemList()      { return systems;     }
    List<Star>&          Stars()              { return stars;       }
    double               Radius()       const { return radius;      }

    StarSystem*          GetSystem(const char* name);
    StarSystem*          FindSystemByRegion(const char* rgn_name);

    static void          Initialize();
    static void          Close();
    static Galaxy*       GetInstance();

protected:
    char                 filename[64];
    Text                 name;
    Text                 description;
    double               radius;           // radius in parsecs

    List<StarSystem>     systems;
    List<Star>           stars;
};

#endif Galaxy_h

