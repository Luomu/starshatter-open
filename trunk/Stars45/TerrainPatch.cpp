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
    FILE:         TerrainPatch.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "Terrain.h"
#include "TerrainLayer.h"
#include "TerrainPatch.h"
#include "TerrainRegion.h"
#include "Sim.h"

#include "Light.h"
#include "CameraView.h"
#include "Projector.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"
#include "Scene.h"
#include "Water.h"

// +====================================================================+

// #define DEBUG_DETAIL 1

// +====================================================================+

const int MAX_DETAIL       =  4;
const int PATCH_SIZE       = 17;
const int HALF_PATCH_SIZE  =  8;
const int MAX_VERTS        = PATCH_SIZE * PATCH_SIZE;

static bool illuminating   = false;

// +--------------------------------------------------------------------+

TerrainPatch::TerrainPatch(Terrain* terr,const Bitmap* patch, const Rect& r,
const Point&  p1,    const Point& p2)
: ndetail(0), terrain(terr), rect(r), water(0), min_height(1e9), max_height(-1e9)
{
    luminous   = true;   // we will do our own lighting
    own_model  = false;  // manage the model lifetimes in this derived class

    max_detail = Terrain::DetailLevel();
    scale      = fabs(p1.x - p2.x) / (PATCH_SIZE-1);
    mtnscale   = p2.y - p1.y;
    base       = p1.y;
    size       = p2.x - p1.x;

    ZeroMemory(detail_levels, sizeof(detail_levels));

    terrain_width = patch->Width();

    loc   = (p1 + p2) * 0.5;
    loc.y = base;

    radius    = (float) (size * 0.75);
    heights   = new(__FILE__,__LINE__) float[MAX_VERTS];

    float* pHeight = heights;
    int    tscale  = rect.w / (PATCH_SIZE-1);
    int    i, j;

    for (i = 0; i < PATCH_SIZE; i++) {
        int ty = rect.y + i * tscale;

        if (ty < 0)
        ty = 0;

        if (ty > patch->Height()-1)
        ty = patch->Height()-1;

        for (j = 0; j < PATCH_SIZE; j++) {
            int tx = rect.x + (PATCH_SIZE-1 - j) * tscale;

            if (tx < 0)
            tx = 0;

            if (tx > patch->Width()-1)
            tx = patch->Width()-1;

            int   red = patch->GetColor(tx,ty).Red();
            float alt = (float) (red * mtnscale);

            if (alt < min_height)
            min_height = alt;
            if (alt > max_height)
            max_height = alt;

            if (terrain->WaterTexture() && red < 2)
            alt = -5000.0f;

            *pHeight++ = alt;
        }
    }

    Material* mtl = new(__FILE__,__LINE__) Material;
    mtl->Ka = ColorValue(0.5f, 0.5f, 0.5f);
    mtl->Kd = ColorValue(0.3f, 0.6f, 0.2f);
    mtl->Ks = Color::Black;

    mtl->tex_diffuse = terrain->Texture();

    materials.append(mtl);

    List<TerrainLayer>& layers = terrain->GetLayers();
    for (i = 0; i < layers.size(); i++) {
        Bitmap* tex0 = layers.at(i)->GetTileTexture();
        Bitmap* tex1 = 0;
        Bitmap* texd = layers.at(i)->GetDetailTexture();

        if (i < layers.size()-1)
        tex1 = layers.at(i+1)->GetTileTexture();

        if (!texd)
        texd = terrain->DetailTexture(0);

        mtl = new(__FILE__,__LINE__) Material;
        mtl->Ka = ColorValue(0.5f, 0.5f, 0.5f);
        mtl->Kd = ColorValue(0.3f, 0.6f, 0.2f);
        mtl->Ks = Color::Black;

        if ((i & 1) != 0) {
            mtl->tex_diffuse     = tex1;
            mtl->tex_alternate   = tex0;
        }
        else {
            mtl->tex_diffuse     = tex0;
            mtl->tex_alternate   = tex1;
        }

        mtl->tex_detail         = texd;

        materials.append(mtl);
    }

    for (i = 0; i <= max_detail; i++)
    BuildDetailLevel(i);

    model = detail_levels[1];
}

// +--------------------------------------------------------------------+

TerrainPatch::TerrainPatch(Terrain*       terr,
const Rect&    r, 
const Point&   p1,
const Point&   p2,
double         sea_level)
: ndetail(0), terrain(terr), rect(r), water(0)
{
    luminous   = true;   // water is lit by the graphics engine
    own_model  = false;  // manage the model lifetimes in this derived class

    max_detail = Terrain::DetailLevel();
    scale      = fabs(p1.x - p2.x) / (PATCH_SIZE-1);
    mtnscale   = 0;
    base       = sea_level;
    size       = p2.x - p1.x;

    ZeroMemory(detail_levels, sizeof(detail_levels));

    terrain_width = 0;

    loc   = (p1 + p2) * 0.5;
    loc.y = base;

    radius    = (float) (size * 0.75);
    heights   = new(__FILE__,__LINE__) float[MAX_VERTS];

    float* pHeight = heights;
    int    i, j;

    for (i = 0; i < PATCH_SIZE; i++) {
        for (j = 0; j < PATCH_SIZE; j++) {
            *pHeight++ = (float) sea_level;
        }
    }

    Material* mtl = new(__FILE__,__LINE__) Material;
    mtl->Ka = Color::Gray;
    mtl->Kd = Color::White;
    mtl->Ks = Color::White;
    mtl->power       = 30.0f;
    mtl->shadow      = false;
    mtl->tex_diffuse = terrain->WaterTexture();
    //strcpy_s(mtl->shader, "WaterReflections");
    materials.append(mtl);

    water = terrain->GetWater(1);

    for (i = 0; i <= max_detail; i++)
    BuildDetailLevel(i);

    model = detail_levels[1];
}

// +--------------------------------------------------------------------+

TerrainPatch::~TerrainPatch()
{
    delete [] heights;

    for (int i = 0; i < MAX_LOD; i++) {
        Model* m = detail_levels[i];

        if (m) {
            m->GetMaterials().clear();
            delete m;
        }
    }

    materials.destroy();
}

// +--------------------------------------------------------------------+

void
TerrainPatch::SetScales(double s, double m, double b)
{
    scale    = s;
    mtnscale = m;
    base     = b;
}

// +--------------------------------------------------------------------+

static int mcomp(const void* a, const void* b)
{
    Poly* pa = (Poly*) a;
    Poly* pb = (Poly*) b;

    if (pa->sortval == pb->sortval)
    return 0;

    if (pa->sortval < pb->sortval)
    return 1;

    return -1;
}


static void bisect(VertexSet* vset, int v[4])
{
    double d1 = fabs(vset->loc[ v[0] ].y -
    vset->loc[ v[3] ].y);

    double d2 = fabs(vset->loc[ v[1] ].y -
    vset->loc[ v[2] ].y);

    if (d2 < 0.7*d1) {
        int odd[4] = { v[1], v[3], v[0], v[2] };
        for (int i = 0; i < 4; i++)
        v[i] = odd[i];
    }
}

bool
TerrainPatch::BuildDetailLevel(int level)
{
    int i, j;

    int detail_size = 1 << level;
    int ds1 = detail_size+1;

    if (detail_size > PATCH_SIZE)
    return false;

    Model* model   = new(__FILE__,__LINE__) Model;
    detail_levels[level] = model;

    model->SetLuminous(luminous);
    model->SetDynamic(true);

    const int   NUM_STRIPS        = 4;
    const int   NUM_INDICES_TRI   = 3;
    const int   NUM_INDICES_QUAD  = 6;

    int nverts     = ds1*ds1 + ds1*2*NUM_STRIPS;
    int npolys     = detail_size*detail_size*2;
    int strip_len  = detail_size;
    int total      = npolys + strip_len*NUM_STRIPS;

    if (water) {
        nverts      = ds1*ds1;
        strip_len   = 0;
        total       = npolys;
    }

    Surface*    s     = new(__FILE__,__LINE__) Surface;
    VertexSet*  vset  = 0;

    if (s) {
        s->SetName("default");
        s->CreateVerts(nverts);
        s->CreatePolys(total);
        s->AddIndices(npolys*NUM_INDICES_TRI + strip_len*NUM_STRIPS*NUM_INDICES_QUAD);

        vset = s->GetVertexSet();
        if (!water)
        vset->CreateAdditionalTexCoords();

        ZeroMemory(vset->loc,      nverts * sizeof(Vec3));
        ZeroMemory(vset->diffuse,  nverts * sizeof(DWORD));
        ZeroMemory(vset->specular, nverts * sizeof(DWORD));
        ZeroMemory(vset->tu,       nverts * sizeof(float));
        ZeroMemory(vset->tv,       nverts * sizeof(float));
        if (!water) {
            ZeroMemory(vset->tu1,      nverts * sizeof(float));
            ZeroMemory(vset->tv1,      nverts * sizeof(float));
        }
        ZeroMemory(vset->rw,       nverts * sizeof(float));

        // initialize vertices
        Vec3*  pVert   = vset->loc;
        float* pTu     = vset->tu;
        float* pTv     = vset->tv;
        float* pTu1    = vset->tu1;
        float* pTv1    = vset->tv1;
        DWORD* pSpec   = vset->specular;

        int    dscale  = (PATCH_SIZE-1)/detail_size;
        double dt      = 0.0625 / (ds1-1); // terrain texture scale
        double dtt     = 2.0000 / (ds1-1); // tile texture scale
        double tu0     = (double) rect.x / rect.w / 16.0 + 1.0/16.0;
        double tv0     = (double) rect.y / rect.h / 16.0;

        // surface verts
        for (i = 0; i < ds1; i++) {
            for (j = 0; j < ds1; j++) {
                *pVert   = Vec3((float) (j* scale * dscale - (HALF_PATCH_SIZE*scale)),
                (float) (heights[i*dscale*PATCH_SIZE + j*dscale]),
                (float) (i* scale * dscale - (HALF_PATCH_SIZE*scale)));

                if (level >= 2) {
                    *pTu++   = (float) (-j*dtt);
                    *pTv++   = (float) ( i*dtt);

                    if (level >= 4 && !water) {
                        *pTu1++  = (float) (-j*dtt*3);
                        *pTv1++  = (float) ( i*dtt*3);
                    }

                    *pSpec++ = BlendValue(pVert->y);
                }

                else {
                    *pTu++   = (float) (tu0 - j*dt);
                    *pTv++   = (float) (tv0 + i*dt);
                }

                pVert++;
            }
        }

        if (!water) {
            // strip 1 & 2 verts
            for (i = 0; i < ds1; i += detail_size) {
                for (j = 0; j < ds1; j++) {
                    Vec3 vl  = Vec3((float) (j* scale * dscale - (HALF_PATCH_SIZE*scale)),
                    (float) (heights[i*dscale*PATCH_SIZE + j*dscale]),
                    (float) (i* scale * dscale - (HALF_PATCH_SIZE*scale)));

                    *pVert++ = vl;

                    DWORD blend = 0;

                    if (level >= 2) {
                        blend = BlendValue(vl.y);

                        *pSpec++ = blend;
                        *pTu++   = (float) (-j*dtt);
                        *pTv++   = (float) ( i*dtt);
                    }

                    else {
                        *pTu++   = (float) (tu0 - j*dt);
                        *pTv++   = (float) (tv0 + i*dt);
                    }

                    vl.y     = -5000.0f;

                    *pVert++ = vl;

                    if (level >= 2) {
                        *pSpec++ = blend;
                        *pTu++   = (float) (-j*dtt);
                        *pTv++   = (float) ( i*dtt);
                    }

                    else {
                        *pTu++   = (float) (tu0 - j*dt);
                        *pTv++   = (float) (tv0 + i*dt);
                    }
                }
            }

            // strip 3 & 4 verts
            for (j = 0; j < ds1; j += detail_size) {
                for (i = 0; i < ds1; i++) {
                    Vec3 vl  = Vec3((float) (j* scale * dscale - (HALF_PATCH_SIZE*scale)),
                    (float) (heights[i*dscale*PATCH_SIZE + j*dscale]),
                    (float) (i* scale * dscale - (HALF_PATCH_SIZE*scale)));

                    *pVert++ = vl;

                    DWORD blend = 0;

                    if (level >= 2) {
                        blend = BlendValue(vl.y);

                        *pSpec++ = blend;
                        *pTu++   = (float) (-j*dtt);
                        *pTv++   = (float) ( i*dtt);
                    }

                    else {
                        *pTu++   = (float) (tu0 - j*dt);
                        *pTv++   = (float) (tv0 + i*dt);
                    }

                    vl.y     = -5000.0f;

                    *pVert++ = vl;

                    if (level >= 2) {
                        *pSpec++ = blend;
                        *pTu++   = (float) (-j*dtt);
                        *pTv++   = (float) ( i*dtt);
                    }

                    else {
                        *pTu++   = (float) (tu0 - j*dt);
                        *pTv++   = (float) (tv0 + i*dt);
                    }
                }
            }
        }

        Material* m = materials.first();

        // initialize the polys
        for (i = 0; i < npolys; i++) {
            Poly* p        = s->GetPolys() + i;
            p->nverts      = 3;
            p->vertex_set  = vset;
            p->visible     = 1;
            p->sortval     = 0;
            p->material    = m;

            if (level >= 2 && !water) {
                p->material = materials.at(1);
                p->sortval  = 1;
            }
        }

        for (i = npolys; i < total; i++) {
            Poly* p        = s->GetPolys() + i;
            p->nverts      = 4;
            p->vertex_set  = vset;
            p->visible     = 1;
            p->sortval     = 0;
            p->material    = m;
        }

        int index = 0;

        // build main patch polys:
        for (i = 0; i < detail_size; i++) {
            for (j = 0; j < detail_size; j++) {
                int v[4] = {
                    (ds1 * (i  ) + (j  )),
                    (ds1 * (i  ) + (j+1)),
                    (ds1 * (i+1) + (j  )),
                    (ds1 * (i+1) + (j+1)) };

                bisect(vset, v);

                // first triangle
                Poly* p = s->GetPolys() + index++;
                p->verts[0]   = v[0];
                p->verts[1]   = v[1];
                p->verts[2]   = v[3];

                if (level >= 2 && !water) {
                    int layer = CalcLayer(p) + 1;
                    p->material = materials.at(layer);
                    p->sortval  = layer;
                }

                // second triangle
                p = s->GetPolys() + index++;
                p->verts[0]   = v[0];
                p->verts[1]   = v[3];
                p->verts[2]   = v[2];

                if (level >= 2 && !water) {
                    int layer = CalcLayer(p) + 1;
                    p->material = materials.at(layer);
                    p->sortval  = layer;
                }
            }
        }

        // build vertical edge strip polys:

        if (!water) {
            for (i = 0; i < NUM_STRIPS; i++) {
                Poly* p = s->GetPolys() + npolys + i*strip_len;
                int   base_index = ds1*ds1 + ds1*2*i;

                for (j = 0; j < strip_len; j++) {
                    int v       = base_index + j * 2;
                    p->nverts   = 4;

                    if (i == 1 || i == 2) {
                        p->verts[0] = v;
                        p->verts[1] = v+2;
                        p->verts[2] = v+3;
                        p->verts[3] = v+1;
                    }

                    else {
                        p->verts[0] = v;
                        p->verts[1] = v+1;
                        p->verts[2] = v+3;
                        p->verts[3] = v+2;
                    }

                    if (level >= 2) {
                        int layer = CalcLayer(p) + 1;
                        p->material = materials.at(layer);
                        p->sortval  = layer;
                    }

                    p++;
                }
            }
        }

        // update the poly planes:
        for (i = 0; i < total; i++) {
            Poly*   p      = s->GetPolys() + i;
            Plane&  plane  = p->plane;
            WORD*   v      = p->verts;

            plane = Plane(vset->loc[v[0]] + loc,
            vset->loc[v[1]] + loc,
            vset->loc[v[2]] + loc);
        }

        s->Normalize();

        // create continguous segments for each material:
        // sort the polys by material index:
        qsort((void*) s->GetPolys(), s->NumPolys(), sizeof(Poly), mcomp);

        // then assign them to cohesive segments:
        Segment* segment = 0;
        Poly*    spolys  = s->GetPolys();

        for (int n = 0; n < s->NumPolys(); n++) {
            if (segment && segment->material == spolys[n].material) {
                segment->npolys++;
            }
            else {
                segment = 0;
            }

            if (!segment) {
                segment = new(__FILE__,__LINE__) Segment;

                segment->npolys   = 1;
                segment->polys    = &spolys[n];
                segment->material = segment->polys->material;
                segment->model    = model;

                s->GetSegments().append(segment);
            }
        }

        Solid::EnableCollision(false);
        model->AddSurface(s);
        Solid::EnableCollision(true);

        // copy vertex normals:
        const Vec3B* tnorms = terrain->Normals();

        for (i = 0; i < ds1; i++) {
            for (j = 0; j < ds1; j++) {

                if (water) {
                    vset->nrm[i*ds1+j] = Point(0,1,0);
                }

                // blend adjacent normals:
                else if (dscale > 1) {
                    Point normal;

                    // but don't blend more than 16 normals per vertex:
                    int step = 1;
                    if (dscale > 4)
                    step = dscale / 4;

                    for (int dy = -dscale/2; dy < dscale/2; dy += step) {
                        for (int dx = -dscale/2; dx < dscale/2; dx += step) {
                            int ix = rect.x + (ds1-1-j)*dscale + dx;
                            int iy = rect.y + i*dscale + dy;

                            if (ix < 0)                ix = 0;
                            if (ix > terrain_width-1)  ix = terrain_width-1;
                            if (iy < 0)                iy = 0;
                            if (iy > terrain_width-1)  iy = terrain_width-1;

                            Vec3B vbn = tnorms[iy*terrain_width + ix];
                            normal += Point((128-vbn.x)/127.0, (vbn.z-128)/127.0, (vbn.y-128)/127.0);
                        }
                    }

                    normal.Normalize();
                    vset->nrm[i*ds1+j] = normal;
                }

                // just copy the one normal:
                else {
                    Vec3B vbn    = tnorms[(rect.y + i*dscale)*terrain_width + (rect.x + (ds1-1-j) * dscale)];
                    Point normal = Point((128-vbn.x)/127.0, (vbn.z-128)/127.0, (vbn.y-128)/127.0);
                    vset->nrm[i*ds1+j] = normal;
                }
            }
        }

        if (!water) {
            pVert = &vset->nrm[ds1*ds1];

            // strip 1 & 2 verts
            for (i = 0; i < ds1; i += detail_size) {
                for (j = 0; j < ds1; j++) {
                    Vec3 vn  = vset->nrm[i*ds1 + j];

                    *pVert++ = vn;
                    *pVert++ = vn;
                }
            }

            // strip 3 & 4 verts
            for (j = 0; j < ds1; j += detail_size) {
                for (i = 0; i < ds1; i++) {
                    Vec3 vn  = vset->nrm[i*ds1 + j];

                    *pVert++ = vn;
                    *pVert++ = vn;
                }
            }
        }
    }

    if (level > max_detail)
    max_detail = level;

    return true;
}

// +--------------------------------------------------------------------+

DWORD
TerrainPatch::BlendValue(double y)
{
    if (terrain && y >= 0 && !water) {
        // find the proper layer:
        for (int i = 0; i < terrain->GetLayers().size(); i++) {
            TerrainLayer* layer = terrain->GetLayers().at(i);

            if (y >= layer->GetMinHeight() && y < layer->GetMaxHeight()) {
                double scale = (y-layer->GetMinHeight()) / (layer->GetMaxHeight()-layer->GetMinHeight());

                if (scale < 0.2)
                scale = 0;
                else if (scale > 0.8)
                scale = 1;
                else
                scale = (scale - 0.2) / 0.6;

                if ((i & 1) == 0) {
                    scale = 1 - scale;
                }

                DWORD val = (DWORD) (scale*255);

                return val << 24;
            }
        }
    }

    return 0;
}

int
TerrainPatch::CalcLayer(Poly* poly)
{
    if (terrain && poly) {
        if (water)
        return 0;

        double y = 1e6;

        for (int i = 0; i < poly->nverts; i++) {
            double h = poly->vertex_set->loc[ poly->verts[i] ].y;

            if (h >= 0 && h < y) {
                y = h;
            }
        }

        if (y <= terrain->GetLayers().first()->GetMinHeight())
        return 0;

        for (int i = 0; i < terrain->GetLayers().size(); i++) {
            TerrainLayer* layer = terrain->GetLayers().at(i);

            if (y >= layer->GetMinHeight() && y < layer->GetMaxHeight()) {
                return i;
            }
        }
    }

    return -1;
}

// +--------------------------------------------------------------------+

void
TerrainPatch::UpdateSurfaceWaves(Vec3& eyePos)
{
    if (water && model && model->NumVerts() > 1) {
        Surface* s = model->GetSurfaces().first();
        if (s) {
            VertexSet* vset = s->GetVertexSet();
            for (int i = 0; i < vset->nverts; i++)
            vset->loc[i].y = 0.0f;

            water->UpdateSurface(eyePos, vset);
        }
    }
}

// +--------------------------------------------------------------------+

int
TerrainPatch::CollidesWith(Graphic& o)
{
    return 0;
}

// +--------------------------------------------------------------------+

void
TerrainPatch::SelectDetail(Projector* projector)
{
    // This is where all the work is done,
    // Delegate to the overall terrain to
    // compute a detail level for every patch:

    if (terrain) {
        terrain->SelectDetail(projector);
    }

    // Build detail levels on demand:

    if (detail_levels[ndetail] == 0)
    BuildDetailLevel(ndetail);
}

void
TerrainPatch::SetDetailLevel(int nd)
{
    if (nd >= 0 && nd <= max_detail) {
        if (ndetail != nd)
        DeletePrivateData();

        ndetail = nd;

        // build detail levels on demand:
        if (detail_levels[ndetail] == 0)
        BuildDetailLevel(ndetail);

        model = detail_levels[ndetail];

        if (water)
        water = terrain->GetWater(ndetail);
    }
}

// +--------------------------------------------------------------------+

void
TerrainPatch::Illuminate(Color ambient, List<Light>& lights)
{
    if (!model || model->NumVerts() < 1) return;
    Surface* s = model->GetSurfaces().first();
    if (!s) return;

    illuminating = true;

    // clear the solid lights to ambient:
    VertexSet*  vset   = s->GetVertexSet();
    int         nverts = vset->nverts;
    DWORD       aval   = ambient.Value();

    for (int i = 0; i < nverts; i++) {
        vset->diffuse[i]  = aval;
    }

    TerrainRegion* trgn     = terrain->GetRegion();
    bool           eclipsed = false;
    bool           first    = terrain->IsFirstPatch(this);

    if (trgn && !first) {
        eclipsed = trgn->IsEclipsed();
    }

    // for sun and back lights:
    ListIter<Light> iter = lights;
    while (++iter) {
        Light* light = iter.value();

        if (!light->IsDirectional())                                // only do sun and
        continue;                                                // back lights

        if (light->CastsShadow() && first) {
            eclipsed  = light->Location().y < -100 ||                // has sun set, or

            scene->IsLightObscured(vset->loc[0],         // is sun in eclipse
            light->Location(),    // by orbital body
            radius);              // such as a moon?
        }

        if (!light->CastsShadow() || !eclipsed) {
            Vec3 vl = light->Location();
            vl.Normalize();

            for (int i = 0; i < nverts; i++) {
                Vec3&  nrm = vset->nrm[i];
                double val = 0;
                double gain = vl * nrm;

                if (gain > 0) {
                    val = light->Intensity() * (0.85 * gain);

                    if (val > 1)
                    val = 1;
                }

                if (val > 0.01)
                vset->diffuse[i] = ((light->GetColor().dim(val)) + vset->diffuse[i]).Value();
            }
        }
    }

    // combine blend weights:
    if (ndetail >= 2) {
        for (int i = 0; i < nverts; i++) {
            vset->diffuse[i] = vset->specular[i] | (vset->diffuse[i] & 0x00ffffff);
        }
    }

    if (trgn && first) {
        trgn->SetEclipsed(eclipsed);
    }

    InvalidateSurfaceData();
    illuminating = false;
}

// +--------------------------------------------------------------------+

void
TerrainPatch::Render(Video* video, DWORD flags)
{
    if (max_height < 0)
    return;

    if (flags & RENDER_ADDITIVE)
    return;

    if (!model)
    model = detail_levels[0];

    if (scene) {
        /***
    *** TWO PASS LIGHTING FOR TERRAIN SHADOWS DOESN'T
    *** WORK - IT MESSES UP THE HARDWARE FOG CALCS
    ***
    *** PLUS, IT'S INCREDIBLY SLOW!!!
    ***/

        if ((flags & RENDER_ADD_LIGHT) != 0)
        return;

        if (water) {
            UpdateSurfaceWaves(Vec3(0,0,0));
            Illuminate(scene->Ambient(), scene->Lights());
        }
        else {
            Illuminate(scene->Ambient(), scene->Lights());
        }
    }
    else {
        if ((flags & RENDER_ADD_LIGHT) != 0)
        return;
    }

    Bitmap* details[16];
    ZeroMemory(details, sizeof(details));

    if (ndetail < 3) {
        for (int i = 0; i < 16 && i < materials.size(); i++) {
            Material* mtl = materials[i];

            if (mtl->tex_detail) {
                details[i] = mtl->tex_detail;
                mtl->tex_detail = 0;
            }
        }
    }

    double   visibility  = terrain->GetRegion()->GetWeather().Visibility();
    FLOAT    fog_density = (FLOAT) (terrain->GetRegion()->FogDensity() * 2.5e-5 * 1/visibility);

    video->SetRenderState(Video::LIGHTING_ENABLE,   false);
    video->SetRenderState(Video::SPECULAR_ENABLE,   false); //water != 0);
    video->SetRenderState(Video::FOG_ENABLE,        true);
    video->SetRenderState(Video::FOG_COLOR,         terrain->GetRegion()->FogColor().Value());
    video->SetRenderState(Video::FOG_DENSITY,       *((DWORD*) &fog_density));

    // Too bad this doesn't work right.  But it makes the
    // ground mostly disappear. :-(
    //
    //video->SetRenderState(Video::Z_BIAS,            -2);

    Solid::Render(video, flags);

    video->SetRenderState(Video::LIGHTING_ENABLE,   true);
    video->SetRenderState(Video::SPECULAR_ENABLE,   true);
    video->SetRenderState(Video::FOG_ENABLE,        false);
    //video->SetRenderState(Video::Z_BIAS,            0);

    if (ndetail < 3) {
        for (int i = 0; i < 16 && i < materials.size(); i++) {
            Material* mtl = materials[i];

            if (details[i] && !mtl->tex_detail) {
                mtl->tex_detail = details[i];
            }
        }
    }
}

// +--------------------------------------------------------------------+

int
TerrainPatch::CheckRayIntersection(Point obj_pos, Point dir, double len, Point& ipt, bool ttpas)
{
    Point  light_pos = obj_pos + dir * len;
    int    impact    = light_pos.y < -100;

    // Special case for illumination -
    // just check if sun is above or below the horizon:

    if (illuminating || impact) {
        return impact;
    }

    if (obj_pos.x != 0 || obj_pos.y != 0 || obj_pos.z != 0) {
        return impact;
    }

    // the rest of this code is only used for eclipsing
    // the solar lens flare:

    // check right angle spherical distance:
    Point  d0 = loc;
    Point  d1 = d0.cross(dir);
    double dlen = d1.length();          // distance of point from line

    if (dlen > radius)                  // clean miss
    return 0;                        // (no impact)

    // make sure some part of this patch falls between
    // the camera and the sun:

    Point closest = loc + dir * radius;

    if (closest * dir < 0)
    return 0;

    // probable hit at this point...
    // test for polygon intersection:
    if (!model)
    return 0;

    Surface* s = model->GetSurfaces().first();

    if (!s)
    return 0;


    // transform ray into object space:
    Matrix xform(Orientation());

    Vec3 tmp = dir;

    dir.x = tmp * Vec3(xform(0,0), xform(0,1), xform(0,2));
    dir.y = tmp * Vec3(xform(1,0), xform(1,1), xform(1,2));
    dir.z = tmp * Vec3(xform(2,0), xform(2,1), xform(2,2));

    tmp = obj_pos-loc;

    obj_pos.x = tmp * Vec3(xform(0,0), xform(0,1), xform(0,2));
    obj_pos.y = tmp * Vec3(xform(1,0), xform(1,1), xform(1,2));
    obj_pos.z = tmp * Vec3(xform(2,0), xform(2,1), xform(2,2));

    double min = 2 * len;

    // check each polygon:
    Poly*    p = s->GetPolys();

    for (int i = 0; i < s->NumPolys(); i++) {
        Point  v = p->plane.normal;
        double d = p->plane.distance;

        double denom = dir*v;

        if (denom < -1.0e-5) {
            Point  P    = v * d;
            double ilen = ((P-obj_pos)*v)/denom;

            if (ilen > 0 && ilen < min) {
                Point intersect = obj_pos + dir * ilen;

                if (p->Contains(intersect)) {
                    ipt = intersect;
                    min = ilen;
                    impact = 1;
                }
            }
        }

        p++;
    }

    // xform impact point back into world coordinates:

    if (impact) {
        ipt = (ipt * Orientation()) + loc;
    }

    return impact;
}

// +--------------------------------------------------------------------+

double
TerrainPatch::Height(double x, double z) const
{
    if (water) return base;

    double   height = 0;

    x /= scale;
    z /= scale;

    int x0 = (int) x;
    int z0 = (int) z;

    if (x0 >= 0 && x0 < PATCH_SIZE && z0 >= 0 && z0 < PATCH_SIZE) {
        double dx = x-x0;
        double dz = z-z0;

        double h[4];

        h[0] = heights[(z0*PATCH_SIZE     + x0)];
        h[1] = heights[((z0+1)*PATCH_SIZE + x0)];
        h[2] = heights[(z0*PATCH_SIZE     + x0+1)];
        h[3] = heights[((z0+1)*PATCH_SIZE + x0+1)];

        // if level, just return height of one vertex:
        if (h[0] == h[1] && h[1] == h[2] && h[2] == h[3]) {
            height = h[0];
        }

        // if sloped, interpolate between vertex heights:
        else {
            double hl = h[0]*(1-dz) + h[1]*dz;
            double hr = h[2]*(1-dz) + h[3]*dz;

            height = hl*(1-dx) + hr*dx + 5; // fudge
        }
    }

    if (height < 0)
    height = 0;

    return height;
}
