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
    FILE:         Sky.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Celestial sphere, stars, planets, space dust...
*/

#ifndef Sky_h
#define Sky_h

#include "Types.h"
#include "Solid.h"
#include "Bitmap.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class StarSystem;

// +--------------------------------------------------------------------+

class Stars : public Graphic
{
public:
    Stars(int nstars);
    virtual ~Stars();

    virtual void      Illuminate(double scale);
    virtual void      Render(Video* video, DWORD flags);

protected:
    VertexSet*        vset;
    Color*            colors;
};

// +--------------------------------------------------------------------+

class Dust : public Graphic
{
public:
    Dust(int ndust, bool bright=false);
    virtual ~Dust();

    virtual void      Render(Video* video, DWORD flags);
    virtual void      Reset(const Point& ref);
    virtual void      ExecFrame(double factor, const Point& ref);

    virtual void      Hide();
    virtual void      Show();

protected:
    bool              really_hidden;
    bool              bright;
    VertexSet*        vset;
};

// +--------------------------------------------------------------------+

class PlanetRep : public Solid
{
public:
    PlanetRep(const char* img_west, const char* img_glow, 
    double rad, const Vec3& pos, double tscale = 1,
    const char* rngname=0, double minrad = 0, double maxrad = 0,
    Color atmos = Color::Black, const char* img_gloss=0);
    virtual ~PlanetRep();

    virtual Color     Atmosphere()  const    { return atmosphere; }
    virtual void      SetAtmosphere(Color a) { atmosphere = a; }
    virtual void      SetDaytime(bool d);
    virtual void      SetStarSystem(StarSystem* system);

    virtual void      Render(Video* video, DWORD flags);

    virtual int       CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
    bool treat_translucent_polys_as_solid=true);

protected:
    void  CreateSphere(double radius, int nrings, int nsections,
    double minrad, double maxrad, int rsections,
    double tscale);

    Material*         mtl_surf;
    Material*         mtl_limb;
    Material*         mtl_ring;
    int               has_ring;
    int               ring_verts;
    int               ring_polys;
    double            ring_rad;
    double            body_rad;
    Color             atmosphere;
    bool              daytime;

    StarSystem*       star_system;
};

#endif Sky_h

