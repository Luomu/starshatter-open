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
    FILE:         NavLight.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation Lights System class
*/

#ifndef NavLight_h
#define NavLight_h

#include "Types.h"
#include "System.h"
#include "DriveSprite.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class NavLight : public System
{
public:
    enum Constants { MAX_LIGHTS = 8 };

    NavLight(double period, double scale);
    NavLight(const NavLight& rhs);
    virtual ~NavLight();

    static void    Initialize();
    static void    Close();

    virtual void   ExecFrame(double seconds);

    int            NumBeacons()      const { return nlights;       }
    Sprite*        Beacon(int index) const { return beacon[index]; }
    bool           IsEnabled()       const { return enable;        }

    virtual void   Enable();
    virtual void   Disable();
    virtual void   AddBeacon(Point loc, DWORD pattern, int type=1);
    virtual void   SetPeriod(double p);
    virtual void   SetPattern(int index, DWORD p);
    virtual void   SetOffset(DWORD o);

    virtual void   Orient(const Physical* rep);

protected:
    double      period;
    double      scale;
    bool        enable;

    int         nlights;

    Point          loc[MAX_LIGHTS];
    DriveSprite*   beacon[MAX_LIGHTS];
    DWORD          pattern[MAX_LIGHTS];
    int            beacon_type[MAX_LIGHTS];
    DWORD          offset;
};

#endif NavLight_h

