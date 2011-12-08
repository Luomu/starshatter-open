/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TerrainApron.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "Terrain.h"
#include "TerrainApron.h"
#include "TerrainRegion.h"

#include "CameraView.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"
#include "Light.h"
#include "Scene.h"

// +====================================================================+

const int PATCH_SIZE       = 17;
const int HALF_PATCH_SIZE  =  8;
const int MAX_VERTS        = PATCH_SIZE * PATCH_SIZE;
const int NUM_INDICES_TRI  = 3;

// +--------------------------------------------------------------------+

TerrainApron::TerrainApron(Terrain* terr,const Bitmap* patch, const Rect& r,
                           const Point&  p1,    const Point& p2)
   : terrain(terr), rect(r)
{
   size       = fabs(p2.x - p1.x);
   scale      = size / (PATCH_SIZE-1);
   mtnscale   = 1.3 * (p2.y - p1.y);
   base       = p1.y;

   terrain_width = patch->Width();

   loc   = (p1 + p2) * 0.5;
   loc.y = base;

   radius    = (float) (size * 0.75);
   heights   = new(__FILE__,__LINE__) float[MAX_VERTS];

   float* pHeight = heights;

   int i, j;

   for (i = 0; i < PATCH_SIZE; i++) {
      int ty = rect.y + i;

      if (ty < 0)
         ty = 0;

      if (ty > patch->Height()-1)
         ty = patch->Height()-1;

      for (j = 0; j < PATCH_SIZE; j++) {
         int tx = rect.x + (PATCH_SIZE-1 - j);

         if (tx < 0)
            tx = 0;

         if (tx > patch->Width()-1)
            tx = patch->Width()-1;

         *pHeight++ = (float) (patch->GetColor(tx,ty).Red() * mtnscale);
      }
   }
}

// +--------------------------------------------------------------------+

TerrainApron::~TerrainApron()
{
   delete [] heights;
}

// +--------------------------------------------------------------------+

void
TerrainApron::SetScales(double s, double m, double b)
{
   scale    = s;
   mtnscale = m;
   base     = b;
}

// +--------------------------------------------------------------------+

bool
TerrainApron::BuildApron()
{
   int i, j;

   int detail_size   = PATCH_SIZE-1;
   int ds1           = PATCH_SIZE;
   int nverts        = MAX_VERTS;
   int npolys        = detail_size * detail_size * 2;

   model = new(__FILE__,__LINE__) Model;
   model->SetLuminous(true);
   model->SetDynamic(true);

   Material* mtl = new(__FILE__,__LINE__) Material;
   mtl->Ka = ColorValue(0.5f, 0.5f, 0.5f);
   mtl->Kd = ColorValue(0.3f, 0.6f, 0.2f);
   mtl->Ks = Color::Black;

   mtl->tex_diffuse = terrain->ApronTexture();
   strcpy(mtl->name, "Terrain Apron");

   model->GetMaterials().append(mtl);

   Surface*    s     = new(__FILE__,__LINE__) Surface;
   VertexSet*  vset  = 0;

   if (s) {
      s->SetName("default");
      s->CreateVerts(nverts);
      s->CreatePolys(npolys);
      s->AddIndices(npolys*NUM_INDICES_TRI);

      vset = s->GetVertexSet();

      ZeroMemory(vset->loc,      nverts * sizeof(Vec3));
      ZeroMemory(vset->diffuse,  nverts * sizeof(DWORD));
      ZeroMemory(vset->specular, nverts * sizeof(DWORD));
      ZeroMemory(vset->tu,       nverts * sizeof(float));
      ZeroMemory(vset->tv,       nverts * sizeof(float));
      ZeroMemory(vset->rw,       nverts * sizeof(float));


      // initialize vertices
      Vec3*  pVert   = vset->loc;
      float* pTu     = vset->tu;
      float* pTv     = vset->tv;
      double dt      = (1.0/3.0) / (double) detail_size;
      double tu0     = (double) rect.x / rect.w / 3.0 + (1.0/3.0);
      double tv0     = (double) rect.y / rect.h / 3.0;

      for (i = 0; i < ds1; i++) {
         for (j = 0; j < ds1; j++) {
            *pVert++ = Vec3((float) (j* scale - (HALF_PATCH_SIZE*scale)),
                            (float) (heights[i*PATCH_SIZE + j]),
                            (float) (i* scale - (HALF_PATCH_SIZE*scale)));

            *pTu++   = (float) (tu0 - j*dt);
            *pTv++   = (float) (tv0 + i*dt);
         }
      }

      // create the polys
      for (i = 0; i < npolys; i++) {
         Poly* p = s->GetPolys() + i;
         p->nverts      = 3;
         p->vertex_set  = vset;
         p->material    = mtl;
         p->visible     = 1;
         p->sortval     = 0;
      }

      int index = 0;

      // build main patch polys:
      for (i = 0; i < detail_size; i++) {
         for (j = 0; j < detail_size; j++) {
            // first triangle
            Poly* p = s->GetPolys() + index++;
            p->verts[0]   = (ds1 * (i  ) + (j  ));
            p->verts[1]   = (ds1 * (i  ) + (j+1));
            p->verts[2]   = (ds1 * (i+1) + (j+1));

            // second triangle
            p = s->GetPolys() + index++;
            p->verts[0]   = (ds1 * (i  ) + (j  ));
            p->verts[1]   = (ds1 * (i+1) + (j+1));
            p->verts[2]   = (ds1 * (i+1) + (j  ));
         }
      }

      // update the verts and colors of each poly:
      for (i = 0; i < npolys; i++) {
         Poly*   p      = s->GetPolys() + i;
         Plane&  plane  = p->plane;
         WORD*   v      = p->verts;

         plane = Plane(vset->loc[v[0]] + loc,
                       vset->loc[v[1]] + loc,
                       vset->loc[v[2]] + loc);
      }

      // create continguous segments for each material:
      s->Normalize();

      Segment* segment = new(__FILE__,__LINE__) Segment(npolys, s->GetPolys(), mtl, model);
      s->GetSegments().append(segment);

      model->AddSurface(s);


      // copy vertex normals:
      Vec3 normal = Vec3(0, 1, 0);

      for (i = 0; i < ds1; i++) {
         for (j = 0; j < ds1; j++) {
            vset->nrm[i*ds1+j] = normal;
         }
      }
   }

   return true;
}

// +--------------------------------------------------------------------+

int
TerrainApron::CollidesWith(Graphic& o)
{
   return 0;
}

// +--------------------------------------------------------------------+

void
TerrainApron::Update()
{
}

// +--------------------------------------------------------------------+

void
TerrainApron::Illuminate(Color ambient, List<Light>& lights)
{
   if (!model || model->NumVerts() < 1) return;
   Surface* s = model->GetSurfaces().first();
   if (!s) return;

   // clear the solid lights to ambient:
   VertexSet*  vset   = s->GetVertexSet();
   int         nverts = vset->nverts;
   DWORD       aval   = ambient.Value();

   for (int i = 0; i < nverts; i++) {
      vset->diffuse[i]  = aval;
   }

   TerrainRegion* trgn     = terrain->GetRegion();
   bool           eclipsed = false;

   // for each light:
   ListIter<Light> iter = lights;
   while (++iter) {
      Light* light = iter.value();

      if (light->CastsShadow())
         eclipsed  = light->Location().y < -100;

      if (!light->CastsShadow() || !eclipsed) {
         Vec3 vl = light->Location();
         vl.Normalize();

         for (i = 0; i < nverts; i++) {
            Vec3&  nrm = vset->nrm[i];
            double val = 0;

            if (light->IsDirectional()) {
               double gain = vl * nrm;

               if (gain > 0) {
                  val = light->Intensity() * (0.85 * gain);

                  if (val > 1)
                     val = 1;
               }
            }

            if (val > 0.01)
               vset->diffuse[i] = ((light->GetColor().dim(val)) + vset->diffuse[i]).Value();
         }
      }
   }

   InvalidateSurfaceData();
}

// +--------------------------------------------------------------------+

void
TerrainApron::Render(Video* video, DWORD flags)
{
   if (!video || (flags & RENDER_ADDITIVE) || (flags & RENDER_ADD_LIGHT)) return;

   if (!model)
      BuildApron();

   if (scene) {
      Illuminate(scene->Ambient(), scene->Lights());
   }

   double   visibility  = terrain->GetRegion()->GetWeather().Visibility();
   FLOAT    fog_density = (FLOAT) (terrain->GetRegion()->FogDensity() * 2.5e-5 * 1/visibility);

   video->SetRenderState(Video::LIGHTING_ENABLE,   false);
   video->SetRenderState(Video::SPECULAR_ENABLE,   false);
   video->SetRenderState(Video::FOG_ENABLE,        true);
   video->SetRenderState(Video::FOG_COLOR,         terrain->GetRegion()->FogColor().Value());
   video->SetRenderState(Video::FOG_DENSITY,       *((DWORD*) &fog_density));

   Solid::Render(video, flags);

   video->SetRenderState(Video::LIGHTING_ENABLE,   true);
   video->SetRenderState(Video::SPECULAR_ENABLE,   true);
   video->SetRenderState(Video::FOG_ENABLE,        false);
}

// +--------------------------------------------------------------------+

int
TerrainApron::CheckRayIntersection(Point Q, Point w, double len, Point& ipt, bool ttpas)
{
   // compute leading edge of ray:
   Point  sun = Q + w*len;

   if (sun.y < loc.y)
      return 1;

   return 0;
}
