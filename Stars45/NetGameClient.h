/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetGameClient.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Game Manager class
*/

#ifndef NetGameClient_h
#define NetGameClient_h

#include "NetGame.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class NetJoinAnnounce;

// +--------------------------------------------------------------------+

class NetGameClient : public NetGame, public SimObserver
{
public:
	NetGameClient();
	virtual ~NetGameClient();

	virtual bool         IsClient() const { return true;  }
	virtual bool         IsServer() const { return false; }

	virtual void         ExecFrame();
	virtual void         Send();
	virtual void         SendData(NetData* data);
	virtual void         Respawn(DWORD objid, Ship* spawn);

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

protected:
	virtual void         DoJoinRequest(NetMsg* msg);
	virtual void         DoJoinAnnounce(NetMsg* msg);
	virtual void         DoQuitRequest(NetMsg* msg);
	virtual void         DoQuitAnnounce(NetMsg* msg);
	virtual void         DoGameOver(NetMsg* msg);
	virtual void         DoDisconnect(NetMsg* msg);

	virtual void         DoObjLoc(NetMsg* msg);
	virtual void         DoObjDamage(NetMsg* msg);
	virtual void         DoObjKill(NetMsg* msg);
	virtual void         DoObjSpawn(NetMsg* msg);
	virtual void         DoObjHyper(NetMsg* msg);
	virtual void         DoObjTarget(NetMsg* msg);
	virtual void         DoObjEmcon(NetMsg* msg);
	virtual void         DoSysDamage(NetMsg* msg);
	virtual void         DoSysStatus(NetMsg* msg);

	virtual void         DoElemCreate(NetMsg* msg);
	virtual void         DoShipLaunch(NetMsg* msg);
	virtual void         DoNavData(NetMsg* msg);
	virtual void         DoNavDelete(NetMsg* msg);

	virtual void         DoWepTrigger(NetMsg* msg);
	virtual void         DoWepRelease(NetMsg* msg);
	virtual void         DoWepDestroy(NetMsg* msg);

	virtual void         DoCommMsg(NetMsg* msg);
	virtual void         DoChatMsg(NetMsg* msg);
	virtual void         DoSelfDestruct(NetMsg* msg);

	virtual void         SendJoinRequest();

	virtual bool         DoJoinBacklog(NetJoinAnnounce* join_ann);

	DWORD                server_id;
	DWORD                join_req_time;
	List<NetJoinAnnounce>   join_backlog;
};

// +--------------------------------------------------------------------+

#endif NetGameClient_h

