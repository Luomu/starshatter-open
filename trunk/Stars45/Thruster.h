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
    FILE:         Thruster.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Conventional Thruster (system) class
*/

#ifndef Thruster_h
#define Thruster_h

#include "Types.h"
#include "System.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

struct ThrusterPort {
    static const char* TYPENAME() { return "ThrusterPort"; }

    ThrusterPort(int t, const Point& l, DWORD f, float s);
    ~ThrusterPort();

    int         type;
    DWORD       fire;
    float       burn;
    float       scale;
    Point       loc;
    Graphic*    flare;
    Graphic*    trail;
};

// +--------------------------------------------------------------------+

class Thruster : public System
{
public:
    static const char* TYPENAME() { return "Thruster"; }

    enum Constants {
        LEFT,  RIGHT, FORE,    AFT,     TOP,    BOTTOM,
        YAW_L, YAW_R, PITCH_D, PITCH_U, ROLL_L, ROLL_R
    };

    Thruster(int dtype, double thrust, float flare_scale=0);
    Thruster(const Thruster& rhs);
    virtual ~Thruster();

    static void       Initialize();
    static void       Close();

    virtual void      ExecFrame(double seconds);
    virtual void      ExecTrans(double x, double y, double z);
    virtual void      SetShip(Ship* s);

    virtual double    TransXLimit();
    virtual double    TransYLimit();
    virtual double    TransZLimit();

    virtual void      AddPort(int type, const Point& loc, DWORD fire, float flare_scale=0);
    virtual void      CreatePort(int type, const Point& loc, DWORD fire, float flare_scale);

    int               NumThrusters()       const;
    Graphic*          Flare(int engine)    const;
    Graphic*          Trail(int engine)    const;
    virtual void      Orient(const Physical* rep);
    virtual double    GetRequest(double seconds) const;

protected:
    void              IncBurn(int inc, int dec);
    void              DecBurn(int a,   int b);

    Ship*             ship;
    float             thrust;
    float             scale;
    float             burn[12];

    float             avail_x,   avail_y,    avail_z;
    float             trans_x,   trans_y,    trans_z;
    float             roll_rate, pitch_rate, yaw_rate;
    float             roll_drag, pitch_drag, yaw_drag;

    List<ThrusterPort>   ports;
};

#endif Thruster_h

