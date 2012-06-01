/*  Project nGen
	John DiCamillo
	Copyright © 1997-2002. All Rights Reserved.

	SUBSYSTEM:    nGen.lib
	FILE:         Joystick.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Joystick Input class
*/

#ifndef Joystick_h
#define Joystick_h

#include "MotionController.h"

// +--------------------------------------------------------------------+

class Joystick : public MotionController
{
public:
	static const char* TYPENAME() { return "Joystick"; }

	Joystick();
	virtual ~Joystick();

	// setup
	virtual void   MapKeys(KeyMapEntry* mapping, int nkeys);

	// sample the physical device
	virtual void   Acquire();

	// translations
	virtual double X()         { return x; }
	virtual double Y()         { return y; }
	virtual double Z()         { return z; }

	// rotations
	virtual double Pitch()     { return p; }
	virtual double Roll()      { return r; }
	virtual double Yaw()       { return w; }
	virtual int    Center()    { return 0; }

	// throttle
	virtual double Throttle()  { return t; }
	virtual void   SetThrottle(double throttle) { t = throttle; }

	// actions
	virtual int    Action(int n)     { return action[n];     }
	virtual int    ActionMap(int n)  { return KeyDownMap(n); }

	static bool    KeyDown(int key);
	static bool    KeyDownMap(int key);

	static Joystick*  GetInstance();
	static void       EnumerateDevices();
	static int        NumDevices();
	static const char* GetDeviceName(int i);

	static int     ReadRawAxis(int axis);
	static int     GetAxisMap(int n);
	static int     GetAxisInv(int n);

protected:
	double         ReadAxisDI(int axis);
	double         ReadAxisMM(int axis);
	void           ProcessAxes(double joy_x, double joy_y, double joy_r, double joy_t);
	void           ProcessHat(int i, DWORD joy_pov);

	double         x,y,z,p,r,w,t;
	bool           action[MotionController::MaxActions];
	bool           hat[4][4];
	int            map[KEY_MAP_SIZE];
	int            map_axis[4];
	bool           inv_axis[4];
};

#endif Joystick_h

