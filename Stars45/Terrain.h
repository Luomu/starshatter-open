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

