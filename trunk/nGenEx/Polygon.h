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

    SUBSYSTEM:    nGenEx.lib
    FILE:         Polygon.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Polygon structures: VertexSet, Poly, Material
*/

#ifndef Polygon_h
#define Polygon_h

#include "Geometry.h"
#include "Color.h"

// +--------------------------------------------------------------------+

class  Bitmap;
struct Poly;
struct Material;
struct VertexSet;

// +--------------------------------------------------------------------+

struct Poly
{
    static const char* TYPENAME() { return "Poly"; }

    enum { MAX_VERTS = 4 };

    Poly()  { }
    Poly(int init);
    ~Poly() { }

    int operator <  (const Poly& p) const { return sortval < p.sortval;  }
    int operator == (const Poly& p) const { return this == &p;           }

    int Contains(const Vec3& pt) const;

    BYTE           nverts;
    BYTE           visible;
    WORD           verts[MAX_VERTS];
    WORD           vlocs[MAX_VERTS];
    VertexSet*     vertex_set;
    Material*      material;
    int            sortval;
    float          flatness;
    Plane          plane;
};

// +--------------------------------------------------------------------+

struct Material
{
    static const char* TYPENAME() { return "Material"; }

    enum BLEND_TYPE { MTL_SOLID=1, MTL_TRANSLUCENT=2, MTL_ADDITIVE=4 };
    enum            { NAMELEN=32 };

    Material();
    ~Material();

    int operator == (const Material& m) const;

    void Clear();

    char        name[NAMELEN];
    char        shader[NAMELEN];

    ColorValue  Ka;         // ambient color
    ColorValue  Kd;         // diffuse color
    ColorValue  Ks;         // specular color
    ColorValue  Ke;         // emissive color
    float       power;      // highlight sharpness (big=shiny)
    float       brilliance; // diffuse power function
    float       bump;       // bump level (0=none)
    DWORD       blend;      // alpha blend type
    bool        shadow;     // casts shadow
    bool        luminous;   // verts have their own lighting

    Bitmap*     tex_diffuse;
    Bitmap*     tex_specular;
    Bitmap*     tex_bumpmap;
    Bitmap*     tex_emissive;
    Bitmap*     tex_alternate;
    Bitmap*     tex_detail;

    bool        IsSolid()         const { return blend == MTL_SOLID;       }
    bool        IsTranslucent()   const { return blend == MTL_TRANSLUCENT; }
    bool        IsGlowing()       const { return blend == MTL_ADDITIVE;    }
    const char* GetShader(int n)  const;

    //
    // Support for Magic GUI
    //

    Color       ambient_color;
    Color       diffuse_color;
    Color       specular_color;
    Color       emissive_color;

    float       ambient_value;
    float       diffuse_value;
    float       specular_value;
    float       emissive_value;

    Bitmap*     thumbnail;        // preview image

    void        CreateThumbnail(int size=128);
    DWORD       GetThumbColor(int i, int j, int size);
};

// +--------------------------------------------------------------------+

struct VertexSet
{
    static const char* TYPENAME() { return "VertexSet"; }

    enum VertexSpaces { OBJECT_SPACE, WORLD_SPACE, VIEW_SPACE, SCREEN_SPACE };

    VertexSet(int m);
    ~VertexSet();

    void     Resize(int m, bool preserve=false);
    void     Delete();
    void     Clear();
    void     CreateTangents();
    void     CreateAdditionalTexCoords();
    bool     CopyVertex(int dst, int src);
    void     CalcExtents(Point& plus, Point& minus);

    VertexSet*  Clone() const;

    int      nverts;
    int      space;

    Vec3*    loc;
    Vec3*    nrm;
    Vec3*    s_loc;
    float*   rw;
    float*   tu;
    float*   tv;
    float*   tu1;
    float*   tv1;
    DWORD*   diffuse;
    DWORD*   specular;
    Vec3*    tangent;
    Vec3*    binormal;
};

// +--------------------------------------------------------------------+

#endif Polygon_h

