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
    FILE:         ShieldRep.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ShieldRep Solid class
*/

#include "MemDebug.h"
#include "ShieldRep.h"
#include "Random.h"

#include "Game.h"
#include "Light.h"
#include "Solid.h"
#include "Bitmap.h"
#include "Color.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+

const int MAX_SHIELD_HITS = 16;

// +--------------------------------------------------------------------+

struct ShieldHit
{
    Vec3     hitloc;
    double   damage;
    double   age;
    Shot*    shot;

    ShieldHit() : damage(0), age(0), shot(0) { }
};

// +--------------------------------------------------------------------+

ShieldRep::ShieldRep()
{
    bubble      = false;
    luminous    = true;
    trans       = true;
    nhits       = 0;

    hits = new(__FILE__,__LINE__) ShieldHit[MAX_SHIELD_HITS];
}

ShieldRep::~ShieldRep()
{
    delete [] hits;
}

// +--------------------------------------------------------------------+

void
ShieldRep::Hit(Vec3 impact, Shot* shot, double damage)
{
    if (!model || model->GetSurfaces().size() < 1)
    return;

    // transform impact into object space:
    Matrix xform(Orientation());

    Vec3 tmp = impact - loc;

    impact.x = tmp * Vec3(xform(0,0), xform(0,1), xform(0,2));
    impact.y = tmp * Vec3(xform(1,0), xform(1,1), xform(1,2));
    impact.z = tmp * Vec3(xform(2,0), xform(2,1), xform(2,2));

    // find slot to store the hit:
    int    i;
    int    slot = -1;
    double age  = -1;

    for (i = 0; i < MAX_SHIELD_HITS; i++) {
        if (hits[i].shot == shot) {
            slot = i;
            break;
        }
    }

    if (slot < 0) {
        for (i = 0; i < MAX_SHIELD_HITS; i++) {
            if (hits[i].damage <= 0) {
                slot = i;
                break;
            }

            if (hits[i].age > age) {
                slot = i;
                age  = hits[i].age;
            }
        }
    }

    if (slot >= 0 && slot < MAX_SHIELD_HITS) {
        // record the hit in the slot:
        hits[slot].hitloc  = impact;
        hits[slot].damage  = damage;
        hits[slot].age     = 1;
        hits[slot].shot    = shot;

        if (nhits < MAX_SHIELD_HITS)
        nhits++;
    }
}

// +--------------------------------------------------------------------+

void
ShieldRep::Energize(double seconds, bool b)
{
    bubble = b;

    if (nhits < 1) return;

    nhits = 0;

    for (int i = 0; i < MAX_SHIELD_HITS; i++) {
        if (hits[i].damage > 0) {
            // age the hit:
            hits[i].age += seconds;
            hits[i].damage -= (hits[i].damage * 4 * seconds);

            // collect garbage:
            if (hits[i].damage < 10) {
                hits[i].age     = 0;
                hits[i].damage  = 0;
                hits[i].shot    = 0;
            }
            else {
                nhits++;
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
ShieldRep::TranslateBy(const Point& ref)
{
    true_eye_point = ref;
    Solid::TranslateBy(ref);
}

// +--------------------------------------------------------------------+

void
ShieldRep::Illuminate()
{
    if (!model) return;

    Surface*   surf   = model->GetSurfaces().first();
    VertexSet* vset   = surf->GetVertexSet();
    int        nverts = vset->nverts;

    for (int i = 0; i < nverts; i++) {
        vset->diffuse[i]  = 0;
        vset->specular[i] = 0;
    }

    double all_damage = 0;

    if (nhits < 1) return;

    for (int i = 0; i < MAX_SHIELD_HITS; i++) {
        if (hits[i].damage > 0) {
            // add the hit's contribution to the shield verts:
            Vec3   hitloc = hits[i].hitloc;
            double hitdam = hits[i].damage * 2000;

            all_damage += hits[i].damage;

            if (!bubble) {

                double limit = radius * radius;
                if (hitdam > limit)
                hitdam = limit;

                for (int v = 0; v < nverts; v++) {
                    double dist  = (vset->loc[v] - hitloc).length();

                    if (dist < 1)
                    dist  = 1;  // can't divide by zero!

                    else
                    dist = pow(dist, 2.7);

                    double pert      = Random(0.1, 1.5);
                    double intensity = pert*hitdam/dist;

                    if (intensity > 0.003)
                    vset->diffuse[v] = ((Color::White * intensity)  + vset->diffuse[v]).Value();
                }

            }
        }
    }

    if (bubble) {
        double shield_gain = 1;

        if (all_damage < 1000) {
            shield_gain = all_damage / 1000;
        }

        for (int i = 0; i < nverts; i++) {
            Vec3  vloc = (vset->loc[i] * orientation) + loc;
            Vec3  vnrm = (vset->nrm[i] * orientation);

            Vec3  V   = vloc * -1.0f;
            V.Normalize();

            double intensity = 1 - V*vnrm;

            if (intensity > 0) {
                intensity *= intensity;

                if (intensity > 1) intensity = 1;

                intensity *= (shield_gain * Random(0.75, 1.0));

                Color vs = Color::White * intensity;
                vset->diffuse[i] = vs.Value();
            }
        }
    }

    InvalidateSurfaceData();
}

void
ShieldRep::Render(Video* video, DWORD flags)
{
    if ((flags & RENDER_ADDITIVE) == 0)
    return;

    if (nhits > 0) {
        Illuminate();
        Solid::Render(video, RENDER_ALPHA); // have to lie about the render flag
        // or the engine will reject the solid
    }
}