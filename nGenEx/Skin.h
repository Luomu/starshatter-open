/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Skin.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for managing run-time selectable skins on solid objects
*/

#ifndef Skin_h
#define Skin_h

#include "Polygon.h"
#include "Graphic.h"
#include "Video.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Solid;
class Model;
class Surface;
class Segment;

class Skin;
class SkinCell;

// +--------------------------------------------------------------------+

class Skin
{
public:
   static const char* TYPENAME() { return "Skin"; }
   enum            { NAMELEN=64 };

   Skin(const char* name = 0);
   virtual ~Skin();

   // operations
   void              ApplyTo(Model* model)   const;
   void              Restore(Model* model)   const;

   // accessors / mutators
   const char*       Name()                  const { return name;          }
   const char*       Path()                  const { return path;          }
   int               NumCells()              const { return cells.size();  }

   void              SetName(const char* n);
   void              SetPath(const char* n);
   void              AddMaterial(const Material* mtl);

protected:
   char              name[NAMELEN];
   char              path[256];
   List<SkinCell>    cells;
};

// +--------------------------------------------------------------------+

class SkinCell
{
   friend class Skin;

public:
   static const char* TYPENAME() { return "SkinCell"; }

   SkinCell(const Material* mtl=0);
   ~SkinCell();

   int operator == (const SkinCell& other) const;

   const char*       Name()         const;
   const Material*   Skin()         const { return skin; }
   const Material*   Orig()         const { return orig; }

   void              SetSkin(const Material* mtl);
   void              SetOrig(const Material* mtl);

private:
   const Material*   skin;
   const Material*   orig;
};

// +--------------------------------------------------------------------+

#endif Skin_h

