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
    FILE:         GroundAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Ground Unit (low-level) Artifical Intelligence class
*/

#ifndef GroundAI_h
#define GroundAI_h

#include "Types.h"
#include "SimObject.h"
#include "Director.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Ship;
class System;
class CarrierAI;

// +--------------------------------------------------------------------+

class GroundAI : public Director, 
public SimObserver
{
public:
    GroundAI(SimObject* self);
    virtual ~GroundAI();

    virtual void      ExecFrame(double seconds);
    virtual void      SetTarget(SimObject* targ, System* sub=0);
    virtual SimObject* GetTarget()   const { return target; }
    virtual System*   GetSubTarget() const { return subtarget; }
    virtual int       Type()         const;

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

protected:
    virtual void      SelectTarget();

    Ship*             ship;
    SimObject*        target;
    System*           subtarget;
    double            exec_time;
    CarrierAI*        carrier_ai;
};


// +--------------------------------------------------------------------+

#endif GroundAI_h

