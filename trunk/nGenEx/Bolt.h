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

    SUBSYSTEM:    nGenEx.lib
    FILE:         Bolt.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Bolt (Polygon) Object
*/

#ifndef Bolt_h
#define Bolt_h

#include "Graphic.h"
#include "Polygon.h"

// +--------------------------------------------------------------------+

class Bolt : public Graphic
{
public:
    static const char* TYPENAME() { return "Bolt"; }

    Bolt(double len=16, double wid=1, Bitmap* tex=0, int share=0);
    virtual ~Bolt();

    // operations
    virtual void   Render(Video* video, DWORD flags);
    virtual void   Update();

    // accessors / mutators
    virtual void   SetOrientation(const Matrix& o);
    void           SetDirection(const Point& v);
    void           SetEndPoints(const Point& from, const Point& to);
    void           SetTextureOffset(double from, double to);

    virtual void   TranslateBy(const Point& ref);

    double         Shade()     const    { return shade;   }
    void           SetShade(double s)   { shade = s;      }
    virtual bool   IsBolt()       const { return true;    }

protected:
    double         length;
    double         width;
    double         shade;

    Poly           poly;
    Material       mtl;
    VertexSet      vset;
    Bitmap*        texture;
    int            shared;

    Point          vpn;
    Point          origin;
};

// +--------------------------------------------------------------------+

#endif Bolt_h

