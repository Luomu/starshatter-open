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
    FILE:         RLoc.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Relative Location (RLoc) class declaration
*/

#ifndef RLoc_h
#define RLoc_h

#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class RLoc
{
public:
    RLoc();
    RLoc(const Point& loc, double d, double dv=5e3);
    RLoc(RLoc* rloc, double d, double dv=5e3);
    RLoc(const RLoc& r);
    ~RLoc();

    // accessors:
    const Point&      Location();
    const Point&      BaseLocation()          const { return base_loc;   }
    RLoc*             ReferenceLoc()          const { return rloc;       }
    double            Distance()              const { return dex;        }
    double            DistanceVar()           const { return dex_var;    }
    double            Azimuth()               const { return az;         }
    double            AzimuthVar()            const { return az_var;     }
    double            Elevation()             const { return el;         }
    double            ElevationVar()          const { return el_var;     }

    void              Resolve();

    // mutators:
    void              SetBaseLocation(const Point& l);
    void              SetReferenceLoc(RLoc* r)         { rloc = r;             }
    void              SetDistance(double d)            { dex = (float) d;      }
    void              SetDistanceVar(double dv)        { dex_var = (float) dv; }
    void              SetAzimuth(double a)             { az = (float) a;       }
    void              SetAzimuthVar(double av)         { az_var = (float) av;  }
    void              SetElevation(double e)           { el = (float) e;       }
    void              SetElevationVar(double ev)       { el_var = (float) ev;  }

private:
    Point             loc;
    Point             base_loc;
    RLoc*             rloc;

    float             dex;
    float             dex_var;
    float             az;
    float             az_var;
    float             el;
    float             el_var;
};

// +--------------------------------------------------------------------+

#endif RLoc_h

