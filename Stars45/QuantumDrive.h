/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         QuantumDrive.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Quantum (JUMP) Drive (system) class
*/

#ifndef QuantumDrive_h
#define QuantumDrive_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;
class SimRegion;

// +--------------------------------------------------------------------+

class QuantumDrive : public System
{
public:
	enum SUBTYPE { QUANTUM, HYPER };

	QuantumDrive(SUBTYPE s, double capacity, double sink_rate);
	QuantumDrive(const QuantumDrive& rhs);
	virtual ~QuantumDrive();

	enum ACTIVE_STATES {
		ACTIVE_READY, ACTIVE_COUNTDOWN, ACTIVE_PREWARP, ACTIVE_POSTWARP
	};

	void              SetDestination(SimRegion* rgn, const Point& loc);
	bool              Engage(bool immediate=false);
	int               ActiveState() const { return active_state; }
	double            WarpFactor()  const { return warp_fov;     }
	double            JumpTime()    const { return jump_time;    }
	virtual void      PowerOff();

	virtual void      ExecFrame(double seconds);

	void              SetShip(Ship* s)  { ship = s;    }
	Ship*             GetShip() const   { return ship; }

	double            GetCountdown() const    { return countdown; }
	void              SetCountdown(double d)  { countdown = d;    }

protected:
	void              Jump();
	void              AbortJump();

	int               active_state;

	Ship*             ship;
	double            warp_fov;
	double            jump_time;
	double            countdown;

	SimRegion*        dst_rgn;
	Point             dst_loc;
};

#endif // QuantumDrive_h

