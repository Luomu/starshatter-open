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

    SUBSYSTEM:    nGenEx.lib
    FILE:         MouseController.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Joystick Input class
*/

#ifndef MouseController_h
#define MouseController_h

#include "MotionController.h"

// +--------------------------------------------------------------------+

class MouseController : public MotionController
{
public:
    static const char* TYPENAME() { return "MouseController"; }

    MouseController();
    virtual ~MouseController();

    // setup
    virtual void   MapKeys(KeyMapEntry* mapping, int nkeys);

    // sample the physical device
    virtual void   Acquire();

    // translations
    virtual double X()         { return 0; }
    virtual double Y()         { return 0; }
    virtual double Z()         { return 0; }

    // rotations
    virtual double Pitch()     { if (active) return p; return 0; }
    virtual double Roll()      { if (active) return r; return 0; }
    virtual double Yaw()       { if (active) return w; return 0; }
    virtual int    Center()    { return 0; }

    // throttle
    virtual double Throttle()  { if (active) return t; return 0; }
    virtual void   SetThrottle(double throttle) { t = throttle; }

    // actions
    virtual int    Action(int n)     { return action[n];     }
    virtual int    ActionMap(int n);

    // actively sampling?
    virtual bool   Active()                { return active;   }
    virtual void   SetActive(bool a)       { active = a;      }

    static MouseController* GetInstance();

protected:
    double         p,r,w, dx, dy, t;
    int            action[MotionController::MaxActions];
    int            map[32];
    bool           active;
    int            active_key;
};

#endif MouseController_h

