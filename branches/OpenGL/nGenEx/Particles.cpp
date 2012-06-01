/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Particles.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Particle Burst class
*/

#include "MemDebug.h"
#include "Particles.h"
#include "Projector.h"
#include "Light.h"
#include "Bitmap.h"
#include "Game.h"
#include "Random.h"

// +--------------------------------------------------------------------+

inline float randf() { return (rand()-16384.0f)/32768.0f; }

// +--------------------------------------------------------------------+

Particles::Particles(Bitmap* bitmap, int np, const Vec3& base_loc, const Vec3& vel,
float bspeed, float dr, float s, float bloom, float dec, float rate,
bool cont, bool trail, bool rise, int a, int nframes)
: nparts(np), base_speed(bspeed), max_speed(bspeed*3.0f), 
drag(dr), min_scale(s), max_scale(bloom), decay(dec),
release_rate(rate), continuous(cont), trailing(trail), rising(rise),
blend(a), extra(0.0f), point_sprite(0), emitting(true)
{
	MoveTo(base_loc);
	ref_loc     = base_loc;

	trans       = true;
	luminous    = true;
	shadow      = false;
	nverts      = nparts;

	if (max_scale < min_scale)
	max_scale = min_scale;

	velocity    = new(__FILE__,__LINE__) Point[nverts];
	part_loc    = new(__FILE__,__LINE__) Point[nverts];
	release     = new(__FILE__,__LINE__) Point[nverts];
	intensity   = new(__FILE__,__LINE__) float[nverts];
	timestamp   = new(__FILE__,__LINE__) float[nverts];
	scale       = new(__FILE__,__LINE__) float[nverts];
	angle       = new(__FILE__,__LINE__) float[nverts];
	frame       = new(__FILE__,__LINE__) BYTE[nverts];

	float speed = base_speed;

	for (int i = 0; i < nverts; i++) {
		intensity[i]   = 1.0f;
		timestamp[i]   = (float) (Game::GameTime() / 1000.0);
		scale[i]       = (float) (min_scale);
		angle[i]       = (float) (Random(0, 2*PI));
		frame[i]       = 0;

		part_loc[i]    = Point();
		release[i]     = ref_loc;
		velocity[i]    = RandomVector(speed);
		velocity[i] += vel;

		if (speed < max_speed)
		speed += (float) Random(max_speed/15.0, max_speed/5.0);
		else
		speed = base_speed;
	}

	radius = 15000.0f;

	if (decay > 2)
	decay /= 256.0f;

	if (nparts < 8) {
		nverts = 1;
	}

	else if (nparts > 50 || continuous) {
		nverts = (int) (nparts * 0.125 * release_rate);
	}

	point_sprite = new(__FILE__,__LINE__) Sprite(bitmap, nframes);
	point_sprite->Scale(s);
	point_sprite->SetBlendMode(blend);
	point_sprite->SetFrameRate(nframes * decay);
}

Particles::~Particles()
{
	delete    point_sprite;
	delete [] velocity;
	delete [] part_loc;
	delete [] release;
	delete [] timestamp;
	delete [] intensity;
	delete [] scale;
	delete [] angle;
	delete [] frame;
}

// +--------------------------------------------------------------------+

void Particles::ExecFrame(double seconds)
{
	point_sprite->Update();

	ref_loc = loc;
	radius += max_speed * (float) seconds;

	float scaled_drag = (float) exp(-drag * seconds);
	float scale_inc   = (float) ((max_scale-min_scale)*seconds*2);

	for (int i = 0; i < nverts; i++) {
		part_loc[i] += velocity[i] * (float) seconds;

		if (rising) {
			part_loc[i].y += (float) ((randf() + 1) * scale[i] * 80 * seconds);
		}

		// do the (chunky) blooming effect:
		if (max_scale > 0 && scale[i] < max_scale) {
			scale[i] += scale_inc * (float) ((i%3)/3.0);
		}

		double      rho = angle[i];
		int         rot = i%4;
		switch (rot) {
		case 0:     rho += seconds * 0.13;  break;
		case 1:     rho -= seconds * 0.11;  break;
		case 2:     rho += seconds * 0.09;  break;
		case 3:     rho -= seconds * 0.07;  break;
		default:                            break;
		}

		angle[i]      = (float) rho;
		intensity[i] -= (float) (decay * seconds);

		if (point_sprite->NumFrames() > 1) {
			double age = Game::GameTime()/1000.0 - timestamp[i];
			int    n   = (int) (age * point_sprite->FrameRate());

			if (n >= point_sprite->NumFrames())
			n = point_sprite->NumFrames() - 1;

			frame[i] = n;
		}

		velocity[i] *= scaled_drag;
	}

	if (nverts < nparts && emitting) {
		int    nv        = nverts;
		double delta     = nparts * release_rate * seconds;
		int    new_parts = (int)   (delta + extra);
		extra     = (float) (delta + extra - new_parts);
		nverts   += new_parts;

		if (nverts > nparts)
		nverts = nparts;

		for (int i = nv; i < nverts; i++) {
			intensity[i]   = 1;
			timestamp[i]   = (float) (Game::GameTime() / 1000.0);
			scale[i]       = (float) (min_scale);
			angle[i]       = (float) (Random(0, 2*PI));
			frame[i]       = 0;
			part_loc[i]    = Point();
			release[i]     = ref_loc;
		}
	}
	
	if (nverts > nparts)
	nverts = nparts;

	// recycle dead particles:
	if (continuous) {
		float speed = base_speed;

		for (int i = 0; i < nverts; i++) {
			if (intensity[i] <= 0) {
				part_loc[i]    = Point();
				release[i]     = ref_loc;

				intensity[i]   = 1;
				timestamp[i]   = (float) (Game::GameTime() / 1000.0);
				scale[i]       = (float) (min_scale);
				angle[i]       = (float) (PI * rand() / 16384.0);
				frame[i]       = 0;
				velocity[i]    = RandomVector(speed);

				if (speed < max_speed)
				speed += (float) Random(max_speed/25.0, max_speed/18.0);
				else
				speed = base_speed;
			}
		}
	}
}

// +--------------------------------------------------------------------+

bool
Particles::CheckVisibility(Projector& projector)
{
	float base = 256;

	if (point_sprite && point_sprite->Frame())
	base = (float) point_sprite->Frame()->Width();

	float particle_radius = base * max_scale;

	if (projector.IsVisible(     Location(), Radius()) &&
			projector.ApparentRadius(Location(), particle_radius) > 1) {

		visible = true;
	}
	else {
		visible = false;
	}
	
	return visible;
}

// +--------------------------------------------------------------------+

void
Particles::Render(Video* video, DWORD flags)
{
	if (hidden || !visible || !video || !point_sprite)
	return;

	if (blend == 2 && !(flags & Graphic::RENDER_ALPHA))
	return;

	if (blend == 4 && !(flags & Graphic::RENDER_ADDITIVE))
	return;

	for (int i = 0; i < nverts; i++) {
		Point vloc;

		if (trailing) {
			vloc = part_loc[i] + release[i] - offset;
		}
		else {
			vloc = part_loc[i] + loc;
		}

		point_sprite->MoveTo(vloc);
		point_sprite->SetShade(intensity[i]);
		point_sprite->Rescale(scale[i]);
		point_sprite->SetAngle(angle[i]);
		point_sprite->SetFrameIndex(frame[i]);

		point_sprite->Render(video, flags);
	}
}
