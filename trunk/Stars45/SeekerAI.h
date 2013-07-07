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
    FILE:         SeekerAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Seeker Missile (low-level) Artifical Intelligence class
*/

#ifndef SeekerAI_h
#define SeekerAI_h

#include "Types.h"
#include "SteerAI.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class Ship;
class Shot;

class SeekerAI : public SteerAI
{
public:
    SeekerAI(SimObject* s);
    virtual ~SeekerAI();

    virtual int       Type()      const { return 1001; }
    virtual int       Subframe()  const { return true; }

    virtual void      ExecFrame(double seconds);
    virtual void      FindObjective();
    virtual void      SetTarget(SimObject* targ, System* sub=0);
    virtual bool      Overshot();

    virtual void      SetPursuit(int p)  { pursuit = p;    }
    virtual int       GetPursuit() const { return pursuit; }

    virtual void      SetDelay(double d) { delay = d;      }
    virtual double    GetDelay() const   { return delay;   }

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

protected:
    // behaviors:
    virtual Steer     AvoidCollision();
    virtual Steer     SeekTarget();

    // accumulate behaviors:
    virtual void      Navigator();

    virtual void      CheckDecoys(double distance);

    Ship*             orig_target;
    Shot*             shot;
    int               pursuit;    // type of pursuit curve
    // 1: pure pursuit
    // 2: lead pursuit

    double            delay;      // don't start seeking until then
    bool              overshot;
};

// +--------------------------------------------------------------------+

#endif SeekerAI_h

