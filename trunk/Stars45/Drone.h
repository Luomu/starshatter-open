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
    FILE:         Drone.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Decoy / Weapons Drone class
*/

#ifndef Drone_h
#define Drone_h

#include "Types.h"
#include "Geometry.h"
#include "Shot.h"

// +--------------------------------------------------------------------+

class Camera;
class Ship;
class Trail;
class System;
class WeaponDesign;
class Sprite3D;
class Sound;

// +--------------------------------------------------------------------+

class Drone : public Shot
{
public:
    static const char* TYPENAME() { return "Drone"; }

    Drone(const Point& pos, const Camera& cam, WeaponDesign* design, const Ship* ship=0);
    virtual ~Drone();

    virtual void      SeekTarget(SimObject* target, System* sub=0);
    virtual void      ExecFrame(double factor);

    virtual bool      IsDrone()   const    { return true;             }
    virtual bool      IsDecoy()   const    { return decoy_type != 0;  }
    virtual bool      IsProbe()   const    { return probe?true:false; }

    virtual void      Disarm();
    virtual void      Destroy();

    // SENSORS AND VISIBILITY:
    virtual double       PCS()             const;
    virtual double       ACS()             const;
    virtual const char*  ClassName()       const;
    virtual int          Class()           const;

    // DAMAGE RESOLUTION:
    void        SetLife(int seconds) { life = seconds; }
    virtual int HitBy(Shot* shot, Point& impact);

protected:
    int               iff_code;
    int               decoy_type;
    int               probe;
};

#endif Drone_h

