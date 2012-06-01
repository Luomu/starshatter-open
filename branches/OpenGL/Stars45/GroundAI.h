/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         GroundAI.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Ground Unit (low-level) Artifical Intelligence class
*/

#ifndef GroundAI_h
#define GroundAI_h

#include "Types.h"
#include "SimObject.h"
#include "Director.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;
class System;
class CarrierAI;

// +--------------------------------------------------------------------+

class GroundAI : public Director, 
public SimObserver
{
public:
	GroundAI(SimObject* self);
	virtual ~GroundAI();

	virtual void      ExecFrame(double seconds);
	virtual void      SetTarget(SimObject* targ, System* sub=0);
	virtual SimObject* GetTarget()   const { return target; }
	virtual System*   GetSubTarget() const { return subtarget; }
	virtual int       Type()         const;

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

protected:
	virtual void      SelectTarget();

	Ship*             ship;
	SimObject*        target;
	System*           subtarget;
	double            exec_time;
	CarrierAI*        carrier_ai;
};


// +--------------------------------------------------------------------+

#endif GroundAI_h

