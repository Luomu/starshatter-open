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

    SUBSYSTEM:    Magic.exe Application
    FILE:         Editor.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Source file for implementation of Selector
*/


#include "stdafx.h"
#include "Editor.h"
#include "MagicDoc.h"
#include "ModelView.h"
#include "Selection.h"

// +----------------------------------------------------------------------+


static float project_u(Vec3& v, int style)
{
   switch (style) {
   case 0:  return v.x;    // PLAN
   case 1:  return v.x;    // FRONT
   case 2:  return v.z;    // SIDE
   }
   
   return v.x;
}

static float project_v(Vec3& v, int style)
{
   switch (style) {
   case 0:  return -v.y;    // PLAN
   case 1:  return -v.z;    // FRONT
   case 2:  return -v.y;    // SIDE
   }
   
   return -v.y;
}

static float project_u_cylindrical(Vec3& v, int axis)
{
   float t = 0.0f;

   switch (axis) {
   // PLAN
   case 0:  if (v.x == 0)
               return 0.0f;
            t = v.y/v.x;
            return (float) atan(t);

   // FRONT
   case 1:  if (v.x == 0)
               return 0.0f;
            t = v.z/v.x;
            return (float) atan(t);

   // SIDE
   case 2:  return (float) atan2(v.z, v.y);    // SIDE
   }
   
   return project_u(v, axis);
}

static float project_v_cylindrical(Vec3& v, int axis)
{
   switch (axis) {
   case 0:  return v.z;    // PLAN
   case 1:  return v.y;    // FRONT
   case 2:  return v.x;    // SIDE
   }
   
   return project_v(v, axis);
}

void
Editor::ApplyMaterial(Material* material, List<Poly>& polys,
                    int mapping, int axis, float scale_u, float scale_v,
                    int flip, int mirror, int rotate)
{
   // save state:
   EditCommand* command = new EditCommand("ApplyMaterial", document);
   document->Exec(command);

   // do the job:
   if (mapping == MAP_CYLINDRICAL) {
      ApplyMaterialCylindrical(material, polys, axis, scale_u, scale_v, flip, mirror, rotate);
      return;
   }

   if (mapping == MAP_SPHERICAL) {
      ApplyMaterialSpherical(material, polys, axis, scale_u, scale_v, flip, mirror, rotate);
      return;
   }

   VertexSet*  vset = polys.first()->vertex_set;

   Vec3*    loc   = vset->loc;
   float    min_u = 100000.0f, max_u = -100000.0f;
   float    min_v = 100000.0f, max_v = -100000.0f;

   ListIter<Poly> iter = polys;

   // compute range and scale:
   if (mapping == MAP_PLANAR) {
      while (++iter) {
         Poly* poly = iter.value();
         for (int i = 0; i < poly->nverts; i++) {
            int v = poly->verts[i];
            
            float u0 = project_u(loc[v], axis);
            float v0 = project_v(loc[v], axis);

            if (u0 < min_u) min_u = u0;
            if (v0 < min_v) min_v = v0;
            if (u0 > max_u) max_u = u0;
            if (v0 > max_v) max_v = v0;
         }
      }
   }

   float base_u = 0.0f;
   float base_v = 0.0f;
   
   if (max_u != min_u) base_u = 1.0f / (max_u - min_u);
   if (max_v != min_v) base_v = 1.0f / (max_v - min_v);

   iter.reset();

   // assign texture id and coordinates:
   while (++iter) {
      Poly* poly = iter.value();
      
      poly->material = material;

      if (mapping == MAP_NONE)
         continue;

      for (int i = 0; i < poly->nverts; i++) {
         int v = poly->verts[i];

         // planar projection
         if (mapping == MAP_PLANAR) {
            if (!rotate) {
               if (mirror)
                  vset->tu[v] = (1.0f - base_u * (project_u(loc[v], axis) - min_u)) * scale_u;
               else
                  vset->tu[v] = (project_u(loc[v], axis) - min_u) * scale_u * base_u;

               if (flip)
                  vset->tv[v] = (1.0f - base_v * (project_v(loc[v], axis) - min_v)) * scale_v;
               else   
                  vset->tv[v] = (project_v(loc[v], axis) - min_v) * scale_v * base_v;
            }
            else {
               if (!mirror)
                  vset->tv[v] = (1.0f - base_u * (project_u(loc[v], axis) - min_u)) * scale_u;
               else
                  vset->tv[v] = (project_u(loc[v], axis) - min_u) * scale_u * base_u;

               if (flip)
                  vset->tu[v] = (1.0f - base_v * (project_v(loc[v], axis) - min_v)) * scale_v;
               else   
                  vset->tu[v] = (project_v(loc[v], axis) - min_v) * scale_v * base_v;
            }
         }

         // stretch to fit
         else if (mapping == MAP_STRETCH) {
            if (scale_u < 0.001) scale_u = 1;
            if (scale_v < 0.001) scale_v = 1;

            if (!rotate) {
               if (mirror)
                  vset->tu[v] = scale_u * (float) (i < 1 || i > 2);
               else
                  vset->tu[v] = scale_u * (float) (i > 0 && i < 3);

               if (flip)
                  vset->tv[v] = scale_v * (float) (i <= 1);
               else   
                  vset->tv[v] = scale_v * (float) (i >  1);
            }
            else {
               if (!mirror)
                  vset->tv[v] = scale_v * (float) (i < 1 || i > 2);
               else
                  vset->tv[v] = scale_v * (float) (i > 0 && i < 3);

               if (flip)
                  vset->tu[v] = scale_u * (float) (i <= 1);
               else   
                  vset->tu[v] = scale_u * (float) (i >  1);
            }
         }
      }
   }

   Resegment();
}

void
Editor::ApplyMaterialCylindrical(Material* material, List<Poly>& polys,
                    int axis, float scale_u, float scale_v,
                    int flip, int mirror, int rotate)
{
   VertexSet*  vset = polys.first()->vertex_set;

   Vec3*    loc   = vset->loc;
   float    min_u = 100000.0f, max_u = -100000.0f;
   float    min_v = 100000.0f, max_v = -100000.0f;

   ListIter<Poly> iter = polys;

   // compute range and scale:
   while (++iter) {
      Poly* poly = iter.value();
      for (int i = 0; i < poly->nverts; i++) {
         int v = poly->verts[i];
         
         float u0 = project_u_cylindrical(loc[v], axis);
         float v0 = project_v_cylindrical(loc[v], axis);

         if (u0 < min_u) min_u = u0;
         if (v0 < min_v) min_v = v0;
         if (u0 > max_u) max_u = u0;
         if (v0 > max_v) max_v = v0;
      }
   }

   float base_u = 0.0f;
   float base_v = 0.0f;
   
   if (max_u != min_u) base_u = 1.0f / (max_u - min_u);
   if (max_v != min_v) base_v = 1.0f / (max_v - min_v);

   iter.reset();

   // assign texture id and coordinates:
   while (++iter) {
      Poly* poly = iter.value();
      
      poly->material = material;
      
      for (int i = 0; i < poly->nverts; i++) {
         int   v  = poly->verts[i];
         float u0 = project_u_cylindrical(loc[v], axis);
         float v0 = project_v_cylindrical(loc[v], axis);

         if (!rotate) {
            if (mirror)
               vset->tu[v] = (1.0f - base_u * (u0 - min_u)) * scale_u;
            else
               vset->tu[v] = (u0 - min_u) * scale_u * base_u;

            if (flip)
               vset->tv[v] = (1.0f - base_v * (v0 - min_v)) * scale_v;
            else   
               vset->tv[v] = (v0 - min_v) * scale_v * base_v;
         }
         else {
            if (!mirror)
               vset->tv[v] = (1.0f - base_u * (u0 - min_u)) * scale_u;
            else
               vset->tv[v] = (u0 - min_u) * scale_u * base_u;

            if (flip)
               vset->tu[v] = (1.0f - base_v * (v0 - min_v)) * scale_v;
            else   
               vset->tu[v] = (v0 - min_v) * scale_v * base_v;
         }
      }
   }

   Resegment();
}

void
Editor::ApplyMaterialSpherical(Material* material, List<Poly>& polys,
                    int axis, float scale_u, float scale_v,
                    int flip, int mirror, int rotate)
{
}

// +----------------------------------------------------------------------+

static int mcomp(const void* a, const void* b)
{
   Poly* pa = (Poly*) a;
   Poly* pb = (Poly*) b;

   if (pa->sortval == pb->sortval)
      return 0;

   if (pa->sortval < pb->sortval)
      return -1;

   return 1;
}

void
Editor::Resegment()
{
   if (model) {
      ListIter<Surface> iter = model->GetSurfaces();
      while (++iter) {
         Surface* surface = iter.value();
         Poly*    polys   = surface->GetPolys();
         int      npolys  = surface->NumPolys();

         for (int n = 0; n < npolys; n++) {
            Poly*     p = polys + n;
            Material* m = p->material;
            int sortval = model->GetMaterials().index(m) + 1;

            if (p->sortval != sortval)
               p->sortval = sortval;
         }

         // destroy the old segments and video data:
         VideoPrivateData* video_data = surface->GetVideoPrivateData();
         surface->SetVideoPrivateData(0);
         surface->GetSegments().destroy();

         delete video_data;

         // sort the polys by material index:
         qsort((void*) polys, npolys, sizeof(Poly), mcomp);

         // create new cohesive segments:
         Segment* segment = 0;

         for (int n = 0; n < npolys; n++) {
            if (segment && segment->material == polys[n].material) {
               segment->npolys++;
            }
            else {
               segment = 0;
            }

            if (!segment) {
               segment = new Segment;

               segment->npolys   = 1;
               segment->polys    = polys + n;
               segment->material = segment->polys->material;

               surface->GetSegments().append(segment);
            }
         }
      }
   }
}


// +----------------------------------------------------------------------+
// +----------------------------------------------------------------------+
// +----------------------------------------------------------------------+

EditCommand::EditCommand(const char* n, MagicDoc* d)
   : Command(n, d), model1(0), model2(0)
{
}

EditCommand::~EditCommand()
{
   delete model1;
   delete model2;
}

// +----------------------------------------------------------------------+

void
EditCommand::Do()
{
   if (document) {
      Solid* solid = document->GetSolid();

      // first application:
      if (!model2) {
         if (!model1)
            model1 = new Model(*solid->GetModel());
      }
      // re-do:
      else {
         solid->GetModel()->operator=(*model2);
      }
   }
}

// +----------------------------------------------------------------------+

void
EditCommand::Undo()
{
   if (document && model1) {
      Solid* solid = document->GetSolid();

      // save current state for later re-do:
      if (!model2)
         model2 = new Model(*solid->GetModel());

      solid->GetModel()->operator=(*model1);
   }
}

