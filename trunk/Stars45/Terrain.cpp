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
    FILE:         Terrain.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "Terrain.h"
#include "TerrainApron.h"
#include "TerrainClouds.h"
#include "TerrainLayer.h"
#include "TerrainPatch.h"
#include "TerrainRegion.h"
#include "Water.h"

#include "CameraView.h"
#include "Projector.h"
#include "Scene.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "Game.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+

int   Terrain::detail_level      = 3; // default = MEDIUM DETAIL

const int PATCH_SIZE             = 16;

// +--------------------------------------------------------------------+

Terrain::Terrain(TerrainRegion* trgn)
: region(trgn), patches(0), water_patches(0), water(0), 
aprons(0), clouds(0), terrain_normals(0)
{
    detail_frame      = 0;
    datapath          = "Galaxy/";
    terrain_texture   = 0;
    apron_texture     = 0;
    water_texture     = 0;

    for (int i = 0; i < 2; i++) {
        cloud_texture[i] = 0;
        shade_texture[i] = 0;
        noise_texture[i] = 0;
    }

    if (region) {
        scale    = region->LateralScale();
        mtnscale = region->MountainScale();

        ListIter<TerrainLayer> iter = region->GetLayers();
        while (++iter) {
            TerrainLayer* orig = iter.value();
            TerrainLayer* copy = new(__FILE__,__LINE__) TerrainLayer;
            *copy = *orig;
            layers.append(copy);
        }

        layers.sort();
    }

    else {
        scale    = 1;
        mtnscale = 1;
    }
}

// +--------------------------------------------------------------------+

Terrain::~Terrain()
{
    int i, j;

    if (patches)
    for (i = 0; i < subdivisions; i++)
    for (j = 0; j < subdivisions; j++)
    GRAPHIC_DESTROY(patches[i*subdivisions+j]);

    if (water_patches)
    for (i = 0; i < subdivisions; i++)
    for (j = 0; j < subdivisions; j++)
    GRAPHIC_DESTROY(water_patches[i*subdivisions+j]);

    if (aprons)
    for (i = 0; i < 8; i++)
    GRAPHIC_DESTROY(aprons[i]);

    if (clouds)
    for (i = 0; i < nclouds; i++)
    GRAPHIC_DESTROY(clouds[i]);

    if (water)
    for (i = 0; i < 6; i++)
    delete water[i];

    delete [] aprons;
    delete [] clouds;
    delete [] patches;
    delete [] water;
    delete [] terrain_normals;

    terrain_patch.ClearImage();
    terrain_apron.ClearImage();

    layers.destroy();
}

// +--------------------------------------------------------------------+

void
Terrain::BuildTerrain()
{
    DataLoader* loader = DataLoader::GetLoader();
    loader->SetDataPath(datapath);
    loader->LoadBitmap( region->PatchName(),     terrain_patch);
    loader->LoadBitmap( region->ApronName(),     terrain_apron);
    loader->LoadTexture(region->PatchTexture(),  terrain_texture);
    loader->LoadTexture(region->ApronTexture(),  apron_texture);
    if (region->WaterTexture().length()) {
        loader->LoadTexture(region->WaterTexture(),  water_texture);

        if (region->EnvironmentTexture(0).length() > 0) {
            loader->LoadTexture(region->EnvironmentTexture(0), env_texture[0]);
            loader->LoadTexture(region->EnvironmentTexture(1), env_texture[1]);
            loader->LoadTexture(region->EnvironmentTexture(2), env_texture[2]);
            loader->LoadTexture(region->EnvironmentTexture(3), env_texture[3]);
            loader->LoadTexture(region->EnvironmentTexture(4), env_texture[4]);
            loader->LoadTexture(region->EnvironmentTexture(5), env_texture[5]);
        }
    }

    loader->LoadTexture(region->CloudsHigh(),    cloud_texture[0], Bitmap::BMP_TRANSLUCENT);
    loader->LoadTexture(region->CloudsLow(),     cloud_texture[1], Bitmap::BMP_TRANSLUCENT);
    loader->LoadTexture(region->ShadesLow(),     shade_texture[1], Bitmap::BMP_TRANSLUCENT);

    if (region->DetailTexture0().length())
    loader->LoadTexture(region->DetailTexture0(),noise_texture[0], Bitmap::BMP_TRANSLUCENT, false, true);

    if (region->DetailTexture1().length())
    loader->LoadTexture(region->DetailTexture1(),noise_texture[1], Bitmap::BMP_TRANSLUCENT, false, true);

    subdivisions   = terrain_patch.Width() / PATCH_SIZE;
    patch_size     = terrain_patch.Width() / subdivisions;

    BuildNormals();

    int     i, j;
    int     ntiles = terrain_patch.Width()/2 * terrain_patch.Height()/2;
    double  dx     = scale * patch_size;
    double  dz     = scale * patch_size;
    double  offset = -subdivisions/2;

    if (water_texture) {
        water = new(__FILE__,__LINE__) Water*[6];
        for (i = 0; i < 6; i++) {
            water[i] = new(__FILE__,__LINE__) Water();
            int n = (1<<i) + 1;
            water[i]->Init(n, (float) (scale*patch_size), 90.0f);
        }
    }

    // load tile textures:
    for (i = 0; i < layers.size(); i++) {
        TerrainLayer* layer = layers.at(i);

        if (i < layers.size()-1)
        layer->max_height = layers.at(i+1)->min_height;
        else
        layer->max_height = 1e6;

        if (layer->tile_name.length())
        loader->LoadTexture(layer->tile_name, layer->tile_texture);

        if (layer->detail_name.length())
        loader->LoadTexture(layer->detail_name, layer->detail_texture);
    }

    patches = new(__FILE__,__LINE__) TerrainPatch*[subdivisions*subdivisions];

    if (water_texture)
    water_patches = new(__FILE__,__LINE__) TerrainPatch*[subdivisions*subdivisions];

    for (i = 0; i < subdivisions; i++) {
        for (j = 0; j < subdivisions; j++) {
            int j1 = subdivisions - j;
            Rect rect(j * patch_size, i * patch_size, patch_size, patch_size);
            Point p1((j1 + offset  )*dx, 0,        (i + offset  )*dz);
            Point p2((j1 + offset+1)*dx, mtnscale, (i + offset+1)*dz);

            int index = i*subdivisions+j;
            patches[index] = new(__FILE__,__LINE__) TerrainPatch(this, &terrain_patch, rect, p1, p2);

            if (water_texture && patches[index]->MinHeight() < 3)
            water_patches[index] = new(__FILE__,__LINE__) TerrainPatch(this, rect, p1, p2, 30);
            
            else if (water_patches != 0)
            water_patches[index] = 0;
        }
    }

    int a      = 0;
    dx     = scale * terrain_patch.Width();
    dz     = scale * terrain_patch.Height();
    offset = -3.0/2;
    double xoffset = offset + 1.0/16.0;

    aprons = new(__FILE__,__LINE__) TerrainApron*[8];

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int j1 = 2 - j;
            if (i != 1 || j1 != 1) {
                Rect rect(j * subdivisions, i * subdivisions, subdivisions, subdivisions);
                Point p1((j1 + xoffset  )*dx, 0,        (i + offset  )*dz);
                Point p2((j1 + xoffset+1)*dx, mtnscale, (i + offset+1)*dz);

                aprons[a++] = new(__FILE__,__LINE__) TerrainApron(this, &terrain_apron, rect, p1, p2);
            }
        }
    }

    Weather::STATE state = region->GetWeather().State();

    if (state == Weather::HIGH_CLOUDS || state == Weather::MODERATE_CLOUDS) {
        double altitude = region->CloudAltHigh();
        nclouds = 9;

        if (state == Weather::MODERATE_CLOUDS)
        nclouds *= 2;

        clouds = new(__FILE__,__LINE__) TerrainClouds*[nclouds];

        a = 0;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                clouds[a] = new(__FILE__,__LINE__) TerrainClouds(this, 0);

                double xloc = (j-1) * 75000 + (rand()/32768.0 - 0.5) * 50000;
                double yloc = (i-1) * 75000 + (rand()/32768.0 - 0.5) * 50000;

                clouds[a]->MoveTo(Point(xloc, altitude, yloc));
                a++;
            }
        }

        if (state == Weather::MODERATE_CLOUDS) {
            altitude = region->CloudAltLow();

            for (i = 0; i < 3; i++) {
                for (j = 0; j < 3; j++) {
                    clouds[a] = new(__FILE__,__LINE__) TerrainClouds(this, 1);

                    double xloc = (j-1) * 75000 + (rand()/32768.0 - 0.5) * 50000;
                    double yloc = (i-1) * 75000 + (rand()/32768.0 - 0.5) * 50000;

                    clouds[a]->MoveTo(Point(xloc, altitude, yloc));
                    a++;
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
Terrain::BuildNormals()
{
    if (terrain_normals) {
        delete [] terrain_normals;
        terrain_normals = 0;
    }

    int    i, x, y;

    int    w      = terrain_patch.Width();
    int    h      = terrain_patch.Height();
    Color* pix    = terrain_patch.HiPixels();
    BYTE*  alt    = new(__FILE__,__LINE__) BYTE[h*w];
    int    nverts = w * h;
    double scale  = region->MountainScale() / (region->LateralScale() * 2.0);

    terrain_normals = new(__FILE__,__LINE__) Vec3B[nverts];

    ZeroMemory(terrain_normals, sizeof(Vec3B) * nverts);

    for (i = 0; i < w; i++) {
        alt            [          i] = 0;
        alt            [(h-1)*w + i] = 0;
        terrain_normals[          i] = Vec3B(128,128,255);
        terrain_normals[(h-1)*w + i] = Vec3B(128,128,255);
    }

    for (i = 0; i < h; i++) {
        alt            [i*w        ] = 0;
        alt            [i*w + (w-1)] = 0;
        terrain_normals[i*w        ] = Vec3B(128,128,255);
        terrain_normals[i*w + (w-1)] = Vec3B(128,128,255);
    }

    for (y = 1; y < h-1; y++) {
        for (x = 1; x < w-1; x++) {
            alt[y*w+x] = (BYTE) pix[y*w+x].Red();
        }
    }

    for (y = 1; y < h-1; y++) {
        for (x = 1; x < w-1; x++) {
            double dx = (alt[y*w     + (x-1)] - alt[y*w     + (x+1)]) * scale +
            (alt[(y-1)*w + (x-1)] - alt[(y-1)*w + (x+1)]) * scale * 0.5 +
            (alt[(y+1)*w + (x-1)] - alt[(y+1)*w + (x+1)]) * scale * 0.5;

            double dy = (alt[(y-1)*w + x    ] - alt[(y+1)*w + x    ]) * scale +
            (alt[(y-1)*w + (x-1)] - alt[(y+1)*w + (x-1)]) * scale * 0.5 +
            (alt[(y-1)*w + (x+1)] - alt[(y+1)*w + (x+1)]) * scale * 0.5;

            Point norm(dx,dy,1);
            norm.Normalize();

            Vec3B* tnorm = &terrain_normals[y*w + x];

            tnorm->x = (BYTE) (norm.x * 127 + 128);
            tnorm->y = (BYTE) (norm.y * 127 + 128);
            tnorm->z = (BYTE) (norm.z * 127 + 128);

            double foo = dx/dy;
        }
    }

    delete [] alt;
}

// +--------------------------------------------------------------------+

void
Terrain::Activate(Scene& scene)
{
    int i, j;

    StarSystem* system = region->System();
    if (system)
    datapath = system->GetDataPath();

    region->GetWeather().Update();

    if (!patches)
    BuildTerrain();

    if (patches) {
        for (i = 0; i < subdivisions; i++)
        for (j = 0; j < subdivisions; j++)
        if (patches[i*subdivisions+j])
        scene.AddGraphic(patches[i*subdivisions+j]);
    }

    if (water_patches) {
        for (i = 0; i < subdivisions; i++)
        for (j = 0; j < subdivisions; j++)
        if (water_patches[i*subdivisions+j])
        scene.AddGraphic(water_patches[i*subdivisions+j]);
    }

    if (aprons) {
        for (i = 0; i < 8; i++)
        if (aprons[i])
        scene.AddGraphic(aprons[i]);
    }

    if (clouds) {
        for (i = 0; i < nclouds; i++)
        if (clouds[i])
        scene.AddGraphic(clouds[i]);
    }
}

void
Terrain::Deactivate(Scene& scene)
{
    int i, j;

    if (patches) {
        for (i = 0; i < subdivisions; i++) {
            for (j = 0; j < subdivisions; j++) {
                TerrainPatch* p = patches[i*subdivisions+j];

                if (p) {
                    p->DeletePrivateData();
                    scene.DelGraphic(p);
                }
            }
        }
    }

    if (water_patches) {
        for (i = 0; i < subdivisions; i++) {
            for (j = 0; j < subdivisions; j++) {
                TerrainPatch* p = water_patches[i*subdivisions+j];

                if (p) {
                    p->DeletePrivateData();
                    scene.DelGraphic(p);
                }
            }
        }
    }

    if (aprons) {
        for (i = 0; i < 8; i++)
        if (aprons[i])
        scene.DelGraphic(aprons[i]);
    }

    if (clouds) {
        for (i = 0; i < nclouds; i++)
        if (clouds[i])
        scene.DelGraphic(clouds[i]);
    }

    StarSystem* system = region->System();

    // restore sunlight color and brightness on exit:
    if (system) {
        system->RestoreTrueSunColor();
    }
}

// +--------------------------------------------------------------------+

void
Terrain::ExecFrame(double seconds)
{
    if (water) {
        for (int i = 0; i < 6; i++) {
            if (water[i])
            water[i]->CalcWaves(seconds);
        }
    }
}

// +--------------------------------------------------------------------+

void
Terrain::SelectDetail(Projector* projector)
{
    if (!patches)
    return;

    if (detail_frame >= Game::Frame())
    return;

    // compute detail map:
    int  x, z;

    for (z = 0; z < subdivisions; z++) {
        for (x = 0; x < subdivisions; x++) {
            TerrainPatch*  patch    = patches[z*subdivisions + x];
            int            ndetail  = 0;
            Point          loc      = patch->Location();
            float          radius   = patch->Radius();

            if (loc.length() < 2*radius) {
                ndetail = detail_level;
            }

            else {
                double threshold = 4; //16;

                for (int level = 1; level <= detail_level; level++) {
                    double feature_size = radius / (1 << level);

                    if (projector->ApparentRadius(loc, (float) feature_size) > threshold)
                    ndetail = level;
                }
            }

            patch->SetDetailLevel(ndetail);

            if (water_patches) {
                patch = water_patches[z*subdivisions + x];
                if (patch)
                patch->SetDetailLevel(ndetail);
            }
        }
    }

    // compute fog fade level:
    double hour  = region->DayPhase();

    if (hour < 12)
    fog_fade = (hour)  / 12.0;
    else
    fog_fade = (24-hour) / 12.0;

    fog_fade = fog_fade * (1-region->HazeFade()) + region->HazeFade();

    detail_frame = Game::Frame();
}

// +--------------------------------------------------------------------+

double
Terrain::Height(double x, double y) const
{
    double h = 0;

    if (patches) {
        int ix = (int) floor(x / (patch_size * scale));
        int iy = (int) floor(y / (patch_size * scale));

        double px = x - ix * patch_size * scale;
        double py = y - iy * patch_size * scale;

        ix = subdivisions/2 - ix;
        iy = subdivisions/2 + iy;

        TerrainPatch* patch = 0;

        if (ix >= 0 && ix < subdivisions &&
                iy >= 0 && iy < subdivisions)
        patch = patches[iy*subdivisions+ix];

        if (patch)
        h = patch->Height(px, py);
    }

    if (water_patches && h < 30)
    h = 30;

    return h;
}

// +--------------------------------------------------------------------+

void
Terrain::SetDetailLevel(int detail)
{
    if (detail >= 1 && detail <= 4) {

        // limit detail on low memory machines:
        if (detail > 3 && MachineInfo::GetTotalRam() < 64)
        detail = 3;

        detail_level = detail;
    }
}

// +--------------------------------------------------------------------+

bool
Terrain::IsFirstPatch(TerrainPatch* p) const
{
    return (patches && *patches == p);
}