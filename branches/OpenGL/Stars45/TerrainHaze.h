/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2005. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TerrainHaze.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Atmospheric fog along the horizon
*/

#ifndef TerrainHaze_h
#define TerrainHaze_h

#include "Types.h"
#include "Solid.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class TerrainRegion;

// +--------------------------------------------------------------------+

class TerrainHaze : public Solid
{
public:
	TerrainHaze();
	virtual ~TerrainHaze();

	virtual void      Render(Video* video, DWORD flags);

	virtual int       CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
	bool treat_translucent_polys_as_solid=true);

	virtual void      UseTerrainRegion(TerrainRegion* tr) { tregion = tr; }

protected:
	TerrainRegion*    tregion;
};

#endif TerrainHaze_h

