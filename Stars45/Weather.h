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
    FILE:         Weather.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Manages local weather conditions according to the system stardate
*/

#ifndef Weather_h
#define Weather_h

#include "Types.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Weather
{
public:
    Weather();
    virtual ~Weather();

    enum STATE  {  CLEAR, 
        HIGH_CLOUDS, 
        MODERATE_CLOUDS,
        OVERCAST,
        FOG,
        STORM,

        NUM_STATES
    };

    virtual void      Update();

    // accessors:
    STATE             State()           const { return state;            }
    Text              Description()     const;
    double            Period()          const { return period;           }
    double            Chance(STATE s)   const { return chances[(int)s];  }
    double            Ceiling()         const { return ceiling;          }
    double            Visibility()      const { return visibility;       }

    void              SetPeriod(double p)        { period = p;           }
    void              SetChance(int n, double c);

protected:
    void              NormalizeChances();

    STATE             state;
    double            period;
    double            chances[NUM_STATES];
    double            ceiling;
    double            visibility;

    STATE             active_states[NUM_STATES];
    double            thresholds[NUM_STATES];
};


#endif Weather_h

