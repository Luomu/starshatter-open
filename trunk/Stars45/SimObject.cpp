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
    FILE:         SimObject.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simulation Object and Observer classes
*/

#include "MemDebug.h"
#include "SimObject.h"

#include "Graphic.h"
#include "Light.h"
#include "Scene.h"

// +--------------------------------------------------------------------+

SimObserver::~SimObserver()
{
    ListIter<SimObject> observed = observe_list;
    while (++observed)
    observed->Unregister(this);
}

void
SimObserver::Observe(SimObject* obj)
{
    if (obj) {
        obj->Register(this);

        if (!observe_list.contains(obj))
        observe_list.append(obj);
    }
}

void
SimObserver::Ignore(SimObject* obj)
{
    if (obj) {
        obj->Unregister(this);
        observe_list.remove(obj);
    }
}

bool
SimObserver::Update(SimObject* obj)
{
    if (obj)
    observe_list.remove(obj);

    return true;
}

const char*
SimObserver::GetObserverName() const
{
    static char name[32];
    sprintf_s(name, "SimObserver 0x%08x", (DWORD) this);
    return name;
}

// +--------------------------------------------------------------------+

SimObject::~SimObject()
{
    Notify();
}

// +--------------------------------------------------------------------+

void
SimObject::Notify()
{
    if (!notifying) {
        notifying = true;

        int nobservers = observers.size();
        int nupdate    = 0;

        if (nobservers > 0) {
            ListIter<SimObserver> iter = observers;
            while (++iter) {
                SimObserver* observer = iter.value();
                observer->Update(this);
                nupdate++;
            }

            observers.clear();
        }

        if (nobservers != nupdate) {
            ::Print("WARNING: incomplete notify sim object '%s' - %d of %d notified\n",
            Name(), nupdate, nobservers);
        }

        notifying = false;
    }
    else {
        ::Print("WARNING: double notify on sim object '%s'\n", Name());
    }
}

// +--------------------------------------------------------------------+

void
SimObject::Register(SimObserver* observer)
{
    if (!notifying && !observers.contains(observer))
    observers.append(observer);
}

// +--------------------------------------------------------------------+

void
SimObject::Unregister(SimObserver* observer)
{
    if (!notifying)
    observers.remove(observer);
}

// +--------------------------------------------------------------------+

void
SimObject::Activate(Scene& scene)
{
    if (rep)
    scene.AddGraphic(rep);
    if (light)
    scene.AddLight(light);

    active = true;
}

void
SimObject::Deactivate(Scene& scene)
{
    if (rep)
    scene.DelGraphic(rep);
    if (light)
    scene.DelLight(light);

    active = false;
}

