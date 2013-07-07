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
    FILE:         Camera.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Camera class - Position and Point of View
*/

#ifndef Camera_h
#define Camera_h

// +--------------------------------------------------------------------+

#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Camera
{
public:
    static const char* TYPENAME() { return "Camera"; }

    Camera(double x=0.0, double y=0.0, double z=0.0);
    virtual ~Camera();

    void     Aim(double roll, double pitch, double yaw)   { orientation.Rotate(roll, pitch, yaw);   }
    void     Roll(double roll)                            { orientation.Roll(roll);                 }
    void     Pitch(double pitch)                          { orientation.Pitch(pitch);               }
    void     Yaw(double yaw)                              { orientation.Yaw(yaw);                   }

    void     MoveTo(double x,  double y,  double z);
    void     MoveTo(const Point& p);
    void     MoveBy(double dx, double dy, double dz);
    void     MoveBy(const Point& p);

    void     Clone(const Camera& cam);
    void     LookAt(const Point& target);
    void     LookAt(const Point& target, const Point& eye, const Point& up);
    bool     Padlock(const Point& target, double alimit=-1, double e_lo=-1, double e_hi=-1);

    Point    Pos() const { return pos;    }
    Point    vrt() const { return Point(orientation(0,0), orientation(0,1), orientation(0,2)); }
    Point    vup() const { return Point(orientation(1,0), orientation(1,1), orientation(1,2)); }
    Point    vpn() const { return Point(orientation(2,0), orientation(2,1), orientation(2,2)); }

    const Matrix&  Orientation() const { return orientation; }

protected:
    Point    pos;
    Matrix   orientation;
};

#endif Camera_h

