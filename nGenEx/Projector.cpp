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
    FILE:         Projector.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Projection Camera class
*/

#include "MemDebug.h"
#include "Projector.h"

// +--------------------------------------------------------------------+

static const float   CLIP_PLANE_EPSILON   = 0.0001f;
static const double  Z_NEAR               = 1.0;

void Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

static Camera  emergency_cam;

// +--------------------------------------------------------------------+

Projector::Projector(Window* window, Camera* cam)
: camera(cam), infinite(0), depth_scale(1.0f), orthogonal(false), field_of_view(2)
{
    if (!camera)
    camera = &emergency_cam;

    UseWindow(window);
}

Projector::~Projector()
{ }

// +--------------------------------------------------------------------+

void
Projector::UseCamera(Camera* cam)
{
    if (cam)
    camera = cam;
    else
    camera = &emergency_cam;
}

void
Projector::UseWindow(Window* win)
{
    Rect r  = win->GetRect();
    width   = r.w;
    height  = r.h;

    xcenter = (width  / 2.0);
    ycenter = (height / 2.0);

    xclip0  = 0.0f;
    xclip1  = (float) width-0.5f;
    yclip0  = 0.0f;
    yclip1  = (float) height-0.5f;

    SetFieldOfView(field_of_view);
}

void
Projector::SetFieldOfView(double fov)
{
    field_of_view = fov;

    xscreenscale = width  / fov;
    yscreenscale = height / fov;

    maxscale     = max(xscreenscale, yscreenscale);

    xangle       = atan(2.0/fov * maxscale/xscreenscale);
    yangle       = atan(2.0/fov * maxscale/yscreenscale);
}

double
Projector::GetFieldOfView() const
{
    return field_of_view;
}

void
Projector::SetDepthScale(float scale)
{
    depth_scale = scale;
}

double
Projector::GetDepthScale() const
{
    return depth_scale;
}

int
Projector::SetInfinite(int i)
{
    int old = infinite;
    infinite = i;
    return old;
}

// +--------------------------------------------------------------------+

void
Projector::StartFrame()
{
    SetUpFrustum();
    SetWorldSpace();
}

// +--------------------------------------------------------------------+
// Transform a point from worldspace to viewspace.
// +--------------------------------------------------------------------+

void
Projector::Transform(Vec3& vec) const
{
    Vec3 tvert = vec;

    // Translate into a viewpoint-relative coordinate
    if (!infinite)
    tvert -= camera->Pos();

    // old method:
    vec.x = (tvert * camera->vrt());
    vec.y = (tvert * camera->vup());
    vec.z = (tvert * camera->vpn());

    // Rotate into the view orientation
    // vec = tvert * camera->Orientation();
}

// +--------------------------------------------------------------------+
// Transform a point from worldspace to viewspace.
// +--------------------------------------------------------------------+

void
Projector::Transform(Point& point) const
{
    Point tvert = point;

    // Translate into a viewpoint-relative coordinate
    if (!infinite)
    tvert -= camera->Pos();

    // old method:
    point.x = (tvert * camera->vrt());
    point.y = (tvert * camera->vup());
    point.z = (tvert * camera->vpn());

    // Rotate into the view orientation
    // point = tvert * camera->Orientation();
}

// +--------------------------------------------------------------------+
// APPARENT RADIUS OF PROJECTED OBJECT
// Project a viewspace point into screen coordinates.
// Use projected Z to determine apparent radius of object.
// +--------------------------------------------------------------------+

float
Projector::ProjectRadius(const Vec3& v, float radius) const
{
    return (float) fabs((radius * maxscale) / v.z);
}

// +--------------------------------------------------------------------+
// IN PLACE PROJECTION OF POINT
// Project a viewspace point into screen coordinates.
// Note that the y axis goes up in worldspace and viewspace, but
// goes down in screenspace.
// +--------------------------------------------------------------------+

void
Projector::Project(Vec3& v, bool clamp) const
{
    double  zrecip;

    if (orthogonal) {
        double scale = field_of_view/2;
        v.x  = (float)           (xcenter + scale * v.x);
        v.y  = (float) (height - (ycenter + scale * v.y));
        v.z  = (float) (0.0f);
    }

    else {
        //zrecip = 2 * (1.0e5 / (1.0e5-1)) / v.z;
        //zrecip = 2 * 0.97 / v.z; -- what the heck was this version used for?

        zrecip = 2 / v.z;
        v.x  = (float)           (xcenter + maxscale * v.x * zrecip);
        v.y  = (float) (height - (ycenter + maxscale * v.y * zrecip));
        v.z  = (float) (1 - zrecip);
    }

    // clamp the point to the viewport:
    if (clamp) {
        if (v.x < xclip0) v.x = xclip0;
        if (v.x > xclip1) v.x = xclip1;
        if (v.y < yclip0) v.y = yclip0;
        if (v.y > yclip1) v.y = yclip1;
    }
}

// +--------------------------------------------------------------------+
// IN PLACE PROJECTION OF POINT
// Project a viewspace point into screen coordinates.
// Note that the y axis goes up in worldspace and viewspace, but
// goes down in screenspace.
// +--------------------------------------------------------------------+

void
Projector::Project(Point& v, bool clamp) const
{
    double  zrecip;

    if (orthogonal) {
        double scale = field_of_view/2;
        v.x  =           (xcenter + scale * v.x);
        v.y  = (height - (ycenter + scale * v.y));
        v.z  = 0;
    }

    else {
        zrecip = 1 / v.z;
        v.x  =           (xcenter + 2 * maxscale * v.x * zrecip);
        v.y  = (height - (ycenter + 2 * maxscale * v.y * zrecip));
        v.z  = (1 - zrecip);
    }

    // clamp the point to the viewport:
    if (clamp) {
        if (v.x < xclip0) v.x = xclip0;
        if (v.x > xclip1) v.x = xclip1;
        if (v.y < yclip0) v.y = yclip0;
        if (v.y > yclip1) v.y = yclip1;
    }
}

// +--------------------------------------------------------------------+
// IN PLACE UN-PROJECTION OF POINT
// Convert a point in screen coordinates back to viewspace.
// Note that the y axis goes up in worldspace and viewspace, but
// goes down in screenspace.
// +--------------------------------------------------------------------+

void
Projector::Unproject(Point& v) const
{
    double  zrecip = 1 / v.z;

    /***
    * forward projection:
v.x  = (xcenter + maxscale * v.x * zrecip);
v.y  = (height - (ycenter + maxscale * v.y * zrecip));
v.z  = (1 - zrecip);
***/

    v.x  = (         v.x - xcenter) / (maxscale * zrecip);
    v.y  = (height - v.y - ycenter) / (maxscale * zrecip);
}

// +--------------------------------------------------------------------+
// IN PLACE PROJECTION OF RECTANGLE (FOR SPRITES)
// Project a viewspace point into screen coordinates.
// Note that the y axis goes up in worldspace and viewspace, but
// goes down in screenspace.
// +--------------------------------------------------------------------+

void
Projector::ProjectRect(Point& v, double& w, double& h) const
{
    double  zrecip;

    if (orthogonal) {
        double scale = field_of_view/2;
        v.x  =           (xcenter + scale * v.x);
        v.y  = (height - (ycenter + scale * v.y));
        v.z  = 0;
    }

    else {
        zrecip = 1 / v.z;
        v.x  =           (xcenter + 2 * maxscale * v.x * zrecip);
        v.y  = (height - (ycenter + 2 * maxscale * v.y * zrecip));
        v.z  = (1 - Z_NEAR*zrecip);

        w   *= maxscale * zrecip;
        h   *= maxscale * zrecip;
    }
}

// +--------------------------------------------------------------------+
// Set up a clip plane with the specified normal.
// +--------------------------------------------------------------------+

void
Projector::SetWorldspaceClipPlane(Vec3& normal, Plane& plane)
{
    // Rotate the plane normal into worldspace
    ViewToWorld(normal, plane.normal);
    plane.distance = (float) (camera->Pos() * plane.normal + CLIP_PLANE_EPSILON);
}

// +--------------------------------------------------------------------+
// Set up the planes of the frustum, in worldspace coordinates.
// +--------------------------------------------------------------------+

void
Projector::SetUpFrustum()
{
    double  angle, s, c;
    Vec3    normal;

    angle = XAngle();
    s = sin(angle);
    c = cos(angle);

    // Left clip plane
    normal.x = (float) s;
    normal.y = (float) 0;
    normal.z = (float) c;
    view_planes[0].normal   = normal;
    view_planes[0].distance = CLIP_PLANE_EPSILON;
    SetWorldspaceClipPlane(normal, world_planes[0]);

    // Right clip plane
    normal.x = (float) -s;
    view_planes[1].normal   = normal;
    view_planes[1].distance = CLIP_PLANE_EPSILON;
    SetWorldspaceClipPlane(normal, world_planes[1]);

    angle = YAngle();
    s = sin(angle);
    c = cos(angle);

    // Bottom clip plane
    normal.x = (float) 0;
    normal.y = (float) s;
    normal.z = (float) c;
    view_planes[2].normal   = normal;
    view_planes[2].distance = CLIP_PLANE_EPSILON;
    SetWorldspaceClipPlane(normal, world_planes[2]);

    // Top clip plane
    normal.y = (float) -s;
    view_planes[3].normal   = normal;
    view_planes[3].distance = CLIP_PLANE_EPSILON;
    SetWorldspaceClipPlane(normal, world_planes[3]);
}

// +--------------------------------------------------------------------+
// Clip the point against the frustum and return 1 if partially inside
// Return 2 if completely inside
// +--------------------------------------------------------------------+

int
Projector::IsVisible(const Vec3& v, float radius) const
{
    int visible = 1;
    int complete = 1;

    Plane* plane = (Plane*) frustum_planes;
    if (infinite) {
        complete = 0;

        for (int i = 0; visible && (i < NUM_FRUSTUM_PLANES); i++) {
            visible = ((v * plane->normal) >= CLIP_PLANE_EPSILON);
            plane++;
        }
    }
    else {
        for (int i = 0; visible && (i < NUM_FRUSTUM_PLANES); i++) {
            float dot = v * plane->normal;
            visible  = ((dot + radius) >= plane->distance);
            complete = complete && ((dot - radius) >= plane->distance);
            plane++;
        }
    }

    return visible + complete;
}

// +--------------------------------------------------------------------+
// Clip the bouding point against the frustum and return non zero
// if at least partially inside.  This version is not terribly
// efficient as it checks all eight box corners rather than just
// the minimum two.
// +--------------------------------------------------------------------+

int
Projector::IsBoxVisible(const Point* p) const
{
    int   i, j, outside = 0;

    // if all eight corners are outside of the same
    // frustrum plane, then the box is not visible
    Plane* plane = (Plane*) frustum_planes;

    if (infinite) {
        for (i = 0; !outside && (i < NUM_FRUSTUM_PLANES); i++) {
            for (j = 0; j < 8; j++)
            outside += (p[j] * plane->normal) < CLIP_PLANE_EPSILON;

            if (outside < 8)
            outside = 0;

            plane++;
        }
    }
    else {
        for (i = 0; !outside && (i < NUM_FRUSTUM_PLANES); i++) {
            for (j = 0; j < 8; j++)
            outside += (p[j] * plane->normal) < plane->distance;

            if (outside < 8)
            outside = 0;

            plane++;
        }
    }

    // if not outside, then the box is visible
    return !outside;
}

// +--------------------------------------------------------------------+

float
Projector::ApparentRadius(const Vec3& v, float radius) const
{
    Vec3 vloc = v;

    Transform(vloc);                  // transform in place
    return ProjectRadius(vloc, radius);
}


// +--------------------------------------------------------------------+
// Rotate a vector from viewspace to worldspace.
// +--------------------------------------------------------------------+

void
Projector::ViewToWorld(Point& pin, Point& pout)
{
    // Rotate into the world orientation
    pout.x = pin.x * camera->vrt().x + pin.y * camera->vup().x + pin.z * camera->vpn().x;
    pout.y = pin.x * camera->vrt().y + pin.y * camera->vup().y + pin.z * camera->vpn().y;
    pout.z = pin.x * camera->vrt().z + pin.y * camera->vup().z + pin.z * camera->vpn().z;
}

void
Projector::ViewToWorld(Vec3& vin, Vec3& vout)
{
    // Rotate into the world orientation
    vout.x = (float) (vin.x * camera->vrt().x + vin.y * camera->vup().x + vin.z * camera->vpn().x);
    vout.y = (float) (vin.x * camera->vrt().y + vin.y * camera->vup().y + vin.z * camera->vpn().y);
    vout.z = (float) (vin.x * camera->vrt().z + vin.y * camera->vup().z + vin.z * camera->vpn().z);
}

