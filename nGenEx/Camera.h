/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

