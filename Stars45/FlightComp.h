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
    FILE:         FlightComp.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Flight Computer systems class
*/

#ifndef FLIGHT_COMP_H
#define FLIGHT_COMP_H

#include "Types.h"
#include "Computer.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class FlightComp : public Computer
{
public:
    enum CompType { AVIONICS=1, FLIGHT, TACTICAL };

    FlightComp(int comp_type, const char* comp_name);
    FlightComp(const Computer& rhs);
    virtual ~FlightComp();

    virtual void      ExecSubFrame();

    int               Mode()                  const { return mode; }
    double            Throttle()              const { return throttle; }

    void              SetMode(int m)                { mode   = m; }
    void              SetVelocityLimit(double v)    { vlimit = (float) v; }
    void              SetTransLimit(double x, double y, double z);

    void              FullStop()                    { halt   = true; }

protected:
    virtual void      ExecTrans();
    virtual void      ExecThrottle();

    int               mode;
    int               halt;
    float             throttle;

    float             vlimit;
    float             trans_x_limit;
    float             trans_y_limit;
    float             trans_z_limit;
};

#endif FLIGHT_COMP_H

