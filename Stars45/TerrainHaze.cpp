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
    FILE:         TerrainHaze.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "Terrain.h"
#include "TerrainHaze.h"
#include "TerrainRegion.h"

#include "Light.h"
#include "CameraView.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"

// +====================================================================+

static Bitmap terrain_texture;

// +--------------------------------------------------------------------+

TerrainHaze::TerrainHaze()
: tregion(0)
{
}

// +--------------------------------------------------------------------+

TerrainHaze::~TerrainHaze()
{
}

// +--------------------------------------------------------------------+

void
TerrainHaze::Render(Video* video, DWORD flags)
{
    if (flags & RENDER_ADDITIVE)
    return;

    if (model) {
        if (!Luminous()) {
            SetLuminous(true);
            model->SetDynamic(true);
        }

        Surface* surface = model->GetSurfaces().first();

        if (!surface) return;

        int i;
        DWORD sky = 0;
        DWORD fog = 0;

        if (tregion) {
            sky = tregion->SkyColor().Value();
            fog = tregion->FogColor().Value();
        }

        // clear the solid lights to ambient:
        VertexSet* vset = surface->GetVertexSet();

        for (i = 0; i < vset->nverts; i++) {
            if (vset->loc[i].y > 0)
            vset->diffuse[i] = sky;
            else
            vset->diffuse[i] = fog;
        }

        InvalidateSurfaceData();
        Solid::Render(video, flags);
    }
}

// +--------------------------------------------------------------------+

int
TerrainHaze::CheckRayIntersection(Point Q, Point w, double len, Point& ipt, bool ttpas)
{
    return 0;
}
