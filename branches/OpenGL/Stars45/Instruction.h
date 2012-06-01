/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Instruction.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Instruction (NavPoint / Order / Objective) class declaration
*/

#ifndef Instruction_h
#define Instruction_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "Text.h"
#include "RLoc.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class Instruction : public SimObserver
{
public:
	static const char* TYPENAME() { return "Instruction"; }

	enum  ACTION 
	{
		VECTOR,
		LAUNCH,
		DOCK,
		RTB,

		DEFEND,
		ESCORT,
		PATROL,
		SWEEP,
		INTERCEPT,
		STRIKE,     // ground attack
		ASSAULT,    // starship attack
		RECON,

		//RECALL,
		//DEPLOY,

		NUM_ACTIONS
	};

	enum  STATUS 
	{  
		PENDING,
		ACTIVE,
		SKIPPED,
		ABORTED,
		FAILED,
		COMPLETE,

		NUM_STATUS
	};

	enum  FORMATION 
	{
		DIAMOND,
		SPREAD,
		BOX,
		TRAIL,

		NUM_FORMATIONS
	};

	enum  PRIORITY
	{
		PRIMARY = 1,
		SECONDARY,
		BONUS
	};

	Instruction(int action, const char* tgt);
	Instruction(const char* rgn, Point loc, int act=VECTOR);
	Instruction(SimRegion*  rgn, Point loc, int act=VECTOR);
	Instruction(const Instruction& instr);
	virtual ~Instruction();

	Instruction& operator = (const Instruction& n);

	// accessors:
	static const char*   ActionName(int a);
	static const char*   StatusName(int s);
	static const char*   FormationName(int f);
	static const char*   PriorityName(int p);

	const char*       RegionName()      const { return rgn_name;   }
	SimRegion*        Region()          const { return region;     }
	Point             Location()        const;
	RLoc&             GetRLoc()               { return rloc;       }
	int               Action()          const { return action;     }
	int               Status()          const { return status;     }
	int               Formation()       const { return formation;  }
	int               Speed()           const { return speed;      }
	int               EMCON()           const { return emcon;      }
	int               WeaponsFree()     const { return wep_free;   }
	int               Priority()        const { return priority;   }
	int               Farcast()         const { return farcast;    }
	double            HoldTime()        const { return hold_time;  }

	const char*       TargetName()      const { return tgt_name;   }
	const char*       TargetDesc()      const { return tgt_desc;   }
	SimObject*        GetTarget();

	void              Evaluate(Ship* s);
	const char*       GetShortDescription() const;
	const char*       GetDescription()  const;

	// mutators:
	void              SetRegion(SimRegion* r)       { region    = r;  }
	void              SetLocation(const Point& l);
	void              SetAction(int s)              { action    = s;  }
	void              SetStatus(int s);
	void              SetFormation(int s)           { formation = s;  }
	void              SetSpeed(int s)               { speed     = s;  }
	void              SetEMCON(int e)               { emcon     = e;  }
	void              SetWeaponsFree(int f)         { wep_free  = f;  }
	void              SetPriority(int p)            { priority  = p;  }
	void              SetFarcast(int f)             { farcast   = f;  }
	void              SetHoldTime(double t)         { hold_time = t;  }

	void              SetTarget(const char* n);
	void              SetTarget(SimObject* s);
	void              SetTargetDesc(const char* d);
	void              ClearTarget();

	virtual bool         Update(SimObject* s);
	virtual const char*  GetObserverName() const;

protected:
	Text              rgn_name;
	SimRegion*        region;
	RLoc              rloc;
	int               action;
	int               formation;
	int               status;
	int               speed;

	Text              tgt_name;
	Text              tgt_desc;
	SimObject*        target;
	int               emcon;
	int               wep_free;
	int               priority;
	int               farcast;

	double            hold_time;
};

// +--------------------------------------------------------------------+

#endif Instruction_h

