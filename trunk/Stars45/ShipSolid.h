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
    FILE:         ShipSolid.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    3D Solid (Polygon) Object
*/

#ifndef ShipSolid_h
#define ShipSolid_h

#include "Solid.h"

// +--------------------------------------------------------------------+

class Ship;
class Skin;

// +--------------------------------------------------------------------+

class ShipSolid : public Solid
{
public:
    static const char* TYPENAME() { return "ShipSolid"; }

    ShipSolid(Ship* s);
    virtual ~ShipSolid();

    virtual void   Render(Video* video, DWORD flags);
    virtual void   TranslateBy(const Point& ref);

    const Skin*    GetSkin()               const { return skin; }
    void           SetSkin(const Skin* s)        { skin = s;    }

protected:
    Ship*          ship;
    const Skin*    skin;
    Point          true_eye_point;
    Point          fog_loc;
    bool           in_soup;
};

// +--------------------------------------------------------------------+

#endif ShipSolid_h

