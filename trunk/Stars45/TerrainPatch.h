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

