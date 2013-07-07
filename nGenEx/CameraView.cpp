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
    FILE:         CameraView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Projection Camera View class
    uses abstract PolyRender class to draw the triangles
*/

#include "MemDebug.h"
#include "CameraView.h"
#include "Color.h"
#include "Window.h"
#include "Scene.h"
#include "Light.h"
#include "Solid.h"
#include "Shadow.h"
#include "Sprite.h"
#include "Video.h"
#include "Bitmap.h"
#include "Screen.h"
#include "Game.h"

// +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

// +--------------------------------------------------------------------+

static Camera  emergency_cam;
static Scene   emergency_scene;

// +--------------------------------------------------------------------+

CameraView::CameraView(Window* c, Camera* cam, Scene* s)
    : View(c), video(0), camera(cam), projector(c, cam), scene(s),
      lens_flare_enable(0), halo_bitmap(0),  infinite(0),
      projection_type(Video::PROJECTION_PERSPECTIVE)
{
    elem_bitmap[0] = 0;
    elem_bitmap[1] = 0;
    elem_bitmap[2] = 0;

    if (!camera)
    camera = &emergency_cam;

    if (!scene)
    scene = &emergency_scene;

    Rect r = window->GetRect();
    width  = r.w;
    height = r.h;
}

CameraView::~CameraView()
{
}

// +--------------------------------------------------------------------+

void
CameraView::UseCamera(Camera* cam)
{
    if (cam)
    camera = cam;
    else
    camera = &emergency_cam;

    projector.UseCamera(camera);
}

void
CameraView::UseScene(Scene* s)
{
    if (s)
    scene = s;
    else
    scene = &emergency_scene;
}

void
CameraView::SetFieldOfView(double fov)
{
    projector.SetFieldOfView(fov);
}

double
CameraView::GetFieldOfView() const
{
    return projector.GetFieldOfView();
}

void
CameraView::SetProjectionType(DWORD pt)
{
    projector.SetOrthogonal(pt == Video::PROJECTION_ORTHOGONAL);
    projection_type = pt;
}

DWORD
CameraView::GetProjectionType() const
{
    return projection_type;
}

void
CameraView::OnWindowMove()
{
    Rect r = window->GetRect();
    projector.UseWindow(window);

    width  = r.w;
    height = r.h;
}


// +--------------------------------------------------------------------+
// Enable or disable lens flare effect, and provide bitmaps for rendering
// +--------------------------------------------------------------------+

void
CameraView::LensFlare(int on, double dim)
{
    lens_flare_enable = on;
    lens_flare_dim    = dim;
}

void
CameraView::LensFlareElements(Bitmap* halo, Bitmap* e1, Bitmap* e2, Bitmap* e3)
{
    if (halo)
    halo_bitmap = halo;

    if (e1)
    elem_bitmap[0] = e1;

    if (e2)
    elem_bitmap[1] = e2;

    if (e3)
    elem_bitmap[2] = e3;
}

// +--------------------------------------------------------------------+

int
CameraView::SetInfinite(int i)
{
    int old = infinite;
    infinite = i;
    projector.SetInfinite(i);
    return old;
}

// +--------------------------------------------------------------------+
// Compute the Depth of a Graphic
// +--------------------------------------------------------------------+

void
CameraView::FindDepth(Graphic* g)
{
    if (infinite) {
        g->SetDepth(1.0e20f);
        return;
    }

    // Translate into a viewpoint-relative coordinate
    Vec3 loc = g->Location() - camera->Pos();

    // Rotate into the view orientation
    float z = (float) (loc * camera->vpn());
    g->SetDepth(z);
}

// +--------------------------------------------------------------------+

void
CameraView::Refresh()
{
    // disabled:
    if (camera == &emergency_cam)
    return;

    // prologue:
    video = Video::GetInstance();
    if (!video)
    return;

    int cw = window->Width();
    int ch = window->Height();

    cvrt = camera->vrt();
    cvup = camera->vup();
    cvpn = camera->vpn();

    TranslateScene();
    MarkVisibleObjects();

    Rect old_rect;
    video->GetWindowRect(old_rect);

    video->SetCamera(camera);
    video->SetWindowRect(window->GetRect());
    video->SetProjection((float) GetFieldOfView(), 1.0f, 1.0e6f, projection_type);

    // project and render:
    RenderBackground();
    RenderScene();
    RenderForeground();
    RenderSprites();
    RenderLensFlare();

    UnTranslateScene();

    video->SetWindowRect(old_rect);
}

// +--------------------------------------------------------------------+
// Translate all objects and lights to camera relative coordinates:
// +--------------------------------------------------------------------+

void
CameraView::TranslateScene()
{
    camera_loc = camera->Pos();

    ListIter<Graphic> g_iter = scene->Graphics();
    while (++g_iter) {
        Graphic* graphic = g_iter.value();

        if (!graphic->IsInfinite())
        graphic->TranslateBy(camera_loc);
    }

    g_iter.attach(scene->Foreground());
    while (++g_iter) {
        Graphic* graphic = g_iter.value();
        graphic->TranslateBy(camera_loc);
    }

    g_iter.attach(scene->Sprites());
    while (++g_iter) {
        Graphic* graphic = g_iter.value();

        if (!graphic->IsInfinite())
        graphic->TranslateBy(camera_loc);
    }

    ListIter<Light> l_iter = scene->Lights();
    while (++l_iter) {
        Light* light = l_iter.value();
        light->TranslateBy(camera_loc);
    }

    camera->MoveTo(0,0,0);
}

// +--------------------------------------------------------------------+
// Translate all objects and lights back to original positions:
// +--------------------------------------------------------------------+

void
CameraView::UnTranslateScene()
{
    Point reloc = -camera_loc;

    ListIter<Graphic> g_iter = scene->Graphics();
    while (++g_iter) {
        Graphic* graphic = g_iter.value();

        if (!graphic->IsInfinite())
        graphic->TranslateBy(reloc);
    }

    g_iter.attach(scene->Foreground());
    while (++g_iter) {
        Graphic* graphic = g_iter.value();
        graphic->TranslateBy(reloc);
    }

    g_iter.attach(scene->Sprites());
    while (++g_iter) {
        Graphic* graphic = g_iter.value();

        if (!graphic->IsInfinite())
        graphic->TranslateBy(reloc);
    }

    ListIter<Light> l_iter = scene->Lights();
    while (++l_iter) {
        Light* light = l_iter.value();
        light->TranslateBy(reloc);
    }

    camera->MoveTo(camera_loc);
}

// +--------------------------------------------------------------------+
// Mark visible objects
// +--------------------------------------------------------------------+

void
CameraView::MarkVisibleObjects()
{
    projector.StartFrame();
    graphics.clear();

    ListIter<Graphic> graphic_iter = scene->Graphics();
    while (++graphic_iter) {
        Graphic* graphic = graphic_iter.value();

        if (graphic->Hidden())
        continue;
        
        if (graphic->CheckVisibility(projector)) {
            graphic->Update();
            graphics.append(graphic);
        }
        else {
            graphic->ProjectScreenRect(0);
        }
    }
}

void
CameraView::MarkVisibleLights(Graphic* graphic, DWORD flags)
{
    if (flags < Graphic::RENDER_FIRST_LIGHT) {
        flags = flags | Graphic::RENDER_FIRST_LIGHT | Graphic::RENDER_ADD_LIGHT;
    }

    if (graphic->IsVisible()) {
        Vec3 eye = camera->Pos();

        ListIter<Light> light_iter = scene->Lights();

        while (++light_iter) {
            Light* light = light_iter.value();
            bool bright_enough = light->Type()      == Light::LIGHT_DIRECTIONAL ||
            light->Intensity() >= 1e9;

            if (!bright_enough) {
                Point test = graphic->Location() - light->Location();
                if (test.length() < light->Intensity()*10)
                bright_enough = true;
            }

            // turn off lights that won't be used this pass:
            if (light->CastsShadow()) {
                if ((flags & Graphic::RENDER_ADD_LIGHT) == 0)
                bright_enough = false;
            }
            else {
                if ((flags & Graphic::RENDER_FIRST_LIGHT) == 0)
                bright_enough = false;
            }

            double obs_radius = graphic->Radius();
            if (obs_radius < 100)
            obs_radius = 100;

            light->SetActive(bright_enough);
        }
    }
}

// +--------------------------------------------------------------------+

void
CameraView::RenderBackground()
{
    if (scene->Background().isEmpty()) return;

    video->SetRenderState(Video::FILL_MODE,         Video::FILL_SOLID);
    video->SetRenderState(Video::Z_ENABLE,          FALSE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    FALSE);
    video->SetRenderState(Video::STENCIL_ENABLE,    FALSE);
    video->SetRenderState(Video::LIGHTING_ENABLE,   TRUE);

    // solid items:
    ListIter<Graphic> iter = scene->Background();
    while (++iter) {
        Graphic* g = iter.value();

        if (!g->Hidden())
        Render(g, Graphic::RENDER_SOLID);
    }

    // blended items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();

        if (!g->Hidden())
        Render(g, Graphic::RENDER_ALPHA);
    }

    // glowing items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();

        if (!g->Hidden())
        Render(g, Graphic::RENDER_ADDITIVE);
    }
}

// +--------------------------------------------------------------------+

void
CameraView::RenderForeground()
{
    bool foregroundVisible = false;

    ListIter<Graphic> iter = scene->Foreground();
    while (++iter && !foregroundVisible) {
        Graphic* g = iter.value();
        if (g && !g->Hidden())
        foregroundVisible = true;
    }

    if (!foregroundVisible)
    return;

    video->SetRenderState(Video::FILL_MODE,         Video::FILL_SOLID);
    video->SetRenderState(Video::Z_ENABLE,          TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    TRUE);
    video->SetRenderState(Video::STENCIL_ENABLE,    FALSE);
    video->SetRenderState(Video::LIGHTING_ENABLE,   TRUE);
    video->SetProjection((float) GetFieldOfView(), 1.0f,   1.0e6f,  projection_type);

    if (video->IsShadowEnabled() || video->IsBumpMapEnabled()) {
        // solid items, ambient and non-shadow lights:
        iter.reset();
        while (++iter) {
            Graphic* g = iter.value();
            Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_FIRST_LIGHT);
        }

        video->SetAmbient(Color::Black);
        video->SetRenderState(Video::LIGHTING_PASS,     2);

        // solid items, shadow lights:
        iter.reset();
        while (++iter) {
            Graphic* g = iter.value();
            Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_ADD_LIGHT);
        }
    }

    else {
        // solid items:
        iter.reset();
        while (++iter) {
            Graphic* g = iter.value();
            Render(g, Graphic::RENDER_SOLID);
        }
    }

    video->SetAmbient(scene->Ambient());
    video->SetRenderState(Video::LIGHTING_PASS,     0);
    video->SetRenderState(Video::STENCIL_ENABLE,    FALSE);
    video->SetRenderState(Video::Z_ENABLE,          TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    FALSE);

    // blended items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();
        Render(g, Graphic::RENDER_ALPHA);
        g->ProjectScreenRect(&projector);
    }

    // glowing items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();
        Render(g, Graphic::RENDER_ADDITIVE);
        g->ProjectScreenRect(&projector);
    }
}

// +--------------------------------------------------------------------+

void
CameraView::RenderSprites()
{
    if (scene->Sprites().isEmpty()) return;

    video->SetRenderState(Video::FILL_MODE,         Video::FILL_SOLID);
    video->SetRenderState(Video::Z_ENABLE,          TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    FALSE);
    video->SetRenderState(Video::STENCIL_ENABLE,    FALSE);
    video->SetRenderState(Video::LIGHTING_ENABLE,   TRUE);

    // compute depth:
    ListIter<Graphic> iter = scene->Sprites();
    while (++iter) {
        Graphic* g = iter.value();
        if (g && g->IsVisible() && !g->Hidden()) {
            FindDepth(g);
        }
    }

    // sort the list:
    scene->Sprites().sort();

    // blended items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();
        Render(g, Graphic::RENDER_ALPHA);
    }

    // glowing items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();
        Render(g, Graphic::RENDER_ADDITIVE);
    }
}

// +--------------------------------------------------------------------+
// Render the whole scene, sorted back to front
// +--------------------------------------------------------------------+

void
CameraView::RenderScene()
{
    if (graphics.isEmpty()) return;

    int   i         = 0;
    int   ngraphics = graphics.size();

    // compute depth:
    ListIter<Graphic> iter = graphics;
    while (++iter) {
        Graphic* g = iter.value();
        if (g && !g->Hidden()) {
            FindDepth(g);

            if (g->IsSolid()) {
                Solid* solid = (Solid*) g;

                solid->SelectDetail(&projector);

                if (video->IsShadowEnabled()) {
                    MarkVisibleLights(solid, Graphic::RENDER_ADD_LIGHT);
                    solid->UpdateShadows(scene->Lights());
                }
            }
        }
    }

    // sort the list:
    graphics.sort();

    Graphic* g = graphics.last();
    if (g->Depth() > 5e6) {
        RenderSceneObjects(true);
        video->ClearDepthBuffer();
    }

    RenderSceneObjects(false);
}

void
CameraView::RenderSceneObjects(bool distant)
{
    ListIter<Graphic> iter = graphics;

    video->SetAmbient(scene->Ambient());
    video->SetRenderState(Video::FILL_MODE,         Video::FILL_SOLID);
    video->SetRenderState(Video::Z_ENABLE,          TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    TRUE);
    video->SetRenderState(Video::LIGHTING_ENABLE,   TRUE);

    if (distant)
    video->SetProjection((float) GetFieldOfView(), 5.0e6f, 1.0e12f, projection_type);
    else
    video->SetProjection((float) GetFieldOfView(), 1.0f,   1.0e6f,  projection_type);

    if (video->IsShadowEnabled() || video->IsBumpMapEnabled()) {
        // solid items, ambient and non-shadow lights:
        iter.reset();
        while (++iter) {
            Graphic* g = iter.value();

            if (distant && g->Depth() > 5e6 || !distant && g->Depth() < 5e6) {
                Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_FIRST_LIGHT);
            }
        }

        // send shadows to stencil buffer:
        if (video->IsShadowEnabled()) {
            iter.reset();
            while (++iter) {
                Graphic* g = iter.value();
                if (distant && g->Depth() > 5e6 || !distant && g->Depth() < 5e6) {
                    if (g->IsSolid()) {
                        Solid* solid = (Solid*) g;

                        ListIter<Shadow> shadow_iter = solid->GetShadows();
                        while (++shadow_iter) {
                            Shadow* shadow = shadow_iter.value();
                            shadow->Render(video);
                        }
                    }
                }
            }
        }

        video->SetAmbient(Color::Black);
        video->SetRenderState(Video::LIGHTING_PASS,     2);
        video->SetRenderState(Video::STENCIL_ENABLE,    TRUE);

        // solid items, shadow lights:
        iter.reset();
        while (++iter) {
            Graphic* g = iter.value();

            if (distant && g->Depth() > 5e6 || !distant && g->Depth() < 5e6) {
                Render(g, Graphic::RENDER_SOLID | Graphic::RENDER_ADD_LIGHT);
            }
        }
    }

    else {
        // solid items:
        iter.reset();
        while (++iter) {
            Graphic* g = iter.value();

            if (distant && g->Depth() > 5e6 || !distant && g->Depth() < 5e6) {
                Render(g, Graphic::RENDER_SOLID);
            }
        }
    }

    video->SetAmbient(scene->Ambient());
    video->SetRenderState(Video::LIGHTING_PASS,     0);
    video->SetRenderState(Video::STENCIL_ENABLE,    FALSE);
    video->SetRenderState(Video::Z_ENABLE,          TRUE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    FALSE);

    // blended items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();

        if (distant && g->Depth() > 5e6 || !distant && g->Depth() < 5e6) {
            Render(g, Graphic::RENDER_ALPHA);
            g->ProjectScreenRect(&projector);
        }
    }

    // glowing items:
    iter.reset();
    while (++iter) {
        Graphic* g = iter.value();

        if (distant && g->Depth() > 5e6 || !distant && g->Depth() < 5e6) {
            Render(g, Graphic::RENDER_ADDITIVE);
            g->ProjectScreenRect(&projector);
        }
    }
}

void
CameraView::Render(Graphic* g, DWORD flags)
{
    if (g && g->IsVisible() && !g->Hidden()) {
        if (g->IsSolid()) {
            MarkVisibleLights(g, flags);
            video->SetLights(scene->Lights());
        }

        g->Render(video, flags);
    }
}

// +--------------------------------------------------------------------+
// Draw the lens flare effect, if enabled and light source visible
// +--------------------------------------------------------------------+

void
CameraView::RenderLensFlare()
{
    if (!lens_flare_enable || lens_flare_dim < 0.01)
    return;

    if (!halo_bitmap)
    return;

    video->SetRenderState(Video::STENCIL_ENABLE,    FALSE);
    video->SetRenderState(Video::Z_ENABLE,          FALSE);
    video->SetRenderState(Video::Z_WRITE_ENABLE,    FALSE);

    Vec3 flare_pos;
    Vec3 sun_pos;
    Vec3 center((float)width/2.0f, (float)height/2.0f, 1.0f);
    int  flare_visible = 0;

    ListIter<Light> light_iter = scene->Lights();
    while (++light_iter) {
        Light* light = light_iter.value();

        if (!light->IsActive())
        continue;

        if (light->Type() == Light::LIGHT_DIRECTIONAL && light->Intensity() < 1)
        continue;

        double distance = (light->Location()-camera->Pos()).length();

        // only do lens flare for the sun:
        if (distance > 1e9) {
            if (projector.IsVisible(light->Location(), 1.0f)) {
                // FOUND IT: TRANSFORM/PROJECT FLARE LOCATION
                Point sun_pos = light->Location();

                if (light->CastsShadow() && scene->IsLightObscured(camera->Pos(), sun_pos, -1))
                continue;

                projector.Transform(sun_pos);

                if (sun_pos.z < 100)
                continue;

                projector.Project(sun_pos, false);

                int x = (int) (sun_pos.x);
                int y = (int) (sun_pos.y);
                int w = (int) (window->Width() / 4.0);
                int h = w;

                // halo:
                window->DrawBitmap(x-w,y-h,x+w,y+h, halo_bitmap, Video::BLEND_ADDITIVE);

                // lens elements:
                if (elem_bitmap[0]) {
                    Point vector = center - sun_pos;
                    float vlen = (float) vector.length();
                    vector.Normalize();

                    static int   nelem = 12;
                    static int   elem_indx[] = {  0,    1,    1,     1,     0,      0,     0,    0,    2,    0,    0,    2    };
                    static float elem_dist[] = { -0.2f, 0.5f, 0.55f, 0.62f, 1.23f, 1.33f, 1.35f, 0.8f, 0.9f, 1.4f, 1.7f, 1.8f };
                    static float elem_size[] = {  0.3f, 0.2f, 0.4f,  0.3f,  0.4f,  0.2f,  0.6f,  0.1f, 0.1f, 1.6f, 1.0f, 0.2f };

                    for (int elem = 0; elem < nelem; elem++) {
                        Bitmap* img = elem_bitmap[elem_indx[elem]];

                        /***
                if (elem == 10)
                    shade *= 0.5;
                ***/

                        if (img == 0)
                        img = elem_bitmap[0];

                        flare_pos = sun_pos + (vector * elem_dist[elem] * vlen);
                        x = (int) (flare_pos.x);
                        y = (int) (flare_pos.y);
                        w = (int) (window->Width() / 8.0 * elem_size[elem]);
                        h = w;

                        window->DrawBitmap(x-w,y-h,x+w,y+h, img, Video::BLEND_ADDITIVE);
                    }
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+
// Rotate and translate a plane in world space to view space.
// +--------------------------------------------------------------------+

void
CameraView::WorldPlaneToView(Plane& plane)
{
    // Determine the distance from the viewpoint
    Vec3 tnormal = plane.normal;

    if (!infinite)
    plane.distance -= (float) (camera->Pos() * tnormal);

    // Rotate the normal into view orientation
    plane.normal.x = tnormal * cvrt;
    plane.normal.y = tnormal * cvup;
    plane.normal.z = tnormal * cvpn;
}

void
CameraView::SetDepthScale(float scale)
{
    projector.SetDepthScale(scale);
}
