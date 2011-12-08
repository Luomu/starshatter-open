/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

