/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Asteroid.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Asteroid Sprite class
*/

#ifndef Asteroid_h
#define Asteroid_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "Debris.h"

// +--------------------------------------------------------------------+

class Asteroid : public Debris
{
public:
	Asteroid(int type, const Vec3& pos, double mass);

	static void Initialize();
	static void Close();
};

#endif Asteroid_h

