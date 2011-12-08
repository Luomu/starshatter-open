/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Light.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Dynamic Light Source
*/

#include "MemDebug.h"
#include "Light.h"
#include "Scene.h"

// +--------------------------------------------------------------------+

int Light::id_key = 1;

// +--------------------------------------------------------------------+

Light::Light(float l, float dl, int time)
   : id(id_key++), type(LIGHT_POINT), life(time), 
     light(l), dldt(dl), color(255,255,255),
     active(true), shadow(false), scene(0)
{ }

// +--------------------------------------------------------------------+

Light::~Light()
{ }

// +--------------------------------------------------------------------+

void
Light::Update()
{
   if (dldt < 1.0f)
      light *= dldt;

   if (life > 0) life--;
}

// +--------------------------------------------------------------------+

void
Light::Destroy()
{
   if (scene)
      scene->DelLight(this);

   delete this;
}

// +--------------------------------------------------------------------+

void
Light::MoveTo(const Point& dst)
{
   //if (type != LIGHT_DIRECTIONAL)
   loc = dst;
}

void
Light::TranslateBy(const Point& ref)
{
   if (type != LIGHT_DIRECTIONAL)
      loc = loc - ref;
}
