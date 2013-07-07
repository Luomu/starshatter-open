/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

