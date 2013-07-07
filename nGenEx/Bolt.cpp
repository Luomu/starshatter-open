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
    FILE:         Bolt.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Bolt (Polygon) Object
*/

#include "MemDebug.h"
#include "Bolt.h"
#include "Bitmap.h"
#include "Camera.h"
#include "Video.h"

void  Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

Bolt::Bolt(double len, double wid, Bitmap* tex, int share)
    : vset(4), poly(0), texture(tex), length(len), width(wid), shade(1.0),
      vpn(0, 1, 0), shared(share)
{
    trans = true;

    loc = Vec3(0.0f, 0.0f, 1000.0f);

    vset.nverts = 4;

    vset.loc[0] = Point( width, 0,       1000);
    vset.loc[1] = Point( width, -length, 1000);
    vset.loc[2] = Point(-width, -length, 1000);
    vset.loc[3] = Point(-width, 0,       1000);

    vset.tu[0]  = 0.0f;
    vset.tv[0]  = 0.0f;
    vset.tu[1]  = 1.0f;
    vset.tv[1]  = 0.0f;
    vset.tu[2]  = 1.0f;
    vset.tv[2]  = 1.0f;
    vset.tu[3]  = 0.0f;
    vset.tv[3]  = 1.0f;

    Plane plane(vset.loc[0], vset.loc[1], vset.loc[2]);

    for (int i = 0; i < 4; i++) {
        vset.nrm[i] = plane.normal;
    }

    mtl.Ka            = Color::White;
    mtl.Kd            = Color::White;
    mtl.Ks            = Color::Black;
    mtl.Ke            = Color::White;
    mtl.tex_diffuse   = texture;
    mtl.tex_emissive  = texture;
    mtl.blend         = Video::BLEND_ADDITIVE;

    poly.nverts       = 4;
    poly.vertex_set   = &vset;
    poly.material     = &mtl;
    poly.verts[0]     = 0;
    poly.verts[1]     = 1;
    poly.verts[2]     = 2;
    poly.verts[3]     = 3;

    radius = (float) ((length>width) ? (length) : (width*2));

    if (texture) {
        strncpy_s(name, texture->GetFilename(), 31);
        name[31] = 0;
    }
}

// +--------------------------------------------------------------------+

Bolt::~Bolt()
{
}

// +--------------------------------------------------------------------+

void
Bolt::Render(Video* video, DWORD flags)
{
    if ((flags & RENDER_ADDITIVE) == 0)
    return;

    if (visible && !hidden && video && life) {
        const Camera*  camera = video->GetCamera();

        Point head  = loc;
        Point tail  = origin;
        Point vtail = tail - head;
        Point vcam  = camera->Pos() - loc;
        Point vtmp  = vcam.cross(vtail);
        vtmp.Normalize();
        Point vlat  = vtmp * -width;
        Vec3  vnrm  = camera->vpn() * -1;

        vset.loc[0] = head + vlat;
        vset.loc[1] = tail + vlat;
        vset.loc[2] = tail - vlat;
        vset.loc[3] = head - vlat;

        vset.nrm[0] = vnrm;
        vset.nrm[1] = vnrm;
        vset.nrm[2] = vnrm;
        vset.nrm[3] = vnrm;

        ColorValue  white((float) shade, (float) shade, (float) shade);
        mtl.Ka = white;
        mtl.Kd = white;
        mtl.Ks = Color::Black;
        mtl.Ke = white;

        video->DrawPolys(1, &poly);
    }
}

// +--------------------------------------------------------------------+

void
Bolt::Update()
{
}

// +--------------------------------------------------------------------+

void
Bolt::TranslateBy(const Point& ref)
{
    loc    = loc    - ref;
    origin = origin - ref;
}

// +--------------------------------------------------------------------+

void
Bolt::SetOrientation(const Matrix& o)
{
    vpn = Point(o(2,0), o(2,1), o(2,2));
    origin = loc + (vpn * -length);
}

void
Bolt::SetDirection(const Point& v)
{ 
    vpn = v;
    origin = loc + (vpn * -length);
}

void
Bolt::SetEndPoints(const Point& from, const Point& to)
{
    loc    = to;
    origin = from;
    vpn    = to - from;
    length = vpn.Normalize();
    radius = (float) length;
}

void
Bolt::SetTextureOffset(double from, double to)
{
    vset.tu[0] = (float) from;
    vset.tu[1] = (float) to;
    vset.tu[2] = (float) to;
    vset.tu[3] = (float) from;
}


