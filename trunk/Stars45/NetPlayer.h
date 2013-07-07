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

