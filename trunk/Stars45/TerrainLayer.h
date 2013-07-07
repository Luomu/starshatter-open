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
    FILE:         TerrainLayer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    A blended detail texture applied to a terrain patch
    through a specific range of altitudes
*/

#ifndef TerrainLayer_h
#define TerrainLayer_h

#include "Types.h"
#include "Bitmap.h"

// +--------------------------------------------------------------------+

class Terrain;
class TerrainRegion;

// +--------------------------------------------------------------------+

class TerrainLayer
{
    friend class Terrain;
    friend class TerrainRegion;

public:
    static const char* TYPENAME() { return "TerrainLayer"; }

    TerrainLayer() : tile_texture(0), detail_texture(0), min_height(0), max_height(-1) { }
    ~TerrainLayer() { }

    int operator <  (const TerrainLayer& t) const { return min_height <  t.min_height; }
    int operator <= (const TerrainLayer& t) const { return min_height <= t.min_height; }
    int operator == (const TerrainLayer& t) const { return min_height == t.min_height; }

    // accessors:
    const char* GetTileName()        const { return tile_name;        }
    const char* GetDetailName()      const { return detail_name;      }
    Bitmap*     GetTileTexture()     const { return tile_texture;     }
    Bitmap*     GetDetailTexture()   const { return detail_texture;   }
    double      GetMinHeight()       const { return min_height;       }
    double      GetMaxHeight()       const { return max_height;       }

private:
    Text        tile_name;
    Text        detail_name;
    Bitmap*     tile_texture;
    Bitmap*     detail_texture;
    double      min_height;
    double      max_height;
};


#endif TerrainLayer_h

