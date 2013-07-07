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
    FILE:         DriveSprite.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Sprite for rendering drive flares.  Remains visible at extreme ranges.
*/

#include "MemDebug.h"
#include "DriveSprite.h"

#include "Bitmap.h"
#include "Camera.h"
#include "Scene.h"
#include "Video.h"

// +--------------------------------------------------------------------+

DriveSprite::DriveSprite()
    : Sprite(), glow(0), effective_radius(0), front(0,0,0), bias(0)
{ luminous = true; }

DriveSprite::DriveSprite(Bitmap* animation, Bitmap* g)
    : Sprite(animation), glow(g), effective_radius(0), front(0,0,0), bias(0)
{ luminous = true; }

DriveSprite::DriveSprite(Bitmap* animation, int length, int repeat, int share)
    : Sprite(animation, length, repeat, share), glow(0), effective_radius(0),
      front(0,0,0), bias(0)
{ luminous = true; }

DriveSprite::~DriveSprite()
{ }

// +--------------------------------------------------------------------+

void
DriveSprite::SetFront(const Vec3& f)
{
    front = f;
    front.Normalize();
}

void
DriveSprite::SetBias(DWORD b)
{
    bias = b;
}

// +--------------------------------------------------------------------+

void
DriveSprite::Render(Video* video, DWORD flags)
{
    if (!video || ((flags & RENDER_ADDITIVE) == 0))
    return;

    if (shade > 0  && !hidden && (life > 0 || loop)) {
        const Camera*  cam = video->GetCamera();
        bool           z_disable = false;

        if (bias)
        video->SetRenderState(Video::Z_BIAS, bias);

        if (front.length()) {
            Point test = loc;

            if (scene && cam) {
                Vec3   dir = front;

                double intensity = cam->vpn() * dir * -1;
                double distance  = Point(cam->Pos() - test).length();

                if (intensity > 0.05) {
                    if (!scene->IsLightObscured(cam->Pos(), test, 8)) {
                        video->SetRenderState(Video::Z_ENABLE, false);
                        z_disable = true;

                        if (glow) {
                            intensity = pow(intensity, 3);

                            if (distance > 5e3)
                            intensity *= (1 - (distance-5e3)/45e3);

                            if (intensity > 0) {
                                Bitmap*  tmp_frame   = frames;
                                double   tmp_shade   = shade;
                                int      tmp_w       = w;
                                int      tmp_h       = h;

                                if (glow->Width() != frames->Width()) {
                                    double   wscale      = glow->Width()   / frames->Width();
                                    double   hscale      = glow->Height()  / frames->Height();

                                    w = (int) (w * wscale);
                                    h = (int) (h * hscale);
                                }

                                shade  = intensity;
                                frames = glow;

                                Sprite::Render(video, flags);

                                frames = tmp_frame;
                                shade  = tmp_shade;
                                w      = tmp_w;
                                h      = tmp_h;
                            }
                        }
                    }
                }
            }
        }

        if (effective_radius-radius > 0.1) {
            double scale_up = effective_radius / radius;
            int tmp_w = w;
            int tmp_h = h;

            w = (int) (w * scale_up);
            h = (int) (h * scale_up);

            Sprite::Render(video, flags);

            w = tmp_w;
            h = tmp_h;
        }

        else {
            Sprite::Render(video, flags);
        }

        if (bias)      video->SetRenderState(Video::Z_BIAS,   0);
        if (z_disable) video->SetRenderState(Video::Z_ENABLE, true);
    }
}

