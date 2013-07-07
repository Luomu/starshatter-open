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
    FILE:         Skin.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for managing run-time selectable skins on solid objects
*/

#ifndef Skin_h
#define Skin_h

#include "Polygon.h"
#include "Graphic.h"
#include "Video.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Solid;
class Model;
class Surface;
class Segment;

class Skin;
class SkinCell;

// +--------------------------------------------------------------------+

class Skin
{
public:
    static const char* TYPENAME() { return "Skin"; }
    enum            { NAMELEN=64 };

    Skin(const char* name = 0);
    virtual ~Skin();

    // operations
    void              ApplyTo(Model* model)   const;
    void              Restore(Model* model)   const;

    // accessors / mutators
    const char*       Name()                  const { return name;          }
    const char*       Path()                  const { return path;          }
    int               NumCells()              const { return cells.size();  }

    void              SetName(const char* n);
    void              SetPath(const char* n);
    void              AddMaterial(const Material* mtl);

protected:
    char              name[NAMELEN];
    char              path[256];
    List<SkinCell>    cells;
};

// +--------------------------------------------------------------------+

class SkinCell
{
    friend class Skin;

public:
    static const char* TYPENAME() { return "SkinCell"; }

    SkinCell(const Material* mtl=0);
    ~SkinCell();

    int operator == (const SkinCell& other) const;

    const char*       Name()         const;
    const Material*   Skin()         const { return skin; }
    const Material*   Orig()         const { return orig; }

    void              SetSkin(const Material* mtl);
    void              SetOrig(const Material* mtl);

private:
    const Material*   skin;
    const Material*   orig;
};

// +--------------------------------------------------------------------+

#endif Skin_h

