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
    FILE:         Explosion.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Explosion Sprite class
*/

#ifndef Explosion_h
#define Explosion_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "Sound.h"

// +--------------------------------------------------------------------+

class Solid;
class Particles;
class System;

// +--------------------------------------------------------------------+

class Explosion : public SimObject, 
public SimObserver
{
public:
    static const char* TYPENAME() { return "Explosion"; }

    enum Type { SHIELD_FLASH      =  1,
        HULL_FLASH        =  2,
        BEAM_FLASH        =  3,
        SHOT_BLAST        =  4,
        HULL_BURST        =  5,
        HULL_FIRE         =  6,
        PLASMA_LEAK       =  7,
        SMOKE_TRAIL       =  8,
        SMALL_FIRE        =  9, 
        SMALL_EXPLOSION   = 10,
        LARGE_EXPLOSION   = 11,
        LARGE_BURST       = 12,
        NUKE_EXPLOSION    = 13,
        QUANTUM_FLASH     = 14,
        HYPER_FLASH       = 15
    };

    Explosion(int type, const Vec3& pos, const Vec3& vel, 
    float exp_scale, float part_scale,
    SimRegion* rgn=0, SimObject* source=0);
    virtual ~Explosion();

    static void       Initialize();
    static void       Close();

    virtual void      ExecFrame(double seconds);
    Particles*        GetParticles()                { return particles; }

    virtual void      Activate(Scene& scene);
    virtual void      Deactivate(Scene& scene);

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

protected:
    int               type;
    Particles*        particles; 

    float             scale;
    float             scale1;
    float             scale2;

    SimObject*        source;
    Point             mount_rel;
};

#endif Explosion_h

