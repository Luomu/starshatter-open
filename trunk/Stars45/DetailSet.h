/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         DetailSet.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Level of Detail Manger class
*/

#ifndef DetailSet_h
#define DetailSet_h

#include "Types.h"
#include "Geometry.h"
#include "Graphic.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Sim;
class SimRegion;

// +--------------------------------------------------------------------+

class DetailSet
{
public:
	enum { MAX_DETAIL = 4 };

	DetailSet();
	virtual ~DetailSet();

	int            DefineLevel(double r, Graphic* g=0, Point* offset=0, Point* spin=0);
	void           AddToLevel(int level, Graphic* g, Point* offset=0, Point* spin=0);
	int            NumLevels() const { return levels; }
	int            NumModels(int level) const;

	void           ExecFrame(double seconds);
	void           SetLocation(SimRegion* rgn, const Point& loc);
	static void    SetReference(SimRegion* rgn, const Point& loc);

	int            GetDetailLevel();
	Graphic*       GetRep(int level, int n=0);
	Point          GetOffset(int level, int n=0);
	Point          GetSpin(int level, int n=0);
	void           Destroy();

protected:
	List<Graphic>     rep[MAX_DETAIL];
	List<Point>       off[MAX_DETAIL];
	double            rad[MAX_DETAIL];

	List<Point>       spin;
	List<Point>       rate;

	int               index;
	int               levels;
	SimRegion*        rgn;
	Point             loc;

	static SimRegion* ref_rgn;
	static Point      ref_loc;
};

// +--------------------------------------------------------------------+

#endif DetailSet_h

