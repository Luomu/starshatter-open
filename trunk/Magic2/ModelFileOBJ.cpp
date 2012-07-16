/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelFileOBJ.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    File loader for Wavefront/OBJ format models
*/

#include "stdafx.h"
#include "Magic.h"
#include "MagicDoc.h"
#include "ModelFileOBJ.h"

#include "Bitmap.h"
#include "Polygon.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

ModelFileOBJ::ModelFileOBJ(const char* fname)
   : ModelFile(fname)
{
}

ModelFileOBJ::~ModelFileOBJ()
{
}

// +--------------------------------------------------------------------+

const int MAX_OBJ_FACE_VERTS = 32;

struct ObjFace {
   int v[MAX_OBJ_FACE_VERTS];
   int n[MAX_OBJ_FACE_VERTS];
   int t[MAX_OBJ_FACE_VERTS];

   int nverts;

   ObjFace() {
      nverts = 0;

      for (int i = 0; i < MAX_OBJ_FACE_VERTS; i++)
         v[i] = n[i] = t[i] = 0;
   }

   ObjFace(const ObjFace& f) {
      nverts = f.nverts;

      for (int i = 0; i < MAX_OBJ_FACE_VERTS; i++) {
         v[i] = f.v[i];
         n[i] = f.n[i];
         t[i] = f.t[i];
      }
   }

   void ReverseOrder() {
      int i, tmp[MAX_OBJ_FACE_VERTS];

      for (i = 0; i < nverts; i++)  tmp[i] = v[i];
      for (i = 0; i < nverts; i++)  v[i] = tmp[nverts-1-i];

      for (i = 0; i < nverts; i++)  tmp[i] = n[i];
      for (i = 0; i < nverts; i++)  n[i] = tmp[nverts-1-i];

      for (i = 0; i < nverts; i++)  tmp[i] = t[i];
      for (i = 0; i < nverts; i++)  t[i] = tmp[nverts-1-i];
   }
};

static void ParsePoly(const char* line, ObjFace* face)
{
   int v[MAX_OBJ_FACE_VERTS];
   int n[MAX_OBJ_FACE_VERTS];
   int t[MAX_OBJ_FACE_VERTS];

   for (int i = 0; i < MAX_OBJ_FACE_VERTS; i++) {
      v[i] = n[i] = t[i] = 0;
   }

   const char* p = line + 1;

   while (isspace(*p))
      p++;

   int i = 0;
   while (*p && i < MAX_OBJ_FACE_VERTS) {
      int factor = 1;

      if (*p == '-') {
         factor = -1;
         p++;
      }

      while (isdigit(*p)) {
         v[i] = v[i]*10 + *p - '0';
         p++;
      }
      v[i] *= factor;

      if (*p == '/') { p++; // slash one

      factor = 1;

      if (*p == '-') {
         factor = -1;
         p++;
      }

      while (isdigit(*p)) {
         t[i] = t[i]*10 + *p - '0';
         p++;
      }
      t[i] *= factor;

      if (*p == '/') { p++; // slash two

      factor = 1;

      if (*p == '-') {
         factor = -1;
         p++;
      }

      while (isdigit(*p)) {
         n[i] = n[i]*10 + *p - '0';
         p++;
      }
      n[i] *= factor;
      }}

      while (isspace(*p)) p++;
      i++;
   }

   face->nverts = i;

   for (i = 0; i < MAX_OBJ_FACE_VERTS; i++) {
      face->v[i] = v[i];
      face->n[i] = n[i];
      face->t[i] = t[i];
   }
}

static int LoadMatls(const char* lpszPathName, Model* m)
{
   int nmatls = 0;

   FILE* fp = fopen(lpszPathName, "r");
   if (!fp) {
      ::MessageBox(0, "Open Failed:  could not open file", "ERROR", MB_OK);
      return 0;
   }

   Material* mtl = 0;

   while (!feof(fp)) {
      char raw_line[512];
      char line[512];
      fgets(raw_line, 512, fp);

      strcpy(line, Text(raw_line).trim().data());

      if (strstr(line, "newmtl")) {
         mtl = new Material;
         strncpy(mtl->name, line+7, Material::NAMELEN - 1);

         m->GetMaterials().append(mtl);
      }

      else if (line[0] == 'K' && line[1] == 'a') {
         float r,g,b;
         sscanf(line, "Ka %f %f %f", &r, &g, &b);
         mtl->Ka = ColorValue(r,g,b);

         mtl->ambient_value  = 1.0f;
         mtl->ambient_color  = mtl->Ka.ToColor();
      }

      else if (line[0] == 'K' && line[1] == 'd') {
         float r,g,b;
         sscanf(line, "Kd %f %f %f", &r, &g, &b);
         mtl->Kd = ColorValue(r,g,b);

         mtl->diffuse_value  = 1.0f;
         mtl->diffuse_color  = mtl->Kd.ToColor();
      }

      else if (line[0] == 'K' && line[1] == 's') {
         float r,g,b;
         sscanf(line, "Ks %f %f %f", &r, &g, &b);
         mtl->Ks = ColorValue(r,g,b);

         mtl->specular_value = 1.0f;
         mtl->specular_color = mtl->Ks.ToColor();
      }

      else if (line[0] == 'N' && line[1] == 's') {
         float ns;
         sscanf(line, "Ns %f", &ns);
         mtl->power = ns;
      }

      else if (strstr(line, "map_Kd")) {
         const char* src = strstr(line, "map_Kd") + 7;
         while (!isalnum(*src)) src++;

         LoadTexture(src, mtl->tex_diffuse, Bitmap::BMP_SOLID);
      }
   }

   fclose(fp);
   return nmatls;
}

static Material* FindMatl(const char* mtl_name, Model* model)
{
   ListIter<Material> iter = model->GetMaterials();
   while (++iter) {
      Material* m = iter.value();
      if (!strcmp(m->name, mtl_name))
         return m;
   }

   return 0;
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

bool
ModelFileOBJ::Load(Model* m, double scale)
{
   if (m && scale > 0 && strlen(filename) > 0) {
      ModelFile::Load(m, scale);

      FILE* fp = fopen(filename, "rb");

	  if (fp == NULL) {
         ::MessageBox(0, "Wavefront/OBJ Import Failed: Unable to open file", "ERROR", MB_OK);
         return false;
      }

      // ok, now start reading the data:
      int ntex   = 0;
      int nverts = 0;
      int npv    = 0;
      int npolys = 0;
      int vi     = 0;
      int vn     = 0;
      int vt     = 0;

      char root_path[256];
      ZeroMemory(root_path, 256);

      if (strrchr(filename, '\\')) {
         strcpy(root_path, filename);
         char* p = strrchr(root_path, '\\');
         if (p)
            *(p+1) = 0;
      }

      else if (strrchr(filename, '/')) {
         strcpy(root_path, filename);
         char* p = strrchr(root_path, '/');
         if (p)
            *(p+1) = 0;
      }

      // count verts and polys:
      while (!feof(fp)) {
         char line[256];
         fgets(line, 255, fp);

         if (line[0] == 'v') {
            switch (line[1]) {
            case ' ':   vi++; break;
            case 'n':   vn++; break;
            case 't':   vt++; break;
            }
         }

         else if (line[0] == 'f' && line[1] == ' ') {
            npolys++;

            ObjFace f;
            ParsePoly(line, &f);
            f.ReverseOrder();

            npv += f.nverts;
         }

         else if (strstr(line, "mtllib")) {
            const char* libname = strstr(line, "mtllib");
            libname += 7;
            while (isspace(*libname))
               libname++;

            char libpath[256];
            strcpy(libpath, root_path);
            strcat(libpath, libname);
            int n = strlen(libpath);
			if (n > 0) {
				char* p = &libpath[n-1];
				while (isspace(*p)) *p-- = 0;
			}

            int nmatls = LoadMatls(libpath, model);
         }
      }

      nverts = npv;
      if (vi > nverts)  nverts = vi;
      if (vn > nverts)  nverts = vn;
      if (vt > nverts)  nverts = vt;

      if (nverts > Model::MAX_VERTS || npolys > Model::MAX_POLYS) {
         ::MessageBox(0, "Wavefront/OBJ Import Failed: that model is just too darn complicated!", "ERROR", MB_OK);
         return false;
      }

      vi = 0;
      vn = 0;
      vt = 0;

      fseek(fp, 0, SEEK_SET);

      Surface* surface = new Surface;
      m->GetSurfaces().append(surface);

      surface->CreateVerts(nverts);
      surface->CreatePolys(npolys);

      VertexSet*  vset  = surface->GetVertexSet();
      Poly*       polys = surface->GetPolys();

      // read vertex set:
      Vec3*  vloc = new Vec3[nverts];
      Vec3*  vnrm = new Vec3[nverts];
      float* vtu  = new float[nverts];
      float* vtv  = new float[nverts];

      float  radius = 0;

      while (!feof(fp)) {
         char line[256];
         fgets(line, 255, fp);

         if (line[0] == 'v') {
            if (line[1] == ' ') {
               const char* p = line + 2;

               while (isspace(*p)) p++;
               sscanf(p, "%f", &vloc[vi].x);

               while (!isspace(*p)) p++;
               while (isspace(*p))  p++;
               sscanf(p, "%f", &vloc[vi].y);

               while (!isspace(*p)) p++;
               while (isspace(*p))  p++;
               sscanf(p, "%f", &vloc[vi].z);

               float d = vloc[vi].length();
               if (d > radius)
                  radius = d;

               vi++;
            }

            else if (line[1] == 'n') {
               const char* p = line + 2;

               while (isspace(*p)) p++;
               sscanf(p, "%f", &vnrm[vn].x);

               while (!isspace(*p)) p++;
               while (isspace(*p))  p++;
               sscanf(p, "%f", &vnrm[vn].y);

               while (!isspace(*p)) p++;
               while (isspace(*p))  p++;
               sscanf(p, "%f", &vnrm[vn].z);

               vn++;
            }

            else if (line[1] == 't') {
               const char* p = line + 2;

               while (isspace(*p)) p++;
               sscanf(p, "%f", &vtu[vt]);

               while (!isspace(*p)) p++;
               while (isspace(*p))  p++;
               sscanf(p, "%f", &vtv[vt]);

               vtv[vt] = 1.0f - vtv[vt];

               vt++;
            }
         }
      }

      fseek(fp, 0, SEEK_SET);

      // read polys:
      int         poly = 0;
      char        line[256];
      ObjFace     face;
      Material*   material   = 0;
      int         mtl_index  = 0;

      if (m->NumMaterials())
         material = m->GetMaterials().first();

      int         current_v  = 1;
      int         current_vn = 1;
      int         current_vt = 1;
      int         v          = 0;   // vset index pointer

      while (!feof(fp)) {
         char raw_line[256];
         fgets(raw_line, 256, fp);

         strcpy(line, Text(raw_line).trim().data());

         if (strstr(line, "usemtl")) {
            material = FindMatl(line + 7, model);
            ListIter<Material> iter = model->GetMaterials();
            while (++iter)
               if (material == iter.value())
                  mtl_index = iter.index();
         }

         else if (line[0] == 'v') {
            if (line[1] == ' ')        current_v++;
            else if (line[1] == 'n')   current_vn++;
            else if (line[1] == 't')   current_vt++;
         }

         else if (line[0] == 'f') {
            ParsePoly(line, &face);
            face.ReverseOrder();

            for (int n = 0; n < face.nverts; n++) {
               if (face.v[n] < 0)
                  face.v[n] += current_v;

               if (face.n[n] < 0)
                  face.n[n] += current_vn;

               if (face.t[n] < 0)
                  face.t[n] += current_vt;
            }

            if (face.nverts > 4) {
               npolys += face.nverts - 3;

               for (int tri = 2; tri < face.nverts; tri++) {
                  Poly* p = polys + poly;
                  poly++;

                  p->nverts = 3;

                  vset->loc[v+0] = vloc[face.v[ tri ]   -1];
                  vset->loc[v+1] = vloc[face.v[ tri-1 ] -1];
                  vset->loc[v+2] = vloc[face.v[ 0]      -1];

                  if (face.n[0] > 0) {
                     vset->nrm[v+0] = vnrm[face.n[ tri ]   -1];
                     vset->nrm[v+1] = vnrm[face.n[ tri-1 ] -1];
                     vset->nrm[v+2] = vnrm[face.n[ 0]      -1];
                  }
                  else {
                     vset->nrm[v+0] = vloc[v+0];   vset->nrm[v+0].Normalize();
                     vset->nrm[v+1] = vloc[v+1];   vset->nrm[v+1].Normalize();
                     vset->nrm[v+2] = vloc[v+2];   vset->nrm[v+2].Normalize();
                  }

                  if (face.t[0] > 0) {
                     vset->tu[v+0]  = vtu[face.t[ tri ]   -1];
                     vset->tv[v+0]  = vtv[face.t[ tri ]   -1];
                     vset->tu[v+1]  = vtu[face.t[ tri-1 ] -1];
                     vset->tv[v+1]  = vtv[face.t[ tri-1 ] -1];
                     vset->tu[v+2]  = vtu[face.t[ 0 ]     -1];
                     vset->tv[v+2]  = vtv[face.t[ 0 ]     -1];
                  }

                  p->verts[0] = v+0;
                  p->verts[1] = v+1;
                  p->verts[2] = v+2;

                  p->material    = material;
                  p->sortval     = mtl_index;
                  p->vertex_set  = vset;

                  p->plane = Plane(vset->loc[p->verts[0]],
                                   vset->loc[p->verts[2]],
                                   vset->loc[p->verts[1]]);

                  v += p->nverts;
               }
            }

            else if (face.nverts == 3 || face.nverts == 4) {
               Poly* p = polys + poly;
               poly++;

               p->nverts = face.nverts;

               bool flat  = true;
               int  first = v;

               for (int i = 0; i < p->nverts; i++) {
                  int face_index = i;

                  vset->loc[v] = vloc[face.v[face_index]-1];

                  if (face.n[face_index] > 0)
                     vset->nrm[v] = vnrm[face.n[face_index]-1];
                  else
                     vset->nrm[v] = vset->loc[v];

                  vset->nrm[v].Normalize();

                  if (vset->nrm[v] != vset->nrm[first])
                     flat = false;

                  if (face.t[face_index] > 0) {
                     vset->tu[v]  = vtu [face.t[face_index]-1];
                     vset->tv[v]  = vtv [face.t[face_index]-1];
                  }

                  p->verts[i] = v++;
               }

               p->material = material;
               p->sortval  = mtl_index;
               p->flatness = flat ? 1.0f : 0.0f;

               if (p->nverts == 3)
                  surface->AddIndices(3);

               else if (p->nverts == 4)
                  surface->AddIndices(6);

               p->vertex_set = vset;
               p->plane = Plane(vset->loc[p->verts[0]],
                                vset->loc[p->verts[2]],
                                vset->loc[p->verts[1]]);
            }

            if (poly >= npolys)
               break;
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

            surface->GetSegments().append(segment);
         }
      }

      delete [] vloc;
      delete [] vnrm;
      delete [] vtu;
      delete [] vtv;

      *pnverts = nverts;
      *pnpolys = npolys;
      *pradius = radius;

      fclose(fp);

      m->Normalize();
      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

static bool
SaveWaveFrontMatLib(const char* path, const char* root, Model* model)
{
   char filename[256];
   sprintf(filename, "%s%s.mtl", path, root);

   FILE* f = fopen(filename, "w");
   if (f) {
      fprintf(f, "#\n# %s Material Library exported by Magic 2.0\n#\n\n", root);

      ListIter<Material> iter = model->GetMaterials();
      while (++iter) {
         Material* mtl = iter.value();

         fprintf(f, "newmtl %s\n", mtl->name);
         fprintf(f, "Ka %.5f %.5f %.5f\n", mtl->Ka.Red(), mtl->Ka.Green(), mtl->Ka.Blue());
         fprintf(f, "Kd %.5f %.5f %.5f\n", mtl->Kd.Red(), mtl->Kd.Green(), mtl->Kd.Blue());
         fprintf(f, "Ks %.5f %.5f %.5f\n", mtl->Ks.Red(), mtl->Ks.Green(), mtl->Ks.Blue());
         fprintf(f, "Ns %.5f\n",           mtl->power);
         fprintf(f, "illum 2\n");
         if (mtl->tex_diffuse)
         fprintf(f, "map_Kd %s\n", mtl->tex_diffuse->GetFilename());
         fprintf(f, "\n");
      }

      fclose(f);
      return true;
   }

   return false;
}

bool
ModelFileOBJ::Save(Model* m)
{
   if (m) {
      ModelFile::Save(m);
      char pathname[256];
      char rootname[256];

      ZeroMemory(pathname, sizeof(pathname));
      ZeroMemory(rootname, sizeof(rootname));

      const char* ext = strstr(filename, ".obj");
      if (!ext)
         ext = strstr(filename, ".OBJ");

      const char* sep = strrchr(filename, '/');
      if (!sep)
         sep = strrchr(filename, '\\');

      const char* src = filename;
      char*       dst = pathname;

      if (sep) {
         while (src != sep)
            *dst++ = *src++;
         *dst++ = *src++;
      }

      if (ext) {
         dst = rootname;
         while (src != ext)
            *dst++ = *src++;
      }
      else {
         strcpy(rootname, src);
      }

      strcpy(filename, pathname);
      strcat(filename, rootname);
      strcat(filename, ".obj");

      FILE* f = fopen(filename, "w");
      if (!f) {
         ::MessageBox(0, "Export Failed: Magic could not open the file for writing", "ERROR", MB_OK);
         return false;
      }

      fprintf(f, "# Wavefront OBJ exported by Magic 2.0\n\n");
      fprintf(f, "mtllib %s.mtl\n", rootname);

      ListIter<Surface> s_iter = m->GetSurfaces();

      // vertex locations
      fprintf(f, "\n# VERTEX LOCATIONS: %d\n", m->NumVerts());
      while (++s_iter) {
         Surface*    s    = s_iter.value();
         VertexSet*  vset = s->GetVertexSet();

         for (int n = 0; n < vset->nverts; n++) {
            fprintf(f, "v  %12.5f %12.5f %12.5f\n",
               vset->loc[n].x,
               vset->loc[n].y,
               vset->loc[n].z);
         }
      }

      s_iter.reset();

      // vertex normals
      fprintf(f, "\n# VERTEX NORMALS: %d\n", m->NumVerts());
      while (++s_iter) {
         Surface*    s    = s_iter.value();
         VertexSet*  vset = s->GetVertexSet();

         for (int n = 0; n < vset->nverts; n++) {
            fprintf(f, "vn %8.3f %8.3f %8.3f\n",
               vset->nrm[n].x,
               vset->nrm[n].y,
               vset->nrm[n].z);
         }
      }

      s_iter.reset();

      // texture coordinates
      fprintf(f, "\n# TEXTURE COORDINATES: %d\n", m->NumVerts());
      while (++s_iter) {
         Surface*    s    = s_iter.value();
         VertexSet*  vset = s->GetVertexSet();

         for (int n = 0; n < vset->nverts; n++) {
            fprintf(f, "vt %8.3f %8.3f\n",
               vset->tu[n], 1 - vset->tv[n]);
         }
      }

      s_iter.reset();

      // faces
      Material* current_material = 0;

      fprintf(f, "\n# FACES: %d\n", m->NumPolys());
      while (++s_iter) {
         Surface*    s    = s_iter.value();

         for (int n = 0; n < s->NumPolys(); n++) {
            const Poly* p     = s->GetPolys() + n;
            int         nv    = p->nverts;
            Material*   mtl   = p->material;

            if (current_material != mtl) {
               fprintf(f, "\n\nusemtl %s\n", mtl->name);
               current_material = mtl;
            }

            fprintf(f, "\nf ");
            for (int v = nv-1; v >= 0; v--) {
               fprintf(f, "%d/%d/%d ",
                  p->verts[v] + 1,
                  p->verts[v] + 1,
                  p->verts[v] + 1);
            }
         }
      }

      fprintf(f, "\n\n\n# END OF FILE.\n");
      fclose(f);

      return SaveWaveFrontMatLib(pathname, rootname, m);
   }

   return false;
}

// +--------------------------------------------------------------------+
