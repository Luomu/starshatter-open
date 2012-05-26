/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelFileMAG.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    File loader for MAG format models
*/

#include "stdafx.h"
#include "Magic.h"
#include "MagicDoc.h"
#include "ModelFileMAG.h"

#include "Bitmap.h"
#include "Polygon.h"
#include "List.h"

// +--------------------------------------------------------------------+

struct MaterialMag6 {
   char        name[Material::NAMELEN];
   char        shader[Material::NAMELEN];
   float       power;      // highlight sharpness (big=shiny)
   float       brilliance; // diffuse power function
   float       bump;       // bump level (0=none)
   DWORD       blend;      // alpha blend type
   bool        shadow;     // material casts shadow
   bool        luminous;   // verts have their own lighting

   Color       ambient_color;
   Color       diffuse_color;
   Color       specular_color;
   Color       emissive_color;

   float       ambient_value;
   float       diffuse_value;
   float       specular_value;
   float       emissive_value;

   BYTE        tex_diffuse;
   BYTE        tex_specular;
   BYTE        tex_bumpmap;
   BYTE        tex_emissive;
};

// +--------------------------------------------------------------------+

ModelFileMAG::ModelFileMAG(const char* fname)
   : ModelFile(fname)
{
}

ModelFileMAG::~ModelFileMAG()
{
}

// +--------------------------------------------------------------------+

bool
ModelFileMAG::Load(Model* m, double scale)
{
   if (m && scale > 0 && strlen(filename) > 0) {
      ModelFile::Load(m, scale);

      bool        result = false;
      FILE*       fp     = fopen(filename, "rb");

      // check MAG file:
      if (!fp) {
         ::MessageBox(0, "File Open Failed:\nMagic could not open the requested file.", "ERROR", MB_OK);
         return result;
      }

      ZeroMemory(pname, 64);
      strncpy(pname, filename, 63);

      char file_id[5];
      fread(file_id, 4, 1, fp);
      file_id[4] = '\0';
      int version = 1;

      if (!strcmp(file_id, "MAG6")) {
         version = 6;
      }
      else if (!strcmp(file_id, "MAG5")) {
         version = 5;
      }
      else if (!strcmp(file_id, "MAG4")) {
         version = 4;
      }
      else {
         ::MessageBox(0, "File Open Failed:\nThe requested file uses an invalid format.", "ERROR", MB_OK);
         fclose(fp);
         return result;
      }

      // get ready to load, delete existing model:
      m->GetSurfaces().destroy();
      m->GetMaterials().destroy();
      *pnverts   = 0;
      *pnpolys   = 0;

      // now load the model:
      switch (version) {
      case 4:
      case 5:
         result = LoadMag5(fp, m, scale);
         break;

      case 6:
         result = LoadMag6(fp, m, scale);
         break;

      default:
         break;
      }

      fclose(fp);
      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

bool
ModelFileMAG::Save(Model* m)
{
   if (m) {
      ModelFile::Save(m);

      FILE* fp = fopen(filename, "wb");
      if (!fp) {
         ::MessageBox(0, "Save Failed:\nMagic could not open the file for writing.", "ERROR", MB_OK);
         return FALSE;
      }

      fwrite("MAG6", 4, 1, fp);

      int            i      = 0;
      int            ntex   = 0;
      int            nmtls  = 0;
      int            nsurfs = m->NumSurfaces();
      List<Bitmap>   textures;

      ListIter<Material> m_iter = m->GetMaterials();
      while (++m_iter) {
         Material* mtl = m_iter.value();
         Bitmap*   bmp = mtl->tex_diffuse;

         if (bmp && !textures.contains(bmp)) {
            textures.append(bmp);
         }

         bmp = mtl->tex_specular;

         if (bmp && !textures.contains(bmp)) {
            textures.append(bmp);
         }

         bmp = mtl->tex_emissive;

         if (bmp && !textures.contains(bmp)) {
            textures.append(bmp);
         }

         bmp = mtl->tex_bumpmap;

         if (bmp && !textures.contains(bmp)) {
            textures.append(bmp);
         }

         nmtls++;
      }

      ListIter<Bitmap> t_iter = textures;
      while (++t_iter) {
         Bitmap* bmp = t_iter.value();
         ntex += strlen(bmp->GetFilename()) + 1;
      }

      nsurfs = m->GetSurfaces().size();

      fwrite(&ntex,   4, 1, fp);
      fwrite(&nmtls,  4, 1, fp);
      fwrite(&nsurfs, 4, 1, fp);

      if (ntex) {
         t_iter.reset();
         while (++t_iter) {
            Bitmap* bmp = t_iter.value();

            fwrite(bmp->GetFilename(),
                   strlen(bmp->GetFilename()) + 1,
                   1,
                   fp);
         }
      }

      if (nmtls) {
         m_iter.reset();
         while (++m_iter) {
            Material*      mtl = m_iter.value();
            MaterialMag6   m6;

            ZeroMemory(&m6, sizeof(m6));

            CopyMemory(m6.name,   mtl->name,   Material::NAMELEN);
            CopyMemory(m6.shader, mtl->shader, Material::NAMELEN);

            m6.ambient_value  = mtl->ambient_value;
            m6.ambient_color  = mtl->ambient_color;
            m6.diffuse_value  = mtl->diffuse_value;
            m6.diffuse_color  = mtl->diffuse_color;
            m6.specular_value = mtl->specular_value;
            m6.specular_color = mtl->specular_color;
            m6.emissive_value = mtl->emissive_value;
            m6.emissive_color = mtl->emissive_color;

            m6.power          = mtl->power;
            m6.brilliance     = mtl->brilliance;
            m6.bump           = mtl->bump;
            m6.blend          = mtl->blend;
            m6.shadow         = mtl->shadow;
            m6.luminous       = mtl->luminous;

            if (mtl->tex_diffuse)
               m6.tex_diffuse  = textures.index(mtl->tex_diffuse) + 1;

            if (mtl->tex_specular)
               m6.tex_specular = textures.index(mtl->tex_specular) + 1;

            if (mtl->tex_emissive)
               m6.tex_emissive = textures.index(mtl->tex_emissive) + 1;

            if (mtl->tex_bumpmap)
               m6.tex_bumpmap  = textures.index(mtl->tex_bumpmap) + 1;

            fwrite(&m6, sizeof(m6), 1, fp);
         }
      }

      ListIter<Surface> s_iter = m->GetSurfaces();
      while (++s_iter) {
         Surface* s = s_iter.value();

         int   nverts  = s->NumVerts();
         int   npolys  = s->NumPolys();
         BYTE  namelen = strlen(s->Name()) + 1;

         fwrite(&nverts,   4, 1,       fp);
         fwrite(&npolys,   4, 1,       fp);
         fwrite(&namelen,  1, 1,       fp);
         fwrite(s->Name(), 1, namelen, fp);

         VertexSet*  vset  = s->GetVertexSet();
         Poly*       polys = s->GetPolys();

         // write vertex set:
         for (int v = 0; v < nverts; v++) {
            fwrite(&vset->loc[v],         sizeof(float), 3, fp);
            fwrite(&vset->nrm[v],         sizeof(float), 3, fp);
            fwrite(&vset->tu[v],          sizeof(float), 1, fp);
            fwrite(&vset->tv[v],          sizeof(float), 1, fp);
         }

         // write polys:
         for (int n = 0; n < npolys; n++) {
            Poly& poly           = polys[n];
            BYTE  poly_nverts    = (BYTE) poly.nverts;
            BYTE  material_index = 0;
            WORD  poly_verts[8];

            m_iter.reset();
            while (++m_iter && !material_index) {
               if (poly.material == m_iter.value())
                  material_index = m_iter.index() + 1;
            }

            for (int i = 0; i < poly_nverts; i++) {
               poly_verts[i] = poly.verts[i];
            }

            fwrite(&poly_nverts,      sizeof(BYTE),  1, fp);
            fwrite(&material_index,   sizeof(BYTE),  1, fp);
            fwrite(&poly_verts[0],    sizeof(WORD),  poly_nverts, fp);
         }
      }

      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

struct HomogenousPlane
{
   double distance;
   double normal_x;
   double normal_y;
   double normal_z;
   double normal_w;
};

static void LoadPlane(Plane& p, FILE* fp)
{
   HomogenousPlane tmp;
   fread(&tmp, sizeof(HomogenousPlane), 1, fp);
}

static void LoadFlags(LPDWORD flags, FILE* fp)
{
   DWORD magic_flags;
   fread(&magic_flags, sizeof(DWORD), 1, fp);

   /** MAGIC FLAGS
   enum { FLAT_SHADED =   1,
          LUMINOUS    =   2,
          TRANSLUCENT =   4,  \\ must swap
          CHROMAKEY   =   8,  // these two
          FOREGROUND  =  16,  -- not used
          WIREFRAME   =  32,  -- not used
          SPECULAR1   =  64,
          SPECULAR2   = 128  };
   ***/

   const DWORD magic_mask = 0x0fc3;

   *flags = magic_flags & magic_mask;
}

// +--------------------------------------------------------------------+

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

bool
ModelFileMAG::LoadMag5(FILE* fp, Model* m, double scale)
{
   bool        result = false;
   int         ntex   = 0;
   int         nsurfs = 0;
   double      radius = 0;

   fread(&ntex,   sizeof(ntex),   1, fp);
   fread(&nsurfs, sizeof(nsurfs), 1, fp);

   // create a default gray material:
   Material*   mtl = new Material;

   if (mtl) {
      mtl->Ka = Color::DarkGray;
      mtl->Kd = Color::LightGray;
      mtl->Ks = ColorValue(0.1f,0.1f,0.1f);
      mtl->power = 10.0f;

      mtl->ambient_value  = 0.2f;
      mtl->ambient_color  = Color::DarkGray;
      mtl->diffuse_value  = 0.8f;
      mtl->diffuse_color  = Color::LightGray;
      mtl->specular_value = 0.5f;
      mtl->specular_color = Color::White;
      strcpy_s(mtl->name, "(default)");

      m->GetMaterials().append(mtl);
   }

   // read texture list:
   for (int i = 0; i < ntex; i++) {
      Material*   mtl = new Material;
      char        tname[32];

      if (mtl) {
         mtl->Ka = ColorValue(0.5f,0.5f,0.5f);
         mtl->Kd = ColorValue(1.0f,1.0f,1.0f);
         mtl->Ks = ColorValue(0.2f,0.2f,0.2f);
         mtl->power = 20.0f;

         mtl->ambient_value  = 1.0f;
         mtl->ambient_color  = Color::Gray;
         mtl->diffuse_value  = 1.0f;
         mtl->diffuse_color  = Color::White;
         mtl->specular_value = 0.2f;
         mtl->specular_color = Color::White;

         fread(tname, 32, 1, fp);
         LoadTexture(tname, mtl->tex_diffuse, Bitmap::BMP_SOLID);
         strcpy_s(mtl->name, tname);

         char* dot = strrchr(mtl->name, '.');
         if (dot)
            *dot = 0;

         char* plus = strrchr(mtl->name, '+');
         if (plus)
            *plus = 0;

         m->GetMaterials().append(mtl);
      }
   }

   int nverts = 0;
   int npolys = 0;

   fread(&nverts, 4, 1, fp);
   fread(&npolys, 4, 1, fp);

   // plan on creating four verts per poly:
   int mag_nverts = nverts;
   int next_vert  = nverts;

   nverts            = npolys * 4;
   Surface*    s     = new Surface;
   VertexSet*  vset  = 0;
   Poly*       polys = 0;

   if (s) {
      s->SetName("default");
      s->CreateVerts(nverts);
      s->CreatePolys(npolys);

      vset  = s->GetVertexSet();
      polys = s->GetPolys();

      ZeroMemory(vset->loc,      nverts * sizeof(Vec3));
      ZeroMemory(vset->diffuse,  nverts * sizeof(DWORD));
      ZeroMemory(vset->specular, nverts * sizeof(DWORD));
      ZeroMemory(vset->tu,       nverts * sizeof(float));
      ZeroMemory(vset->tv,       nverts * sizeof(float));
      ZeroMemory(vset->rw,       nverts * sizeof(float));

      // read vertex set:
	  int v;
      for (v = 0; v < mag_nverts; v++) {
         Vec3 vert, norm;
         DWORD vstate;

         fread(&vert,    sizeof(Vec3), 1, fp);
         fread(&norm,    sizeof(Vec3), 1, fp);
         fread(&vstate,  sizeof(DWORD), 1, fp);

         vert.SwapYZ();
         vert *= (float) scale;

         norm.SwapYZ();

         vset->loc[v]      = vert;
         vset->nrm[v]      = norm;

         double d = vert.length();
         if (d > radius)
            radius = (float) d;
      }

      while (v < nverts)
         vset->nrm[v++] = Vec3(1,0,0);

      // read polys:
      Vec3  dummy_center;
      DWORD dummy_flags;
      DWORD dummy_color;
      int   texture_num;
      int   poly_nverts;
      int   vert_index_buffer[32];
      float texture_index_buffer[32];

      for (int n = 0; n < npolys; n++) {
         Poly& poly = polys[n];
         poly.vertex_set  = vset;

         fread(&dummy_flags,   sizeof(DWORD), 1, fp);
         fread(&dummy_center,  sizeof(Vec3),  1, fp);
         LoadPlane(poly.plane, fp);
         fread(&dummy_color,   sizeof(DWORD), 1, fp);
         fread(&texture_num,   sizeof(int),   1, fp);

         if (texture_num >= 0 && texture_num < ntex) {
            texture_num++;

            poly.material = m->GetMaterials()[texture_num];
            poly.sortval  = texture_num;

            if (dummy_flags & 2) { // luminous
               Material* mtl = m->GetMaterials()[texture_num];

               mtl->ambient_value  = 0.0f;
               mtl->ambient_color  = Color::Black;
               mtl->diffuse_value  = 0.0f;
               mtl->diffuse_color  = Color::Black;
               mtl->specular_value = 0.0f;
               mtl->specular_color = Color::Black;
               mtl->emissive_value = 1.0f;
               mtl->emissive_color = Color::White;

               mtl->Ka = ColorValue(0,0,0,0);
               mtl->Kd = ColorValue(0,0,0,0);
               mtl->Ks = ColorValue(0,0,0,0);
               mtl->Ke = ColorValue(1,1,1,1);

               mtl->tex_emissive = mtl->tex_diffuse;
            }
         }
         else {
            poly.material = m->GetMaterials().first(); // default material
            poly.sortval  = 1000;
         }

         // hack: store flat shaded flag in unused visible byte
         poly.visible = (BYTE) (dummy_flags & 1);

         fread(&poly_nverts,           sizeof(int),   1, fp);
         fread(vert_index_buffer,      sizeof(int),   poly_nverts, fp);

         if (poly_nverts == 3)
            s->AddIndices(3);

         else if (poly_nverts == 4)
            s->AddIndices(6);

         poly.nverts = poly_nverts;
         for (int vi = 0; vi < poly_nverts; vi++) {
            int v = vert_index_buffer[vi];

            if (vset->rw[v] > 0) {
               vset->CopyVertex(next_vert, v);
               v = next_vert++;
            }

            vset->rw[v] = 1;
            poly.verts[vi] = v;
         }
      
         fread(texture_index_buffer, sizeof(float), poly_nverts, fp); // tu's
         for (int vi = 0; vi < poly_nverts; vi++) {
            int v = poly.verts[vi];
            vset->tu[v] = texture_index_buffer[vi];
         }

         fread(texture_index_buffer, sizeof(float), poly_nverts, fp); // tv's
         for (int vi = 0; vi < poly_nverts; vi++) {
            int v = poly.verts[vi];
            vset->tv[v] = texture_index_buffer[vi];
         }

         DWORD unused[32];
         fread(unused, 16, 1, fp);
      }

      // pass 2 (adjust vertex normals for flat polys):
      for (int n = 0; n < npolys; n++) {
         Poly& poly = polys[n];

         poly.plane = Plane(vset->loc[poly.verts[0]],
                            vset->loc[poly.verts[2]],
                            vset->loc[poly.verts[1]]);

         // hack: retrieve flat shaded flag from unused visible byte
         if (poly.visible) {
            int poly_nverts = poly.nverts;

            for (int vi = 0; vi < poly_nverts; vi++) {
               int v = poly.verts[vi];
               vset->nrm[v] = poly.plane.normal;
            }
         }
      }

      // sort the polys by material index:
      qsort((void*) polys, npolys, sizeof(Poly), mcomp);

      // then assign them to cohesive segments:
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
            segment->polys    = &polys[n];
            segment->material = segment->polys->material;

            s->GetSegments().append(segment);
         }
      }

      s->BuildHull();
      m->GetSurfaces().append(s);

      *pnverts = nverts;
      *pnpolys = npolys;
      *pradius = (float) radius;

      result = nverts && npolys;
   }

   return result;
}

// +--------------------------------------------------------------------+

bool
ModelFileMAG::LoadMag6(FILE* fp, Model* m, double scale)
{
   bool           result = false;
   int            i      = 0;
   int            ntex   = 0;
   int            nmtls  = 0;
   int            nsurfs = 0;
   double         radius = 0;
   List<Bitmap>   textures;

   fread(&ntex,   sizeof(ntex),   1, fp); // size of texture block
   fread(&nmtls,  sizeof(nmtls),  1, fp); // number of materials
   fread(&nsurfs, sizeof(nsurfs), 1, fp); // number of surfaces

   // read texture list:
   if (ntex) {
      char*          buffer      = new char[ntex];
      char*          p           = buffer;
      Bitmap*        bmp         = 0;

      fread(buffer, ntex, 1, fp);

      while (p < buffer + ntex) {
         LoadTexture(p, bmp, Bitmap::BMP_SOLID);
         textures.append(bmp);

         p += strlen(p) + 1;
      }

      delete [] buffer;
   }

   for (i = 0; i < nmtls; i++) {
      MaterialMag6   m6;
      Material*      mtl = new Material;

      fread(&m6, sizeof(m6), 1, fp);

      if (mtl) {
         CopyMemory(mtl->name,   m6.name,   Material::NAMELEN);
         CopyMemory(mtl->shader, m6.shader, Material::NAMELEN);

         mtl->ambient_value   = m6.ambient_value;
         mtl->ambient_color   = m6.ambient_color;
         mtl->diffuse_value   = m6.diffuse_value;
         mtl->diffuse_color   = m6.diffuse_color;
         mtl->specular_value  = m6.specular_value;
         mtl->specular_color  = m6.specular_color;
         mtl->emissive_value  = m6.emissive_value;
         mtl->emissive_color  = m6.emissive_color;

         mtl->Ka = ColorValue(mtl->ambient_color)  * mtl->ambient_value;
         mtl->Kd = ColorValue(mtl->diffuse_color)  * mtl->diffuse_value;
         mtl->Ks = ColorValue(mtl->specular_color) * mtl->specular_value;
         mtl->Ke = ColorValue(mtl->emissive_color) * mtl->emissive_value;

         mtl->power           = m6.power;
         mtl->brilliance      = m6.brilliance;
         mtl->bump            = m6.bump;
         mtl->blend           = m6.blend;
         mtl->shadow          = m6.shadow;
         mtl->luminous        = m6.luminous;

         if (m6.tex_diffuse && m6.tex_diffuse <= textures.size())
            mtl->tex_diffuse = textures[m6.tex_diffuse - 1];

         if (m6.tex_specular && m6.tex_specular <= textures.size())
            mtl->tex_specular = textures[m6.tex_specular - 1];

         if (m6.tex_emissive && m6.tex_emissive <= textures.size())
            mtl->tex_emissive = textures[m6.tex_emissive - 1];

         if (m6.tex_bumpmap && m6.tex_bumpmap <= textures.size())
            mtl->tex_bumpmap = textures[m6.tex_bumpmap - 1];

         m->GetMaterials().append(mtl);
      }
   }

   for (i = 0; i < nsurfs; i++) {
      int   nverts  = 0;
      int   npolys  = 0;
      BYTE  namelen = 0;
      char  name[128];

      fread(&nverts,  4, 1,       fp);
      fread(&npolys,  4, 1,       fp);
      fread(&namelen, 1, 1,       fp);
      fread(name,     1, namelen, fp);

      Surface* surface = new Surface;
      surface->SetName(name);
      surface->CreateVerts(nverts);
      surface->CreatePolys(npolys);

      VertexSet*  vset  = surface->GetVertexSet();
      Poly*       polys = surface->GetPolys();

      ZeroMemory(polys, sizeof(Poly) * npolys);

      // read vertex set:
      for (int v = 0; v < nverts; v++) {
         fread(&vset->loc[v],         sizeof(float), 3, fp);
         fread(&vset->nrm[v],         sizeof(float), 3, fp);
         fread(&vset->tu[v],          sizeof(float), 1, fp);
         fread(&vset->tv[v],          sizeof(float), 1, fp);

         double d = vset->loc[v].length();
         if (d > radius)
            radius = d;
      }

      // read polys:
      for (int n = 0; n < npolys; n++) {
         Poly& poly           = polys[n];
         BYTE  poly_nverts    = 0;
         BYTE  material_index = 0;
         WORD  poly_verts[8];

         fread(&poly_nverts,      sizeof(BYTE),  1, fp);
         fread(&material_index,   sizeof(BYTE),  1, fp);
         fread(&poly_verts[0],    sizeof(WORD),  poly_nverts, fp);

         if (poly_nverts >= 3) {
            poly.nverts = poly_nverts;

            for (int i = 0; i < poly_nverts; i++) {
               poly.verts[i] = poly_verts[i];
            }
         }
         else {
            poly.sortval = 666;
         }

         if (material_index > 0) {
            poly.material = m->GetMaterials()[material_index-1];
            poly.sortval  = material_index;
         }
         else if (m->NumMaterials()) {
            poly.material = m->GetMaterials().first();
            poly.sortval  = 1;
         }
         else {
            poly.sortval  = 1000;
         }

         if (poly.nverts == 3)
            surface->AddIndices(3);

         else if (poly.nverts == 4)
            surface->AddIndices(6);

         poly.vertex_set = vset;
         poly.plane = Plane(vset->loc[poly.verts[0]],
                            vset->loc[poly.verts[2]],
                            vset->loc[poly.verts[1]]);
      }

      // sort the polys by material index:
      qsort((void*) polys, npolys, sizeof(Poly), mcomp);

      // then assign them to cohesive segments:
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
            segment->polys    = &polys[n];
            segment->material = segment->polys->material;

            surface->GetSegments().append(segment);
         }
      }

      surface->ComputeTangents();
      surface->BuildHull();
      m->GetSurfaces().append(surface);

      *pnverts = nverts;
      *pnpolys = npolys;
      *pradius = (float) radius;

      result = nverts && npolys;
   }

   return result;
}

