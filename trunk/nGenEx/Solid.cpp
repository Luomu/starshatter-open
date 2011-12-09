/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Solid.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for rendering solid meshes of polygons
*/

#include "MemDebug.h"
#include "Solid.h"
#include "Scene.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Light.h"
#include "Shadow.h"
#include "Projector.h"
#include "OPCODE.h"

#ifdef for
#undef for
#endif

void  Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

static bool use_collision_detection = true;

bool  Solid::IsCollisionEnabled()      { return use_collision_detection; }
void  Solid::EnableCollision(bool e)   { use_collision_detection = e;    }

// +--------------------------------------------------------------------+

Opcode::AABBTreeCollider   opcode_collider;

class OPCODE_data
{
public:
   OPCODE_data(Surface* s) {
      bool status = false;

      if (s) {
         using namespace Opcode;
         opcode_collider.SetFirstContact(true);

         npolys   = s->NumPolys();
         nverts   = s->NumVerts();
         ntris    = s->NumIndices() / 3;

         locs = new(__FILE__,__LINE__) IcePoint[nverts];
         tris = new(__FILE__,__LINE__) IndexedTriangle[ntris];

         if (locs && tris) {
            int i, n = 0;

            for (i = 0; i < nverts; i++) {
               IcePoint* p = locs + i;
               Vec3*     v = s->GetVertexSet()->loc + i;

               p->Set(v->x, v->y, v->z);
            }

            for (i = 0; i < npolys; i++) {
               Poly* p = s->GetPolys() + i;

               if (p->nverts == 3) {
                  IndexedTriangle& t = tris[n++];

                  t.mVRef[0] = p->verts[0];
                  t.mVRef[1] = p->verts[2];
                  t.mVRef[2] = p->verts[1];
               }
               else {
                  IndexedTriangle& t1 = tris[n++];
                  IndexedTriangle& t2 = tris[n++];

                  t1.mVRef[0] = p->verts[0];
                  t1.mVRef[1] = p->verts[2];
                  t1.mVRef[2] = p->verts[1];

                  t2.mVRef[0] = p->verts[0];
                  t2.mVRef[1] = p->verts[3];
                  t2.mVRef[2] = p->verts[2];
               }
            }

            mesh.SetNbVertices(nverts);
            mesh.SetNbTriangles(ntris);
            mesh.SetPointers(tris, locs);

            OPCODECREATE creator;
            creator.mIMesh = &mesh;
            status = model.Build(creator);
         }
      }
      else {
         tris   = 0;
         locs   = 0;
         npolys = 0;
         nverts = 0;
         ntris  = 0;
      }
   }

   ~OPCODE_data() {
      delete [] tris;
      delete [] locs;
   }

   Opcode::Model           model;
   Opcode::MeshInterface   mesh;
   IndexedTriangle*        tris;
   IcePoint*               locs;
   int                     npolys;
   int                     nverts;
   int                     ntris;
};

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Solid::Solid()
   : model(0), own_model(1),
     roll(0.0f), pitch(0.0f), yaw(0.0f), intersection_poly(0)
{
   shadow = true;
   sprintf_s(name, "Solid %d", id);
}

// +--------------------------------------------------------------------+

Solid::~Solid()
{
   if (own_model)
      delete model;

   shadows.destroy();
}

// +--------------------------------------------------------------------+

void
Solid::Update()
{
}

// +--------------------------------------------------------------------+

void
Solid::SetOrientation(const Matrix& o)
{
   orientation = o;
}

void
Solid::SetLuminous(bool l)
{
   luminous = l;

   if (model && luminous) {
      model->luminous = luminous;

      ListIter<Material> iter = model->GetMaterials();

      while (++iter) {
         Material* mtl = iter.value();

         mtl->Ka = Color::Black;
         mtl->Kd = Color::Black;
         mtl->Ks = Color::Black;
         mtl->Ke = Color::White;

         if (mtl->tex_diffuse && !mtl->tex_emissive)
            mtl->tex_emissive = mtl->tex_diffuse;
      }

      ListIter<Surface> s_iter = model->GetSurfaces();
      while (++s_iter) {
         Surface*    surface  = s_iter.value();
         VertexSet*  vset     = surface->GetVertexSet();

         for (int i = 0; i < vset->nverts; i++) {
            vset->diffuse[i]  = Color::White.Value();
            vset->specular[i] = Color::Black.Value();
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
Solid::SetOrientation(const Solid& match)
{
   if (!model || infinite)
      return;

   // copy the orientation matrix from the solid we are matching:
   orientation = match.Orientation();
}

// +--------------------------------------------------------------------+

void
Solid::Render(Video* video, DWORD flags)
{
   if (flags & RENDER_ADDITIVE)
      return;

   if (video && model && model->NumPolys()) {
      DWORD blend_modes = Video::BLEND_SOLID;

      if (flags == RENDER_ALPHA)
         blend_modes = Video::BLEND_ALPHA | Video::BLEND_ADDITIVE;

      video->DrawSolid(this, blend_modes);
   }
}

// +--------------------------------------------------------------------+

void
Solid::SelectDetail(Projector* p)
{
}

// +--------------------------------------------------------------------+

void
Solid::ProjectScreenRect(Projector* p)
{
   if (model && p) {
      Point tmp = loc;
      p->Transform(tmp);

      if (tmp.z > 1) {
         int l =  2000;
         int r = -2000;
         int t =  2000;
         int b = -2000;

         for (int i = 0; i < 6; i++) {
            Point extent;

            if (i < 2)
               extent.x = model->extents[i];

            else if (i < 4)
               extent.y = model->extents[i];

            else
               extent.z = model->extents[i];

            extent = extent * orientation + loc;

            p->Transform(extent);
            p->Project(extent);

            if (extent.x < l) l = (int) extent.x;
            if (extent.x > r) r = (int) extent.x;
            if (extent.y < t) t = (int) extent.y;
            if (extent.y > b) b = (int) extent.y;
         }

         screen_rect.x = l;
         screen_rect.y = t;
         screen_rect.w = r-l;
         screen_rect.h = b-t;
         return;
      }
   }

   screen_rect.x = 2000;
   screen_rect.y = 2000;
   screen_rect.w = 0;
   screen_rect.h = 0;
}

// +--------------------------------------------------------------------+
// Polygon Interference Detection:

int
Solid::CollidesWith(Graphic& o)
{
   Vec3 delta_loc = Location() - o.Location();

   // bounding spheres test:
   if (delta_loc.length() > Radius() + o.Radius())
      return 0;

   // possible collision, but no further refinement can be done:
   if (!o.IsSolid())
      return 1;

   Solid& s = (Solid&) o;

   // use the OPCODE library to check for polygon interference:
   if (model && s.model) {
      using namespace Opcode;

      bool contact = false;

      // first, reverse the orientation matrices for OPCODE:
      Matrix m1 = orientation;
      Matrix m2 = s.orientation;

      Matrix4x4   world0;
      Matrix4x4   world1;

      world0.m[0][0]  = (float) m1.elem[0][0];
      world0.m[0][1]  = (float) m1.elem[0][1];
      world0.m[0][2]  = (float) m1.elem[0][2];
      world0.m[0][3]  = 0.0f;

      world0.m[1][0]  = (float) m1.elem[1][0];
      world0.m[1][1]  = (float) m1.elem[1][1];
      world0.m[1][2]  = (float) m1.elem[1][2];
      world0.m[1][3]  = 0.0f;

      world0.m[2][0]  = (float) m1.elem[2][0];
      world0.m[2][1]  = (float) m1.elem[2][1];
      world0.m[2][2]  = (float) m1.elem[2][2];
      world0.m[2][3]  = 0.0f;

      world0.m[3][0]  = (float) Location().x;
      world0.m[3][1]  = (float) Location().y;
      world0.m[3][2]  = (float) Location().z;
      world0.m[3][3]  = 1.0f;

      world1.m[0][0]  = (float) m2.elem[0][0];
      world1.m[0][1]  = (float) m2.elem[1][0];
      world1.m[0][2]  = (float) m2.elem[2][0];
      world1.m[0][3]  = 0.0f;

      world1.m[1][0]  = (float) m2.elem[0][1];
      world1.m[1][1]  = (float) m2.elem[1][1];
      world1.m[1][2]  = (float) m2.elem[2][1];
      world1.m[1][3]  = 0.0f;

      world1.m[2][0]  = (float) m2.elem[0][2];
      world1.m[2][1]  = (float) m2.elem[1][2];
      world1.m[2][2]  = (float) m2.elem[2][2];
      world1.m[2][3]  = 0.0f;

      world1.m[3][0]  = (float) s.Location().x;
      world1.m[3][1]  = (float) s.Location().y;
      world1.m[3][2]  = (float) s.Location().z;
      world1.m[3][3]  = 1.0f;

      ListIter<Surface> s1_iter = model->surfaces;
      while (++s1_iter && !contact) {
         Surface* s1 = s1_iter.value();

         ListIter<Surface> s2_iter = s.model->surfaces;
         while (++s2_iter && !contact) {
            Surface* s2 = s2_iter.value();

            if (s1->opcode && s2->opcode) {
               BVTCache bvt;
               bvt.Model0 = &s1->opcode->model;
               bvt.Model1 = &s2->opcode->model;

               if (opcode_collider.Collide(bvt, &world0, &world1))
                  if (opcode_collider.GetContactStatus() != 0)
                     contact = true;
            }
         }
      }

      return contact;
   }


   return 1;
}

// +--------------------------------------------------------------------+
// Find the intersection of the ray (Q + w*len) with the solid.
// If the ray intersects a polygon of the solid, place the intersection
// point in ipt, and return 1.  Otherwise, return 0.

int
Solid::CheckRayIntersection(Point Q, Point w, double len, Point& ipt,
                              bool treat_translucent_polys_as_solid)
{
   int impact        = 0;
   
   if (!model || model->npolys < 1)
      return impact;

   // check right angle spherical distance:
   Point  d0 = loc - Q;
   Point  d1 = d0.cross(w);
   double dlen = d1.length();          // distance of point from line
   
   if (dlen > radius)                  // clean miss
      return 0;                        // (no impact)

   // possible collision course...

   /**********************************


          /--- + leading_edge = Q + w * len
         /    /  \
      delta2 /    delta 0
       /    /      \
      /    *........x <- solid location
     /    /
    /    / delta1
   /--- Q                  * = closest point


   ************************************/

   // find the point on the ray that is closest
   // to the solid's location:
   Point closest = Q + w * (d0 * w);

   // find the leading edge, and it's distance from the location:
   Point  leading_edge  = Q + w*len;
   Point  leading_delta = leading_edge - loc;
   double leading_dist  = leading_delta.length();

   // if the leading edge is not within the bounding sphere,
   if (leading_dist > radius) {
      // check to see if the closest point is between the
      // ray's endpoints:
      Point delta1 = closest      - Q;
      Point delta2 = leading_edge - Q; // this is w*len

      // if the closest point is not between the leading edge
      // and the origin, this ray does not intersect:
      if (delta1 * delta2 < 0 || delta1.length() > len) {
         return 0;
      }
   }

   // probable hit at this point...

   // if not active, that's good enough:
   if (GetScene() == 0) {
      ipt = closest;
      return 1;
   }

   // transform ray into object space:
   Matrix xform(Orientation());

   Vec3 tmp = w;

   w.x = tmp * Vec3(xform(0,0), xform(0,1), xform(0,2));
   w.y = tmp * Vec3(xform(1,0), xform(1,1), xform(1,2));
   w.z = tmp * Vec3(xform(2,0), xform(2,1), xform(2,2));

   tmp = Q-loc;

   Q.x = tmp * Vec3(xform(0,0), xform(0,1), xform(0,2));
   Q.y = tmp * Vec3(xform(1,0), xform(1,1), xform(1,2));
   Q.z = tmp * Vec3(xform(2,0), xform(2,1), xform(2,2));
   
   double min = len;
   intersection_poly = 0;

   // check each polygon:
   ListIter<Surface> iter = model->surfaces;
   while (++iter) {
      Surface* s = iter.value();
      Poly*    p = s->GetPolys();

      for (int i = 0; i < s->NumPolys(); i++) {
         if (!treat_translucent_polys_as_solid && p->material && !p->material->IsSolid()) {
            p++;
            continue;
         }

         Point  v = p->plane.normal;
         double d = p->plane.distance;

         double denom = w*v;

         if (denom < -1.0e-5) {
            Point  P    = v * d;
            double ilen = ((P-Q)*v)/denom;

            if (ilen > 0 && ilen < min) {
               Point intersect = Q + w * ilen;

               if (p->Contains(intersect)) {
                  intersection_poly = p;
                  ipt               = intersect;
                  min               = ilen;
                  impact            = 1;
               }
            }
         }

         p++;
      }
   }

   // xform impact point back into world coordinates:

   if (impact) {
      ipt = (ipt * Orientation()) + loc;
   }

   return impact;
}

// +--------------------------------------------------------------------+

void
Solid::ClearModel()
{
   if (own_model && model) {
      delete model;
      model = 0;
   }

   radius = 0.0f;
}

// +--------------------------------------------------------------------+

void
Solid::UseModel(Model* m)
{
   // get rid of the existing model:
   ClearModel();
   
   // point to the new model:
   own_model = 0;
   model     = m;
   radius    = m->radius;
}

// +--------------------------------------------------------------------+

bool
Solid::Load(const char* mag_file, double scale)
{
   // get ready to load, delete existing model:
   ClearModel();
   
   // loading our own copy, so we own the model:
   model     = new(__FILE__,__LINE__) Model;
   own_model = 1;

   // now load the model:
   if (model->Load(mag_file, scale)) {
      radius = model->radius;
      strncpy_s(name, model->name, sizeof(name));
      return true;
   }

   // load failed:
   ClearModel();
   return false;
}

bool
Solid::Load(ModelFile* mod_file, double scale)
{
   // get ready to load, delete existing model:
   ClearModel();
   
   // loading our own copy, so we own the model:
   model     = new(__FILE__,__LINE__) Model;
   own_model = 1;

   // now load the model:
   if (model->Load(mod_file, scale)) {
      radius = model->radius;
      return true;
   }

   // load failed:
   ClearModel();
   return false;
}

bool
Solid::Rescale(double scale)
{
   if (!own_model || !model)
      return false;

   radius = 0;

   ListIter<Surface> iter = model->GetSurfaces();
   while (++iter) {
      Surface* s = iter.value();

      for (int v = 0; v < s->NumVerts(); v++) {
         s->vertex_set->loc[v]   *= (float) scale;
         s->vloc[v]              *= (float) scale;

         float lvi = s->vloc[v].length();
         if (lvi > radius)
            radius = lvi;
      }
   }

   model->radius = radius;

   InvalidateSurfaceData();

   return true;
}

void
Solid::CreateShadows(int nlights)
{
   while (shadows.size() < nlights) {
      shadows.append(new(__FILE__,__LINE__) Shadow(this));
   }
}

void
Solid::UpdateShadows(List<Light>& lights)
{
   List<Light>       active_lights;
   ListIter<Light>   iter = lights;

   while (++iter) {
      Light* light = iter.value();

      if (light->IsActive() && light->CastsShadow()) {
         double distance  = Point(Location() - light->Location()).length();
         double intensity = light->Intensity();

         if (light->Type() == Light::LIGHT_POINT) {
            if (intensity / distance > 1)
               active_lights.append(light);
         }

         else if (light->Type() == Light::LIGHT_DIRECTIONAL) {
            if (intensity > 0.65)
               active_lights.insert(light);
         }
      }
   }

   iter.attach(active_lights);

   while (++iter) {
      Light* light = iter.value();
      int    index = iter.index();

      if (index < shadows.size()) {
         shadows[index]->Update(light);
      }
   }
}

// +--------------------------------------------------------------------+

void
Solid::DeletePrivateData()
{
   if (model)
      model->DeletePrivateData();
}

// +--------------------------------------------------------------------+

void
Solid::InvalidateSurfaceData()
{
   if (!model)
      return;
   
   bool invalidate = model->IsDynamic();

   ListIter<Surface> iter = model->GetSurfaces();
   while (++iter) {
      Surface* s = iter.value();
      VideoPrivateData* vpd = s->GetVideoPrivateData();

      if (vpd) {
         if (invalidate) {
            vpd->Invalidate();
         }
         else {
            delete vpd;
            s->SetVideoPrivateData(0);
         }
      }
   }
}

void
Solid::InvalidateSegmentData()
{
   if (!model)
      return;
   
   bool invalidate = model->IsDynamic();

   ListIter<Surface> iter = model->GetSurfaces();
   while (++iter) {
      Surface* s = iter.value();

      ListIter<Segment> seg_iter = s->GetSegments();
      while (++seg_iter) {
         Segment* segment = seg_iter.value();
         VideoPrivateData* vpd = segment->GetVideoPrivateData();

         if (vpd) {
            if (invalidate) {
               vpd->Invalidate();
            }
            else {
               delete vpd;
               segment->SetVideoPrivateData(0);
            }
         }
      }
   }
}

// +--------------------------------------------------------------------+

bool
Solid::IsDynamic() const
{
   if (model)
      return model->IsDynamic();

   return false;
}

void
Solid::SetDynamic(bool d)
{
   if (model && own_model)
      model->SetDynamic(d);
}

// +--------------------------------------------------------------------+

void
Solid::GetAllTextures(List<Bitmap>& textures)
{
   if (model)
      model->GetAllTextures(textures);
}

void
Model::GetAllTextures(List<Bitmap>& textures)
{
   ListIter<Material> m_iter = materials;
   while (++m_iter) {
      Material* m = m_iter.value();

      if (m->tex_diffuse && !textures.contains(m->tex_diffuse))
         textures.append(m->tex_diffuse);

      if (m->tex_specular && !textures.contains(m->tex_specular))
         textures.append(m->tex_specular);

      if (m->tex_emissive && !textures.contains(m->tex_emissive))
         textures.append(m->tex_emissive);

      if (m->tex_bumpmap && !textures.contains(m->tex_bumpmap))
         textures.append(m->tex_bumpmap);

      if (m->tex_detail && !textures.contains(m->tex_detail))
         textures.append(m->tex_detail);
   }
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Model::Model()
   : nverts(0), npolys(0), radius(0), luminous(false), dynamic(false)
{ 
   ZeroMemory(name, sizeof(name));
}

Model::Model(const Model& m)
   : nverts(0), npolys(0), radius(0), luminous(false), dynamic(false)
{
   operator=(m);
}

// +--------------------------------------------------------------------+

Model::~Model()
{
   surfaces.destroy();
   materials.destroy();
}

Model&
Model::operator = (const Model& m)
{
   if (this != &m) {
      surfaces.destroy();
      materials.destroy();

      CopyMemory(name, m.name, Solid::NAMELEN);

      nverts      = m.nverts;
      npolys      = m.npolys;
      radius      = m.radius;
      luminous    = m.luminous;
      dynamic     = m.dynamic;

      Model* pmod = (Model*) &m;

      ListIter<Material> m_iter = pmod->materials;
      while (++m_iter) {
         Material* matl1 = m_iter.value();
         Material* matl2 = new(__FILE__,__LINE__) Material;

         CopyMemory(matl2, matl1, sizeof(Material));
         matl2->thumbnail = 0;

         materials.append(matl2);
      }

      ListIter<Surface> s_iter = pmod->surfaces;
      while (++s_iter) {
         Surface* surf1 = s_iter.value();
         Surface* surf2 = new(__FILE__,__LINE__) Surface;

         surf2->Copy(*surf1, this);
         surfaces.append(surf2);
      }
   }

   return *this;
}

// +--------------------------------------------------------------------+

int
Model::NumSegments() const
{
   int nsegments = 0;

   for (int i = 0; i < surfaces.size(); i++) {
      const Surface* s = surfaces[i];
      nsegments += s->NumSegments();
   }

   return nsegments;
}

// +--------------------------------------------------------------------+

inline bool Collinear(const double* a, const double* b, const double* c)
{
   Point ab(b[0]-a[0], b[1]-a[1], b[2]-a[2]);
   Point ac(c[0]-a[0], c[1]-a[1], c[2]-a[2]);
   Point cross = ab.cross(ac);
   return (cross.length() == 0);
}

struct HomogenousPlane
{
   double distance;
   double normal_x;
   double normal_y;
   double normal_z;
   double normal_w;
};

static void LoadPlane(Plane& p, DataLoader* l, BYTE*& fp)
{
   HomogenousPlane tmp;
   l->fread(&tmp, sizeof(HomogenousPlane), 1, fp);
}

static void LoadFlags(LPDWORD flags, DataLoader* l, BYTE*& fp)
{
   DWORD magic_flags;
   l->fread(&magic_flags, sizeof(DWORD), 1, fp);

   /** OLD MAGIC FLAGS
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

bool
Model::Load(const char* mag_file, double scale)
{
   BYTE*       block;
   DataLoader* loader = DataLoader::GetLoader();
   bool        result = false;

   radius      = 0.0f;
   extents[0]  = 0.0f;
   extents[1]  = 0.0f;
   extents[2]  = 0.0f;
   extents[3]  = 0.0f;
   extents[4]  = 0.0f;
   extents[5]  = 0.0f;

   if (!loader) {
      Print("MAG Open Failed:  no data loader for file '%s'\n", mag_file);
      return result;
   }

   int         size   = loader->LoadBuffer(mag_file, block);
   BYTE*       fp     = block;

   // check MAG file:
   if (!size) {
      Print("MAG Open Failed:  could not open file '%s'\n", mag_file);
      return result;
   }

   strncpy_s(name, mag_file, 31);
   name[31] = 0;

   char file_id[5];
   CopyMemory(file_id, block, 4);
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
      Print("MAG Open Failed:  File '%s' Invalid file type '%s'\n", mag_file, file_id);
      loader->ReleaseBuffer(block);
      return result;
   }

   // get ready to load, delete existing model:
   surfaces.destroy();
   materials.destroy();
   nverts   = 0;
   npolys   = 0;

   // now load the model:
   switch (version) {
   case 4:
   case 5:
      result = LoadMag5(block, size, scale);
      break;

   case 6:
      result = LoadMag6(block, size, scale);
      break;

   default:
      break;
   }

   loader->ReleaseBuffer(block);
   return result;
}

// +--------------------------------------------------------------------+

bool
Model::Load(ModelFile* mod_file, double scale)
{
   if (mod_file) {
      return mod_file->Load(this, scale);
   }

   return false;
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
Model::LoadMag5(BYTE* block, int len, double scale)
{
   bool        result = false;

   DataLoader* loader = DataLoader::GetLoader();
   BYTE*       fp     = block + 4;
   int         ntex   = 0;
   int         nsurfs = 0;

   loader->fread(&ntex,   sizeof(ntex),   1, fp);
   loader->fread(&nsurfs, sizeof(nsurfs), 1, fp);

   // create a default gray material:
   Material*   mtl = new Material;

   if (mtl) {
      mtl->Ka = Color::LightGray;
      mtl->Kd = Color::LightGray;
      mtl->Ks = ColorValue(0.2f,0.2f,0.2f);
      mtl->power = 20.0f;

      mtl->ambient_value  = 1.0f;
      mtl->ambient_color  = Color::LightGray;
      mtl->diffuse_value  = 1.0f;
      mtl->diffuse_color  = Color::LightGray;
      mtl->specular_value = 0.2f;
      mtl->specular_color = Color::White;
      strcpy_s(mtl->name, "(default)");

      materials.append(mtl);
   }

   // read texture list:
   for (int i = 0; i < ntex; i++) {
      mtl = new(__FILE__,__LINE__) Material;
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

         loader->fread(tname, 32, 1, fp);
         loader->LoadTexture(tname, mtl->tex_diffuse, Bitmap::BMP_SOLID, true);
         strcpy_s(mtl->name, tname);

         char* dot = strrchr(mtl->name, '.');
         if (dot)
            *dot = 0;

         char* plus = strrchr(mtl->name, '+');
         if (plus)
            *plus = 0;

         materials.append(mtl);
      }
   }


   loader->fread(&nverts, 4, 1, fp);
   loader->fread(&npolys, 4, 1, fp);

   // plan on creating four verts per poly:
   int mag_nverts = nverts;
   int next_vert  = nverts;

   nverts      = npolys * 4;

   Surface*    s    = new(__FILE__,__LINE__) Surface;
   VertexSet*  vset = 0;

   if (s) {
      strcpy_s(s->name, "default");

      s->model       = this;
      s->vertex_set  = new(__FILE__,__LINE__) VertexSet(nverts);
      s->vloc        = new(__FILE__,__LINE__) Vec3[nverts];

      ZeroMemory(s->vertex_set->loc,      nverts * sizeof(Vec3));
      ZeroMemory(s->vertex_set->diffuse,  nverts * sizeof(DWORD));
      ZeroMemory(s->vertex_set->specular, nverts * sizeof(DWORD));
      ZeroMemory(s->vertex_set->tu,       nverts * sizeof(float));
      ZeroMemory(s->vertex_set->tv,       nverts * sizeof(float));
      ZeroMemory(s->vertex_set->rw,       nverts * sizeof(float));
      ZeroMemory(s->vloc,                 nverts * sizeof(Vec3));

      s->npolys      = npolys;
      s->polys       = new(__FILE__,__LINE__) Poly[npolys];

      ZeroMemory(s->polys, sizeof(Poly) * npolys);
      surfaces.append(s);

      vset = s->vertex_set;

	  int v;
      // read vertex set:
      for (v = 0; v < mag_nverts; v++) {
         Vec3 vert, norm;
         DWORD vstate;

         loader->fread(&vert,    sizeof(Vec3), 1, fp);
         loader->fread(&norm,    sizeof(Vec3), 1, fp);
         loader->fread(&vstate,  sizeof(DWORD), 1, fp);

         vert.SwapYZ();
         vert *= (float) scale;

         vset->loc[v]      = vert;
         vset->nrm[v]      = norm;

         double d = vert.length();
         if (d > radius)
            radius = (float) d;

         if (vert.x > extents[0])   extents[0] = vert.x;
         if (vert.x < extents[1])   extents[1] = vert.x;
         if (vert.y > extents[2])   extents[2] = vert.y;
         if (vert.y < extents[3])   extents[3] = vert.y;
         if (vert.z > extents[4])   extents[4] = vert.z;
         if (vert.z < extents[5])   extents[5] = vert.z;
      }

      while (v < nverts)
         vset->nrm[v++] = Vec3(1,0,0);

      // read polys:
      Vec3  dummy_center;
      DWORD dummy_flags;
      DWORD dummy_color;
      Plane dummy_plane;
      int   texture_num;
      int   poly_nverts;
      int   vert_index_buffer[32];
      float texture_index_buffer[32];

      for (int n = 0; n < npolys; n++) {
         Poly& poly = s->polys[n];
         poly.vertex_set  = vset;

         loader->fread(&dummy_flags,  sizeof(DWORD), 1, fp);
         loader->fread(&dummy_center,  sizeof(Vec3),  1, fp);

         LoadPlane(dummy_plane, loader, fp);

         loader->fread(&dummy_color,  sizeof(DWORD), 1, fp);
         loader->fread(&texture_num,   sizeof(int),   1, fp);

         if (texture_num >= 0 && texture_num < ntex) {
            int mtl_num   = texture_num + 1;
            poly.material = materials[mtl_num];
            poly.sortval  = texture_num;

            bool flag_translucent = (dummy_flags & 0x04) ? true : false;
            bool flag_transparent = (dummy_flags & 0x08) ? true : false;

            // luminous
            if (dummy_flags & 2) {
               mtl = materials[mtl_num];

               mtl->Ka = ColorValue(0,0,0,0);
               mtl->Kd = ColorValue(0,0,0,0);
               mtl->Ks = ColorValue(0,0,0,0);
               mtl->Ke = ColorValue(1,1,1,1);

               mtl->tex_emissive = mtl->tex_diffuse;
            }

            // glowing (additive)
            if (flag_translucent && flag_transparent)
               materials[mtl_num]->blend = Material::MTL_ADDITIVE;

            // translucent (alpha)
            else if (flag_translucent)
               materials[mtl_num]->blend = Material::MTL_TRANSLUCENT;

            // transparent (just use alpha for this)
            else if (flag_transparent)
               materials[mtl_num]->blend = Material::MTL_TRANSLUCENT;
         }
         else {
            poly.material = materials.first();
            poly.sortval  = 1000;
         }

         // hack: store flat shaded flag in unused visible byte
         poly.visible = (BYTE) (dummy_flags & 1);

         loader->fread(&poly_nverts,           sizeof(int),   1, fp);
         loader->fread(vert_index_buffer,      sizeof(int),   poly_nverts, fp);

         if (poly_nverts == 3)
            s->nindices += 3;

         else if (poly_nverts == 4)
            s->nindices += 6;

         poly.nverts = poly_nverts;
         for (int vi = 0; vi < poly_nverts; vi++) {
            v = vert_index_buffer[vi];

            if (vset->rw[v] > 0) {
               vset->CopyVertex(next_vert, v);
               v = next_vert++;
            }

            vset->rw[v] = 1;
            poly.verts[vi] = v;
         }
      
         loader->fread(texture_index_buffer, sizeof(float), poly_nverts, fp); // tu's
         for (int vi = 0; vi < poly_nverts; vi++) {
            v = poly.verts[vi];
            vset->tu[v] = texture_index_buffer[vi];
         }

         loader->fread(texture_index_buffer, sizeof(float), poly_nverts, fp); // tv's
         for (int vi = 0; vi < poly_nverts; vi++) {
            v = poly.verts[vi];
            vset->tv[v] = texture_index_buffer[vi];
         }

         fp += 16;
      }

      // pass 2 (adjust vertex normals for flat polys):
      for (int n = 0; n < npolys; n++) {
         Poly& poly  = s->polys[n];
         poly.plane  = Plane(vset->loc[poly.verts[0]],
                             vset->loc[poly.verts[2]],
                             vset->loc[poly.verts[1]]);

         // hack: retrieve flat shaded flag from unused visible byte
         if (poly.visible) {
            poly_nverts = poly.nverts;

            for (int vi = 0; vi < poly_nverts; vi++) {
               v = poly.verts[vi];
               vset->nrm[v] = poly.plane.normal;
            }
         }
      }

      // sort the polys by material index:
      qsort((void*) s->polys, s->npolys, sizeof(Poly), mcomp);

      // then assign them to cohesive segments:
      Segment* segment = 0;

      for (int n = 0; n < npolys; n++) {
         if (segment && segment->material == s->polys[n].material) {
            segment->npolys++;
         }
         else {
            segment = 0;
         }

         if (!segment) {
            segment = new(__FILE__,__LINE__) Segment;

            segment->npolys   = 1;
            segment->polys    = &s->polys[n];
            segment->material = segment->polys->material;
            segment->model    = this;

            s->segments.append(segment);
         }
      }

      s->BuildHull();

      result = nverts && npolys;
   }

   return result;
}

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

bool
Model::LoadMag6(BYTE* block, int len, double scale)
{
   bool           result = false;

   DataLoader*    loader = DataLoader::GetLoader();
   BYTE*          fp     = block + 4;
   int            ntex   = 0;
   int            nmtls  = 0;
   int            nsurfs = 0;
   List<Bitmap>   textures;

   loader->fread(&ntex,   sizeof(ntex),   1, fp); // size of texture block
   loader->fread(&nmtls,  sizeof(nmtls),  1, fp); // number of materials
   loader->fread(&nsurfs, sizeof(nsurfs), 1, fp); // number of surfaces

   // read texture list:
   if (ntex) {
      char*          buffer      = new(__FILE__,__LINE__) char[ntex];
      char*          p           = buffer;
      Bitmap*        bmp         = 0;

      loader->fread(buffer, ntex, 1, fp);

      while (p < buffer + ntex) {
         loader->LoadTexture(p, bmp, Bitmap::BMP_SOLID, true);
         textures.append(bmp);

         p += strlen(p) + 1;
      }

      delete [] buffer;
   }

   for (int i = 0; i < nmtls; i++) {
      MaterialMag6   m6;
      Material*      mtl = new(__FILE__,__LINE__) Material;

      loader->fread(&m6, sizeof(m6), 1, fp);

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

         materials.append(mtl);
      }
   }

   for (int i = 0; i < nsurfs; i++) {
      int   nverts  = 0;
      int   npolys  = 0;
      BYTE  namelen = 0;
      char  name[128];

      loader->fread(&nverts,  4, 1,       fp);
      loader->fread(&npolys,  4, 1,       fp);
      loader->fread(&namelen, 1, 1,       fp);
      loader->fread(name,     1, namelen, fp);

      Surface* surface = new(__FILE__,__LINE__) Surface;
      surface->model = this;
      surface->SetName(name);
      surface->CreateVerts(nverts);
      surface->CreatePolys(npolys);

      VertexSet*  vset  = surface->GetVertexSet();
      Poly*       polys = surface->GetPolys();

      ZeroMemory(polys, sizeof(Poly) * npolys);

      // read vertex set:
      for (int v = 0; v < nverts; v++) {
         loader->fread(&vset->loc[v],         sizeof(float), 3, fp);
         loader->fread(&vset->nrm[v],         sizeof(float), 3, fp);
         loader->fread(&vset->tu[v],          sizeof(float), 1, fp);
         loader->fread(&vset->tv[v],          sizeof(float), 1, fp);

         vset->loc[v] *= (float) scale;

         Vec3 vert = vset->loc[v];

         double d = vert.length();
         if (d > radius)
            radius = (float) d;

         if (vert.x > extents[0])   extents[0] = vert.x;
         if (vert.x < extents[1])   extents[1] = vert.x;
         if (vert.y > extents[2])   extents[2] = vert.y;
         if (vert.y < extents[3])   extents[3] = vert.y;
         if (vert.z > extents[4])   extents[4] = vert.z;
         if (vert.z < extents[5])   extents[5] = vert.z;
      }

      // read polys:
      for (int n = 0; n < npolys; n++) {
         Poly& poly           = polys[n];
         BYTE  poly_nverts    = 0;
         BYTE  material_index = 0;
         WORD  poly_verts[8];

         loader->fread(&poly_nverts,      sizeof(BYTE),  1, fp);
         loader->fread(&material_index,   sizeof(BYTE),  1, fp);
         loader->fread(&poly_verts[0],    sizeof(WORD),  poly_nverts, fp);

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
            poly.material = materials[material_index-1];
            poly.sortval  = material_index;
         }
         else if (materials.size()) {
            poly.material = materials.first();
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
            segment = new(__FILE__,__LINE__) Segment;

            segment->npolys   = 1;
            segment->polys    = &polys[n];
            segment->material = segment->polys->material;
            segment->model    = this;

            surface->GetSegments().append(segment);
         }
      }

      surface->BuildHull();
      surfaces.append(surface);

      this->nverts += nverts;
      this->npolys += npolys;
   }


   result = nverts && npolys;
   return result;
}

void
Model::AddSurface(Surface* surface)
{
   if (surface) {
      surface->model = this;

      ListIter<Segment> iter = surface->segments;
      while (++iter) {
         Segment* segment = iter.value();
         segment->model = this;
      }

      surface->BuildHull();
      surfaces.append(surface);

      nverts += surface->NumVerts();
      npolys += surface->NumPolys();
   }
}


// +--------------------------------------------------------------------+

const Material*
Model::FindMaterial(const char* mtl_name) const
{
   if (mtl_name && *mtl_name) {
      Model* pThis = (Model*) this;

      ListIter<Material> iter = pThis->materials;
      while (++iter) {
         Material* mtl = iter.value();

         if (!strcmp(mtl->name, mtl_name))
            return mtl;
      }
   }

   return 0;
}

const Material*
Model::ReplaceMaterial(const Material* mtl)
{
   const Material* mtl_orig = 0;

   if (mtl) {
      mtl_orig = FindMaterial(mtl->name);

      if (mtl_orig) {
         int n = materials.index(mtl_orig);
         materials[n] = (Material*) mtl;

         ListIter<Surface> surf_iter = surfaces;
         while (++surf_iter) {
            Surface* surf = surf_iter.value();

            ListIter<Segment> seg_iter = surf->GetSegments();
            while (++seg_iter) {
               Segment* segment = seg_iter.value();

               if (segment->material == mtl_orig)
                  segment->material = (Material*) mtl;
            }
         }
      }
   }

   return mtl_orig;
}

// +--------------------------------------------------------------------+

Poly*
Model::AddPolys(int nsurf, int np, int nv)
{
   if (nsurf >= 0 && nsurf < surfaces.size())
      return surfaces[nsurf]->AddPolys(np, nv);

   ::Print("WARNING: AddPolys(%d,%d,%d) invalid surface\n", nsurf, np, nv);
   return 0;
}

// +--------------------------------------------------------------------+

void
Model::ExplodeMesh()
{
   ListIter<Surface> iter = surfaces;

   int nv = 0;
   int np = 0;

   while (++iter) {
      Surface* s = iter.value();
      s->ExplodeMesh();

      nv += s->NumVerts();
      np += s->NumPolys();
   }

   nverts = nv;
   npolys = np;
}

// +--------------------------------------------------------------------+

void
Model::OptimizeMesh()
{
   ListIter<Surface> iter = surfaces;

   int nv = 0;
   int np = 0;

   while (++iter) {
      Surface* s = iter.value();
      s->OptimizeMesh();

      nv += s->NumVerts();
      np += s->NumPolys();
   }

   nverts = nv;
   npolys = np;
}

// +--------------------------------------------------------------------+

void
Model::OptimizeMaterials()
{
   for (int i = 0; i < materials.size(); i++) {
      Material* m1 = materials[i];

      for (int n = i; n < materials.size(); n++) {
         Material* m2 = materials[n];

         // if they match, merge them:
         if (*m1 == *m2) {
            List<Poly> polys;
            SelectPolys(polys, m2);

            ListIter<Poly> iter = polys;
            while (++iter) {
               Poly* p = iter.value();
               p->material = m1;
            }

            // and discard the duplicate:
            materials.remove(m2);
            delete m2;
         }
      }
   }
}

void
Model::ScaleBy(double factor)
{
   ListIter<Surface> iter = surfaces;

   while (++iter) {
      Surface* s = iter.value();
      s->ScaleBy(factor);
   }
}

// +--------------------------------------------------------------------+

void
Model::Normalize()
{
   ListIter<Surface> iter = surfaces;

   while (++iter) {
      Surface* s = iter.value();
      s->Normalize();
   }
}

void
Model::SelectPolys(List<Poly>& polys, Vec3 loc)
{
   ListIter<Surface> iter = surfaces;

   while (++iter) {
      Surface* s = iter.value();
      s->SelectPolys(polys, loc);
   }
}

void
Model::SelectPolys(List<Poly>& polys, Material* m)
{
   ListIter<Surface> iter = surfaces;

   while (++iter) {
      Surface* s = iter.value();
      s->SelectPolys(polys, m);
   }
}

void
Model::ComputeTangents()
{
   ListIter<Surface> iter = surfaces;

   while (++iter) {
      Surface* s = iter.value();
      s->ComputeTangents();
   }
}

// +--------------------------------------------------------------------+

void
Model::DeletePrivateData()
{
   ListIter<Surface> iter = surfaces;
   while (++iter) {
      Surface* s = iter.value();
      VideoPrivateData* vpd = s->GetVideoPrivateData();

      if (vpd) {
         delete vpd;
         s->SetVideoPrivateData(0);
      }

      ListIter<Segment> seg_iter = s->GetSegments();
      while (++seg_iter) {
         Segment* segment = seg_iter.value();
         VideoPrivateData* vpdp = segment->video_data;

         if (vpdp) {
            delete vpdp;
            segment->video_data = 0;
         }
      }
   }
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Surface::Surface()
   : model(0), vertex_set(0), vloc(0), nhull(0), npolys(0), nindices(0),
     polys(0), state(0), video_data(0), opcode(0)
{
   ZeroMemory(name, sizeof(name));
}

Surface::~Surface()
{
   segments.destroy();

   delete    opcode;
   delete    vertex_set;
   delete [] vloc;
   delete [] polys;
   delete    video_data;

   model = 0;
}

// +--------------------------------------------------------------------+

void
Surface::Copy(Surface& s, Model* m)
{
   segments.destroy();

   delete    opcode;
   delete    vertex_set;
   delete [] vloc;
   delete [] polys;
   delete    video_data;

   CopyMemory(name, s.name, Solid::NAMELEN);

   model          = m;
   radius         = s.radius;
   nhull          = s.nhull;
   npolys         = s.npolys;
   nindices       = s.nindices;
   state          = s.state;
   offset         = s.offset;
   orientation    = s.orientation;
   opcode         = 0;
   video_data     = 0;
   
   vertex_set     = s.vertex_set->Clone();

   if (nhull > 0) {
      vloc        = new(__FILE__,__LINE__) Vec3[nhull];
      CopyMemory(vloc, s.vloc, nhull * sizeof(Vec3));
   }
   else {
      vloc        = 0;
   }

   polys          = new(__FILE__,__LINE__) Poly[npolys];
   CopyMemory(polys, s.polys, npolys * sizeof(Poly));

   for (int i = 0; i < npolys; i++) {
      polys[i].vertex_set = vertex_set;
      
      if (s.polys[i].material)
         polys[i].material = (Material*) model->FindMaterial(s.polys[i].material->name);
   }

   ListIter<Segment> iter = s.segments;
   while (++iter) {
      Segment* seg1 = iter.value();
      Segment* seg2 = new(__FILE__,__LINE__) Segment;

      seg2->npolys      = seg1->npolys;
      seg2->polys       = polys + (seg1->polys - s.polys);

      if (seg2->polys[0].material)
      seg2->material    = seg2->polys[0].material;
   
      seg2->model       = model;
      seg2->video_data  = 0;

      segments.append(seg2);
   }
}

// +--------------------------------------------------------------------+

void
Surface::SetName(const char* n)
{
   int len = sizeof(name);

   ZeroMemory(name, len);
   strncpy_s(name, n, len-1);
}

void
Surface::SetHidden(bool b)
{
   if (b)
      state = state | HIDDEN;

   else
      state = state & ~HIDDEN;
}

void
Surface::SetLocked(bool b)
{
   if (b)
      state = state | LOCKED;

   else
      state = state & ~LOCKED;
}

void
Surface::SetSimplified(bool b)
{
   if (b)
      state = state | SIMPLE;

   else
      state = state & ~SIMPLE;
}

void
Surface::CreateVerts(int nverts)
{
   if (!vertex_set && !vloc) {
      vertex_set  = new(__FILE__,__LINE__) VertexSet(nverts);
      vloc        = new(__FILE__,__LINE__) Vec3[nverts];
   }
}

void
Surface::CreatePolys(int np)
{
   if (!polys && !npolys) {
      npolys = np;
      polys  = new(__FILE__,__LINE__) Poly[npolys];

      ZeroMemory(polys, npolys * sizeof(Poly));
   }
}

// +--------------------------------------------------------------------+

Poly*
Surface::AddPolys(int np, int nv)
{
   if (  polys && vertex_set &&
         np > 0 && np + npolys             < MAX_POLYS && 
         nv > 0 && nv + vertex_set->nverts < MAX_VERTS)
   {
      int newverts = nv + vertex_set->nverts;
      int newpolys = np + npolys;

      vertex_set->Resize(newverts, true);

      Poly* pset = new(__FILE__,__LINE__) Poly[newpolys];
      Poly* pnew = pset + npolys;

      CopyMemory(pset, polys, npolys * sizeof(Poly));
      ZeroMemory(pnew,        np     * sizeof(Poly));

      if (segments.size() > 0) {
         Segment*    seg = segments.last();
         Material*   mtl = seg->material;
         
         for (int i = 0; i < np; i++) {
            Poly* p = pnew + i;
            p->material = mtl;
         }

         seg->npolys += np;
      }
   }

   return 0;
}

// +--------------------------------------------------------------------+

void
Surface::ExplodeMesh()
{
   if (!vertex_set || vertex_set->nverts < 3)
      return;

   int i, j, v;
   int nverts = 0;

   // count max verts:
   for (i = 0; i < npolys; i++) {
      Poly* p = polys + i;
      nverts += p->nverts;
   }

   // create target vertex set:
   VertexSet*  vset = new(__FILE__,__LINE__) VertexSet(nverts);
   v = 0;

   // explode verts:
   for (i = 0; i < npolys; i++) {
      Poly* p = polys + i;
      p->vertex_set = vset;

      for (j = 0; j < p->nverts; j++) {
         int vsrc = p->verts[j];

         vset->loc[v]      = vertex_set->loc[vsrc];
         vset->nrm[v]      = vertex_set->nrm[vsrc];
         vset->tu[v]       = vertex_set->tu[vsrc];
         vset->tv[v]       = vertex_set->tv[vsrc];
         vset->rw[v]       = vertex_set->rw[vsrc];
         vset->diffuse[v]  = vertex_set->diffuse[vsrc];
         vset->specular[v] = vertex_set->specular[vsrc];

         p->verts[j]       = v++;
      }
   }

   // finalize:
   if (vset) {
      delete vertex_set;
      vertex_set = vset;
   }

   if (vloc)
      delete [] vloc;

   vloc = new(__FILE__,__LINE__) Vec3[nverts];

   ComputeTangents();
   BuildHull();
}

// +--------------------------------------------------------------------+

const double SELECT_EPSILON = 0.05;
const double SELECT_TEXTURE = 0.0001;

static bool MatchVerts(VertexSet* vset, int i, int j)
{
   double      d   = 0;
   const Vec3& vl1 = vset->loc[i];
   const Vec3& vn1 = vset->nrm[i];
   float       tu1 = vset->tu[i];
   float       tv1 = vset->tv[i];
   const Vec3& vl2 = vset->loc[j];
   const Vec3& vn2 = vset->nrm[j];
   float       tu2 = vset->tu[j];
   float       tv2 = vset->tv[j];

   d = fabs(vl1.x - vl2.x);
   if (d > SELECT_EPSILON)
      return false;

   d = fabs(vl1.y - vl2.y);
   if (d > SELECT_EPSILON)
      return false;

   d = fabs(vl1.z - vl2.z);
   if (d > SELECT_EPSILON)
      return false;

   d = fabs(vn1.x - vn2.x);
   if (d > SELECT_EPSILON)
      return false;

   d = fabs(vn1.y - vn2.y);
   if (d > SELECT_EPSILON)
      return false;

   d = fabs(vn1.z - vn2.z);
   if (d > SELECT_EPSILON)
      return false;

   d = fabs(tu1 - tu2);
   if (d > SELECT_TEXTURE)
      return false;

   d = fabs(tv1 - tv2);
   if (d > SELECT_TEXTURE)
      return false;

   return true;
}

void
Surface::OptimizeMesh()
{
   if (!vertex_set || vertex_set->nverts < 3)
      return;

   int nverts = vertex_set->nverts;
   int used   = 0;
   int final  = 0;
   int nmatch = 0;

   // create vertex maps:
   BYTE* vert_map = new BYTE[nverts];
   WORD* vert_dst = new WORD[nverts];
   ZeroMemory(vert_map, nverts * sizeof(BYTE));
   ZeroMemory(vert_dst, nverts * sizeof(WORD));

   // count used verts:
   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      for (int j = 0; j < p->nverts; j++) {
         WORD vert  = p->verts[j];

         if (vert < nverts) {
            vert_map[vert]++;
            used++;
         }
      }
   }

   // create target vertex set:
   VertexSet*  vset = new(__FILE__,__LINE__) VertexSet(used);
   int v = 0;

   // compress verts:
   for (int i = 0; i < nverts; i++) {
      if (vert_map[i] == 0) continue;

      vert_dst[i]       = v;
      vset->loc[v]      = vertex_set->loc[i];
      vset->nrm[v]      = vertex_set->nrm[i];
      vset->tu[v]       = vertex_set->tu[i];
      vset->tv[v]       = vertex_set->tv[i];
      vset->rw[v]       = vertex_set->rw[i];
      vset->diffuse[v]  = vertex_set->diffuse[i];
      vset->specular[v] = vertex_set->specular[i];

      for (int j = i+1; j < nverts; j++) {
         if (vert_map[j] == 0) continue;

         if (MatchVerts(vertex_set, i, j)) {
            vert_map[j] = 0;
            vert_dst[j] = v;
            nmatch++;
         }
      }

      v++;
   }

   final = v;

   // remap polys:
   for (int n = 0; n < npolys; n++) {
      Poly* p = polys + n;
      p->vertex_set = vset;
      for (int v = 0; v < p->nverts; v++) {
         p->verts[v] = vert_dst[ p->verts[v] ];
      }
   }

   // finalize:
   if (vset && final < nverts) {
      delete vertex_set;
      vertex_set = vset;
      vset->Resize(final, true);
      nverts = final;
   }

   // clean up and rebuild hull:
   delete [] vert_map;

   if (vloc)
      delete [] vloc;

   vloc = new(__FILE__,__LINE__) Vec3[nverts];

   ComputeTangents();
   BuildHull();
}

// +--------------------------------------------------------------------+

void
Surface::ScaleBy(double factor)
{
   offset *= factor;

   if (vertex_set && vertex_set->nverts) {
      for (int i = 0; i < vertex_set->nverts; i++) {
         vertex_set->loc[i] *= (float) factor;
      }
   }
}

// +--------------------------------------------------------------------+

void
Surface::BuildHull()
{
   if (npolys < 1 || !vertex_set || vertex_set->nverts < 1)
      return;

   nhull = 0;

   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      for (int n = 0; n < p->nverts; n++) {
         WORD v = p->verts[n];
         WORD h;

         for (h = 0; h < nhull; h++) {
            Vec3&  vl  = vertex_set->loc[v];
            Vec3&  loc = vloc[h];

            double d  = vl.x - loc.x;

            if (d < -SELECT_EPSILON || d > SELECT_EPSILON)
               continue;

            d = vl.y - loc.y;

            if (d < -SELECT_EPSILON || d > SELECT_EPSILON)
               continue;

            d = vl.z - loc.z;

            if (d < -SELECT_EPSILON || d > SELECT_EPSILON)
               continue;

            // found a match:
            break;
         }

         // didn't find a match:
         if (h >= nhull) {
            vloc[h] = vertex_set->loc[v];
            nhull = h+1;
         }

         p->vlocs[n] = h;
      }
   }

   if (use_collision_detection)
      InitializeCollisionHull();
}

// +--------------------------------------------------------------------+

void
Surface::Normalize()
{
   if (npolys < 1 || !vertex_set || vertex_set->nverts < 1)
      return;

   // STEP ONE: initialize poly planes

   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      p->plane = Plane( vertex_set->loc[ p->verts[0] ],
                        vertex_set->loc[ p->verts[2] ],
                        vertex_set->loc[ p->verts[1] ] );
   }

   // STEP TWO: compute vertex normals by averaging adjecent poly planes

   List<Poly> faces;
   for (int v = 0; v < vertex_set->nverts; v++) {
      faces.clear();
      SelectPolys(faces, vertex_set->loc[v]);

      if (faces.size()) {
         vertex_set->nrm[v] = Vec3(0.0f, 0.0f, 0.0f);

         for (int i = 0; i < faces.size(); i++) {
            vertex_set->nrm[v] += faces[i]->plane.normal;
         }

         vertex_set->nrm[v].Normalize();
      }

      else if (vertex_set->loc[v].length() > 0) {
         vertex_set->nrm[v] = vertex_set->loc[v];
         vertex_set->nrm[v].Normalize();
      }

      else {
         vertex_set->nrm[v] = Vec3(0.0f, 1.0f, 0.0f);
      }
   }

   // STEP THREE: adjust vertex normals for poly flatness

   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      for (int n = 0; n < p->nverts; n++) {
         int v = p->verts[n];

         vertex_set->nrm[v] = vertex_set->nrm[v] * (1.0f - p->flatness) +
                              p->plane.normal    * (       p->flatness);
      }
   }
}

void
Surface::SelectPolys(List<Poly>& selection, Vec3 loc)
{
   const double SELECT_EPSILON = 0.05;

   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      for (int n = 0; n < p->nverts; n++) {
         int    v  = p->verts[n];
         Vec3&  vl = vertex_set->loc[v];
         double d  = vl.x - loc.x;

         if (d < -SELECT_EPSILON || d > SELECT_EPSILON)
            continue;

         d = vl.y - loc.y;

         if (d < -SELECT_EPSILON || d > SELECT_EPSILON)
            continue;

         d = vl.z - loc.z;

         if (d < -SELECT_EPSILON || d > SELECT_EPSILON)
            continue;

         selection.append(p);
         break;
      }
   }
}

void
Surface::SelectPolys(List<Poly>& selection, Material* m)
{
   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      if (p->material == m)
         selection.append(p);
   }
}

// +--------------------------------------------------------------------+

void
Surface::ComputeTangents()
{
   Vec3  tangent;
   Vec3  binormal;

   if (!vertex_set || !vertex_set->nverts)
      return;

   if (vertex_set->tangent)
      return;

   vertex_set->CreateTangents();

   for (int i = 0; i < npolys; i++) {
      Poly* p = polys + i;

      CalcGradients(*p, tangent, binormal);

      for (int n = 0; n < p->nverts; n++) {
         vertex_set->tangent[p->verts[n]]    = tangent;
         vertex_set->binormal[p->verts[n]]   = binormal;
      }
   }
}

void
Surface::CalcGradients(Poly& p, Vec3& tangent, Vec3& binormal)
{
   // using Eric Lengyel's approach with a few modifications
   // from Mathematics for 3D Game Programmming and Computer Graphics
   // want to be able to trasform a vector in Object Space to Tangent Space
   // such that the x-axis cooresponds to the 's' direction and the
   // y-axis corresponds to the 't' direction, and the z-axis corresponds
   // to <0,0,1>, straight up out of the texture map

   VertexSet* vset = p.vertex_set;

   Vec3  P  = vset->loc[p.verts[1]] - vset->loc[p.verts[0]];
   Vec3  Q  = vset->loc[p.verts[2]] - vset->loc[p.verts[0]];

   float s1 = vset->tu[p.verts[1]]  - vset->tu[p.verts[0]];
   float t1 = vset->tv[p.verts[1]]  - vset->tv[p.verts[0]];
   float s2 = vset->tu[p.verts[2]]  - vset->tu[p.verts[0]];
   float t2 = vset->tv[p.verts[2]]  - vset->tv[p.verts[0]];

   float tmp   = 1.0f;
   float denom = s1*t2 - s2*t1;

   if (fabsf(denom) > 0.0001f)
      tmp = 1.0f/(denom);

   tangent.x   = (t2*P.x - t1*Q.x) * tmp;
   tangent.y   = (t2*P.y - t1*Q.y) * tmp;
   tangent.z   = (t2*P.z - t1*Q.z) * tmp;

   tangent.Normalize();

   binormal.x  = (s1*Q.x - s2*P.x) * tmp;
   binormal.y  = (s1*Q.y - s2*P.y) * tmp;
   binormal.z  = (s1*Q.z - s2*P.z) * tmp;

   binormal.Normalize();
}

void
Surface::InitializeCollisionHull()
{
   opcode = new(__FILE__,__LINE__) OPCODE_data(this);
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

Segment::Segment()
{
   ZeroMemory(this, sizeof(Segment));
}

Segment::Segment(int n, Poly* p, Material* mtl, Model* mod)
   : npolys(n), polys(p), material(mtl), model(mod), video_data(0)
{
}

Segment::~Segment()
{
   delete video_data;

   ZeroMemory(this, sizeof(Segment));
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

ModelFile::ModelFile(const char* fname)
   : model(0), pname(0), pnverts(0), pnpolys(0), pradius(0)
{
   int len = sizeof(filename);
   ZeroMemory(filename, len);
   strncpy_s(filename, fname, len);
   filename[len-1] = 0;
}

ModelFile::~ModelFile()
{
}

bool
ModelFile::Load(Model* m, double scale)
{
   model = m;

   // expose model innards for child classes:

   if (model) {
      pname    = model->name;
      pnverts  = &model->nverts;
      pnpolys  = &model->npolys;
      pradius  = &model->radius;
   }

   return false;
}

bool
ModelFile::Save(Model* m)
{
   model = m;

   // expose model innards for child classes:

   if (model) {
      pname    = model->name;
      pnverts  = &model->nverts;
      pnpolys  = &model->npolys;
      pradius  = &model->radius;
   }

   return false;
}

