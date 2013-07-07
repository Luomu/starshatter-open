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
#include "ArrayList.h"

// +--------------------------------------------------------------------+

struct Chunk
{
   WORD  id;
   DWORD len;
   BYTE* start;

   List<Chunk> chunks;
};

// +--------------------------------------------------------------------+

struct Vec2
{
   float u,v;
};

struct Triangle
{
   WORD a,b,c;
};

struct Triangle2
{
   Vec3     vloc[3];
   Vec3     vnrm[3];
   Vec2     vtex[3];
   Vec3     fnrm;
   DWORD    mtl_id;
};

struct TextureMap
{
   char     fname[256];
   float    strength;
   float    u_scale;
   float    v_scale;
   float    u_offset;
   float    v_offset;
   float    angle;
};

// +--------------------------------------------------------------------+

class Object3DS
{
public:
   Object3DS();
   virtual ~Object3DS();

   virtual const char*  GetName();
   virtual bool         IsObject(const char* name);

protected:
    Text    name;
};

// +--------------------------------------------------------------------+

class Material3DS : public Object3DS
{
public:
   Material3DS();
   virtual ~Material3DS();

   DWORD       GetID();
   TextureMap& GetTextureMap1();
   TextureMap& GetTextureMap2();
   TextureMap& GetOpacityMap();
   TextureMap& GetSpecularMap();
   TextureMap& GetBumpMap();
   TextureMap& GetReflectionMap();
   ColorValue  GetAmbientColor();
   ColorValue  GetDiffuseColor();
   ColorValue  GetSpecularColor();
   float       GetShininess();
   float       GetTransparency();
   DWORD       GetShadingType();

   // this methods should not be used by the "user", they're used internally to fill the class
   // with valid data when reading from file. If you're about to add an importer for another format you'LL
   // have to use these methods

   void        SetID(DWORD value);
   void        SetAmbientColor(const ColorValue &color);
   void        SetDiffuseColor(const ColorValue &color);
   void        SetSpecularColor(const ColorValue &color);
   void        SetShininess(float value);
   void        SetTransparency(float value);
   void        SetShadingType(DWORD shading);

protected:
   int         id;
   TextureMap  texMap1;
   TextureMap  texMap2;
   TextureMap  opacMap;
   TextureMap  refTextureMap;
   TextureMap  bumpMap;
   TextureMap  specMap;
   ColorValue  ambient;
   ColorValue  diffuse;
   ColorValue  specular;
   float       shininess;
   float       transparency;
   DWORD       shading;
};

// +--------------------------------------------------------------------+

class Mesh3DS : public Object3DS
{
public:
    Mesh3DS();
    virtual ~Mesh3DS();

    void    Clear();

    DWORD   GetVertexCount();
    void    SetVertexArraySize(DWORD value);
    DWORD   GetTriangleCount();
    void    SetTriangleArraySize(DWORD value);

    const Vec3&   GetVertex(DWORD index);
    const Vec3&   GetNormal(DWORD index);
    const Vec2&   GetUV(DWORD index);
    const Vec3&   GetTangent(DWORD index);
    const Vec3&   GetBinormal(DWORD index);

    void          SetVertex(const Vec3 &vec, DWORD index);
    void          SetNormal(const Vec3 &vec, DWORD index);
    void          SetUV(const Vec2 &vec, DWORD index);
    void          SetTangent(const Vec3 &vec, DWORD index);
    void          SetBinormal(const Vec3 &vec, DWORD index);

    const Triangle&  GetTriangle(DWORD index);
    Triangle2        GetTriangle2(DWORD index);

    Matrix&       GetMatrix();
    void          SetMatrix(Matrix& m);
    void          Optimize(int level);

    DWORD         GetMaterial(DWORD index);
    DWORD         AddMaterial(DWORD id);
    DWORD         GetMaterialCount();

protected:
    // the vertices, normals, etc.
    List<Vec3>       vertices;
    List<Vec3>       normals;
    List<Vec3>       binormals;
    List<Vec3>       tangents;
    List<Vec2>       uv;
    
    // triangles
    List<Triangle>   triangles;

    // the transformation matrix.
    Matrix           matrix;

    // the material ID array
    ArrayList        materials;

    void CalcNormals(bool useSmoothingGroups);
    void CalcTextureSpace();

    void TransformVertices();
};


// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
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
ModelFile3DS::Load(Model* m, double scale)
{
   if (m && scale > 0 && strlen(filename) > 0) {
      ModelFile::Load(m, scale);

      FILE* fp = fopen(filename, "rb");
      fclose(fp);

      m->Normalize();
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
         ::MessageBox(0, "Export Failed: Magic could not open the file for writing", "ERROR", MB_OK);
         return false;
      }

      fclose(f);

      return true;
   }

   return false;
}

// +--------------------------------------------------------------------+
