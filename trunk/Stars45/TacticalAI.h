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
    FILE:         TacticalAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Common base class and interface for mid-level (tactical) AI
*/

#ifndef TacticalAI_h
#define TacticalAI_h

#include "Types.h"
#include "Director.h"

// +--------------------------------------------------------------------+

class Ship;
class ShipAI;
class Instruction;
class CarrierAI;

// +--------------------------------------------------------------------+

class TacticalAI : public Director
{
public:
    TacticalAI(ShipAI* ai);
    virtual ~TacticalAI();

    enum ROE {
        NONE,
        SELF_DEFENSIVE,
        DEFENSIVE,
        DIRECTED,
        FLEXIBLE,
        AGRESSIVE
    };

    virtual void      ExecFrame(double seconds);

    virtual ROE       RulesOfEngagement()  const { return roe;           }
    virtual double    ThreatLevel()        const { return threat_level;  }
    virtual double    SupportLevel()       const { return support_level; }

protected:
    // pick the best target if we don't have one yet:
    virtual void      CheckOrders();
    virtual bool      CheckShipOrders();
    virtual bool      ProcessOrders();
    virtual bool      CheckFlightPlan();
    virtual bool      CheckObjectives();

    virtual void      SelectTarget();
    virtual void      SelectTargetDirected(Ship* tgt=0);
    virtual void      SelectTargetOpportunity();
    virtual void      CheckTarget();
    virtual void      FindThreat();
    virtual void      FindSupport();
    virtual void      FindFormationSlot(int formation);

    virtual bool      CanTarget(Ship* tgt);
    virtual void      ClearRadioOrders();

    Ship*             ship;
    ShipAI*           ship_ai;
    CarrierAI*        carrier_ai;

    Instruction*      navpt;
    Instruction*      orders;

    double            agression;
    ROE               roe;
    int               element_index;
    int               action;
    int               exec_time;
    int               directed_tgtid;

    double            threat_level;
    double            support_level;
};

// +--------------------------------------------------------------------+

#endif TacticalAI_h

