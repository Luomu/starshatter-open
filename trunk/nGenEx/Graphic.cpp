/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Graphic.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract 3D Graphic Object
*/

#include "MemDebug.h"
#include "Graphic.h"
#include "Scene.h"
#include "Projector.h"

// +--------------------------------------------------------------------+

int Graphic::id_key = 1;

// +--------------------------------------------------------------------+

Graphic::Graphic()
   : id(id_key++), visible(true), loc(0.0f, 0.0f, 0.0f),
     radius(0.0f), infinite(0), foreground(0), hidden(0), life(-1),
     trans(false), shadow(false), luminous(false), depth(0.0f), scene(0)
{
   screen_rect.x = 0;
   screen_rect.y = 0;
   screen_rect.w = 0;
   screen_rect.h = 0;

   ZeroMemory(name, sizeof(name));
   strcpy_s(name, "Graphic");
}

// +--------------------------------------------------------------------+

Graphic::~Graphic()
{ }

int
Graphic::operator < (const Graphic& g) const
{
   if (!infinite && g.infinite)
      return 1;

   else if (infinite && !g.infinite)
      return 0;

   double za = fabs(Depth());
   double zb = fabs(g.Depth());

   return (za < zb);
}

int
Graphic::operator <= (const Graphic& g) const
{
   if (!infinite && g.infinite)
      return 1;

   else if (infinite && !g.infinite)
      return 0;

   double za = fabs(Depth());
   double zb = fabs(g.Depth());

   return (za <= zb);
}

// +--------------------------------------------------------------------+

void
Graphic::SetInfinite(bool b)
{
   infinite = (BYTE) b;

   if (infinite)
      depth = 1.0e9f;
}

// +--------------------------------------------------------------------+

int
Graphic::Nearer(Graphic* a, Graphic* b)
{
   if (a->depth < b->depth) return -1;
   else if (a->depth == b->depth) return 0;
   else return 1;
}

// +--------------------------------------------------------------------+

int
Graphic::Farther(Graphic* a, Graphic* b)
{
   if (a->depth > b->depth) return -1;
   else if (a->depth == b->depth) return 0;
   else return 1;
}

// +--------------------------------------------------------------------+

void
Graphic::Destroy()
{
   if (scene)
      scene->DelGraphic(this);

   delete this;
}

// +--------------------------------------------------------------------+

int
Graphic::CollidesWith(Graphic& o)
{
   Point delta_loc = loc - o.loc;

   // bounding spheres test:
   if (delta_loc.length() > radius + o.radius)
      return 0;

   return 1;
}

// +--------------------------------------------------------------------+

int
Graphic::CheckRayIntersection(Point Q, Point w, double len, Point& ipt,
                              bool treat_translucent_polys_as_solid)
{
   return 0;
}

// +--------------------------------------------------------------------+

void
Graphic::ProjectScreenRect(Projector* p)
{
   screen_rect.x = 2000;
   screen_rect.y = 2000;
   screen_rect.w = 0;
   screen_rect.h = 0;
}

// +--------------------------------------------------------------------+

bool
Graphic::CheckVisibility(Projector& projector)
{
   if (projector.IsVisible(     Location(), Radius()) &&
       projector.ApparentRadius(Location(), Radius()) > 1) {

      visible        = true;
   }
   else {
      visible        = false;
      screen_rect.x  = 2000;
      screen_rect.y  = 2000;
      screen_rect.w  = 0;
      screen_rect.h  = 0;
   }
       
   return visible;
}
