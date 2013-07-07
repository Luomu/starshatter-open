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
    FILE:         RLoc.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation Point class implementation
*/

#include "MemDebug.h"
#include "RLoc.h"
#include "Random.h"

// +----------------------------------------------------------------------+

RLoc::RLoc()
    : rloc(0), dex(0), dex_var(5.0e3f), az(0), az_var(3.1415f), el(0), el_var(0.1f)
{ }

RLoc::RLoc(const Point& l, double d, double dv)
    : loc(l), base_loc(l), rloc(0), dex((float) d), dex_var((float) dv),
      az(0), az_var(3.1415f), el(0), el_var(0.1f)
{ }

RLoc::RLoc(RLoc* l, double d, double dv)
    : rloc(l), dex((float) d), dex_var((float) dv),
      az(0), az_var(3.1415f), el(0), el_var(0.1f)
{ }

RLoc::RLoc(const RLoc& r)
    : loc(r.loc), base_loc(r.base_loc), rloc(r.rloc),
      dex(r.dex), dex_var(r.dex_var),
      az(r.az),   az_var(r.az_var),
      el(r.el),   el_var(r.el_var)
{ }

RLoc::~RLoc()
{ }

// +----------------------------------------------------------------------+

const Point&
RLoc::Location()
{
    if (rloc || dex > 0) Resolve();
    return loc;
}

// +----------------------------------------------------------------------+

void
RLoc::Resolve()
{
    if (rloc) {
        base_loc = rloc->Location();
        rloc = 0;
    }

    if (dex > 0) {
        double d = dex + Random(-dex_var, dex_var);
        double a = az  + Random(-az_var,  az_var);
        double e = el  + Random(-el_var,  el_var);

        Point  p = Point(d *  sin(a),
        d * -cos(a),
        d *  sin(e));

        loc = base_loc + p;
        dex = 0;
    }
    else {
        loc = base_loc;
    }
}

// +----------------------------------------------------------------------+

void
RLoc::SetBaseLocation(const Point& l)
{
    base_loc = l;
    loc      = l;
}
