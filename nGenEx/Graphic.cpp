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
