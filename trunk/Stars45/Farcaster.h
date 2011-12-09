/*  Project STARSHATTER
	John DiCamillo
	Copyright © 1997-2002. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Farcaster.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
*/

#ifndef Farcaster_h
#define Farcaster_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"
#include "SimObject.h"
#include "text.h"

// +----------------------------------------------------------------------+

class Ship;
class ShipDesign;
class Farcaster;

// +----------------------------------------------------------------------+

class Farcaster : public System, public SimObserver
{
public:
	Farcaster(double capacity, double sink_rate);
	Farcaster(const Farcaster& rhs);
	virtual ~Farcaster();

	enum CONSTANTS { NUM_APPROACH_PTS = 4 };

	virtual void   ExecFrame(double seconds);
	void           SetShip(Ship* s)     { ship  = s; }
	void           SetDest(Ship* d)     { dest  = d; }

	Point          ApproachPoint(int i) const { return approach_point[i];               }
	Point          StartPoint()         const { return start_point;                     }
	Point          EndPoint()           const { return end_point;                       }

	virtual void   SetApproachPoint(int i, Point loc);
	virtual void   SetStartPoint(Point loc);
	virtual void   SetEndPoint(Point loc);
	virtual void   SetCycleTime(double time);

	virtual void   Orient(const Physical* rep);

	// SimObserver:
	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

	// accessors:
	const Ship*    GetShip()            const { return ship; }
	const Ship*    GetDest()            const { return dest; }

	int            ActiveState()        const { return active_state; }
	double         WarpFactor()         const { return warp_fov;     }

protected:
	virtual void      Jump();
	virtual void      Arrive(Ship* s);

	Ship*             ship;
	Ship*             dest;
	Ship*             jumpship;

	Point             start_rel;
	Point             end_rel;
	Point             approach_rel[NUM_APPROACH_PTS];

	Point             start_point;
	Point             end_point;
	Point             approach_point[NUM_APPROACH_PTS];

	double            cycle_time;
	int               active_state;
	double            warp_fov;

	bool              no_dest;
};

// +----------------------------------------------------------------------+

#endif Farcaster_h

