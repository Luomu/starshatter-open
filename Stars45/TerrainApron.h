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
    FILE:         TerrainApron.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A Single Edge Section of a Terrain Object
*/

#ifndef TerrainApron_h
#define TerrainApron_h

#include "Types.h"
#include "Solid.h"

// +--------------------------------------------------------------------+

class Terrain;

// +--------------------------------------------------------------------+

class TerrainApron : public Solid
{
public:
    TerrainApron(Terrain* terrain,
    const Bitmap* patch, const Rect& rect,
    const Point&  p1,    const Point& p2);
    virtual ~TerrainApron();

    virtual void      Render(Video* video, DWORD flags);
    virtual void      Update();

    virtual int       CollidesWith(Graphic& o);
    virtual bool      Luminous()    const { return false; }
    virtual bool      Translucent() const { return false; }

    // accessors:
    double            Scale()           const { return scale;    }
    double            MountainScale()   const { return mtnscale; }
    double            SeaLevel()        const { return base;     }
    void              SetScales(double scale, double mtnscale, double base);

    void              Illuminate(Color ambient, List<Light>& lights);
    virtual int       CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
    bool treat_translucent_polys_as_solid=true);

protected:
    virtual bool      BuildApron();

    Terrain*          terrain;
    int               nverts;
    int               npolys;
    int               terrain_width;

    Rect              rect;
    float*            heights;

    double            scale;
    double            mtnscale;
    double            base;
    double            size;
};


#endif TerrainApron_h

