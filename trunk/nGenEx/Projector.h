/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Projector.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Projection Camera class
*/

#ifndef Projector_h
#define Projector_h

#include "Geometry.h"
#include "Window.h"
#include "Camera.h"
#include "Polygon.h"

// +--------------------------------------------------------------------+

class Projector
{
public:
   Projector(Window* win, Camera* cam);
   virtual ~Projector();

   // Operations:
   virtual void   UseWindow(Window* win);
   virtual void   UseCamera(Camera* cam);
   virtual void   SetDepthScale(float scale);
   virtual double GetDepthScale() const;
   virtual void   SetFieldOfView(double fov);
   virtual double GetFieldOfView() const;
   virtual int    SetInfinite(int i);
   virtual void   StartFrame();

   // accessor:
   Point          Pos() const { return camera->Pos(); }
   Point          vrt()       { return camera->vrt(); }
   Point          vup()       { return camera->vup(); }
   Point          vpn()       { return camera->vpn(); }
   const Matrix&  Orientation() const { return camera->Orientation(); }

   double         XAngle() const { return xangle; }
   double         YAngle() const { return yangle; }

   bool           IsOrthogonal()    const { return orthogonal; }
   void           SetOrthogonal(bool o)   { orthogonal = o;    }

   // projection and clipping geometry:
   virtual void   Transform(Vec3& vec)    const;
   virtual void   Transform(Point& point) const;

   virtual void   Project(Vec3& vec, bool clamp=true) const;
   virtual void   Project(Point& point, bool clamp=true) const;
   virtual void   ProjectRect(Point& origin, double& w, double& h) const;

   virtual float  ProjectRadius(const Vec3& vec, float radius) const;

   virtual void   Unproject(Point& point) const;
   int            IsVisible(const Vec3& v, float radius) const;
   int            IsBoxVisible(const Point* p) const;

   float          ApparentRadius(const Vec3& v, float radius) const;

   virtual void   SetWorldSpace()   { frustum_planes = world_planes; }
   virtual void   SetViewSpace()    { frustum_planes = view_planes;  }

   Plane*         GetCurrentClipPlanes()  { return frustum_planes; }

   void           SetUpFrustum();
   void           ViewToWorld(Point& pin, Point& pout);
   void           ViewToWorld(Vec3&  vin, Vec3&  vout);
   void           SetWorldspaceClipPlane(Vec3& normal, Plane& plane);

protected:
   Camera*        camera;
   
   int            width, height;
   double         field_of_view;
   double         xscreenscale, yscreenscale, maxscale;
   double         xcenter, ycenter;
   double         xangle,  yangle;

   int            infinite;
   float          depth_scale;
   bool           orthogonal;

   enum DISPLAY_CONST {
      NUM_FRUSTUM_PLANES= 4,
   };

   Plane*         frustum_planes;
   Plane          world_planes[NUM_FRUSTUM_PLANES];
   Plane          view_planes[NUM_FRUSTUM_PLANES];

   float          xclip0, xclip1;
   float          yclip0, yclip1;
};

#endif Projector_h

