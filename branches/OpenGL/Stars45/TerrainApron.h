/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2005. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         TerrainApron.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	A Single Edge Section of a Terrain Object
*/

#ifndef TerrainApron_h
#define TerrainApron_h

#include "Types.h"
#include "Solid.h"

// +--------------------------------------------------------------------+

class Terrain;

// +--------------------------------------------------------------------+

class TerrainApron : public Solid
{
public:
	TerrainApron(Terrain* terrain,
	const Bitmap* patch, const Rect& rect,
	const Point&  p1,    const Point& p2);
	virtual ~TerrainApron();

	virtual void      Render(Video* video, DWORD flags);
	virtual void      Update();

	virtual int       CollidesWith(Graphic& o);
	virtual bool      Luminous()    const { return false; }
	virtual bool      Translucent() const { return false; }

	// accessors:
	double            Scale()           const { return scale;    }
	double            MountainScale()   const { return mtnscale; }
	double            SeaLevel()        const { return base;     }
	void              SetScales(double scale, double mtnscale, double base);

	void              Illuminate(Color ambient, List<Light>& lights);
	virtual int       CheckRayIntersection(Point pt, Point vpn, double len, Point& ipt,
	bool treat_translucent_polys_as_solid=true);

protected:
	virtual bool      BuildApron();

	Terrain*          terrain;
	int               nverts;
	int               npolys;
	int               terrain_width;

	Rect              rect;
	float*            heights;

	double            scale;
	double            mtnscale;
	double            base;
	double            size;
};


#endif TerrainApron_h

