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
    FILE:         TerrainClouds.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A Single Edge Section of a Terrain Object
*/

#ifndef TerrainClouds_h
#define TerrainClouds_h

#include "Types.h"
#include "Graphic.h"
#include "Geometry.h"
#include "Polygon.h"

// +--------------------------------------------------------------------+

class Terrain;
class TerrainRegion;

// +--------------------------------------------------------------------+

class TerrainClouds : public Graphic
{
public:
    TerrainClouds(Terrain* terr, int type);
    virtual ~TerrainClouds();

    virtual void      Render(Video* video, DWORD flags);
    virtual void      Update();

    // accessors:
    virtual int       CollidesWith(Graphic& o)   { return 0;    }
    virtual bool      Luminous()           const { return true; }
    virtual bool      Translucent()        const { return true; }

    void              Illuminate(Color ambient, List<Light>& lights);

protected:
    void              BuildClouds();

    Terrain*          terrain;
    Vec3*             mverts;
    VertexSet*        verts;
    Poly*             polys;
    Material          mtl_cloud;
    Material          mtl_shade;

    int               type;
    int               nbanks;
    int               nverts;
    int               npolys;
};


#endif TerrainClouds_h

