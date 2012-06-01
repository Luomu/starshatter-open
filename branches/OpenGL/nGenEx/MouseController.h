/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         MouseController.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Joystick Input class
*/

#ifndef MouseController_h
#define MouseController_h

#include "MotionController.h"

// +--------------------------------------------------------------------+

class MouseController : public MotionController
{
public:
	static const char* TYPENAME() { return "MouseController"; }

	MouseController();
	virtual ~MouseController();

	// setup
	virtual void   MapKeys(KeyMapEntry* mapping, int nkeys);

	// sample the physical device
	virtual void   Acquire();

	// translations
	virtual double X()         { return 0; }
	virtual double Y()         { return 0; }
	virtual double Z()         { return 0; }

	// rotations
	virtual double Pitch()     { if (active) return p; return 0; }
	virtual double Roll()      { if (active) return r; return 0; }
	virtual double Yaw()       { if (active) return w; return 0; }
	virtual int    Center()    { return 0; }

	// throttle
	virtual double Throttle()  { if (active) return t; return 0; }
	virtual void   SetThrottle(double throttle) { t = throttle; }

	// actions
	virtual int    Action(int n)     { return action[n];     }
	virtual int    ActionMap(int n);

	// actively sampling?
	virtual bool   Active()                { return active;   }
	virtual void   SetActive(bool a)       { active = a;      }

	static MouseController* GetInstance();

protected:
	double         p,r,w, dx, dy, t;
	int            action[MotionController::MaxActions];
	int            map[32];
	bool           active;
	int            active_key;
};

#endif MouseController_h

