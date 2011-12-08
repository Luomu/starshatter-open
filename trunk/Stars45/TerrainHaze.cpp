/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

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
