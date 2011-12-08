/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Sky.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Celestial sphere, stars, planets, space dust...
*/

#include "MemDebug.h"
#include "Sky.h"
#include "StarSystem.h"

#include "Game.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Light.h"
#include "Random.h"

void Print(const char* ftm, ...);


// +====================================================================+

Stars::Stars(int nstars)
{
   infinite = true;
   luminous = true;
   shadow   = false;

   vset     = new(__FILE__,__LINE__) VertexSet(nstars);
   colors   = new(__FILE__,__LINE__) Color[nstars];

   for (int i = 0; i < nstars; i++) {
      vset->loc[i]   = RandomVector(1000);

      ColorValue val = ColorValue((float) Random(0.7, 0.8),
                                  (float) Random(0.7, 0.8),
                                  (float) Random(0.7, 0.8));
      Color      c   = val.ToColor();

      colors[i]         = c;
      vset->diffuse[i]  = c.Value();
      vset->specular[i] = 0;
   }

   strcpy(name, "Stars");
}

Stars::~Stars()
{
   delete [] colors;
   delete    vset;
}

// +--------------------------------------------------------------------+

void
Stars::Illuminate(double scale)
{
   if (!vset)
      return;

   for (int i = 0; i < vset->nverts; i++) {
      Color c = colors[i] * scale;
      vset->diffuse[i] = c.Value();
   }
}

// +--------------------------------------------------------------------+

void
Stars::Render(Video* video, DWORD flags)
{
   if (!vset || !video || (flags & Graphic::RENDER_ADDITIVE) == 0)
      return;

   video->SetBlendType(Video::BLEND_ADDITIVE);
   video->DrawPoints(vset);
}

// +====================================================================+

static const double BOUNDARY   = 3000;
static const double BOUNDARYx2 = BOUNDARY * 2;

Dust::Dust(int ndust, bool b)
   : really_hidden(false), bright(b)
{
   radius   = (float) BOUNDARYx2;
   luminous = true;
   vset     = new(__FILE__,__LINE__) VertexSet(ndust);

   Reset(Point(0, 0, 0));
   strcpy(name, "Dust");
}

// +--------------------------------------------------------------------+

Dust::~Dust()
{
   delete vset;
}

// +--------------------------------------------------------------------+

void
Dust::Reset(const Point& ref)
{
   BYTE c = 0;

   for (int i = 0; i < vset->nverts; i++) {
      vset->loc[i] = Vec3( Random(-BOUNDARY, BOUNDARY),
                           Random(-BOUNDARY, BOUNDARY),
                           Random(-BOUNDARY, BOUNDARY) );

      if (bright)
         c = (BYTE) Random(96,200);
      else
         c = (BYTE) Random(64,156);

      vset->diffuse[i]  = Color(c,c,c).Value();
      vset->specular[i] = 0;
   }
}

// +--------------------------------------------------------------------+

void
Dust::ExecFrame(double factor, const Point& ref)
{
   if (Game::TimeCompression() > 4) {
      Hide();
      return;
   }

   Show();

   Point  delta = ref - loc;
   double dlen  = delta.length();
   
   if (dlen < 0.0001)
      return;

   if (dlen > BOUNDARY) {
      Reset(ref);
   }
   else {
      // wrap around if necessary to keep in view
      for (int i = 0; i < vset->nverts; i++) {
         Vec3 v = vset->loc[i];

         v -= delta;

         if (v.x >  BOUNDARY) v.x -= (float) BOUNDARYx2;
         if (v.x < -BOUNDARY) v.x += (float) BOUNDARYx2;
         if (v.y >  BOUNDARY) v.y -= (float) BOUNDARYx2;
         if (v.y < -BOUNDARY) v.y += (float) BOUNDARYx2;
         if (v.z >  BOUNDARY) v.z -= (float) BOUNDARYx2;
         if (v.z < -BOUNDARY) v.z += (float) BOUNDARYx2;

         vset->loc[i] = v;
      }
   }

   MoveTo(ref);
}

// +--------------------------------------------------------------------+

void
Dust::Render(Video* video, DWORD flags)
{
   if (hidden || really_hidden)
      return;

   if (!vset || !video || (flags & Graphic::RENDER_SOLID) == 0 || (flags & Graphic::RENDER_ADD_LIGHT) != 0)
      return;

   video->SetBlendType(Video::BLEND_SOLID);
   video->SetRenderState(Video::Z_ENABLE,       false);
   video->SetRenderState(Video::Z_WRITE_ENABLE, false);
   
   video->DrawPoints(vset);
   
   video->SetRenderState(Video::Z_ENABLE,       true);
   video->SetRenderState(Video::Z_WRITE_ENABLE, true);
}

// +--------------------------------------------------------------------+

void
Dust::Hide()
{
   hidden = true;
   really_hidden = true;
}

void
Dust::Show()
{
   hidden = false;
   really_hidden = false;
}

// +====================================================================+

PlanetRep::PlanetRep(const char* surface_name, const char* glow_name,
               double rad, const Vec3& pos, double tscale,
               const char* rngname, double minrad, double maxrad,
               Color atmos, const char* gloss_name)
   : mtl_surf(0), mtl_limb(0), mtl_ring(0), star_system(0)
{
   loc = pos;

   radius     = (float) rad;
   has_ring   = 0;
   ring_verts = -1;
   ring_polys = -1;
   ring_rad   = 0;
   body_rad   = rad;
   daytime    = false;
   atmosphere = atmos;
   star_system = 0;

   if (!surface_name || !*surface_name) {
      Print("   invalid Planet patch - no surface texture specified\n");
      return;
   }

   Print("   constructing Planet patch %s\n", surface_name);
   strncpy(name, surface_name, 31);
   name[31] = 0;

   Bitmap*  bmp_surf = 0;
   Bitmap*  bmp_spec = 0;
   Bitmap*  bmp_glow = 0;
   Bitmap*  bmp_ring = 0;
   Bitmap*  bmp_limb = 0;

   DataLoader* loader = DataLoader::GetLoader();
   loader->LoadTexture(surface_name, bmp_surf, Bitmap::BMP_SOLID, true);

   if (glow_name && *glow_name) {
      Print("   loading glow texture %s\n", glow_name);
      loader->LoadTexture(glow_name, bmp_glow, Bitmap::BMP_SOLID, true);
   }

   if (gloss_name && *gloss_name) {
      Print("   loading gloss texture %s\n", gloss_name);
      loader->LoadTexture(gloss_name, bmp_spec, Bitmap::BMP_SOLID, true);
   }

   mtl_surf = new(__FILE__,__LINE__) Material;

   mtl_surf->Ka            = Color::LightGray;
   mtl_surf->Kd            = Color::White;
   mtl_surf->Ke            = bmp_glow ? Color::White     : Color::Black;
   mtl_surf->Ks            = bmp_spec ? Color::LightGray : Color::Black;
   mtl_surf->power         = 25.0f;
   mtl_surf->tex_diffuse   = bmp_surf;
   mtl_surf->tex_specular  = bmp_spec;
   mtl_surf->tex_emissive  = bmp_glow;
   mtl_surf->blend         = Material::MTL_SOLID;

   if (bmp_spec && Video::GetInstance()->IsSpecMapEnabled()) {
      if (glow_name && strstr(glow_name, "light"))
         strcpy(mtl_surf->shader, "SimplePix/PlanetSurfNightLight");

      else if (glow_name)
         strcpy(mtl_surf->shader, "SimplePix/PlanetSurf");
   }

   if (atmosphere != Color::Black) {
      mtl_limb = new(__FILE__,__LINE__) Material;

      mtl_limb->Ka = atmosphere;

      strcpy(mtl_limb->shader, "PlanetLimb");

      Print("   loading atmospheric limb texture PlanetLimb.pcx\n");
      loader->LoadTexture("PlanetLimb.pcx", bmp_limb, Bitmap::BMP_TRANSLUCENT, true);
      mtl_limb->tex_diffuse = bmp_limb;
      mtl_limb->blend       = Material::MTL_TRANSLUCENT;
   }

   if (maxrad > 0 && minrad > 0) {
      has_ring = 1;
      radius   = (float) maxrad;
      ring_rad = (maxrad + minrad)/2;
      loader->LoadTexture(rngname, bmp_ring, Bitmap::BMP_SOLID, true);

      mtl_ring = new(__FILE__,__LINE__) Material;

      mtl_ring->Ka            = Color::LightGray;
      mtl_ring->Kd            = Color::White;
      mtl_ring->Ks            = Color::Gray;
      mtl_ring->Ke            = Color::Black;
      mtl_ring->power         = 30.0f;
      mtl_ring->tex_diffuse   = bmp_ring;
      mtl_ring->blend         = Material::MTL_TRANSLUCENT;
   }

   if (rad > 2e6 && rad < 1e8)
      CreateSphere(rad, 24, 32, minrad, maxrad, 48, tscale);
   else
      CreateSphere(rad, 16, 24, minrad, maxrad, 48, tscale);
}

// +--------------------------------------------------------------------+

PlanetRep::~PlanetRep()
{
}

// +--------------------------------------------------------------------+

void
PlanetRep::CreateSphere(double radius, int nrings, int nsections,
                     double minrad, double maxrad, int rsections,
                     double tscale)
{
   const int sect_verts = nsections + 1;

   model     = new(__FILE__,__LINE__) Model;
   own_model = 1;

   Surface* surface = new(__FILE__,__LINE__) Surface;

   int i, j, m, n;

   int npolys = (nrings + 2) * nsections;
   int nverts = (nrings + 3) * sect_verts;

   int ppolys = npolys;
   int pverts = nverts;

   int apolys = 0;
   int averts = 0;

   if (atmosphere != Color::Black) {
      apolys = npolys;
      averts = nverts;

      npolys *= 2;
      nverts *= 2;
   }

   if (has_ring) {
      ring_verts = nverts;
      ring_polys = npolys;

      npolys += rsections * 3;   // top, bottom, edge
      nverts += rsections * 6;
   }

   surface->SetName(name);
   surface->CreateVerts(nverts);
   surface->CreatePolys(npolys);

   VertexSet* vset  = surface->GetVertexSet();

   if (!vset || vset->nverts < nverts) {
      ::Print("WARNING: insufficient memory for planet '%s'\n", name);
      return;
   }

   Poly* polys = surface->GetPolys();

   if (!polys) {
      ::Print("WARNING: insufficient memory for planet '%s'\n", name);
      return;
   }

   ZeroMemory(polys, sizeof(Poly) * npolys);

   // Generate vertex points for planetary rings:
   double dtheta = PI / (nrings + 2);
   double dphi   = 2 * PI / nsections;
   double theta  = 0;
   n = 0; // vertex being generated

   for (i = 0; i < nrings+3; i++) {
      double y = radius * cos(theta);  // y is the same for entire ring
      double v = theta / PI;           // v is the same for entire ring
      double rsintheta = radius * sin(theta);
      double phi = 0;

      for (j = 0; j < sect_verts; j++) {
         double x = rsintheta * sin(phi);
         double z = rsintheta * cos(phi);

         vset->loc[n] = Vec3(x, y, z);
         vset->nrm[n] = Vec3(x, y, z);
         vset->tu[n]  = (float) (tscale * (1 - (phi/(2.0*PI))));
         vset->tv[n]  = (float) (tscale * v);

         vset->nrm[n].Normalize();

         phi += dphi;
         n++;
      }

      theta += dtheta;
   }

   // Generate vertex points for rings:
   if (has_ring) {
      n = ring_verts;

      double dphi  = 2.0 * PI / rsections;
      double y     = 0;  // y is the same for entire ring

      // top of ring:
      double phi   = 0;
      for (j = 0; j < rsections; j++) {
         double x = minrad * sin(phi);
         double z = minrad * cos(phi);

         vset->loc[n] = Vec3(x, y, z);
         vset->nrm[n] = Vec3(0, 1, 0);
         vset->tu[n]  = (j & 1) ? 1.0f : 0.0f;
         vset->tv[n]  = 0.0f;
         n++;

         x = maxrad * sin(phi);
         z = maxrad * cos(phi);

         vset->loc[n] = Vec3(x, y, z);
         vset->nrm[n] = Vec3(0, 1, 0);
         vset->tu[n]  = (j & 1) ? 1.0f : 0.0f;
         vset->tv[n]  = 1.0f;
         n++;

         phi += dphi;
      }

      // bottom of ring:
      phi   = 0;
      for (j = 0; j < rsections; j++) {
         double x = minrad * sin(phi);
         double z = minrad * cos(phi);

         vset->loc[n] = Vec3(x, y, z);
         vset->nrm[n] = Vec3(0, -1, 0);
         vset->tu[n]  = (j & 1) ? 1.0f : 0.0f;
         vset->tv[n]  = 0.0f;
         n++;

         x = maxrad * sin(phi);
         z = maxrad * cos(phi);

         vset->loc[n] = Vec3(x, y, z);
         vset->nrm[n] = Vec3(0, -1, 0);
         vset->tu[n]  = (j & 1) ? 1.0f : 0.0f;
         vset->tv[n]  = 1.0f;
         n++;

         phi += dphi;
      }

      // edge of ring:
      phi   = 0;
      for (j = 0; j < rsections; j++) {
         double x = maxrad * sin(phi);
         double z = maxrad * cos(phi);

         Point normal = Point(x,0,z);
         normal.Normalize();

         double thickness = maxrad/333;

         vset->loc[n] = Vec3(x, y+thickness, z);
         vset->nrm[n] = normal;
         vset->tu[n]  = (j & 1) ? 1.0f : 0.0f;
         vset->tv[n]  = 1.0f;
         n++;

         vset->loc[n] = Vec3(x, y-thickness, z);
         vset->nrm[n] = normal;
         vset->tu[n]  = (j & 1) ? 1.0f : 0.0f;
         vset->tv[n]  = 1.0f;
         n++;

         phi += dphi;
      }
   }

   for (i = 0; i < npolys; i++) {
      polys[i].nverts      = 3;
      polys[i].vertex_set  = vset;
      polys[i].material    = mtl_surf;
   }

   // Generate triangles for top and bottom caps.
   for (i = 0; i < nsections; i++) {
      Poly& p0 = polys[i];
      p0.verts[2]          = i;
      p0.verts[1]          = sect_verts + i;
      p0.verts[0]          = sect_verts + ((i+1) % sect_verts);
      
      Poly& p1 = polys[ppolys - nsections + i];
      p1.verts[2]          = pverts - 1 - i;
      p1.verts[1]          = pverts - 1 - sect_verts - i;
      p1.verts[0]          = pverts - 2 - sect_verts - i;

      surface->AddIndices(6);
   }

   // Generate triangles for the planetary rings
   m = sect_verts;   // first vertex in current ring
   n = nsections;    // triangle being generated, skip the top cap

   for (i = 0; i < nrings; i++) {
      for (j = 0; j < nsections; j++) {
         Poly& p0 = polys[n];
         p0.nverts            = 4;
         p0.verts[3]          = m + j;
         p0.verts[2]          = m + (sect_verts) + j;
         p0.verts[1]          = m + (sect_verts) + ((j + 1) % (sect_verts));
         p0.verts[0]          = m + ((j + 1) % (sect_verts));
         n++;

         surface->AddIndices(6);
      }

      m += sect_verts;
   }

   if (averts && apolys && mtl_limb) {
      for (i = 0; i < pverts; i++) {
         vset->loc[averts + i] = vset->loc[i];
         vset->nrm[averts + i] = vset->nrm[i];
      }

      for (i = 0; i < ppolys; i++) {
         Poly& p0 = polys[i];
         Poly& p1 = polys[apolys + i];

         p1.vertex_set  = vset;
         p1.material    = mtl_limb;

         p1.nverts      = p0.nverts;
         p1.verts[0]    = p0.verts[0];
         p1.verts[1]    = p0.verts[1];
         p1.verts[2]    = p0.verts[2];
         p1.verts[3]    = p0.verts[3];

         surface->AddIndices(p1.nverts == 3 ? 3 : 6);
      }
   }

   if (has_ring) {
      // Generate quads for the rings
      m = ring_verts;    // first vertex in top of ring, after planet verts
      n = ring_polys;    // quad being generated, after planet polys

      // top of ring:
      for (j = 0; j < rsections; j++) {
         Poly& p0 = polys[n];
         p0.nverts            = 4;
         p0.material          = mtl_ring;

         p0.verts[3]          = m + 2*j;
         p0.verts[2]          = m + 2*j + 1;
         p0.verts[1]          = m + ((2*j + 3) % (rsections*2));
         p0.verts[0]          = m + ((2*j + 2) % (rsections*2));

         surface->AddIndices(6);

         n++;
      }

      // bottom of ring:
      // first vertex in bottom of ring, after top ring verts
      m = ring_verts + 2*rsections;

      for (j = 0; j < rsections; j++) {
         Poly& p0 = polys[n];
         p0.nverts            = 4;
         p0.material          = mtl_ring;

         p0.verts[0]          = m + 2*j;
         p0.verts[1]          = m + 2*j + 1;
         p0.verts[2]          = m + ((2*j + 3) % (rsections*2));
         p0.verts[3]          = m + ((2*j + 2) % (rsections*2));

         surface->AddIndices(6);

         n++;
      }

      // edge of ring:
      // first vertex in edge of ring, after bottom ring verts
      m = ring_verts + 4*rsections;

      for (j = 0; j < rsections; j++) {
         Poly& p0 = polys[n];
         p0.nverts            = 4;
         p0.material          = mtl_ring;

         p0.verts[3]          = m + 2*j;
         p0.verts[2]          = m + 2*j + 1;
         p0.verts[1]          = m + ((2*j + 3) % (rsections*2));
         p0.verts[0]          = m + ((2*j + 2) % (rsections*2));

         surface->AddIndices(6);

         n++;
      }
   }

   // then assign them to cohesive segments:
   Segment* segment = 0;

   for (n = 0; n < npolys; n++) {
      Poly& poly  = polys[n];
      poly.plane  = Plane(vset->loc[poly.verts[0]],
                          vset->loc[poly.verts[2]],
                          vset->loc[poly.verts[1]]);

      if (segment && segment->material == polys[n].material) {
         segment->npolys++;
      }
      else {
         segment = new(__FILE__,__LINE__) Segment;

         segment->npolys   = 1;
         segment->polys    = &polys[n];
         segment->material = segment->polys->material;

         surface->GetSegments().append(segment);
      }
   }

   model->AddSurface(surface);
}


int
PlanetRep::CheckRayIntersection(Point Q, Point w, double len, Point& ipt,
                                bool treat_translucent_polys_as_solid)
{
   // compute leading edge of ray:
   Point  dst = Q + w*len;

   // check right angle spherical distance:
   Point  d0 = loc - Q;
   Point  d1 = d0.cross(w);
   double dlen = d1.length();          // distance of point from line
   
   if (dlen > body_rad)                // clean miss
      return 0;                        // (no impact)

   // possible collision course...
   Point d2     = Q + w * (d0 * w);

   // so check the leading edge:
   Point delta0 = dst - loc;

   if (delta0.length() > radius) {
      // and the endpoints:
      Point delta1 = d2  - Q;
      Point delta2 = dst - Q;

      // if d2 is not between Q and dst, we missed:
      if (delta1 * delta2 < 0 ||
          delta1.length() > delta2.length()) {
         return 0;
      }
   }

   return 1;
}

void
PlanetRep::SetDaytime(bool d)
{
   daytime = d;

   if (daytime) {
      if (mtl_surf)  mtl_surf->blend = Material::MTL_ADDITIVE;
      if (mtl_ring)  mtl_ring->blend = Material::MTL_ADDITIVE;
   }

   else {
      if (mtl_surf)  mtl_surf->blend = Material::MTL_SOLID;
      if (mtl_ring)  mtl_ring->blend = Material::MTL_TRANSLUCENT;
   }
}

void
PlanetRep::SetStarSystem(StarSystem* system)
{
   star_system = system;
}

// +--------------------------------------------------------------------+

void
PlanetRep::Render(Video* video, DWORD flags)
{
   Solid::Render(video, flags);

   /***
    *** DEBUG
    ***

   Matrix orient  = Orientation();
   orient.Transpose();

   video->SetObjTransform(orient, Location());

   Surface* surf  = model->GetSurfaces().first();
   Poly*    polys = surf->GetPolys();

   for (int i = 0; i < 5; i++)
      video->DrawPolyOutline(polys + i);
   /***/
}

