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
    FILE:         Hoop.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ILS Hoop (HUD display) class
*/

#include "MemDebug.h"
#include "Hoop.h"

#include "Game.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Window.h"

static Color  ils_color;

// +--------------------------------------------------------------------+

Hoop::Hoop()
: width(360), height(180), mtl(0)
{
    foreground  = 1;
    radius      = (float) width;

    DataLoader* loader = DataLoader::GetLoader();

    loader->SetDataPath("HUD/");
    loader->LoadTexture("ILS.pcx", hoop_texture, Bitmap::BMP_TRANSLUCENT);
    loader->SetDataPath(0);

    CreatePolys();
}

Hoop::~Hoop()
{ }

// +--------------------------------------------------------------------+

void Hoop::SetColor(Color c)
{ 
    ils_color = c; 
}

// +--------------------------------------------------------------------+

void
Hoop::CreatePolys()
{
    Material* mtl = new(__FILE__,__LINE__) Material;

    mtl->tex_diffuse   = hoop_texture;
    mtl->blend         = Material::MTL_ADDITIVE;

    int w = width /2;
    int h = height/2;

    model     = new(__FILE__,__LINE__) Model;
    own_model = 1;

    Surface* surface = new(__FILE__,__LINE__) Surface;

    surface->SetName("hoop");
    surface->CreateVerts(4);
    surface->CreatePolys(2);

    VertexSet*  vset  = surface->GetVertexSet();
    Poly*       polys = surface->GetPolys();

    ZeroMemory(polys, sizeof(Poly) * 2);

    for (int i = 0; i < 4; i++) {
        int   x = w;
        int   y = h;
        float u = 0;
        float v = 0;

        if (i == 0 || i == 3)
        x = -x;
        else
        u = 1;

        if (i < 2)
        y = -y;
        else
        v = 1;

        vset->loc[i]   = Vec3(x, y, 0);
        vset->nrm[i]   = Vec3(0, 0, 0);

        vset->tu[i]    = u;
        vset->tv[i]    = v;
    }

    for (int i = 0; i < 2; i++) {
        Poly& poly        = polys[i];

        poly.nverts       = 4;
        poly.vertex_set   = vset;
        poly.material     = mtl;

        poly.verts[0]     = i ? 3 : 0;
        poly.verts[1]     = i ? 2 : 1;
        poly.verts[2]     = i ? 1 : 2;
        poly.verts[3]     = i ? 0 : 3;

        poly.plane        = Plane(vset->loc[poly.verts[0]],
        vset->loc[poly.verts[2]],
        vset->loc[poly.verts[1]]);

        surface->AddIndices(6);
    }

    // then assign them to cohesive segments:
    Segment* segment = new(__FILE__,__LINE__) Segment;
    segment->npolys   = 2;
    segment->polys    = &polys[0];
    segment->material = segment->polys->material;

    surface->GetSegments().append(segment);

    model->AddSurface(surface);


    SetLuminous(true);
}

// +--------------------------------------------------------------------+

void
Hoop::Update()
{
    if (mtl)
    mtl->Ke = ils_color;

    if (model && luminous) {
        ListIter<Surface> s_iter = model->GetSurfaces();
        while (++s_iter) {
            Surface*    surface  = s_iter.value();
            VertexSet*  vset     = surface->GetVertexSet();

            for (int i = 0; i < vset->nverts; i++) {
                vset->diffuse[i]  = ils_color.Value();
            }
        }

        InvalidateSurfaceData();
    }
}
