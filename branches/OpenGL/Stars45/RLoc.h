/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         RLoc.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Relative Location (RLoc) class declaration
*/

#ifndef RLoc_h
#define RLoc_h

#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class RLoc
{
public:
	RLoc();
	RLoc(const Point& loc, double d, double dv=5e3);
	RLoc(RLoc* rloc, double d, double dv=5e3);
	RLoc(const RLoc& r);
	~RLoc();

	// accessors:
	const Point&      Location();
	const Point&      BaseLocation()          const { return base_loc;   }
	RLoc*             ReferenceLoc()          const { return rloc;       }
	double            Distance()              const { return dex;        }
	double            DistanceVar()           const { return dex_var;    }
	double            Azimuth()               const { return az;         }
	double            AzimuthVar()            const { return az_var;     }
	double            Elevation()             const { return el;         }
	double            ElevationVar()          const { return el_var;     }

	void              Resolve();

	// mutators:
	void              SetBaseLocation(const Point& l);
	void              SetReferenceLoc(RLoc* r)         { rloc = r;             }
	void              SetDistance(double d)            { dex = (float) d;      }
	void              SetDistanceVar(double dv)        { dex_var = (float) dv; }
	void              SetAzimuth(double a)             { az = (float) a;       }
	void              SetAzimuthVar(double av)         { az_var = (float) av;  }
	void              SetElevation(double e)           { el = (float) e;       }
	void              SetElevationVar(double ev)       { el_var = (float) ev;  }

private:
	Point             loc;
	Point             base_loc;
	RLoc*             rloc;

	float             dex;
	float             dex_var;
	float             az;
	float             az_var;
	float             el;
	float             el_var;
};

// +--------------------------------------------------------------------+

#endif RLoc_h

