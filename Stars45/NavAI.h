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
    FILE:         NavAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Automatic Navigator
*/

#ifndef NavAI_h
#define NavAI_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"
#include "ShipAI.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Farcaster;

// +--------------------------------------------------------------------+

class NavAI : public ShipAI
{
public:
    NavAI(Ship* s);
    virtual ~NavAI();

    enum { DIR_TYPE = 2000 };
    virtual int       Type() const { return DIR_TYPE; }

    virtual void      ExecFrame(double seconds);
    virtual int       Subframe()  const { return true; }
    void              Disengage();
    bool              Complete()  const { return complete; }

protected:
    // behaviors:
    virtual Steer     SeekTarget();

    // steering functions:
    virtual Point     Transform(const Point& pt);
    virtual Steer     Seek(const Point& point);
    virtual Steer     Flee(const Point& point);
    virtual Steer     Avoid(const Point& point, float radius);
    virtual Steer     AvoidTerrain();

    // accumulate behaviors:
    virtual void      Navigator();
    virtual void      FindObjective();

    virtual void      HelmControl();
    virtual void      ThrottleControl();

    bool              complete;
    int               drop_state;
    int               quantum_state;
    int               terrain_warning;
    double            brakes;
    Farcaster*        farcaster;
};

// +--------------------------------------------------------------------+


#endif NavAI_h

