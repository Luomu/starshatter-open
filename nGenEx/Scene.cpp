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
    FILE:         Scene.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A 3D Scene
*/

#include "MemDebug.h"
#include "Scene.h"
#include "Graphic.h"
#include "Light.h"

void Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

Scene::Scene()
{ }

Scene::~Scene()
{ 
    background.destroy();
    foreground.destroy();
    graphics.destroy();
    sprites.destroy();

    lights.destroy();
}

// +--------------------------------------------------------------------+

void
Scene::AddBackground(Graphic* g)
{ 
    if (g) {
        if (!background.contains(g))
        background.append(g); 

        g->SetScene(this);
    }
}

void
Scene::DelBackground(Graphic* g)
{
    if (g) {
        background.remove(g);
        g->SetScene(0);
    }
}

// +--------------------------------------------------------------------+

void
Scene::AddForeground(Graphic* g)
{ 
    if (g) {
        if (!foreground.contains(g))
        foreground.append(g); 

        g->SetScene(this);
    }
}

void
Scene::DelForeground(Graphic* g)
{
    if (g) {
        foreground.remove(g);
        g->SetScene(0);
    }
}

// +--------------------------------------------------------------------+

void
Scene::AddGraphic(Graphic* g)
{ 
    if (g) {
        if (!graphics.contains(g))
        graphics.append(g); 

        g->SetScene(this);
    }
}

void
Scene::DelGraphic(Graphic* g)
{
    if (g) {
        graphics.remove(g)   || // it's gotta be in here somewhere!
        foreground.remove(g) || // use the logical-or operator to early
        sprites.remove(g)    || // out when we find it...
        background.remove(g);

        g->SetScene(0);
    }
}

// +--------------------------------------------------------------------+

void
Scene::AddSprite(Graphic* g)
{ 
    if (g) {
        if (!sprites.contains(g))
        sprites.append(g); 

        g->SetScene(this);
    }
}

void
Scene::DelSprite(Graphic* g)
{
    if (g) {
        sprites.remove(g);
        g->SetScene(0);
    }
}

// +--------------------------------------------------------------------+

void
Scene::AddLight(Light* l)
{
    if (l) {
        if (!lights.contains(l))
        lights.append(l);
        l->SetScene(this);
    }
}

void
Scene::DelLight(Light* l)
{
    if (l) {
        lights.remove(l);
        l->SetScene(0);
    }
}

// +--------------------------------------------------------------------+

void
Scene::Collect()
{
    ListIter<Graphic> iter = graphics;

    while (++iter) {
        Graphic* g = iter.value();
        if (g->Life() == 0) {
            delete iter.removeItem();
        }
    }   

    iter.attach(sprites);

    while (++iter) {
        Graphic* g = iter.value();
        if (g->Life() == 0) {
            delete iter.removeItem();
        }
    }   

    ListIter<Light> iter1 = lights;

    while (++iter1) {
        Light* l = iter1.value();
        if (l->Life() == 0) {
            delete iter1.removeItem();
        }
    }   
}


// +--------------------------------------------------------------------+

bool
Scene::IsLightObscured(const Point& obj_pos, const Point& light_pos, double obj_radius, Point* impact_point) const
{
    Point  dir = light_pos - obj_pos;
    double len = dir.Normalize();

    Scene*      pThis    = (Scene*) this; // cast-away const
    Graphic*    g        = 0;
    bool        obscured = false;

    ListIter<Graphic> g_iter = pThis->graphics;
    while (++g_iter && !obscured) {
        g = g_iter.value();

        if (g->CastsShadow() && !g->Hidden() && !g->IsInfinite()) {
            double gdist = (g->Location() - obj_pos).length();
            if (gdist > 0.1 &&                     // different than object being obscured
                    g->Radius() > obj_radius &&        // larger than object being obscured
                    (g->Radius()*400)/gdist > 10) {    // projects to a resonable size

                Point  delta = (g->Location() - light_pos);

                if (delta.length() > g->Radius() / 100) { // not the object that is emitting the light
                    Point  impact;
                    obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;

                    if (impact_point)
                    *impact_point = impact;
                }
            }

            else if (obj_radius < 0 && gdist < 0.1) { // special case for camera (needed for cockpits)
                Point  delta = (g->Location() - light_pos);

                if (delta.length() > g->Radius() / 100) { // not the object that is emitting the light
                    Point  impact;
                    obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;
                }
            }
        }
    }

    g_iter.attach(pThis->foreground);
    while (++g_iter && !obscured) {
        g = g_iter.value();

        if (g->CastsShadow() && !g->Hidden()) {
            double gdist = (g->Location() - obj_pos).length();
            if (gdist > 0.1 &&                     // different than object being obscured
                    g->Radius() > obj_radius &&        // larger than object being obscured
                    (g->Radius()*400)/gdist > 10) {    // projects to a resonable size

                Point  delta = (g->Location() - light_pos);

                if (delta.length() > g->Radius() / 100) { // not the object that is emitting the light
                    Point  impact;
                    obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;

                    if (impact_point)
                    *impact_point = impact;
                }
            }

            else if (obj_radius < 0 && gdist < 0.1) { // special case for camera (needed for cockpits)
                Point  delta = (g->Location() - light_pos);

                if (delta.length() > g->Radius() / 100) { // not the object that is emitting the light
                    Point  impact;
                    obscured = g->CheckRayIntersection(obj_pos, dir, len, impact, false) ? true : false;
                }
            }
        }
    }

    return obscured;
}
