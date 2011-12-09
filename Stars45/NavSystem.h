/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NavSystem.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Nav Points and so on...
*/

#ifndef NavSystem_h
#define NavSystem_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"

// +--------------------------------------------------------------------+

class StarSystem;
class Orbital;
class OrbitalBody;
class OrbitalRegion;
class Ship;

// +--------------------------------------------------------------------+

class NavSystem : public System
{
public:
	NavSystem();
	NavSystem(const NavSystem& rhs);
	virtual ~NavSystem();

	virtual void   ExecFrame(double seconds);

	virtual void   Distribute(double delivered_energy, double seconds);

	bool           AutoNavEngaged();
	void           EngageAutoNav();
	void           DisengageAutoNav();

protected:
	bool           autonav;
};

// +--------------------------------------------------------------------+


#endif NavSystem_h

