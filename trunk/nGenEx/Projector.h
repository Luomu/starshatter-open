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

