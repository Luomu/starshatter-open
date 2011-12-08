/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TerrainLayer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A blended detail texture applied to a terrain patch
    through a specific range of altitudes
*/

#ifndef TerrainLayer_h
#define TerrainLayer_h

#include "Types.h"
#include "Bitmap.h"

// +--------------------------------------------------------------------+

class Terrain;
class TerrainRegion;

// +--------------------------------------------------------------------+

class TerrainLayer
{
   friend class Terrain;
   friend class TerrainRegion;

public:
   static const char* TYPENAME() { return "TerrainLayer"; }

   TerrainLayer() : tile_texture(0), detail_texture(0), min_height(0), max_height(-1) { }
   ~TerrainLayer() { }

   int operator <  (const TerrainLayer& t) const { return min_height <  t.min_height; }
   int operator <= (const TerrainLayer& t) const { return min_height <= t.min_height; }
   int operator == (const TerrainLayer& t) const { return min_height == t.min_height; }

   // accessors:
   const char* GetTileName()        const { return tile_name;        }
   const char* GetDetailName()      const { return detail_name;      }
   Bitmap*     GetTileTexture()     const { return tile_texture;     }
   Bitmap*     GetDetailTexture()   const { return detail_texture;   }
   double      GetMinHeight()       const { return min_height;       }
   double      GetMaxHeight()       const { return max_height;       }

private:
   Text        tile_name;
   Text        detail_name;
   Bitmap*     tile_texture;
   Bitmap*     detail_texture;
   double      min_height;
   double      max_height;
};


#endif TerrainLayer_h

