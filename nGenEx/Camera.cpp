/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

