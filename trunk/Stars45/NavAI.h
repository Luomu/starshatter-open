/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NavAI.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Automatic Navigator
*/

#ifndef NavAI_h
#define NavAI_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"
#include "ShipAI.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Farcaster;

// +--------------------------------------------------------------------+

class NavAI : public ShipAI
{
public:
	NavAI(Ship* s);
	virtual ~NavAI();

	enum { DIR_TYPE = 2000 };
	virtual int       Type() const { return DIR_TYPE; }

	virtual void      ExecFrame(double seconds);
	virtual int       Subframe()  const { return true; }
	void              Disengage();
	bool              Complete()  const { return complete; }

protected:
	// behaviors:
	virtual Steer     SeekTarget();

	// steering functions:
	virtual Point     Transform(const Point& pt);
	virtual Steer     Seek(const Point& point);
	virtual Steer     Flee(const Point& point);
	virtual Steer     Avoid(const Point& point, float radius);
	virtual Steer     AvoidTerrain();

	// accumulate behaviors:
	virtual void      Navigator();
	virtual void      FindObjective();

	virtual void      HelmControl();
	virtual void      ThrottleControl();

	bool              complete;
	int               drop_state;
	int               quantum_state;
	int               terrain_warning;
	double            brakes;
	Farcaster*        farcaster;
};

// +--------------------------------------------------------------------+


#endif NavAI_h

