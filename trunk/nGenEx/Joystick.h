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

    SUBSYSTEM:    nGen.lib
    FILE:         Joystick.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Joystick Input class
*/

#ifndef Joystick_h
#define Joystick_h

#include "MotionController.h"

// +--------------------------------------------------------------------+

class Joystick : public MotionController
{
public:
    static const char* TYPENAME() { return "Joystick"; }

    Joystick();
    virtual ~Joystick();

    // setup
    virtual void   MapKeys(KeyMapEntry* mapping, int nkeys);

    // sample the physical device
    virtual void   Acquire();

    // translations
    virtual double X()         { return x; }
    virtual double Y()         { return y; }
    virtual double Z()         { return z; }

    // rotations
    virtual double Pitch()     { return p; }
    virtual double Roll()      { return r; }
    virtual double Yaw()       { return w; }
    virtual int    Center()    { return 0; }

    // throttle
    virtual double Throttle()  { return t; }
    virtual void   SetThrottle(double throttle) { t = throttle; }

    // actions
    virtual int    Action(int n)     { return action[n];     }
    virtual int    ActionMap(int n)  { return KeyDownMap(n); }

    static bool    KeyDown(int key);
    static bool    KeyDownMap(int key);

    static Joystick*  GetInstance();
    static void       EnumerateDevices();
    static int        NumDevices();
    static const char* GetDeviceName(int i);

    static int     ReadRawAxis(int axis);
    static int     GetAxisMap(int n);
    static int     GetAxisInv(int n);

protected:
    double         ReadAxisDI(int axis);
    double         ReadAxisMM(int axis);
    void           ProcessAxes(double joy_x, double joy_y, double joy_r, double joy_t);
    void           ProcessHat(int i, DWORD joy_pov);

    double         x,y,z,p,r,w,t;
    bool           action[MotionController::MaxActions];
    bool           hat[4][4];
    int            map[KEY_MAP_SIZE];
    int            map_axis[4];
    bool           inv_axis[4];
};

#endif Joystick_h

