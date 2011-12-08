/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Selection.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for rendering solid meshes of polygons
*/

#include "stdafx.h"
#include "Magic.h"
#include "Selection.h"
#include "ModelView.h"
#include "Projector.h"

#include "Solid.h"
#include "Scene.h"
#include "Bitmap.h"

#include <stdio.h>

// +--------------------------------------------------------------------+

Selection::Selection()
   : model(0), model_view(0)
{
   strcpy(name, "Selection");
}

// +--------------------------------------------------------------------+

Selection::~Selection()
{
}

// +--------------------------------------------------------------------+

void
Selection::Render(Video* video, DWORD flags)
{
   if (video && flags == Graphic::RENDER_ALPHA) {
      ListIter<Poly> iter = polys;
      while (++iter) {
         Poly* p = iter.value();
         Vec3  vloc[16];

         if (p->nverts >= 3) {
            for (int i = 0; i < p->nverts; i++) {
               int n = (i==p->nverts-1) ? 0 : i+1;

               vloc[2*i+0] = loc + p->vertex_set->loc[ p->verts[i] ];
               vloc[2*i+1] = loc + p->vertex_set->loc[ p->verts[n] ];
            }

            video->SetRenderState(Video::Z_ENABLE, FALSE);
            video->DrawLines(p->nverts, vloc, Color(255,255,128));
         }
      }

      if (model) {
         for (int i = 0; i < verts.size(); i++) {
            DWORD value = verts[i];
            WORD  index = (WORD) (value >> 16);
            WORD  vert  = (WORD) (value & 0xffff);

            if (index < model->NumSurfaces()) {
               Surface* s = model->GetSurfaces()[index];
               Vec3     v = loc + s->GetVertexSet()->loc[vert];

               if (model_view) {
                  CPoint p = model_view->ProjectPoint(v);
                  float  handle[16];
                  int    x1 = p.x-1;
                  int    y1 = p.y-1;
                  int    x2 = p.x+1;
                  int    y2 = p.y+1;

                  handle[ 0] = (float) x1;
                  handle[ 1] = (float) y1;
                  handle[ 2] = (float) x2;
                  handle[ 3] = (float) y1;

                  handle[ 4] = (float) x2;
                  handle[ 5] = (float) y1;
                  handle[ 6] = (float) x2;
                  handle[ 7] = (float) y2;

                  handle[ 8] = (float) x2;
                  handle[ 9] = (float) y2;
                  handle[10] = (float) x1;
                  handle[11] = (float) y2;

                  handle[12] = (float) x1;
                  handle[13] = (float) y2;
                  handle[14] = (float) x1;
                  handle[15] = (float) y1;

                  video->DrawScreenLines(4, handle, Color(255,255,128));
               }

               else {
                  Vec3     vloc[8];

                  vloc[0]  = v + Vec3(-1.0f, -1.0f,  0.0f);
                  vloc[1]  = v + Vec3( 1.0f, -1.0f,  0.0f);
                  vloc[2]  = v + Vec3( 1.0f, -1.0f,  0.0f);
                  vloc[3]  = v + Vec3( 1.0f,  1.0f,  0.0f);
                  vloc[4]  = v + Vec3( 1.0f,  1.0f,  0.0f);
                  vloc[5]  = v + Vec3(-1.0f,  1.0f,  0.0f);
                  vloc[6]  = v + Vec3(-1.0f,  1.0f,  0.0f);
                  vloc[7]  = v + Vec3(-1.0f, -1.0f,  0.0f);

                  video->SetRenderState(Video::Z_ENABLE, FALSE);
                  video->DrawLines(4, vloc, Color(255,255,128));
               }
            }
         }
      }

      video->UseMaterial(0);
   }
}

// +--------------------------------------------------------------------+

void
Selection::AddPoly(Poly* p)
{
   if (!polys.contains(p))
      polys.append(p);
}

void
Selection::RemovePoly(Poly* p)
{
   polys.remove(p);
}

bool
Selection::Contains(Poly* p) const
{
   return polys.contains(p);
}

void
Selection::AddVert(WORD s, WORD v)
{
   DWORD value = (s << 16) | v;

   if (!verts.contains(value))
      verts.append(value);
}

void
Selection::RemoveVert(WORD s, WORD v)
{
   DWORD value = (s << 16) | v;
   verts.remove(value);
}

bool
Selection::Contains(WORD s, WORD v) const
{
   DWORD value = (s << 16) | v;
   return verts.contains(value);
}