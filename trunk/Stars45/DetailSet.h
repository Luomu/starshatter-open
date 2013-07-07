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
    FILE:         DetailSet.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Level of Detail Manger class
*/

#ifndef DetailSet_h
#define DetailSet_h

#include "Types.h"
#include "Geometry.h"
#include "Graphic.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Sim;
class SimRegion;

// +--------------------------------------------------------------------+

class DetailSet
{
public:
    enum { MAX_DETAIL = 4 };

    DetailSet();
    virtual ~DetailSet();

    int            DefineLevel(double r, Graphic* g=0, Point* offset=0, Point* spin=0);
    void           AddToLevel(int level, Graphic* g, Point* offset=0, Point* spin=0);
    int            NumLevels() const { return levels; }
    int            NumModels(int level) const;

    void           ExecFrame(double seconds);
    void           SetLocation(SimRegion* rgn, const Point& loc);
    static void    SetReference(SimRegion* rgn, const Point& loc);

    int            GetDetailLevel();
    Graphic*       GetRep(int level, int n=0);
    Point          GetOffset(int level, int n=0);
    Point          GetSpin(int level, int n=0);
    void           Destroy();

protected:
    List<Graphic>     rep[MAX_DETAIL];
    List<Point>       off[MAX_DETAIL];
    double            rad[MAX_DETAIL];

    List<Point>       spin;
    List<Point>       rate;

    int               index;
    int               levels;
    SimRegion*        rgn;
    Point             loc;

    static SimRegion* ref_rgn;
    static Point      ref_loc;
};

// +--------------------------------------------------------------------+

#endif DetailSet_h

