/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Debris.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Debris Sprite class
*/

#ifndef Debris_h
#define Debris_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class Solid;
class Model;
class Shot;

// +--------------------------------------------------------------------+

class Debris : public SimObject
{
public:
	Debris(Model* model, const Vec3& pos, const Vec3& vel, double mass);

	void              SetLife(int seconds) { life = seconds; }
	virtual int       HitBy(Shot* shot, Point& impact);

	virtual void      ExecFrame(double seconds);
	virtual double    AltitudeAGL()        const;
};

#endif Debris_h

