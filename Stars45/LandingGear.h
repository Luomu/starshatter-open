/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         LandingGear.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Fighter undercarriage (landing gear) system class
*/

#ifndef LandingGear_h
#define LandingGear_h

#include "Types.h"
#include "System.h"
#include "Solid.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class LandingGear : public System
{
public:
	enum CONSTANTS  { MAX_GEAR = 4 };
	enum GEAR_STATE { GEAR_UP, GEAR_LOWER, GEAR_DOWN, GEAR_RAISE };

	LandingGear();
	LandingGear(const LandingGear& rhs);
	virtual ~LandingGear();

	virtual int       AddGear(Model* m, const Point& s, const Point& e);
	virtual void      ExecFrame(double seconds);
	virtual void      Orient(const Physical* rep);

	GEAR_STATE        GetState()        const { return state; }
	void              SetState(GEAR_STATE s);
	int               NumGear()         const { return ngear; }
	Solid*            GetGear(int i);
	Point             GetGearStop(int i);
	double            GetTouchDown();
	double            GetClearance()    const { return clearance; }

	static void       Initialize();
	static void       Close();

protected:
	GEAR_STATE        state;
	double            transit;
	double            clearance;

	int               ngear;
	Model*            models[MAX_GEAR];
	Solid*            gear[MAX_GEAR];
	Point             start[MAX_GEAR];
	Point             end[MAX_GEAR];
};

#endif LandingGear_h

