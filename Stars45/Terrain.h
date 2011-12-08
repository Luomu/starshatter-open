/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Terrain.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Test pseudo-random terrain heightfield, based on Solid
*/

#ifndef Terrain_h
#define Terrain_h

#include "Types.h"
#include "Graphic.h"
#include "Geometry.h"
#include "Bitmap.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Projector;
class Scene;
class TerrainApron;
class TerrainClouds;
class TerrainLayer;
class TerrainPatch;
class TerrainRegion;
class Water;

// +--------------------------------------------------------------------+

struct Vec3B
{
   Vec3B()                                          { }
   Vec3B(BYTE a, BYTE b, BYTE c) : x(a), y(b), z(c) { }

   BYTE x, y, z;
};

// +--------------------------------------------------------------------+

class Terrain
{
public:
   Terrain(TerrainRegion* region);
   virtual ~Terrain();

   virtual void      Activate(Scene& scene);
   virtual void      Deactivate(Scene& scene);

   virtual void      SelectDetail(Projector* proj);
   virtual void      BuildTerrain();
   virtual void      BuildNormals();

   virtual void      ExecFrame(double seconds);

   double            Height(double x, double y) const;

   const Vec3B*      Normals()      const { return terrain_normals;  }
   TerrainRegion*    GetRegion()          { return region;           }
   double            FogFade()      const { return fog_fade;         }

   Bitmap*           Texture()            { return terrain_texture;  }
   Bitmap*           ApronTexture()       { return apron_texture;    }
   Bitmap*           WaterTexture()       { return water_texture;    }
   Bitmap**          EnvironmentTexture() { return env_texture;      }
   Bitmap*           TileTexture(int n)   { return tiles[n];         }
   Bitmap*           CloudTexture(int n)  { return cloud_texture[n]; }
   Bitmap*           ShadeTexture(int n)  { return shade_texture[n]; }
   Bitmap*           DetailTexture(int n) { return noise_texture[n]; }
   Water*            GetWater(int level)  { return water[level];     }
   List<TerrainLayer>& GetLayers()        { return layers;           }

   bool              IsFirstPatch(TerrainPatch* p) const;

   static int        DetailLevel()        { return detail_level;     }
   static void       SetDetailLevel(int detail);

protected:
   TerrainRegion*    region;
   TerrainPatch**    patches;
   TerrainPatch**    water_patches;
   Water**           water;
   TerrainApron**    aprons;
   TerrainClouds**   clouds;
   int               nclouds;

   Bitmap            terrain_patch;
   Bitmap            terrain_apron;
   Bitmap*           terrain_texture;
   Bitmap*           apron_texture;
   Bitmap*           water_texture;
   Bitmap*           env_texture[6];
   Bitmap*           tiles[256];
   Bitmap*           cloud_texture[2];
   Bitmap*           shade_texture[2];
   Bitmap*           noise_texture[2];

   Vec3B*            terrain_normals;
   List<TerrainLayer> layers;

   Text              datapath;
   double            scale;
   double            mtnscale;
   int               subdivisions;
   int               patch_size;
   DWORD             detail_frame;
   double            fog_fade;

   static int        detail_level;
};

#endif Terrain_h

