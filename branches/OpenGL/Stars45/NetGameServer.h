/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetGameServer.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Server-Side Network Game Manager class
*/

#ifndef NetGameServer_h
#define NetGameServer_h

#include "NetGame.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class NetChatMsg;

// +--------------------------------------------------------------------+

class NetGameServer : public NetGame, public SimObserver
{
public:
	NetGameServer();
	virtual ~NetGameServer();

	virtual bool         IsClient() const { return false; }
	virtual bool         IsServer() const { return true; }

	virtual void         ExecFrame();
	virtual void         CheckSessions();

	virtual void         Send();
	virtual void         SendData(NetData* data);
	virtual void         Respawn(DWORD objid, Ship* spawn);

	virtual bool         Update(SimObject* obj);
	virtual const char*  GetObserverName() const;

	virtual void         RouteChatMsg(NetChatMsg& chat_msg);

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

	virtual void         DoElemRequest(NetMsg* msg);
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

	virtual NetPlayer*   FindZombieByObjID(DWORD objid);
	virtual void         SendDisconnect(NetPlayer* zombie);

	List<Ship>           ships;
	List<NetPlayer>      zombies;
};

// +--------------------------------------------------------------------+

#endif NetGameServer_h

