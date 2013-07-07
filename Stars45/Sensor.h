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
    FILE:         Sensor.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Integrated (Passive and Active) Sensor Package class
*/

#ifndef Sensor_h
#define Sensor_h

#include "Types.h"
#include "SimObject.h"
#include "System.h"
#include "Geometry.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Shot;
class Contact;

// +--------------------------------------------------------------------+

class Sensor : public System, public SimObserver
{
public:
    enum Mode {
        PAS, STD, ACM, GM,   // fighter modes
        PST, CST             // starship modes
    };

    Sensor();
    Sensor(const Sensor& rhs);
    virtual ~Sensor();

    virtual void      ExecFrame(double seconds);
    virtual SimObject* LockTarget(int  type=SimObject::SIM_SHIP,
    bool closest=false,
    bool hostile=false);
    virtual SimObject* LockTarget(SimObject* candidate);
    virtual bool      IsTracking(SimObject* tgt);
    virtual void      DoEMCON(int emcon);

    virtual void      ClearAllContacts();

    virtual Mode      GetMode()         const { return mode; }
    virtual void      SetMode(Mode m);
    virtual double    GetBeamLimit()    const;
    virtual double    GetBeamRange()    const;
    virtual void      IncreaseRange();
    virtual void      DecreaseRange();
    virtual void      AddRange(double r);

    Contact*          FindContact(Ship* s);
    Contact*          FindContact(Shot* s);

    // borrow this sensor for missile seeker
    SimObject*        AcquirePassiveTargetForMissile();
    SimObject*        AcquireActiveTargetForMissile();

    // SimObserver:
    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

protected:
    void              ProcessContact(Ship* contact, double az1, double az2);
    void              ProcessContact(Shot* contact, double az1, double az2);

    Mode              mode;
    int               nsettings;
    int               range_index;
    float             range_settings[8];
    SimObject*        target;

    List<Contact>     contacts;
};

#endif Sensor_h

