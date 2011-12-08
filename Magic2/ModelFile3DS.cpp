/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelFile3DS.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    File loader for 3DStudio MAX 3DS format models
*/

#include "stdafx.h"
#include "Magic.h"
#include "MagicDoc.h"
#include "ModelFile3DS.h"

#include "Bitmap.h"
#include "Polygon.h"
#include "Text.h"
#include "List.h"
#include "l3ds.h"

// +--------------------------------------------------------------------+

ModelFile3DS::ModelFile3DS(const char* fname)
   : ModelFile(fname)
{
}

ModelFile3DS::~ModelFile3DS()
{
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
ModelFile3DS::Load(Model* model, double scale)
{
   if (model && scale > 0 && strlen(filename) > 0) {
      ModelFile::Load(model, scale);

      L3DS loader;

      if (!loader.LoadFile(filename)) {
         ::MessageBox(0, "3DS Import Failed: Magic could not open the file for reading", "ERROR", MB_OK);
         return false;
      }

      int ntex   = 0;
      int nsurfs = 0;
      int nverts = 0;
      int npolys = 0;
      int nmatls = 0;
      int nmeshs = loader.GetMeshCount();

      int*   mesh_verts = new int[nmeshs];

      for (int m = 0; m < nmeshs; m++) {
         LMesh& mesh = loader.GetMesh(m);

         mesh_verts[m] = nverts;

         // count verts and polys:
         nverts += mesh.GetVertexCount();
         npolys += mesh.GetTriangleCount();
      }

      if (nverts > Model::MAX_VERTS || npolys > Model::MAX_POLYS) {
         ::MessageBox(0, "3DS Import Failed:  model was too large (max 16000 polys)", "ERROR", MB_OK);
         delete [] mesh_verts;
         return FALSE;
      }

      // get materials:
      nmatls = loader.GetMaterialCount();

      for (int i = 0; i < nmatls; i++) {
         LMaterial& matl      = loader.GetMaterial(i);
         LMap&      tex_diff  = matl.GetTextureMap1();
         LMap&      tex_spec  = matl.GetSpecularMap();
         LMap&      tex_bump  = matl.GetBumpMap();
         LMap&      tex_glow  = matl.GetTextureMap2();

         Material*  material  = new Material;

         strncpy(material->name, matl.GetName().c_str(), Material::NAMELEN);

         material->Ka.Set( matl.GetAmbientColor().r,
                           matl.GetAmbientColor().g,
                           matl.GetAmbientColor().b );

         material->ambient_value  = 1.0f;
         material->ambient_color  = material->Ka.ToColor();

         material->Kd.Set( matl.GetDiffuseColor().r,
                           matl.GetDiffuseColor().g,
                           matl.GetDiffuseColor().b );

         material->diffuse_value  = 1.0f;
         material->diffuse_color  = material->Kd.ToColor();

         material->Ks.Set( matl.GetSpecularColor().r,
                           matl.GetSpecularColor().g,
                           matl.GetSpecularColor().b );

         material->specular_value  = 1.0f;
         material->specular_color  = material->Ks.ToColor();

         material->power = matl.GetShininess() * 100;

         if (tex_diff.mapName[0])
            LoadTexture(tex_diff.mapName, material->tex_diffuse);

         if (tex_spec.mapName[0])
            LoadTexture(tex_spec.mapName, material->tex_specular);

         if (tex_bump.mapName[0])
            LoadTexture(tex_bump.mapName, material->tex_bumpmap);

         if (tex_glow.mapName[0])
            LoadTexture(tex_glow.mapName, material->tex_emissive);

         model->GetMaterials().append(material);
      }

      Surface* surface = new Surface;
      model->GetSurfaces().append(surface);

      surface->CreateVerts(nverts);
      surface->CreatePolys(npolys);

      VertexSet*  vset     = surface->GetVertexSet();
      Poly*       polys    = surface->GetPolys();
      float       radius   = 0;
      int         v        = 0;

      for (int m = 0; m < nmeshs && v < nverts; m++) {
         LMesh& mesh = loader.GetMesh(m);

         // read vertex set:
         for (int i = 0; i < mesh.GetVertexCount() && v < nverts; i++) {
            vset->loc[v].x = mesh.GetVertex(i).x;
            vset->loc[v].y = mesh.GetVertex(i).z;
            vset->loc[v].z = mesh.GetVertex(i).y;

            vset->nrm[v].x = mesh.GetNormal(i).x;
            vset->nrm[v].y = mesh.GetNormal(i).z;
            vset->nrm[v].z = mesh.GetNormal(i).y;

            vset->tu[v]    = mesh.GetUV(i).x;
            vset->tv[v]    = mesh.GetUV(i).y;

            float d = vset->loc[v].length();
            if (d > radius)
               radius = d;

            v++;
         }
      }

      if (radius < 16) {
         model->ScaleBy(256.0f / radius);
         radius = 256.0f;
      }

      int n = 0;

      for (int m = 0; m < nmeshs && n < npolys; m++) {
         LMesh& mesh = loader.GetMesh(m);

         // read polys:
         int mesh_tris = mesh.GetTriangleCount();
         for (int i = 0; i < mesh_tris && n < npolys; i++) {
            Poly*             p     = surface->GetPolys() + n++;
            const LTriangle&  tri   = mesh.GetTriangle(i);
            LTriangle2        tri2  = mesh.GetTriangle2(i);

            p->nverts = 3;

            p->verts[0] = tri.a + mesh_verts[m];
            p->verts[1] = tri.b + mesh_verts[m];
            p->verts[2] = tri.c + mesh_verts[m];

            if (p->verts[0] > nverts || p->verts[1] > nverts || p->verts[2] > nverts) {
               p->verts[0] = 0;
               p->verts[1] = 1;
               p->verts[2] = 2;
            }

            if (tri2.vertexNormals[0] == tri2.vertexNormals[1] &&
                tri2.vertexNormals[0] == tri2.vertexNormals[2])
               p->flatness = 1.0f;
            else
               p->flatness = 0.0f;

            p->vertex_set  = vset;
            p->material    = model->GetMaterials()[ tri2.materialId ];
            p->sortval     = tri2.materialId;

            p->plane       = Plane(vset->loc[ p->verts[0] ],
                                   vset->loc[ p->verts[2] ],
                                   vset->loc[ p->verts[1] ]);

            surface->AddIndices(p->nverts);
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

      *pnverts = nverts;
      *pnpolys = npolys;
      *pradius = radius;

      model->Normalize();
      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+

bool
ModelFile3DS::Save(Model* m)
{
   if (m) {
      ModelFile::Save(m);

      FILE* f = fopen(filename, "w");
      if (!f) {
         ::MessageBox(0, "3DS Export Failed: Magic could not open the file for writing", "ERROR", MB_OK);
         return false;
      }

      fclose(f);

      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+
