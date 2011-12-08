/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Trail.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Missile Trail (Graphic) class
*/

#ifndef Trail_h
#define Trail_h

#include "Types.h"
#include "Geometry.h"
#include "Polygon.h"
#include "SimObject.h"
#include "Graphic.h"

// +--------------------------------------------------------------------+

class Trail : public Graphic
{
public:
   Trail(Bitmap* tex, int n=512);
   virtual ~Trail();

   virtual void      UpdateVerts(const Point& cam_pos);
   virtual void      Render(Video* video, DWORD flags);
   virtual void      AddPoint(const Point& v);
   virtual double    AverageLength();

   virtual void      SetWidth(double w)   { width = w; }
   virtual void      SetDim(int d)        { dim   = d; }

protected:
   int            ntrail;
   int            maxtrail;
   Point*         trail;

   double         length;
   double         width;
   int            dim;
   
   int            npolys, nverts;
   Poly*          polys;
   VertexSet*     verts;
   Bitmap*        texture;
   Material       mtl;

   double         length0, length1;
   double         last_point_time;
};

#endif Trail_h

