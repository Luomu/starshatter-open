/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetGame.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Game Manager class
*/

#ifndef NetGame_h
#define NetGame_h

#include "Types.h"
#include "Geometry.h"
#include "NetLink.h"
#include "Director.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Sim;
class Ship;
class Shot;
class NetData;
class NetMsg;
class NetPlayer;

// +--------------------------------------------------------------------+

class NetGame
{
public:
	static const char* TYPENAME() { return "NetGame"; }

	enum { SHIP, SHOT };

	NetGame();
	virtual ~NetGame();

	virtual bool         IsClient() const { return false; }
	virtual bool         IsServer() const { return false; }
	virtual bool         IsActive() const { return active; }

	virtual DWORD        GetNetID() const { return netid; }
	virtual DWORD        GetObjID() const;

	virtual void         ExecFrame();
	virtual void         Recv();
	virtual void         Send();

	virtual void         SendData(NetData* data) { }

	virtual NetPlayer*   FindPlayerByName(const char* name);
	virtual NetPlayer*   FindPlayerByNetID(DWORD netid);
	virtual NetPlayer*   FindPlayerByObjID(DWORD objid);
	virtual Ship*        FindShipByObjID(DWORD objid);
	virtual Shot*        FindShotByObjID(DWORD objid);

	virtual NetPeer*     GetPeer(NetPlayer* player);

	virtual void         Respawn(DWORD objid, Ship* spawn);

	static NetGame*      Create();
	static NetGame*      GetInstance();
	static bool          IsNetGame();
	static bool          IsNetGameClient();
	static bool          IsNetGameServer();
	static int           NumPlayers();

	static DWORD         GetNextObjID(int type=SHIP);

protected:
	virtual void         DoJoinRequest(NetMsg* msg)    { }
	virtual void         DoJoinAnnounce(NetMsg* msg)   { }
	virtual void         DoQuitRequest(NetMsg* msg)    { }
	virtual void         DoQuitAnnounce(NetMsg* msg)   { }
	virtual void         DoGameOver(NetMsg* msg)       { }
	virtual void         DoDisconnect(NetMsg* msg)     { }

	virtual void         DoObjLoc(NetMsg* msg)         { }
	virtual void         DoObjDamage(NetMsg* msg)      { }
	virtual void         DoObjKill(NetMsg* msg)        { }
	virtual void         DoObjSpawn(NetMsg* msg)       { }
	virtual void         DoObjHyper(NetMsg* msg)       { }
	virtual void         DoObjTarget(NetMsg* msg)      { }
	virtual void         DoObjEmcon(NetMsg* msg)       { }
	virtual void         DoSysDamage(NetMsg* msg)      { }
	virtual void         DoSysStatus(NetMsg* msg)      { }

	virtual void         DoElemCreate(NetMsg* msg)     { }
	virtual void         DoElemRequest(NetMsg* msg)    { }
	virtual void         DoShipLaunch(NetMsg* msg)     { }
	virtual void         DoNavData(NetMsg* msg)        { }
	virtual void         DoNavDelete(NetMsg* msg)      { }

	virtual void         DoWepTrigger(NetMsg* msg)     { }
	virtual void         DoWepRelease(NetMsg* msg)     { }
	virtual void         DoWepDestroy(NetMsg* msg)     { }

	virtual void         DoCommMsg(NetMsg* msg)        { }
	virtual void         DoChatMsg(NetMsg* msg)        { }
	virtual void         DoSelfDestruct(NetMsg* msg)   { }

	List<NetPlayer>      players;
	NetLink*             link;

	DWORD                objid;
	DWORD                netid;
	Ship*                local_player;
	Text                 player_name;
	Text                 player_pass;
	Ship*                target;
	Sim*                 sim;
	bool                 active;

	DWORD                last_send_time;
};

// +--------------------------------------------------------------------+

#endif NetGame_h

