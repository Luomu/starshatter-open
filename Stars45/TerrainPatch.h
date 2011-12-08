/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TerrainPatch.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A Single Multi-LOD Section of a Terrain Object
*/

#ifndef TerrainPatch_h
#define TerrainPatch_h

#include "Types.h"
#include "Solid.h"
#include "Geometry.h"
#include "Polygon.h"

// +--------------------------------------------------------------------+

class Projector;
class Terrain;
class Water;

// +--------------------------------------------------------------------+

class TerrainPatch : public Solid
{
public:
   TerrainPatch(Terrain* terrain,
                const Bitmap* patch, const Rect& rect,
                const Point&  p1,    const Point& p2);
   TerrainPatch(Terrain* terrain,    const Rect& rect,
                const Point&  p1,    const Point& p2,
                double sea_level);
   virtual ~TerrainPatch();

   virtual void      SelectDetail(Projector* projector);
   virtual void      Render(Video* video, DWORD flags);

   virtual int       CollidesWith(Graphic& o);

   // accessors:
   double            Scale()           const { return scale;      }
   double            MountainScale()   const { return mtnscale;   }
   double            SeaLevel()        const { return base;       }
   double            MinHeight()       const { return min_height; }
   double            MaxHeight()       const { return max_height; }
   bool              IsWater()         const { return water != 0; }

   void              UpdateSurfaceWaves(Vec3& eyePos);

   void              SetScales(double scale, double mtnscale, double base);
   void              SetDetailLevel(int nd);

   virtual int       CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
                                          bool treat_translucent_polys_as_solid=true);

   double            Height(double x, double y) const;
   DWORD             BlendValue(double y);
   int               CalcLayer(Poly* p);
   void              Illuminate(Color ambient, List<Light>& lights);

protected:
   virtual bool      BuildDetailLevel(int level);

   enum { MAX_LOD=8 };

   Terrain*          terrain;
   int               patch_size;
   int               ndetail;
   int               max_detail;
   int               terrain_width;

   Rect              rect;
   float*            heights;
   Model*            detail_levels[MAX_LOD];
   List<Material>    materials;
   Water*            water;

   double            scale;
   double            mtnscale;
   double            base;
   double            size;
   float             min_height;
   float             max_height;
};

#endif TerrainPatch_h

