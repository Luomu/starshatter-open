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
    FILE:         CarrierAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    "Air Boss" AI class for managing carrier fighter squadrons
*/

#ifndef CarrierAI_h
#define CarrierAI_h

#include "Types.h"
#include "Director.h"

// +--------------------------------------------------------------------+

class Sim;
class Ship;
class ShipAI;
class Instruction;
class Hangar;
class Element;
class FlightPlanner;

// +--------------------------------------------------------------------+

class CarrierAI : public Director
{
public:
    CarrierAI(Ship* s, int level);
    virtual ~CarrierAI();

    virtual void      ExecFrame(double seconds);

protected:
    virtual bool      CheckPatrolCoverage();
    virtual bool      CheckHostileElements();

    virtual bool      CreateStrike(Element* elem);

    virtual Element*  CreatePackage(int squad, int size, int code, const char* target=0, const char* loadname=0);
    virtual bool      LaunchElement(Element* elem);

    Sim*              sim;
    Ship*             ship;
    Hangar*           hangar;
    FlightPlanner*    flight_planner;
    int               exec_time;
    int               hold_time;
    int               ai_level;

    Element*          patrol_elem[4];
};

// +--------------------------------------------------------------------+

#endif CarrierAI_h

