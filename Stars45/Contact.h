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
    FILE:         Contact.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Sensor Contact class
*/

#ifndef Contact_h
#define Contact_h

#include "Types.h"
#include "SimObject.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;
class Shot;

class Contact : public SimObserver
{
    friend class Sensor;

public:
    static const char* TYPENAME() { return "Contact"; }

    Contact();
    Contact(Ship* s, float p, float a);
    Contact(Shot* s, float p, float a);
    virtual ~Contact();

    int operator == (const Contact& c) const;

    Ship*    GetShip()   const { return ship;  }
    Shot*    GetShot()   const { return shot;  }
    Point    Location()  const { return loc;   }

    double   PasReturn() const { return d_pas; }
    double   ActReturn() const { return d_act; }
    bool     PasLock()   const;
    bool     ActLock()   const;
    double   Age()       const;
    bool     IsProbed()  const { return probe; }

    DWORD    AcquisitionTime() const { return acquire_time; }

    int      GetIFF(const Ship* observer)    const;
    void     GetBearing(const Ship* observer, double& az, double& el, double& r) const;
    double   Range(const Ship* observer,
    double      limit=75e3)   const;

    bool     InFront(const Ship* observer)   const;
    bool     Threat(const Ship* observer)    const;
    bool     Visible(const Ship* observer)   const;

    void     Reset();
    void     Merge(Contact* c);
    void     ClearTrack();
    void     UpdateTrack();
    int      TrackLength()     const { return ntrack;   }
    Point    TrackPoint(int i) const;

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

private:
    Ship*    ship;
    Shot*    shot;
    Point    loc;
    DWORD    acquire_time;
    DWORD    time;

    Point*   track;
    int      ntrack;
    DWORD    track_time;

    float    d_pas;   // power output
    float    d_act;   // mass, size
    bool     probe;   // scanned by probe
};

#endif Contact_h

