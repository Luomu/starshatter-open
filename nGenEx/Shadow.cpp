/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Shadow.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Dynamic Stencil Shadow Volumes
*/

#include "MemDebug.h"
#include "Shadow.h"
#include "Light.h"
#include "Solid.h"
#include "Scene.h"
#include "Video.h"

static bool visible_shadow_volumes = false;

// +--------------------------------------------------------------------+

Shadow::Shadow(Solid* s)
   : verts(0), nverts(0), max_verts(0), edges(0), num_edges(0), enabled(true)
{
   solid = s;

   if (solid && solid->GetModel()) {
      Model*   model       = solid->GetModel();
      int      npolys      = model->NumPolys();

      max_verts = model->NumVerts() * 4;
      verts     = new(__FILE__,__LINE__) Vec3[max_verts];
      edges     = new(__FILE__,__LINE__) WORD[npolys * 6];
   }
}

// +--------------------------------------------------------------------+

Shadow::~Shadow()
{
   if (verts)  delete [] verts;
   if (edges)  delete [] edges;
}

// +--------------------------------------------------------------------+

void
Shadow::Reset()
{
   num_edges   = 0;
   nverts      = 0;
}

// +--------------------------------------------------------------------+

void
Shadow::Render(Video* video)
{
   if (enabled)
      video->DrawShadow(solid, nverts, verts, visible_shadow_volumes);
}

// +--------------------------------------------------------------------+

void
Shadow::Update(Light* light)
{
   Reset();

   if (!light || !solid || !solid->GetModel() || !edges) return;

   Vec3     lpos        = light->Location();
   bool     directional = light->Type() == Light::LIGHT_DIRECTIONAL;
   Model*   model       = solid->GetModel();

   ListIter<Surface> iter = model->GetSurfaces();
   while (++iter) {
      Surface* s = iter.value();

      // transform light location into surface object space
      Matrix xform(solid->Orientation()); // XXX should be: (s->GetOrientation());

      Vec3 tmp = light->Location();

      if (!directional)
         tmp -= (solid->Location() + s->GetOffset());

      lpos.x = tmp * Vec3(xform(0,0), xform(0,1), xform(0,2));
      lpos.y = tmp * Vec3(xform(1,0), xform(1,1), xform(1,2));
      lpos.z = tmp * Vec3(xform(2,0), xform(2,1), xform(2,2));

      // compute the silohuette for the mesh with respect to the light:
      
      for (int i = 0; i < s->NumPolys(); i++) {
         Poly* p = s->GetPolys() + i;

         // skip polys with non-shadowing materials:
         if (p->material && !p->material->shadow)
            continue;

         // if this poly faces the light:
         if (p->plane.normal * lpos > 0) {
            for (int n = 0; n < p->nverts; n++) {
               if (n < p->nverts-1)
                  AddEdge(p->vlocs[n], p->vlocs[n+1]);
               else
                  AddEdge(p->vlocs[n], p->vlocs[0]);
            }
         }
      }

      // extrude the silohuette away from the light source
      // to create the shadow volume:

      Vec3 extent = lpos * -1;
      extent.Normalize();
      extent *= 50.0e3f; //solid->Radius() * 2.1f;

      for (i = 0; i < (int) num_edges; i++) {
         if (nverts+6 <= max_verts) {
            Vec3 v1 = s->GetVLoc()[edges[2*i+0]];
            Vec3 v2 = s->GetVLoc()[edges[2*i+1]];
            Vec3 v3 = v1 + extent;
            Vec3 v4 = v2 + extent;

            verts[nverts++] = v1;
            verts[nverts++] = v2;
            verts[nverts++] = v3;

            verts[nverts++] = v2;
            verts[nverts++] = v4;
            verts[nverts++] = v3;
         }
      }
   }
}

void
Shadow::AddEdge(WORD v1, WORD v2)
{
   // Remove interior edges (which appear in the list twice)
   for (DWORD i = 0; i < num_edges; i++) {
      if ((edges[2*i+0] == v1 && edges[2*i+1] == v2) ||
          (edges[2*i+0] == v2 && edges[2*i+1] == v1))
      {
         if (num_edges > 1) {
            edges[2*i+0] = edges[2*(num_edges-1)+0];
            edges[2*i+1] = edges[2*(num_edges-1)+1];
         }

         num_edges--;
         return;
      }
   }

   edges[2*num_edges+0] = v1;
   edges[2*num_edges+1] = v2;

   num_edges++;
}

bool
Shadow::GetVisibleShadowVolumes()
{
   return visible_shadow_volumes;
}

void
Shadow::SetVisibleShadowVolumes(bool vis)
{
   visible_shadow_volumes = vis;
}
