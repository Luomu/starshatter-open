/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Trail.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Missile Trail representation class
*/

#include "MemDebug.h"
#include "Trail.h"
#include "Weapon.h"
#include "Sim.h"

#include "Game.h"
#include "Light.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Sound.h"

// +--------------------------------------------------------------------+

Trail::Trail(Bitmap* tex, int n)
   : ntrail(0), length0(0), length1(0), texture(tex), maxtrail(n), dim(1)
{
   trans    = true;
   luminous = true;

   mtl.Kd            = Color::White;
   mtl.tex_diffuse   = texture;
   mtl.tex_emissive  = texture;
   mtl.blend         = Video::BLEND_ADDITIVE;
   mtl.luminous      = true;

   if (maxtrail < 4) maxtrail = 512;
   trail = new(__FILE__,__LINE__) Point[maxtrail];
   verts = new(__FILE__,__LINE__) VertexSet(maxtrail*2);
   verts->Clear();
   verts->nverts = 0;
   
   for (int i = 0; i < maxtrail*2; i++) {
      verts->diffuse[i]  = D3DCOLOR_RGBA(255,255,255,255);
      verts->specular[i] = D3DCOLOR_RGBA(  0,  0,  0,255);
      verts->tu[i]       = 0.0f;
      verts->tv[i]       = (i & 1) ? 1.0f : 0.0f;
   }

   polys = new(__FILE__,__LINE__) Poly[maxtrail];
   
   for (int i = 0; i < maxtrail; i++) {
      polys[i].vertex_set  = verts;
      polys[i].nverts      = 4;
      polys[i].material    = &mtl;
   }
   
   npolys = 0;
   nverts = 0;
   width  = 6;
   length = 0;
   dim    = 3;

   last_point_time = 0;
}

Trail::~Trail()
{
   delete [] trail;
   delete [] polys;
   delete    verts;
}

void
Trail::UpdateVerts(const Point& cam_pos)
{
   if (ntrail < 2) return;

   int bright = 255 - dim*ntrail;

   Point head  = trail[1] + loc;
   Point tail  = trail[0] + loc;
   Point vcam  = cam_pos - head;
   Point vtmp  = vcam.cross(head-tail);
   vtmp.Normalize();
   Point vlat  = vtmp * (width + (0.1 * width * ntrail));

   verts->loc[0]     = tail - vlat;
   verts->loc[1]     = tail + vlat;
   verts->diffuse[0] = 0;
   verts->diffuse[1] = 0;

   for (int i = 0; i < ntrail-1; i++) {
      bright+=dim;
      Point head  = trail[i+1] + loc;
      Point tail  = trail[i] + loc;
      Point vcam  = cam_pos - head;
      Point vtmp  = vcam.cross(head-tail);
      vtmp.Normalize();
      
      float trail_width = (float) (width + (ntrail-i) * width * 0.1);
      if (i == ntrail-2) trail_width = (float) (width * 0.7);
      Point vlat  = vtmp * trail_width;

      verts->loc[2*i+2] = head - vlat;
      verts->loc[2*i+3] = head + vlat;

      if (bright <= 0) {
         verts->diffuse[2*i+2] = 0;
         verts->diffuse[2*i+3] = 0;
      }
      else {
         verts->diffuse[2*i+2] = D3DCOLOR_RGBA(bright,bright,bright,bright);
         verts->diffuse[2*i+3] = D3DCOLOR_RGBA(bright,bright,bright,bright);
      }
   }
}

void
Trail::Render(Video* video, DWORD flags)
{
   if (!npolys) return;

   if ((flags & RENDER_ADDITIVE) == 0)
      return;

   if (video && life) {
      const Camera* cam = video->GetCamera();

      if (cam)
         UpdateVerts(cam->Pos());

      video->DrawPolys(npolys, polys);
   }
}

void
Trail::AddPoint(const Point& v)
{
   if (ntrail >= maxtrail-1) return;

   double real_time = Game::RealTime() / 1000.0;

   if (ntrail == 0) {
      radius   = 1000;
   }
   else {
      radius   = (float) Point(v-loc).length();
   }

   // just adjust the last point:
   if (ntrail > 1 && real_time - last_point_time < 0.05) {
      trail[ntrail-1] = v;
   }

   // add a new point:
   else {
      last_point_time = real_time;
      trail[ntrail++] = v;

      nverts += 2;
      verts->nverts = nverts;
   
      if (ntrail > 1) {
         length0 = length1;
         length1 = length0 + (trail[ntrail-1]-trail[ntrail-2]).length();

         polys[npolys].vertex_set = verts;
         polys[npolys].nverts = 4;

         polys[npolys].verts[0] = nverts-4;
         polys[npolys].verts[1] = nverts-2;
         polys[npolys].verts[2] = nverts-1;
         polys[npolys].verts[3] = nverts-3;

         float tu1 =  (float) length1 / 250.0f;

         verts->tu[2*ntrail-1] = tu1;
         verts->tu[2*ntrail-2] = tu1;

         npolys++;
      }
   }
}

double
Trail::AverageLength()
{
   double avg = 0;

   for (int i = 0; i < ntrail-1; i++)
      avg += (trail[i+1]-trail[i]).length();
   avg /= ntrail;
   
   return avg;
}

