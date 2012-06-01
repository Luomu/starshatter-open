/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2005. All Rights Reserved.

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

