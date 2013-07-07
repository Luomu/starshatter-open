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
    FILE:         Drive.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Conventional Drive (system) class
*/

#ifndef Drive_h
#define Drive_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Bolt;
class DriveSprite;
class Light;
class Sound;
class Ship;

// +--------------------------------------------------------------------+

struct DrivePort {
    static const char* TYPENAME() { return "DrivePort"; }

    DrivePort(const Point& l, float s);
    ~DrivePort();

    Point          loc;
    float          scale;

    DriveSprite*   flare;
    Bolt*          trail;
};

// +--------------------------------------------------------------------+

class Drive : public System
{
public:
    enum SUBTYPE   { PLASMA, FUSION, GREEN, RED, BLUE, YELLOW, STEALTH };
    enum Constants { MAX_ENGINES=16 };

    Drive(SUBTYPE s, float max_thrust, float max_aug, bool show_trail=true);
    Drive(const Drive& rhs);
    virtual ~Drive();

    static void       Initialize();
    static void       Close();
    static void       StartFrame();

    float             Thrust(double seconds);
    float             MaxThrust()          const { return thrust;        }
    float             MaxAugmenter()       const { return augmenter;     }
    int               NumEngines()         const;
    DriveSprite*      GetFlare(int port)   const;
    Bolt*             GetTrail(int port)   const;
    bool              IsAugmenterOn()      const;

    virtual void      AddPort(const Point& loc, float flare_scale=0);
    virtual void      CreatePort(const Point& loc, float flare_scale);

    virtual void      Orient(const Physical* rep);

    void              SetThrottle(double t, bool aug=false);
    virtual double    GetRequest(double seconds) const;

protected:
    float             thrust;
    float             augmenter;
    float             scale;
    float             throttle;
    float             augmenter_throttle;
    float             intensity;

    List<DrivePort>   ports;

    Sound*            sound;
    Sound*            burner_sound;
    bool              show_trail;
};

#endif Drive_h

