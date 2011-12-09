/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Keyboard.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Keyboard Input class
*/

#ifndef Keyboard_h
#define Keyboard_h

#include "MotionController.h"

// +--------------------------------------------------------------------+

class Keyboard : public MotionController
{
public:
	static const char* TYPENAME() { return "Keyboard"; }

	Keyboard();
	virtual ~Keyboard();

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
	virtual int    Center()    { return c; }

	// throttle
	virtual double Throttle()  { return t; }
	virtual void   SetThrottle(double throttle) { t = throttle; }

	// actions
	virtual int    Action(int n)     { return action[n];     }
	virtual int    ActionMap(int n)  { return KeyDownMap(n); }

	static bool    KeyDown(int key);
	static bool    KeyDownMap(int key);
	static void    FlushKeys();

	static Keyboard* GetInstance();

protected:
	double         x,y,z,p,r,w,t;
	double         p1, r1, w1;
	int            c;
	int            action[MotionController::MaxActions];

	static int     map[KEY_MAP_SIZE];
	static int     alt[KEY_MAP_SIZE];
};

// +--------------------------------------------------------------------+

#endif Keyboard_h

