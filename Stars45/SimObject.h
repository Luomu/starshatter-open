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
    FILE:         SimObject.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simulation Object and Observer classes
*/

#ifndef SimObject_h
#define SimObject_h

#include "Types.h"
#include "Physical.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Sim;
class SimRegion;
class SimObject;
class SimObserver;
class Scene;

// +--------------------------------------------------------------------+

class SimObject : public Physical
{
    friend class SimRegion;

public:
    static const char* TYPENAME() { return "SimObject"; }

    enum TYPES { 
        SIM_SHIP=100,
        SIM_SHOT,
        SIM_DRONE,
        SIM_EXPLOSION,
        SIM_DEBRIS,
        SIM_ASTEROID
    };

    SimObject()                        :                region(0), objid(0), active(0), notifying(0) { }
    SimObject(const char* n, int t=0)  : Physical(n,t), region(0), objid(0), active(0), notifying(0) { }
    virtual ~SimObject();

    virtual SimRegion*   GetRegion()                const { return region; }
    virtual void         SetRegion(SimRegion* rgn)        { region = rgn;  }

    virtual void         Notify();
    virtual void         Register(SimObserver* obs);
    virtual void         Unregister(SimObserver* obs);

    virtual void         Activate(Scene& scene);
    virtual void         Deactivate(Scene& scene);

    virtual DWORD        GetObjID()                 const { return objid;   }
    virtual void         SetObjID(DWORD id)               { objid = id;     }

    virtual bool         IsHostileTo(const SimObject* o)
    const { return false;   }

protected:
    SimRegion*           region;
    List<SimObserver>    observers;
    DWORD                objid;
    bool                 active;
    bool                 notifying;
};

// +--------------------------------------------------------------------+

class SimObserver
{
public:
    static const char* TYPENAME() { return "SimObserver"; }

    virtual ~SimObserver();

    int operator == (const SimObserver& o) const { return this == &o; }

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    virtual void         Observe(SimObject* obj);
    virtual void         Ignore(SimObject* obj);


protected:
    List<SimObject>      observe_list;
};

#endif SimObject_h

