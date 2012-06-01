/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Particles.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Particle burst class
*/

#ifndef Particles_h
#define Particles_h

#include "Types.h"
#include "Geometry.h"
#include "Graphic.h"
#include "Sprite.h"

// +--------------------------------------------------------------------+

class Particles : public Graphic
{
public:
	Particles(Bitmap* bitmap, 
	int     np, 
	const   Vec3& base_loc, 
	const   Vec3& vel,
	float   base_speed  = 500.0f, 
	float   drag        = 1.0f,
	float   scale       = 1.0f, 
	float   bloom       = 0.0f, 
	float   decay       = 100.0f, 
	float   release     = 1.0f, 
	bool    cont        = false, 
	bool    trail       = true,
	bool    rise        = false,
	int     blend       = 3,
	int     nframes     = 1);

	virtual ~Particles();

	virtual void   Render(Video* video, DWORD flags);
	virtual void   ExecFrame(double seconds);
	virtual void   TranslateBy(const Point& ref) { offset = ref; loc = loc - ref; }
	virtual bool   CheckVisibility(Projector& projector);

	virtual bool   IsEmitting()   const { return emitting;  }
	virtual void   StopEmitting()       { emitting = false; }

protected:
	int         nparts;
	int         nverts;
	int         blend;
	bool        continuous;
	bool        trailing;
	bool        rising;
	bool        emitting;

	float       base_speed;
	float       max_speed;
	float       drag;
	float       release_rate;
	float       decay;
	float       min_scale;
	float       max_scale;
	float       extra;

	Point       ref_loc;
	Point       offset;
	Point*      velocity;
	Point*      part_loc;
	Point*      release;
	float*      timestamp;
	float*      intensity;
	float*      scale;
	float*      angle;
	BYTE*       frame;

	Sprite*     point_sprite;
};

#endif Particles_h
