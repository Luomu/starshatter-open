/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Explosion.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Explosion Sprite class
*/

#ifndef Explosion_h
#define Explosion_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "Sound.h"

// +--------------------------------------------------------------------+

class Solid;
class Particles;
class System;

// +--------------------------------------------------------------------+

class Explosion : public SimObject, 
public SimObserver
{
public:
	static const char* TYPENAME() { return "Explosion"; }

	enum Type { SHIELD_FLASH      =  1,
		HULL_FLASH        =  2,
		BEAM_FLASH        =  3,
		SHOT_BLAST        =  4,
		HULL_BURST        =  5,
		HULL_FIRE         =  6,
		PLASMA_LEAK       =  7,
		SMOKE_TRAIL       =  8,
		SMALL_FIRE        =  9, 
		SMALL_EXPLOSION   = 10,
		LARGE_EXPLOSION   = 11,
		LARGE_BURST       = 12,
		NUKE_EXPLOSION    = 13,
		QUANTUM_FLASH     = 14,
		HYPER_FLASH       = 15
	};

	Explosion(int type, const Vec3& pos, const Vec3& vel, 
	float exp_scale, float part_scale,
	SimRegion* rgn=0, SimObject* source=0);
	virtual ~Explosion();

	static void       Initialize();
	static void       Close();

	virtual void      ExecFrame(double seconds);
	Particles*        GetParticles()                { return particles; }

	virtual void      Activate(Scene& scene);
	virtual void      Deactivate(Scene& scene);

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

protected:
	int               type;
	Particles*        particles; 

	float             scale;
	float             scale1;
	float             scale2;

	SimObject*        source;
	Point             mount_rel;
};

#endif Explosion_h

