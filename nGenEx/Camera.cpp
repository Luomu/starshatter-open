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
    FILE:         Camera.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Camera Class - Position and Point of View
*/

#include "MemDebug.h"
#include "Camera.h"

// +--------------------------------------------------------------------+

Camera::Camera(double x, double y, double z)
    : pos(x,y,z)
{ }

Camera::~Camera()
{ }

// +--------------------------------------------------------------------+

void
Camera::MoveTo(double x, double y, double z)
{
    pos.x = x;
    pos.y = y;
    pos.z = z;
}

void
Camera::MoveTo(const Point& p)
{
    pos.x = p.x;
    pos.y = p.y;
    pos.z = p.z;
}

// +--------------------------------------------------------------------+

void
Camera::MoveBy(double dx, double dy, double dz)
{
    pos.x += dx;
    pos.y += dz;
    pos.z += dy;
}

void
Camera::MoveBy(const Point& p)
{
    pos.x += p.x;
    pos.y += p.y;
    pos.z += p.z;
}

// +--------------------------------------------------------------------+

void
Camera::Clone(const Camera& cam)
{
    pos         = cam.pos;
    orientation = cam.orientation;
}

// +--------------------------------------------------------------------+

void
Camera::LookAt(const Point& target, const Point& eye, const Point& up)
{
    Point zaxis = target - eye;         zaxis.Normalize();
    Point xaxis = up.cross(zaxis);      xaxis.Normalize();
    Point yaxis = zaxis.cross(xaxis);   yaxis.Normalize();

    orientation(0,0)  = xaxis.x;
    orientation(0,1)  = xaxis.y;
    orientation(0,2)  = xaxis.z;

    orientation(1,0)  = yaxis.x;
    orientation(1,1)  = yaxis.y;
    orientation(1,2)  = yaxis.z;

    orientation(2,0)  = zaxis.x;
    orientation(2,1)  = zaxis.y;
    orientation(2,2)  = zaxis.z;

    pos               = eye;
}

// +--------------------------------------------------------------------+

void
Camera::LookAt(const Point& target)
{
    // No navel gazing:
    if (target == Pos())
    return;

    Point tgt, tmp = target - Pos();

    // Rotate into the view orientation:
    tgt.x = (tmp * vrt());
    tgt.y = (tmp * vup());
    tgt.z = (tmp * vpn());

    if (tgt.z == 0) {
        Pitch(0.5);
        Yaw(0.5);
        LookAt(target);
        return;
    }

    double az = atan(tgt.x/tgt.z);
    double el = atan(tgt.y/tgt.z);

    // if target is behind, offset by 180 degrees:
    if (tgt.z < 0)
    az -= PI;

    Pitch(-el);
    Yaw(az);

    // roll to upright position:
    double deflection = vrt().y;
    while  (fabs(deflection) > 0.001) {
        double theta = asin(deflection/vrt().length());
        Roll(-theta);

        deflection = vrt().y;
    }
}


// +--------------------------------------------------------------------+

bool
Camera::Padlock(const Point& target, double alimit, double e_lo, double e_hi)
{
    // No navel gazing:
    if (target == Pos())
    return false;

    Point tgt, tmp = target - Pos();

    // Rotate into the view orientation:
    tgt.x = (tmp * vrt());
    tgt.y = (tmp * vup());
    tgt.z = (tmp * vpn());

    if (tgt.z == 0) {
        Yaw(0.1);

        tgt.x = (tmp * vrt());
        tgt.y = (tmp * vup());
        tgt.z = (tmp * vpn());

        if (tgt.z == 0)
        return false;
    }

    bool   locked  = true;
    double az      = atan(tgt.x/tgt.z);
    double orig    = az;

    // if target is behind, offset by 180 degrees:
    if (tgt.z < 0)
    az -= PI;

    while (az >  PI) az -= 2*PI;
    while (az < -PI) az += 2*PI;

    if (alimit > 0) {
        if (az < -alimit) {
            az = -alimit;
            locked = false;
        }
        else if (az > alimit) {
            az = alimit;
            locked = false;
        }
    }

    Yaw(az);

    // Rotate into the new view orientation:
    tgt.x = (tmp * vrt());
    tgt.y = (tmp * vup());
    tgt.z = (tmp * vpn());

    double el      = atan(tgt.y/tgt.z);

    if (e_lo > 0 && el < -e_lo) {
        el = -e_lo;
        locked = false;
    }

    else if (e_hi > 0 && el > e_hi) {
        el = e_hi;
        locked = false;
    }

    Pitch(-el);

    return locked;
}

