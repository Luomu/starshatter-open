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

    SUBSYSTEM:    Stars.exe
    FILE:         Asteroid.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Asteroid Sprite animation class
*/

#include "MemDebug.h"
#include "Asteroid.h"
#include "Shot.h"
#include "Explosion.h"
#include "Sim.h"

#include "Solid.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"

// +--------------------------------------------------------------------+

static Point   asteroid_velocity = Point(0,0,0);
static Model*  asteroid_model[32];

// +--------------------------------------------------------------------+

Asteroid::Asteroid(int t, const Vec3& pos, double m)
    : Debris(asteroid_model[t%6], pos, asteroid_velocity, m)
{
    life = -1;
}

// +--------------------------------------------------------------------+

void
Asteroid::Initialize()
{
    ZeroMemory(asteroid_model, sizeof(asteroid_model));

    DataLoader* loader = DataLoader::GetLoader();
    Text old_path = loader->GetDataPath();
    loader->SetDataPath("Galaxy/Asteroids/");

    int n = 0;

    Model* a = new(__FILE__,__LINE__) Model;
    if (a) {
        a->Load("a1.mag", 100);
        asteroid_model[n++] = a;
    }

    a = new(__FILE__,__LINE__) Model;
    if (a) {
        a->Load("a2.mag", 50);
        asteroid_model[n++] = a;
    }

    a = new(__FILE__,__LINE__) Model;
    if (a) {
        a->Load("a1.mag",  8);
        asteroid_model[n++] = a;
    }

    a = new(__FILE__,__LINE__) Model;
    if (a) {
        a->Load("a2.mag", 10);
        asteroid_model[n++] = a;
    }

    a = new(__FILE__,__LINE__) Model;
    if (a) {
        a->Load("a3.mag", 30);
        asteroid_model[n++] = a;
    }

    a = new(__FILE__,__LINE__) Model;
    if (a) {
        a->Load("a4.mag", 20);
        asteroid_model[n++] = a;
    }

    List<Text> mod_asteroids;
    loader->SetDataPath("Mods/Galaxy/Asteroids/");
    loader->ListFiles("*.mag", mod_asteroids);

    ListIter<Text> iter = mod_asteroids;
    while (++iter && n < 32) {
        a = new(__FILE__,__LINE__) Model;
        if (a) {
            a->Load(*iter.value(), 50);
            asteroid_model[n++] = a;
        }
    }

    loader->SetDataPath(old_path);
}

void
Asteroid::Close()
{
    for (int i = 0; i < 32; i++)
    delete asteroid_model[i];

    ZeroMemory(asteroid_model, sizeof(asteroid_model));
}

// +--------------------------------------------------------------------+



