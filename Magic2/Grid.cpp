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

    SUBSYSTEM:    Magic.exe
    FILE:         Grid.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the Grid class
*/

#include "stdafx.h"
#include "Magic.h"

#include "MagicDoc.h"
#include "Grid.h"

#include "ActiveWindow.h"
#include "Color.h"
#include "Polygon.h"
#include "Scene.h"
#include "Screen.h"
#include "Solid.h"
#include "Video.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MAJOR_COUNT = 64;

// +--------------------------------------------------------------------+

Grid::Grid()
   : x_major(0), y_major(0), x_minor(0), y_minor(0),
     bmp_plan(0), bmp_front(0), bmp_side(0)
{
   strcpy(name, "Grid");

   plane    = GRID_XZ; // plan

   show     = true;
   show_ref = true;
   snap     = true;
   minor    = true;

   x_size   = 4;
   y_size   = 4;
   x_skip   = 4;
   y_skip   = 4;

   x_major  = new Vec3[2*MAJOR_COUNT];
   y_major  = new Vec3[2*MAJOR_COUNT];
}

Grid::~Grid()
{
   delete [] x_major;
   delete [] y_major;
   delete [] x_minor;
   delete [] y_minor;
}

// +--------------------------------------------------------------------+

const char*
Grid::GetReferencePlan() const
{
   if (bmp_plan)
      return bmp_plan->GetFilename();

   return "";
}

void
Grid::SetReferencePlan(const char* fname)
{
   bmp_plan = 0;

   if (fname && *fname)
      LoadTexture(fname, bmp_plan);
}

// +--------------------------------------------------------------------+

const char*
Grid::GetReferenceFront() const
{
   if (bmp_front)
      return bmp_front->GetFilename();

   return "";
}

void
Grid::SetReferenceFront(const char* fname)
{
   bmp_front = 0;

   if (fname && *fname)
      LoadTexture(fname, bmp_front);
}

// +--------------------------------------------------------------------+

const char*
Grid::GetReferenceSide() const
{
   if (bmp_side)
      return bmp_side->GetFilename();

   return "";
}

void
Grid::SetReferenceSide(const char* fname)
{
   bmp_side = 0;

   if (fname && *fname)
      LoadTexture(fname, bmp_side);
}

// +--------------------------------------------------------------------+

void
Grid::Render(Video* video, DWORD flags)
{
   Vec3 origin[4];

   for (int i = 0; i < 4; i++)
      origin[i] = loc;

   const float EXTENT = (float) MAJOR_COUNT * 64.0f;

   switch (plane) {
   case GRID_XY: // FRONT
      origin[0].x += -EXTENT;
      origin[1].x +=  EXTENT;
      origin[2].y += -EXTENT;
      origin[3].y +=  EXTENT;

      for (int i = 0; i < 2*MAJOR_COUNT; i += 2) {
         float x = (float) (i-MAJOR_COUNT) * 64.0f;
         float y = (float) (i-MAJOR_COUNT) * 64.0f;

         x_major[i  ] = loc + Vec3(x, -EXTENT, 0.0f);
         x_major[i+1] = loc + Vec3(x,  EXTENT, 0.0f);
         y_major[i  ] = loc + Vec3(-EXTENT, y, 0.0f);
         y_major[i+1] = loc + Vec3( EXTENT, y, 0.0f);
      }

      break;

   case GRID_XZ: // PLAN
      origin[0].x += -EXTENT;
      origin[1].x +=  EXTENT;
      origin[2].z += -EXTENT;
      origin[3].z +=  EXTENT;

      for (int i = 0; i < 2*MAJOR_COUNT; i += 2) {
         float x = (float) (i-MAJOR_COUNT) * 64.0f;
         float z = (float) (i-MAJOR_COUNT) * 64.0f;

         x_major[i  ] = loc + Vec3(x, 0.0f, -EXTENT);
         x_major[i+1] = loc + Vec3(x, 0.0f,  EXTENT);
         y_major[i  ] = loc + Vec3(-EXTENT, 0.0f, z);
         y_major[i+1] = loc + Vec3( EXTENT, 0.0f, z);
      }

      break;

   case GRID_YZ: // SIDE
      origin[0].y += -EXTENT;
      origin[1].y +=  EXTENT;
      origin[2].z += -EXTENT;
      origin[3].z +=  EXTENT;

      for (int i = 0; i < 2*MAJOR_COUNT; i += 2) {
         float y = (float) (i-MAJOR_COUNT) * 64.0f;
         float z = (float) (i-MAJOR_COUNT) * 64.0f;

         x_major[i  ] = loc + Vec3(0.0f, y, -EXTENT);
         x_major[i+1] = loc + Vec3(0.0f, y,  EXTENT);
         y_major[i  ] = loc + Vec3(0.0f, -EXTENT, z);
         y_major[i+1] = loc + Vec3(0.0f,  EXTENT, z);
      }

      break;
   }

   RenderReference(video);

   video->SetRenderState(Video::FILL_MODE,         Video::FILL_SOLID);
   video->SetRenderState(Video::LIGHTING_ENABLE,   FALSE);
   video->SetRenderState(Video::Z_ENABLE,          TRUE);
   video->SetRenderState(Video::Z_WRITE_ENABLE,    TRUE);

   video->DrawLines(MAJOR_COUNT, x_major, Color::Gray,     1);
   video->DrawLines(MAJOR_COUNT, y_major, Color::Gray,     1);
   video->DrawLines(          2, origin,  Color::DarkGray, 1);
}

// +--------------------------------------------------------------------+

void
Grid::RenderReference(Video* video)
{
   if (!show_ref)    return;

   Bitmap*           bmp = 0;

   switch (plane) {
   case GRID_XY:     bmp = bmp_front;  break;
   case GRID_XZ:     bmp = bmp_plan;   break;
   case GRID_YZ:     bmp = bmp_side;   break;
   }

   if (!bmp)         return;

   Material          mtl;
   VertexSet         vset(4);
   Poly              poly;
   Vec3              nrm;

   float vx = (float) bmp->Width();
   float vy = (float) bmp->Height();
   float vz = -100.0f;

   if (vx <= 256 || vy <= 256) {
      vx *= 2.0f;
      vy *= 2.0f;
   }

   switch (plane) {
   case GRID_XY: // FRONT
      nrm         = Vec3(0.0f, 0.0f, 1.0f);
      vset.loc[0] = loc + Vec3(-vx, -vy,  vz);
      vset.loc[1] = loc + Vec3( vx, -vy,  vz);
      vset.loc[2] = loc + Vec3( vx,  vy,  vz);
      vset.loc[3] = loc + Vec3(-vx,  vy,  vz);
      break;

   case GRID_XZ: // PLAN
      nrm         = Vec3(0.0f, 1.0f, 0.0f);
      vset.loc[0] = loc + Vec3(-vx,  vz,  vy);
      vset.loc[1] = loc + Vec3( vx,  vz,  vy);
      vset.loc[2] = loc + Vec3( vx,  vz, -vy);
      vset.loc[3] = loc + Vec3(-vx,  vz, -vy);
      break;

   case GRID_YZ: // SIDE
      nrm         = Vec3(1.0f, 0.0f, 0.0f);
      vset.loc[0] = loc + Vec3( vz, -vx, -vy);
      vset.loc[1] = loc + Vec3( vz,  vx, -vy);
      vset.loc[2] = loc + Vec3( vz,  vx,  vy);
      vset.loc[3] = loc + Vec3( vz, -vx,  vy);
      break;

   default:
      return;
   }

   mtl.Ka            = Color::White;
   mtl.Kd            = Color::Black;
   mtl.tex_diffuse   = bmp;

   vset.nrm[0]       = nrm;
   vset.nrm[1]       = nrm;
   vset.nrm[2]       = nrm;
   vset.nrm[3]       = nrm;

   vset.tu[0]        = 0.0f;
   vset.tv[0]        = 0.0f;
   vset.tu[1]        = 1.0f;
   vset.tv[1]        = 0.0f;
   vset.tu[2]        = 1.0f;
   vset.tv[2]        = 1.0f;
   vset.tu[3]        = 0.0f;
   vset.tv[3]        = 1.0f;

   poly.nverts       = 4;
   poly.verts[0]     = 0;
   poly.verts[1]     = 1;
   poly.verts[2]     = 2;
   poly.verts[3]     = 3;
   poly.material     = &mtl;
   poly.vertex_set   = &vset;

   video->SetRenderState(Video::FILL_MODE,         Video::FILL_SOLID);
   video->SetRenderState(Video::LIGHTING_ENABLE,   TRUE);
   video->SetRenderState(Video::Z_ENABLE,          FALSE);
   video->SetRenderState(Video::Z_WRITE_ENABLE,    FALSE);
   video->SetAmbient(Color::White);

   video->DrawPolys(1, &poly);
}

// +----------------------------------------------------------------------+

CPoint&
Grid::Snap(CPoint& p)
{
   p.x = snapto(p.x, x_size);
   p.y = snapto(p.y, y_size);

   return p;
}

// +----------------------------------------------------------------------+

int
Grid::snapto(int i, int dim)
{
   if (!snap)
      return i;

   int n = i + dim/2;
   int m = n % dim;

   return (n - m);
}

// +----------------------------------------------------------------------+

void
Grid::SetSize(int x, int y)
{
   x_size = x;
   y_size = y ? y : x;
}

// +----------------------------------------------------------------------+

void
Grid::SetSkip(int x, int y)
{
   x_skip = x;
   y_skip = y ? y : x;
}

