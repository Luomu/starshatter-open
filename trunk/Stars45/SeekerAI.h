/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         SeekerAI.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Seeker Missile (low-level) Artifical Intelligence class
*/

#ifndef SeekerAI_h
#define SeekerAI_h

#include "Types.h"
#include "SteerAI.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class Ship;
class Shot;

class SeekerAI : public SteerAI
{
public:
	SeekerAI(SimObject* s);
	virtual ~SeekerAI();

	virtual int       Type()      const { return 1001; }
	virtual int       Subframe()  const { return true; }

	virtual void      ExecFrame(double seconds);
	virtual void      FindObjective();
	virtual void      SetTarget(SimObject* targ, System* sub=0);
	virtual bool      Overshot();

	virtual void      SetPursuit(int p)  { pursuit = p;    }
	virtual int       GetPursuit() const { return pursuit; }

	virtual void      SetDelay(double d) { delay = d;      }
	virtual double    GetDelay() const   { return delay;   }

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

protected:
	// behaviors:
	virtual Steer     AvoidCollision();
	virtual Steer     SeekTarget();

	// accumulate behaviors:
	virtual void      Navigator();

	virtual void      CheckDecoys(double distance);

	Ship*             orig_target;
	Shot*             shot;
	int               pursuit;    // type of pursuit curve
	// 1: pure pursuit
	// 2: lead pursuit

	double            delay;      // don't start seeking until then
	bool              overshot;
};

// +--------------------------------------------------------------------+

#endif SeekerAI_h

