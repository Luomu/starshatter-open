/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetPlayer.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Player (Director) class
*/

#ifndef NetPlayer_h
#define NetPlayer_h

#include "Types.h"
#include "Geometry.h"
#include "Director.h"
#include "SimObject.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Sim;
class Ship;
class NetMsg;
class NetObjLoc;
class NetObjHyper;
class NetObjTarget;
class NetObjEmcon;
class NetSysDamage;
class NetSysStatus;
class NetWepTrigger;
class NetWepRelease;
class NetWepDestroy;
class NetCommMsg;

// +--------------------------------------------------------------------+

class NetPlayer : public Director, public SimObserver
{
public:
	static const char* TYPENAME() { return "NetPlayer"; }

	NetPlayer(DWORD nid) : netid(nid), objid(0), ship(0), iff(0) { }
	~NetPlayer();

	int operator == (const NetPlayer& p) const { return netid == p.netid; }

	DWORD                GetNetID()  const { return netid;   }
	DWORD                GetObjID()  const { return objid;   }
	void                 SetObjID(DWORD o) { objid = o;      }

	int                  GetIFF()    const { return iff;     }
	Ship*                GetShip()   const { return ship;    }
	void                 SetShip(Ship* s);

	const char*          Name()                     const { return name;    }
	void                 SetName(const char* n)           { name = n;       }
	const char*          SerialNumber()             const { return serno;   }
	void                 SetSerialNumber(const char* n)   { serno = n;      }

	virtual void         ExecFrame(double seconds);

	bool                 DoObjLoc(NetObjLoc*         obj_loc);
	bool                 DoObjHyper(NetObjHyper*     obj_hyper);
	bool                 DoObjTarget(NetObjTarget*   obj_target);
	bool                 DoObjEmcon(NetObjEmcon*     obj_emcon);
	bool                 DoWepTrigger(NetWepTrigger* trigger);
	bool                 DoWepRelease(NetWepRelease* release);
	bool                 DoCommMessage(NetCommMsg*   comm_msg);
	bool                 DoSysDamage(NetSysDamage*   sys_damage);
	bool                 DoSysStatus(NetSysStatus*   sys_status);

	virtual int          Type()      const { return 2;       }

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

protected:
	DWORD                netid;
	DWORD                objid;
	Text                 name;
	Text                 serno;
	Ship*                ship;
	int                  iff;

	Point                loc_error;
	double               bleed_time;
};

// +--------------------------------------------------------------------+

#endif NetPlayer_h

